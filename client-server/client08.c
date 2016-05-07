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
 *
 * version 08
 * Oops return to USE TCP to send fp and file, bcs UDP unreliable
 * Implement a reliable UDP is not easy(tried!)
 * Communicate with POX use UDP[client -> POX -> server]
 * TODO transfer SHA-1 by raw 20 bytes, and then convert to 40 bytes representation
 */

#define CHUNK_SIZE 1400
#define QUICK_PORT 9999

static int sdn_switch = 1;

static int sockfd4pox = -1; // recv SDN controller action
static int sockfd2pox = -1;  // fp channel
static int sockfd2server = -1;	// transfer file channel
static struct sockaddr_in servaddr_pox;
static struct sockaddr_in servaddr;
char *serverip = "127.0.0.1";
static int file_dedu = 0;
static int file_dedu_pox = 0;
static int file_new_pox = 0;
static int pox_reply_count = 0;


typedef struct {
	unit_type type; 
	int len;
	char value[CHUNK_SIZE];
}TransferUnit;

int get_tcp_socket(char *ip, int port);
int get_udp_socket(int port);

int isExistsFile(char *fp_filename, char *filename);
void transfer_file(char *fp_filename, char *filename);
void uploadDir(const char *name);
void uploadDir_recur(const char *name);
int upload(char *filename);
void get_all_file_paths(const char *name, FILE *outfile);
void start_backup(char *dir);
void end_backup();

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

	//----------------- UDP to pox -----------------------//
	bzero(&servaddr_pox, sizeof(servaddr_pox));
	servaddr_pox.sin_family = AF_INET;
	servaddr_pox.sin_port = htons(SERV_PORT);
	// DEBUG(20151215): the fake ip should be exist, otherwise block bcs ARP failed
	Inet_pton(AF_INET, serverip, &servaddr_pox.sin_addr); // faked IP addr
	sockfd2pox = Socket(AF_INET, SOCK_DGRAM, 0);

	//--------------- TCP to server --------------------//
	sockfd2server = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT); //
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd2server, (SA *) &servaddr, sizeof(servaddr));
	int a = 65535;
	Setsockopt(sockfd2server, SOL_SOCKET, SO_SNDBUF, &a, sizeof(int));
	//---------------- start ---------------------------//
	start_backup(directory);

	uploadDir(directory);
	// uploadDir_recur(directory);

	gettimeofday(&tv2, NULL);
	printf("File deduplicated = %d\n", file_dedu);
	printf("File deduplicated(POX) = %d, new = %d,pox_reply_count=%d\n", file_dedu_pox, file_new_pox, pox_reply_count);
	printf("Time cost = %lfms\n",(tv2.tv_sec-tv1.tv_sec)*1000.0+(tv2.tv_usec-tv1.tv_usec)/1000.0);

    /* Upload complete, tell server to sync data to SDN controller
     * Need a alg(by dedu ratio) to control this sync
     */
    end_backup();

	close(sockfd4pox);
	
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

int isExistsFile(char *fp_filename, char *filename) {
	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);
	TransferUnit unit; // buffer 
	memset(&unit, 0, sizeof(unit));

	int len  = strlen(fp_filename);
	unit.type = FILE_FP_QUERY;
	unit.len = len;
	strcpy(unit.value, fp_filename);
	// lookup SDN controller cache
	if(sdn_switch){
		// TODO : use UDP communicate with POX pupu
		Sendto(sockfd2pox, fp_filename, strlen(fp_filename), 0, (SA *) &servaddr_pox, sizeof(servaddr_pox));
		memset(recvline, 0, sizeof(recvline));
		Recvfrom(sockfd2pox, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen);
		pox_reply_count ++;
		
		if(strcmp(recvline, "new_file") == 0){ // Bloom filter ,Quick decision
			// printf("**File(%s) new, so start transfer(POX).**\n", filename);
			file_new_pox ++;
			return FILE_QUERY_STATE_NEW_POX;
		}else if(strcmp(recvline, "duplicated") == 0){// cache hit, Quick decision
			// printf("**File(%s) duplicated, so transfer complete(POX)..**\n", filename);
			file_dedu_pox ++;
			return FILE_QUERY_STATE_DUPLICATED_POX;
		}
	}

	
	// Else: Go to server
	Write(sockfd2server, &unit, len + 8);
	memset(recvline, 0, sizeof(recvline));
	Read(sockfd2server, recvline, MAXLINE);

	if(strcmp(recvline, "duplicated") == 0){
		// printf("**File duplicated(%s), so transfer complete..**\n", filename);
		file_dedu ++ ;
		return FILE_QUERY_STATE_DUPLICATED;
	}else{
		// printf("**File new(%s), so start transfer..**\n", filename);
		return FILE_QUERY_STATE_NEW;
	}

	return -1; // cannot get here
}

/*
 * Transfer file data to server
 */
void transfer_file(char *fp_filename, char *filename){
	char	recvline[MAXLINE];
	char buf[CHUNK_SIZE];
	char *sendline = "file chunk data";
	int fd, res, len;
	char firstpkt[MAXLINE]={0};
	TransferUnit unit; // buffer 
	memset(&unit, 0, sizeof(unit));

	FILE *fs = fopen(filename, "r");
	
	strcat(firstpkt, fp_filename);
	len  = strlen(firstpkt);
	unit.type = FILE_FIRST_PACKET;
	unit.len = len;
	strcpy(unit.value, firstpkt);
	Write(sockfd2server, &unit, len + 8);
	// 2. then transfer the whole file chunkly
	memset(&unit, 0, sizeof(unit));
	while((res = fread(unit.value, sizeof(char), CHUNK_SIZE, fs)) > 0){
		unit.type = FILE_DATA_PACKET;
		unit.len = res;
		Write(sockfd2server, &unit, res+8);
		// we DO NOT need reply
		memset(&unit, 0, sizeof(unit));
	}

	// 3. eof it
	sendline = "##file_eof##";
	unit.type = FILE_EOF_PACKET;
	unit.len = strlen(sendline);
	Write(sockfd2server, &unit, unit.len + 8);

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

// transfer file one by one, can be optimized by batch 
int upload(char *filename){
	// For simple, I use 40B hex string instead
	unsigned char fp[20];
	char hash[41];
	int res = 0;
	char fp_filename[MAXLINE];

	// 1. inquiry the server if the file is duplicated by fingerprint (a RTT)
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
		case FILE_QUERY_STATE_DUPLICATED_POX:
			// yes fall through
		case FILE_QUERY_STATE_DUPLICATED:
			break;
		case FILE_QUERY_STATE_NEW_POX:
			// yes fall through
		case FILE_QUERY_STATE_NEW:
			transfer_file(fp_filename, filename);
			break;
		default:
			printf("ERROR!!!\n");
	}
	
	return 0;
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
	int file_count = 0;
	while((nread = getline(&line, &len, file)) > 0 ) {
		// printf("read:%s\n", line);
		line[nread-1] = '\0';  // BUG: strip last \n
		upload(line);
		file_count ++;
	}

	printf("File Count : %d\n", file_count);
	
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
			// printf("------------%s\n", path);
		}

	} while (entry = readdir(dir));

    closedir(dir);
}

void start_backup(char *dir){
	TransferUnit unit; // buffer 
	memset(&unit, 0, sizeof(unit));
	int len  = strlen(dir);
	unit.type = START_BACKUP;
	unit.len = len;
	strcpy(unit.value, dir);
	Write(sockfd2server, &unit, len + 8);
}

void end_backup(){
	char *msg = "##end_backup##";
	TransferUnit unit; // buffer 
	memset(&unit, 0, sizeof(unit));
	int len  = strlen(msg);
	unit.type = END_BACKUP;
	unit.len = len;
	strcpy(unit.value, msg);
	Write(sockfd2server, &unit, len + 8);
}
