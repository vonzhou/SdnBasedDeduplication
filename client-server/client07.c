#include	"global.h"

/* maybe return the list of fp indicating the chunk needed to transfer to server TODO
 * here just a whole file ,so i return 0/1
 * bug 1: Use read() to get reply from serer NOT readline() ...
 * bug 2: buffer should be memset to 0
 *
 * client 02
 * add file fp
 * transfer file by fixed chunk
 * bug 1: in function var should be set 0
 *
 * client 03
 * recv quik reply from SDN controller (here i use UDP, low cost)
 * bug 1: recvfrom error: Bad address, recvfrom last arg should be a socklent pointer
 *
 * client 04
 * reconstruct use UDP, count time
 *
 * client 05
 * Transfer files in a folder
 *
 * Client 06
 * Triger the server to sync bloom filter to SDN controller
 * Inform the start and end of a backup
 * If file is new telled by POX, then server just store (not dedu)
 *
 * client 07 
 * UDP port for fp and control flow, and TCP port file data flow
 * UDP packets loss problem when large flow in the SDN network (set timeout to recvfrom)
 * Try to handle UDP packet loss, but too hard !
 */

#define CHUNK_SIZE 1400
#define QUICK_PORT 9999

static int sdn_switch = 0;

static int sockfd4pox = -1; // recv SDN controller action
static int sockfd2serverfp = -1;  // fp channel
static int sockfd2server = -1;	// transfer file channel
static struct sockaddr_in servaddrfp;
static struct sockaddr_in servaddr;
char *serverip = "127.0.0.1";

typedef struct {
	int type;
	int len;
	char value[CHUNK_SIZE];
}TransferUnit;

int get_tcp_socket(char *ip, int port);
int get_udp_socket(int port);

int isExistsFile(char *fp_filename, char *filename);
void transfer_file(char *fp_filename, char *filename, int);
void uploadDir(const char *name);
void uploadDir_recur(const char *name);
int upload(char *filename);
void get_all_file_paths(const char *name, FILE *outfile);
void start_backup();
void end_backup();

int file_count = 0; // udp gram rate control


int main(int argc, char **argv){
	int res = 0;
	struct timeval tv1, tv2;
	char *directory = NULL;

	if (argc != 3)
		err_quit("usage: client <IPaddress> <file dir>");
	serverip = argv[1];
	directory = argv[2];

	gettimeofday(&tv1, NULL);

	sockfd4pox = get_udp_socket(QUICK_PORT);// channel with POX

	bzero(&servaddrfp, sizeof(servaddrfp));
	servaddrfp.sin_family = AF_INET;
	servaddrfp.sin_port = htons(SERV_PORT+1);
	Inet_pton(AF_INET, argv[1], &servaddrfp.sin_addr);
	sockfd2serverfp = Socket(AF_INET, SOCK_DGRAM, 0);
	// Connect(sockfd2serverfp, (SA *)&servaddrfp, sizeof(servaddrfp));

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;// 
	setsockopt(sockfd2serverfp, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	int a = 65535;
	Setsockopt(sockfd2serverfp, SOL_SOCKET, SO_SNDBUF, &a, sizeof(int));
	Setsockopt(sockfd2serverfp, SOL_SOCKET, SO_RCVBUF, &a, sizeof(int));


#ifdef USE_UDP
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
	sockfd2server = Socket(AF_INET, SOCK_DGRAM, 0);
#else
	sockfd2server = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT); //
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd2server, (SA *) &servaddr, sizeof(servaddr));
	Setsockopt(sockfd2server, SOL_SOCKET, SO_SNDBUF, &a, sizeof(int));
#endif
	// start 
	start_backup();

	uploadDir(directory);
	// uploadDir_recur(directory);

	gettimeofday(&tv2, NULL);
	printf("Time cost = %lfms\n",(tv2.tv_sec-tv1.tv_sec)*1000.0+(tv2.tv_usec-tv1.tv_usec)/1000.0);

    /* Upload complete, tell server to sync data to SDN controller
     * Need a alg(by dedu ratio) to control this sync
     */
    end_backup();


	close(sockfd4pox);
	
	// shutdown(sockfd2server, SHUT_WR);
	// shutdown(sockfd2serverfp, SHUT_WR);
	close(sockfd2serverfp);
	close(sockfd2server);
	exit(0);
}

int get_tcp_socket(char *ip, int port){
	int					sockfd, sockfd2;
	struct sockaddr_in	servaddr;
	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT+1); //
	Inet_pton(AF_INET, ip, &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	return sockfd;
}

/*
 *	return:
 *		1 -	file exists , complete
 *		-1 - 	file new , POX
 *		-2 -	file new , server
 */
int isExistsFile_1(char *fp_filename, char *filename) {
	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);
	printf("is exists file ?\n");

	// WE NEED THE REPLY 

		Sendto(sockfd2serverfp, fp_filename, strlen(fp_filename), 0, (SA *) &servaddrfp, sizeof(servaddrfp));
		// Write(sockfd2serverfp, fp_filename, strlen(fp_filename));	
		printf("sendto server fp ----\n");
		/* recv quick reply from POX with timeout */
		// struct timeval tv;
		// tv.tv_sec = 2;
		// tv.tv_usec = 0;
		// setsockopt(sockfd4pox, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		memset(recvline, 0, sizeof(recvline));
		if(sdn_switch){
			Recvfrom(sockfd4pox, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen);
			// printf("=============%s\n", recvline);
		}
		if(strcmp(recvline, "new_file") == 0){ // Bloom filter ,Quick decision
			printf("**File(%s) new, so start transfer(POX).**\n", filename);
			// File is new, transfer file to server
			// But , also need update the bloom filter in server
			return -1;
		}else if(strcmp(recvline, "duplicated") == 0){// cache hit, Quick decision
			printf("**File(%s) duplicated, so transfer complete(POX)..**\n", filename);
			return 1;
		}else{// go to server , wait for the reply from server
			
			int n = 0;
			memset(recvline, 0, sizeof(recvline));
			n = recvfrom(sockfd2serverfp, recvline, MAXLINE, 0,  NULL, NULL);
			
			
			// // int n = Read(sockfd2serverfp, recvline, MAXLINE);
			if(n < 0){
				if(errno == EWOULDBLOCK || errno == EAGAIN){
					fprintf(stderr, "socket recvfrom timeout\n");
					printf("Not get answer from server , fake it new \n");
					return -2;
					//continue;// re send 
				}else err_sys("recvfrom error!");
			}
			// if (Recvfrom(sockfd2serverfp, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen)==0)
			// 	err_quit("client: server terminated prematurely");
			printf("get answer from server\n");
			// printf("=============%s\n", recvline);
			if(strcmp(recvline, "duplicated") == 0){
				printf("**File duplicated(%s), so transfer complete..**\n", filename);
				return 1;
			}else{
				printf("**File new(%s), so start transfer..**\n", filename);
				return -2;
			}
		} // end go to server

	return -1; // cannot get here
}

int isExistsFile(char *fp_filename, char *filename) {
	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);

	// WE NEED THE REPLY 
	while(1){

		Sendto(sockfd2serverfp, fp_filename, strlen(fp_filename), 0, (SA *) &servaddrfp, sizeof(servaddrfp));
		// Write(sockfd2serverfp, fp_filename, strlen(fp_filename));	
		printf("sendto server fp ----\n");
		/* recv quick reply from POX with timeout */
		// struct timeval tv;
		// tv.tv_sec = 2;
		// tv.tv_usec = 0;
		// setsockopt(sockfd4pox, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		memset(recvline, 0, sizeof(recvline));
		if(sdn_switch){
			Recvfrom(sockfd4pox, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen);
			// printf("=============%s\n", recvline);
		}
		if(strcmp(recvline, "new_file") == 0){ // Bloom filter ,Quick decision
			printf("**File(%s) new, so start transfer(POX).**\n", filename);
			// File is new, transfer file to server
			// But , also need update the bloom filter in server
			return -1;
		}else if(strcmp(recvline, "duplicated") == 0){// cache hit, Quick decision
			printf("**File(%s) duplicated, so transfer complete(POX)..**\n", filename);
			return 1;
		}else{// go to server , wait for the reply from server
			
			memset(recvline, 0, sizeof(recvline));
			int n = 0;
			long timer = 0;
			n = recvfrom(sockfd2serverfp, recvline, MAXLINE, 0,  NULL, NULL);
			// do{
			// 	n = recvfrom(sockfd2serverfp, recvline, MAXLINE, 0,  NULL, NULL);
			// 	timer ++;
			// }while(timer < 100 && n < 0);
			
			// int n = Read(sockfd2serverfp, recvline, MAXLINE);
			if(n < 0){
				if(errno == EWOULDBLOCK || errno == EAGAIN){
					fprintf(stderr, "socket recvfrom timeout\n");
					sleep(2);
					continue;// re send 
				}else err_sys("recvfrom error!");
			}
			// if (Recvfrom(sockfd2serverfp, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen)==0)
			// 	err_quit("client: server terminated prematurely");
			printf("get answer from server\n");
			// printf("=============%s\n", recvline);
			if(strcmp(recvline, "duplicated") == 0){
				printf("**File duplicated(%s), so transfer complete..**\n", filename);
				return 1;
			}else{
				printf("**File new(%s), so start transfer..**\n", filename);
				return -2;
			}
		} // end go to server
	}// end while 1

	return -1; // cannot get here
}

/*
 * Transfer file data to server
 * If this decision-making is from POX, then need update bloom so set updatebloom = 1
 */
void transfer_file(char *fp_filename, char *filename, int updatebloom){
	char	recvline[MAXLINE];
	char buf[CHUNK_SIZE];
	char *sendline = "file chunk data";
	int fd, res, len;
	char firstpkt[MAXLINE]={0};
	TransferUnit unit; // buffer 
	memset(&unit, 0, sizeof(unit));


	int chunk_count = 4, i = 0;
	FILE *fs = fopen(filename, "r");

	// 1. transfer fp+file name, update bloom flag
	
	if(updatebloom){
		strcat(firstpkt, "#");
	}
	strcat(firstpkt, fp_filename);
	len  = strlen(firstpkt);
	unit.type = 1;
	unit.len = len;
	strcpy(unit.value, firstpkt);
	// Sendto(sockfd2server, firstpkt,strlen(firstpkt), 0,  (SA *) &servaddr, sizeof(servaddr));
	Write(sockfd2server, &unit, len + 8);
	// if (Read(sockfd2server, recvline, MAXLINE) == 0)
	// 	err_quit("transfer_file: server terminated prematurely");
	// printf("----client first unit ACK\n");
	// Write(sockfd, fp_filename, strlen(fp_filename));
	// fp+file name should be a separate packet
	// 
	// 2. then transfer the whole file chunkly
	memset(&unit, 0, sizeof(unit));
	while((res = fread(unit.value, sizeof(char), CHUNK_SIZE, fs)) > 0){
		// Sendto(sockfd2server, buf, res, 0,  (SA *) &servaddr, sizeof(servaddr));
		// printf("----client chunk unit sent\n");
		// Read(sockfd2server, recvline, MAXLINE);
		unit.type = 2;
		unit.len = res;
		Write(sockfd2server, &unit, res+8);
		// we DO NOT need reply
		memset(&unit, 0, sizeof(unit));
	}

	// 3. eof it
	sendline = "##file_eof##";
	unit.type = 3;
	unit.len = strlen(sendline);
	Write(sockfd2server, &unit, unit.len + 8);
	// Sendto(sockfd2server, sendline, strlen(sendline), 0, (SA *) &servaddr, sizeof(servaddr));

	// // 4. ACK for this file transmission
	// if (Read(sockfd2server, recvline, MAXLINE) == 0)
	// 	err_quit("transfer_file: server terminated prematurely");


	fclose(fs);
}

void transfer_file_udp(char *fp_filename, char *filename, int updatebloom){
	char	recvline[MAXLINE];
	char buf[CHUNK_SIZE];
	char *sendline = "file chunk data";
	int fd, res;
	char firstpkt[MAXLINE]={0};


	int chunk_count = 4, i = 0;
	FILE *fs = fopen(filename, "r");

	// 1. transfer fp+file name, update bloom flag
	
	if(updatebloom){
		strcat(firstpkt, "#");
	}
	strcat(firstpkt, fp_filename);
	Sendto(sockfd2server, firstpkt,strlen(firstpkt), 0,  (SA *) &servaddr, sizeof(servaddr));
	printf("----client first unit sent\n");
	if (Read(sockfd2server, recvline, MAXLINE) == 0)
		err_quit("transfer_file: server terminated prematurely");
	printf("----client first unit ACK\n");
	// Write(sockfd, fp_filename, strlen(fp_filename));
	// fp+file name should be a separate packet
	// 
	// 2. then transfer the whole file chunkly
	while((res = fread(buf, sizeof(char), CHUNK_SIZE, fs)) > 0){
		Sendto(sockfd2server, buf, res, 0,  (SA *) &servaddr, sizeof(servaddr));
		printf("----client chunk unit sent\n");
		Read(sockfd2server, recvline, MAXLINE);

		// we DO NOT need reply
		memset(buf, 0, CHUNK_SIZE);
	}

	// 3. eof it
	sendline = "##file_eof##";
	Sendto(sockfd2server, sendline, strlen(sendline), 0, (SA *) &servaddr, sizeof(servaddr));

	// 4. ACK for this file transmission
	if (Read(sockfd2server, recvline, MAXLINE) == 0)
		err_quit("transfer_file: server terminated prematurely");


	fclose(fs);
}

int get_udp_socket(int port){
	int					sockfd;
	struct sockaddr_in	servaddr, cliaddr;

	sockfd = Socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(port);

	Bind(sockfd, (SA *) &servaddr, sizeof(servaddr));
	return sockfd;
}

int upload(char *filename){
	// For simple, I use 40B hex string instead
	unsigned char fp[20];
	char hash[41];
	int res = 0;
	char fp_filename[MAXLINE];

	// 1. inquiry the server if the file is duplicated by fingerprint (a RTT)
	printf("upload:filename=%s\n", filename);
	res = SHA1File(filename, fp);
	if(res){
		printf("Get file sha1 hash failed.\n");
		return -1;
	}
	digestToHash(fp, hash); // rabin.h
	fp_filename[0] = '\0';
	strcat(fp_filename, hash);
	strcat(fp_filename, filename);

	res = isExistsFile(fp_filename, filename);

	switch(res){
		case 1:
			return 0;
			break;
		case -1:
			transfer_file(fp_filename, filename, 1);
			break;
		case -2:
			transfer_file(fp_filename, filename, 0);
			break;
		default:
			printf("ERROR!!!\n");
	}
	// sleep(1);
	// file_count ++;
	// if(file_count % 10 == 0)
		// sleep(10);
	return 0;
}

// upload a folder
void uploadDir_recur(const char *name){
	DIR *dir;
  	struct dirent *entry;
	long count = 0; // file count
  	if (!(dir = opendir(name)))
    	return;
  	if (!(entry = readdir(dir)))
      	return;

  	do {
      	if (entry->d_type == DT_DIR) {
			char path[MAXLINE];
        	int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;
        	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            	continue;
				// recursive
      		uploadDir_recur(path);
      	}
      	else{
			char path[MAXLINE];
			int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			// printf("file path: **%s**\n", path);
			upload(path);
		}

	} while (entry = readdir(dir));

    closedir(dir);
}
// upload a folder
void uploadDir(const char *name){
	FILE *outfile = fopen("files.all", "w");
	if(outfile == NULL){
		printf("error opening file\n");
		exit(-1);
	}
	get_all_file_paths(name, outfile);
	fflush(outfile);
	if(fclose(outfile)){
		printf("error closing file\n");
		exit(-1);
	}

	// Get all the files path and then send to server
	FILE *file = fopen("files.all", "r");
	if(outfile == NULL){
		printf("error opening file\n");
		exit(-1);
	}
	char buf[MAXLINE] = {0};
	char *line = NULL;
	int len = 0;
	int nread = -1;
	while((nread = getline(&line, &len, file)) > 0 ) {
		// printf("read:%s\n", line);
		line[nread-1] = '\0';  // BUG: strip last \n
		upload(line);
	}
	
	if(fclose(file)){
		printf("error closing file\n");
		exit(-1);
	}
}

void get_all_file_paths(const char *name, FILE *outfile){
	DIR *dir;
  	struct dirent *entry;
	long count = 0; // file count

  	if (!(dir = opendir(name)))
    	return;
  	if (!(entry = readdir(dir)))
      	return;

  	do {
      	if (entry->d_type == DT_DIR) {
			char path[MAXLINE];
        	int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;
        	if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            	continue;
				// recursive
      		get_all_file_paths(path, outfile);
      	}
      	else{
			char path[MAXLINE];
			int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;
			fprintf(outfile, "%s\n", path);
			printf("------------%s\n", path);
			// upload(path);
		}

	} while (entry = readdir(dir));

    closedir(dir);
}

void start_backup(){
	char recvline[MAXLINE];
	char *msg = "##start_backup##";
	Sendto(sockfd2serverfp, msg, strlen(msg), 0, (SA *) &servaddrfp, sizeof(servaddrfp));
	if (Read(sockfd2serverfp, recvline, MAXLINE) == 0)
		err_quit("transfer_file: server terminated prematurely");
}

void end_backup(){
	char *msg = "##sync_bloom##";
	Sendto(sockfd2serverfp, msg, strlen(msg), 0, (SA *) &servaddrfp, sizeof(servaddrfp));
	printf("---end_backup---\n");
}
