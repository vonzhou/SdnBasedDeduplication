/*
 * Get file fingerprint from client
 * see if the file is duplicated
 * reply with yes/no
 *
 * server 02
 * recv file by fixed chunk, redis store index
 *
 * server 03
 * use hashtable not redis
 * IPC use mmap
 * a worker process to maintain the hashtable (command by fifo pipe)
 *
 * server 04
 * reconstruct use UDP
 * recv and write chunk in a directory
 * bug : open error for /tmp/fifo.20093: Too many open files
 *
 * server 05
 * do not need hash table worker, bcs this is a single process
 * All the fp are in the memory(hash table)
 *
 * server 06
 * Add bloom filter all fps (NOT USED YET)
 * All unique fps append to a file  (Index File on Disk)
 * Delta updated fps will sync to POX for updating BloomFilter and Cache
 * bug: some chunk begin with '#', so error interpreted as a new file fp
 * 
 */
#include	"global.h"

#define FP_LEN 40	// 40B hex for 20B sha1

// get fp+filename to dedu
void recv_fp(int sockfd);
// get file data
void recv_chunk(int sockfd);
void make_dir(char *name);
void sync_bloom_to_pox();
void update_fp(char *fp);

long count = 0;   // file count;
static bloom_t *filter = NULL;
FILE *fpfile_all = NULL;   // log all the fp
FILE *fpfile_latest = NULL;   // log the lastest fp this backup window

int sockfdbloom;
struct sockaddr_in fakeaddr;	// to a FAKE IP


int main(int argc, char **argv){
	int					sockfd_fp, sockfd_data, connfd, udpfd, nready, maxfdp1;
	char				mesg[MAXLINE];
	pid_t				childpid, workerpid;
	fd_set				rset;
	const int			on = 1;
	void				sig_chld(int);

	/* hash table init
	 * Contain all the fps, maybe should be a cache and read from disk
	 */
	hash_init(1024);

	/*
	 * Create a bloom filter, the size NEED TO tradeoff
	 */
	filter = bloom_filter_new(5000);

	if (!(fpfile_all = fopen("/tmp/fp.all.out", "w"))) {
        fprintf(stderr, "E: Couldn't open file for write all fp\n");
        fflush (stderr);
        return;
    }

    



	/* Channel : fingerprint packet recv */
	sockfd_fp = get_udp_socket(SERV_PORT+1);

	/* Channel : other packet */
	sockfd_data = get_udp_socket(SERV_PORT);

	// Channel to POX controller
	bzero(&fakeaddr, sizeof(fakeaddr));
	fakeaddr.sin_family = AF_INET;
	fakeaddr.sin_port = htons(BLOOM_PORT);
	Inet_pton(AF_INET, "10.0.0.1", &fakeaddr.sin_addr);

	sockfdbloom = Socket(AF_INET, SOCK_DGRAM, 0);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	FD_ZERO(&rset);
	maxfdp1 = max(sockfd_fp, sockfd_data) + 1;
	for ( ; ; ) {
		FD_SET(sockfd_fp, &rset);
		FD_SET(sockfd_data, &rset);
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("select error");
		}

		if (FD_ISSET(sockfd_fp, &rset)) {
			recv_fp(sockfd_fp);
		}

		if (FD_ISSET(sockfd_data, &rset)) {
			recv_chunk(sockfd_data);
		}
	}
}


void recv_fp(int sockfd) {
	ssize_t		n;
	char		buf[MAXLINE];
	char *res = "new";
	char fp[FP_LEN+1];
	char filename[MAXLINE];
	fp[FP_LEN] = '\0';

	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);


	memset(recvline, 0, sizeof(recvline));
	n = Recvfrom(sockfd, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen);

	if(strcmp(recvline, "##sync_bloom##") == 0){
		// printf("bloom filter count : %u\n", bloom_filter_count(filter));
    	// printf("bloom filter size  : %u\n", bloom_filter_size(filter));
    	sync_bloom_to_pox();
		printf("---sync bloom complete------------------\n");
		return;
	}else if(strcmp(recvline, "##start_backup##") == 0){
		if (!(fpfile_latest = fopen("/tmp/fp.latest.out", "w"))) {
        	fprintf(stderr, "E: Couldn't open file for write latest fp\n");
        	fflush (stderr);
        	return;
    	}
    	res = "ack start_backup";
    	Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);
		printf("---this backup starting ------------------\n");
		return;
	}

	memcpy(fp, recvline, FP_LEN);
	memcpy(filename, &recvline[FP_LEN], n - FP_LEN);
	filename[n - FP_LEN] = '\0';
	// printf("**File FP: %s, File name: %s**\n", fp, filename);

	// look up this fp for dedu in the hash table
	int exists = isExistsFP(fp);
	// printf("recv_fp: isExistsFP=%d\n", exists);
	if(exists){
		res = "duplicated";
		Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);
		// addFilename2FP(fp, filename);
	}else{
		Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);
		// addFilename2FP(fp, filename);
		insertFP(fp); // just 1 to indicate, this is cache

		update_fp(fp);
	}
}

void closeNagle(int sockfd){
	int flag = 1;
	Setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
}

void recv_chunk(int sockfd){
	ssize_t		n;
	char		buf[MAXLINE];
	char *res = "data recv done";
	FILE *fs = NULL; // for store
	int chunk_count = 0;
	char fp[FP_LEN+1];
	char filename[MAXLINE] = {0};
	fp[FP_LEN] = '\0';
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);
	memset(buf, 0, sizeof(buf));
	// recv the data chunk and store
	while ( (n = Recvfrom(sockfd, buf, MAXLINE, 0, (SA *) &cliaddr, &cliaddrlen)) > 0){
		// printf("**recv chunk** %s\n", buf);
		if(strcmp(buf, "##file_eof##") == 0){			//3. EOF
			char *s = "file ok";
			Sendto(sockfd, s,strlen(s), 0,  (SA *) &cliaddr, cliaddrlen);
			// printf("File EOF\n");
			break;
		}else {
			//1. we know the first packet is fp+file name
			if(chunk_count == 0){
				// printf("**recv chunk** %s\n", buf);
				// get file fp and file name, update bloom flag
				int flag = 0;
				char *bufp = buf;
				if(buf[0] == '#'){
					flag = 1;
					bufp ++;
					n--;
				}
				memcpy(fp, bufp, FP_LEN);
				memcpy(filename, bufp + FP_LEN, n - FP_LEN);
				filename[n - FP_LEN] = '\0';
				printf("File FP: %s, File name: %s\n", fp, filename);
				if(flag){
					update_fp(fp);
				}



				// index has been updated in recv_fp, so here just create file for store
				char path[MAXLINE];
				path[0] = '\0';
				strcat(path, "store/");
				strcat(path, fp);
				printf("%s\n", path);
				make_dir("store");
				fs = fopen(path, "wb");
				if(!fs){
					printf("fopen error\n");
					exit(-1);
				}
				char *s = "ack";
				Sendto(sockfd, s,strlen(s), 0,  (SA *) &cliaddr, cliaddrlen);
				printf("----server  first unit ok\n");
			}else{
				// 2. store this file and index
				printf("**%d bytes recved and stored**\n", n);
				fwrite(buf, sizeof(char), n, fs);
				char *s = "ack";
				Sendto(sockfd, s,strlen(s), 0,  (SA *) &cliaddr, cliaddrlen);
				printf("----server  chunk unit ok\n");
				// if need reply to the client ?
			}
		}

		chunk_count ++;
		memset(buf, 0, sizeof(buf));
	}
	fclose(fs);
}

int get_udp_socket(int port){
	int					sockfd;
	const int on = 1;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);
	Setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	return sockfd;
}

void send_msg(int sockfd, char *msg, const SA *pservaddr, socklen_t servlen)
{
	int	n;
	char	recvline[MAXLINE + 1];
	char *sendline = "hello";
	Sendto(sockfd, msg, strlen(msg), 0, pservaddr, servlen);

	n = Recvfrom(sockfd, recvline, MAXLINE, 0, NULL, NULL);

	recvline[n] = 0;	/* null terminate */
	printf("%s\n", recvline);
}

void make_dir(char *name){
	struct stat st = {0};
	if((stat(name, &st)) == -1){
		mkdir(name, 0700);
	}
}

void update_fp(char *fp){
	fprintf(fpfile_all, "%s\n", fp);
	fprintf(fpfile_latest, "%s\n", fp);
	// fflush(fpfp);
}

/*
 * Send bloom filter vector to POX 
 * I tried , BUT cannot match 
 * so here I just fake it by write to a local file
 * trigger the POX to update the bloom filter by read this fp file
 */
void sync_bloom_to_pox(){
	char *s = "bloom filter";
	// latest fp file close
	fflush(fpfile_latest);
	fclose(fpfile_latest);

	Sendto(sockfdbloom, s,strlen(s), 0,  (SA *) &fakeaddr, sizeof(fakeaddr));
}

void sync_bloom_to_pox_1(){
	char *s = "bloom filter";
	int i=0;

	char str[2 * filter->size_bytes + 1];
	// error: variable-sized object may not be initialized
	memset(str, 0, sizeof str);
	// cannot send unsigned char arr directly, convert them to string 
	for(i=0; i < filter->size_bytes; i++){
		sprintf(str + (i*2), "%02x", filter->bitset[i]);
	}
	printf("------- %s ------------\n", str);
	Sendto(sockfdbloom, str, strlen(str), 0,  (SA *) &fakeaddr, sizeof(fakeaddr));
	// Sendto(sockfdbloom, s,strlen(s), 0,  (SA *) &fakeaddr, sizeof(fakeaddr));
}
