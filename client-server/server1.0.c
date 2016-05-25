
/*
## Backup Server

### 0.1

 * Get file fingerprint from client
 * see if the file is duplicated
 * reply with yes/no

### 0.2

 * recv file by fixed chunk, redis store index
 *

### 0.3

 * use hashtable not redis
 * IPC use mmap
 * a worker process to maintain the hashtable (command by fifo pipe)

### 0.4

 * reconstruct use UDP
 * recv and write chunk in a directory
 * bug : open error for /tmp/fifo.20093: Too many open files
 *

### 0.5

 * do not need hash table worker, bcs this is a single process
 * All the fp are in the memory(hash table)
 *


### 0.6

 * Add bloom filter all fps (NOT USED YET)
 * All unique fps append to a file  (Index File on Disk)
 * Delta updated fps will sync to POX for updating BloomFilter and Cache
 * bug: some chunk begin with '#', so error interpreted as a new file fp
 *

### 0.7

 * UDP port for fp and control flow, and TCP port file data flow
 * Single process model
 * Need readn not read to get the size you want
 * Try to handle UDP packet loss, but too hard !
 * 

### 0.8

 * Oops return to USE TCP to send fp and file, bcs UDP unreliable
 * Implement a reliable UDP is not easy(tried!)
 * Not update fp cache and fp file when check dedu, BUT when we recv the file 

### 0.9

 * Use a config file to index the files 
 * Not use hash table to cache fingerprint, file exists by syscall ! 
 * Only need to record the latest fp into a file, afer backup, sync to the sdn controller, not need file containing all fps
 * store folder: all files named by fp
 * index folder: store the config file of every backup

### 1.0

use option to choose different deduplication method, same as client1.0
*/
#include	"global.h"

#define FP_LEN 40	// 40B hex for 20B sha1
#define STORE_PREFIX "store/"
#define DATA_PATH "data"
#define INDEX_PATH "index"
#define BLOOM_FILTER_SIZE 50000   //  FIXME: this should be same with what in POX, do not forget!!!

void recv_chunk(int sockfd);
void make_storage_dir(char *name);
void sync_bloom_to_pox();
void sync_bloom_to_pox_1();
void update_fp(char *fp);
int isExistsFile(char *fname);

static int bloom_on = 0;
static int sdna_on = 0;

static bloom_t *filter = NULL;
//FILE *fpfile_all = NULL;   // log all the fp
FILE *fpfile_latest = NULL;   // log the lastest fp this backup window

int sockfdbloom;
struct sockaddr_in fakeaddr;	// to a FAKE IP

typedef struct {
	unit_type type;
	int len;
	char value[0];
}TransferUnit;


int main(int argc, char **argv){
	int					sockfd_fp, sockfd_data, connfd, udpfd, nready, maxfdp1, listenfd2;
	char				mesg[MAXLINE];
	pid_t				childpid, workerpid;
	struct sockaddr_in	servaddr, cliaddr;
	fd_set				rset;
	const int			on = 1;
	int client_fds[FD_SETSIZE];
	void				sig_chld(int);
	char *dedu_method = NULL;

	//handle the options 
	int c;
	int opterr = 0;
	while ((c = getopt (argc, argv, "m:")) != -1){
		switch (c){
			case 'm':
        		dedu_method = optarg;
        		break;
      		case '?':
          		fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
        		return 1;
      		default:
        		abort ();
      	}	
	}

	if(strcmp(dedu_method, "sdna") == 0){
		sdna_on = 1;
		bloom_on = 1;
	}else if(strcmp(dedu_method, "bloom") ==0){
		bloom_on = 1;
	}
		

	/*
	 * Create a bloom filter, the size NEED TO tradeoff
	 */
	if(bloom_on)
		filter = bloom_filter_new(BLOOM_FILTER_SIZE);
	/*
	if (!(fpfile_all = fopen("/tmp/fp.all.out", "w"))) {
        fprintf(stderr, "E: Couldn't open file for write all fp\n");
        fflush (stderr);
        return;
    }
	*/

	listenfd2 = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Setsockopt(listenfd2, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(listenfd2, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd2, LISTENQ);

	// Channel to POX controller
	bzero(&fakeaddr, sizeof(fakeaddr));
	fakeaddr.sin_family = AF_INET;
	fakeaddr.sin_port = htons(BLOOM_PORT);
	Inet_pton(AF_INET, "10.0.0.1", &fakeaddr.sin_addr);

	sockfdbloom = Socket(AF_INET, SOCK_DGRAM, 0);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	FD_ZERO(&rset);
	maxfdp1 = listenfd2;
	connfd = -1;
	for ( ; ; ) {
		FD_SET(listenfd2, &rset);
		if ( (nready = select(maxfdp1 + 1, &rset, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("select error");
		}

		if (FD_ISSET(listenfd2, &rset)) {
			int len = sizeof(cliaddr);
			connfd = Accept(listenfd2, (SA *) &cliaddr, &len);
			int a = 65535;
			Setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, &a, sizeof(int));
			/* here use single process model 
			 * NOT USE MULTI PROCESS BY FORK, otherwise need IPC to maintain hash table
			 */
			recv_chunk(connfd);	// LOOP 
		}
		
	}
	// cannot get here 
}


// core routine: recv the chunk and act by type
void recv_chunk(int sockfd){
	ssize_t		n, rlen;
	char		buf[MAXLINE];
	char *res = "data recv done";
	FILE *fs = NULL; // for store
	FILE *configfile = NULL;
	char fp[FP_LEN+1];
	char filename[MAXLINE] = {0};
	fp[FP_LEN] = '\0';
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);

	memset(buf, 0, sizeof(buf));
	TransferUnit *header = (TransferUnit *)malloc(sizeof(TransferUnit));
	int headerlen = sizeof(TransferUnit);
	
	while(1){
		memset(header, 0, headerlen);
		rlen = Readn(sockfd, header, headerlen);
		// printf("header->type = %d, header->len = %d\n", header->type ,header->len);

		if(rlen == 0 ){// No data now  continue
			// printf("%d===========\n", headerlen);
			printf("Client Closed!\n");
			break;
		}

		if(rlen != headerlen){
			printf("Read Chunk Header Wrong!\n");
			exit(-1);
		}
		//FIXME: use switch
		if(header->type ==  FILE_FIRST_PACKET){ // file first pakcet (is fp), Definitely new File

			// struct timeval tv1, tv2;
			// gettimeofday(&tv1, NULL);

			rlen = Readn(sockfd, buf, header->len);
			char *fp = buf;
			// printf("File FP: %s\n", buf);

			//--------------	update index -----------//
			// NO cache!!!
			update_fp(fp); // persist to disk
			// printf("bloom_filter_add %s\n", fp);
			if(bloom_on)
				bloom_filter_add(filter, fp); // 

			//--------------	create file for store -----------//
			char path[MAXLINE];
			path[0] = '\0';
			strcat(path, "store/");
			strcat(path, fp);
			// printf("%s\n", path);
			make_storage_dir("store");
			fs = fopen(path, "wb"); // open for write store this file 
			if(!fs){
				printf("fopen error, %s\n", strerror(errno));
				exit(-1);
			}

			// gettimeofday(&tv2, NULL);
			// double time_ms = (tv2.tv_sec-tv1.tv_sec)*1000.0+(tv2.tv_usec-tv1.tv_usec)/1000.0;
			// printf("time of first packet handle: %f", time_ms);

		}else if(header->type == FILE_DATA_PACKET){  // file data chunk
			rlen = Readn(sockfd, buf, header->len);
			fwrite(buf, sizeof(char), rlen, fs);

		}else if(header->type == FILE_EOF_PACKET){ // file EOF
			rlen = Readn(sockfd, buf, header->len);
			fflush(fs);
			fclose(fs);

		}else if(header->type == FILE_FP_QUERY){ // fp query
			// struct timeval tv1, tv2;
			// gettimeofday(&tv1, NULL);

			rlen = Readn(sockfd, buf, header->len);
			// If this file exists by fingerprint
			// Now use a hash table TODO
			int exists = isExistsFile(buf);

			// gettimeofday(&tv2, NULL);
			// double time_ms = (tv2.tv_sec-tv1.tv_sec)*1000.0+(tv2.tv_usec-tv1.tv_usec)/1000.0;
			// printf("time of fp query handle: %f", time_ms);

			char *msg = "new";
			if(exists)
				msg = "duplicated";
			Write(sockfd, msg, strlen(msg)); // Reply to Client
		}else if(header->type == START_BACKUP){ // start backup
			char store_dir[MAXLINE];
			rlen = Readn(sockfd, buf, header->len);
			if (!(fpfile_latest = fopen("/tmp/fp.latest.out", "w"))) {
        		fprintf(stderr, "E: Couldn't open file for write latest fp\n");
        		fflush (stderr);
        		exit(-1);
    		}
    		/*
    		// FIXME: mkdir -p XX, 
    		strcpy(store_dir, "mkdir -p ");
    		strcat(store_dir, STORE_PREFIX);
    		strcat(store_dir, buf);
    		printf("Backup(%s) starting...\n", store_dir);
    		system(store_dir);
    		*/
		}else if(header->type == END_BACKUP){ //end backup
			rlen = Readn(sockfd, buf, header->len);
			printf("Backup Complete!\n");
			if(sdna_on){
				sync_bloom_to_pox();
			}
		}else if(header->type == BACKUP_CONFIG_FILE_START){
			rlen = Readn(sockfd, buf, header->len);
			char *dir = buf;

			char path[MAXLINE];
			path[0] = '\0';
			strcat(path, "store/");
			strcat(path, dir); //FIXME: dir not contain '/'
			strcat(path, "_config_file");

			configfile = fopen(path, "wb"); // open for write store this file 
			if(!configfile){
				printf("fopen error, %s\n", strerror(errno));
				exit(-1);
			}
		}else if(header->type == BACKUP_CONFIG_FILE){
			rlen = Readn(sockfd, buf, header->len);
			fwrite(buf, sizeof(char), rlen, configfile);

		}else if(header->type == BACKUP_CONFIG_FILE_END){
			rlen = Readn(sockfd, buf, header->len);
			fflush(configfile);
			fclose(configfile);
		}
		memset(buf, 0, sizeof(buf));
	}

	close(sockfd);
}

void make_storage_dir(char *name){
	struct stat st = {0};
	if((stat(name, &st)) == -1){
		mkdir(name, 0700);
	}
}

void update_fp(char *fp){
	//fprintf(fpfile_all, "%s\n", fp);
	fprintf(fpfile_latest, "%s\n", fp);
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
	// latest fp file close
	fflush(fpfile_latest);
	fclose(fpfile_latest);

	bloom_filter_dump(filter);

	Sendto(sockfdbloom, filter->bitset, filter->size_bytes, 0,  (SA *) &fakeaddr, sizeof(fakeaddr));
}

/* 
 * if the file named by fp exists
 * TODO : performance ???
 */
int isExistsFile(char *fname){
	char path[MAXLINE];
	path[0] = '\0';

	// if not exist in bloom filter, then file is new
	if(bloom_on){
		if(bloom_filter_contains(filter, fname) == 0)
			return 0;
	}

	strcat(path, STORE_PREFIX);
    strcat(path, fname);
	if((access(path, F_OK)) != -1)
		return 1;
	else return 0;
}


