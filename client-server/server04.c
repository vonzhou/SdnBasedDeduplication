#include	"global.h"

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
 */

// 40B hex for 20B sha1
#define FP_LEN 40 

// get fp+filename to dedu
void recv_fp(int sockfd);
// get file data
void recv_chunk(int sockfd);
void make_dir(char *name);


int main(int argc, char **argv){
	int					sockfd_fp, sockfd_data, connfd, udpfd, nready, maxfdp1;
	char				mesg[MAXLINE];
	pid_t				childpid, workerpid;
	fd_set				rset;
	const int			on = 1;
	void				sig_chld(int);

	// hash worker init
	if ( (workerpid = Fork()) == 0) {	
		hash_worker_init(1024);
		exit(0);
	}

	/* 1. Channel : fingerprint packet recv */
	sockfd_fp = get_udp_socket(SERV_PORT+1);

	/* 2. Channel : other packet */
	sockfd_data = get_udp_socket(SERV_PORT);

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
	printf("%d\n", cliaddr.sin_addr.s_addr);
	// printf("----%s----\n", recvline);

	memcpy(fp, recvline, FP_LEN);
	memcpy(filename, &recvline[FP_LEN], n - FP_LEN);
	filename[n - FP_LEN] = '\0';
	printf("**File FP: %s, File name: %s**\n", fp, filename);

	// look up this fp for dedu
	int exists = isExistsFP(fp);
	printf("recv_fp: isExistsFP=%d\n", exists);
	if(exists){
		res = "duplicated";
		Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);
		// addFilename2FP(fp, filename);
	}else{
		Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);
		// addFilename2FP(fp, filename);
		insertFP(fp); // just 1 to indicate 
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
	char filename[MAXLINE];
	fp[FP_LEN] = '\0';
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);


	// recv the data chunk and store 
	while ( (n = Recvfrom(sockfd, buf, MAXLINE, 0, (SA *) &cliaddr, &cliaddrlen)) > 0){
		if(strcmp(buf, "exit") == 0){
			//3. EOF
			printf("File EOF\n");
			break;
		}else{
			//1. we know the first packet is fp+file name
			if(chunk_count == 0){
				// get file fp and file name
				memcpy(fp, buf, FP_LEN);
				memcpy(filename, &buf[FP_LEN], n - FP_LEN);
				filename[n - FP_LEN] = '\0';
				// printf("File FP: %s, File name: %s\n", fp, filename);
				res = "fp_filename";
				//Write(sockfd, res, strlen(res));
				// Sendto(sockfd, res, strlen(res), 0, (SA *) &cliaddr, cliaddrlen);

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
			}else{
				// 2. store this file and index 
				printf("**%d bytes recved and stored**\n", n);
				fwrite(buf, sizeof(char), n, fs);
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



