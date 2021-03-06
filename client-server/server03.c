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
 */

// 40B hex for 20B sha1
#define FP_LEN 40 
#define	FP_SHM	"fp_hashtable_shm" //  fp hashtable share mem

// get fp+filename to dedu
void recv_fp(int sockfd);
// get file data
void recv_chunk(int sockfd);

int main(int argc, char **argv){
	int					listenfd, listenfd2, connfd, udpfd, nready, maxfdp1;
	char				mesg[MAXLINE];
	pid_t				childpid, workerpid;
	fd_set				rset;
	ssize_t				n;
	socklen_t			len;
	const int			on = 1;
	struct sockaddr_in	cliaddr, servaddr;
	void				sig_chld(int);

	// hash worker init
	if ( (workerpid = Fork()) == 0) {	
		hash_worker_init(1024);
		exit(0);
	}

	/* 1. create listening TCP socket : fingerprint packet recv */
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT+1);

	Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	/* 2. create listening TCP socket : other packet */
	listenfd2 = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Setsockopt(listenfd2, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	Bind(listenfd2, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd2, LISTENQ);

	Signal(SIGCHLD, sig_chld);	/* must call waitpid() */

	FD_ZERO(&rset);
	maxfdp1 = max(listenfd, listenfd2) + 1;
	for ( ; ; ) {
		FD_SET(listenfd, &rset);
		FD_SET(listenfd2, &rset);
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0) {
			if (errno == EINTR)
				continue;		/* back to for() */
			else
				err_sys("select error");
		}

		if (FD_ISSET(listenfd, &rset)) {
			len = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &len);
	
			if ( (childpid = Fork()) == 0) {	/* child process TO recv fingerprint packet*/
				Close(listenfd);	/* close listening socket */
				recv_fp(connfd);	
				exit(0);
			}
			
			Close(connfd);			/* parent closes connected socket */
		}

		if (FD_ISSET(listenfd2, &rset)) {
			len = sizeof(cliaddr);
			connfd = Accept(listenfd2, (SA *) &cliaddr, &len);
	
			if ( (childpid = Fork()) == 0) {	/* child process TO recv file data */
				Close(listenfd2);	/* close listening socket */
				recv_chunk(connfd);	
				exit(0);
			}
			
			Close(connfd);			/* parent closes connected socket */
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

	n = read(sockfd, buf, MAXLINE);

	memcpy(fp, buf, FP_LEN);
	memcpy(filename, &buf[FP_LEN], n - FP_LEN);
	filename[n - FP_LEN] = '\0';
	// printf("**File FP: %s, File name: %s**\n", fp, filename);

	// look up this fp for dedu
	int exists = isExistsFP(fp);
	printf("recv_fp: isExistsFP=%d\n", exists);
	if(exists){
		res = "duplicated";
		Write(sockfd, res, strlen(res));
		// addFilename2FP(fp, filename);
	}else{
		Write(sockfd, res, strlen(res));
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


	// recv the data chunk and store 
	while ( (n = read(sockfd, buf, MAXLINE)) > 0){
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
				Write(sockfd, res, strlen(res));

				// index has been updated in recv_fp, so here just create file for store
				fs = fopen(fp, "wb");
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



