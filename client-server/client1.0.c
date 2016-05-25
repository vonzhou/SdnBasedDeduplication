/*
## Backup Client 

### 0.1

 * maybe return the list of fp indicating the chunk needed to transfer to server TODO
 * here just a whole file ,so i return 0/1
 * bug 1: Use read() to get reply from serer NOT readline() ...
 * bug 2: buffer should be memset to 0
 *


### 0.2

 * add file fp
 * transfer file by fixed chunk
 * bug 1: in function var should be set 0


### 0.3

 * recv quik reply from SDN controller (here i use UDP, low cost)
 * bug 1: recvfrom error: Bad address, recvfrom last arg should be a socklent pointer

### 0.4

 * reconstruct use UDP, count time


### 0.5

 * Transfer files in a folder

### 0.6

 * Triger the server to sync bloom filter to SDN controller
 * Inform the start and end of a backup
 * If file is new telled by POX, then server just store (not dedu)
 

### 0.7

 * UDP port for fp and control flow, and TCP port file data flow
 * UDP packets loss problem when large flow in the SDN network (set timeout to recvfrom)

### 0.8

* Oops return to USE TCP to send fp and file, bcs UDP unreliable
* Implement a reliable UDP is not easy(tried!)
* Communicate with POX use UDP[client -> POX -> server]
* TODO transfer SHA-1 by raw 20 bytes, and then convert to 40 bytes representation
 

### 0.9

 Use a config file to index the files

### 1.0

control deduplicatoin method with args
*/
#include	"global.h"

#define CHUNK_SIZE 1400
#define QUICK_PORT 9999
#define CONFIG_FILE "configfile"

static int sockfd4pox = -1; // recv SDN controller action
static int sockfd2pox = -1;  // fp channel
static int sockfd2server = -1;	// transfer file channel
static struct sockaddr_in servaddr_pox;
static struct sockaddr_in servaddr;
char *serverip = "127.0.0.1";

static int files_count = 0;
static int files_deduplicated = 0;
static int files_new = 0;
static int files_deduplicated_pox = 0;
static int files_new_pox = 0;
static int pox_reply_count = 0;
static double time_ms;
static double time_s;

static int sdna_on = 0;

typedef struct {
	unit_type type;
	int len;
	char value[CHUNK_SIZE];
}TransferUnit;

int get_tcp_socket(char *ip, int port);
int get_udp_socket(int port);

int isExistsFile(char *fp);
void transfer_file(char *filepath, char *fp);
void uploadDir(const char *name);
void uploadDir_recur(const char *name);
int upload(char *filename);
void create_config_file(const char *name, FILE *outfile);
void start_backup(char *dir);
void end_backup(char *dir);
void transfer_config_file(char *dir);

void backup_log(){
	printf("file_count=%d,file_deduplicated=%d,file_new=%d,file_deduplicated_pox=%d,file_new_pox=%d,time_ms=%f,time_s=%f\n", files_count,files_deduplicated,files_new,files_deduplicated_pox,files_new_pox,time_ms,time_s);
}

int main(int argc, char **argv){
	int res = 0;
	struct timeval tv1, tv2;
	char *directory = NULL;
	char *dedu_method = NULL;

	//printf("Usage: client -i server_ip_addr -d backup_dir -m [base/bloom/sdna]\n");

	if (argc != 4)
		err_quit("Usage: client server_ip_addr [base/bloom/sdna] backup_dir ");

	serverip = argv[1];
	dedu_method = argv[2];
	directory = argv[3];
	
	if(strcmp(dedu_method, "sdna") == 0)
		sdna_on = 1;

	gettimeofday(&tv1, NULL);

	sockfd4pox = get_udp_socket(QUICK_PORT);// virtual channel with POX

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
	/*
	if(SDN_SWITCH)
		printf("File deduplicated(POX) = %d, new = %d,pox_reply_count=%d\n", file_dedu_pox, file_new_pox, pox_reply_count);
	printf("File deduplicated = %d, new = %d\n", file_dedu_server, file_new_server);
	*/
	time_ms = (tv2.tv_sec-tv1.tv_sec)*1000.0+(tv2.tv_usec-tv1.tv_usec)/1000.0;
	time_s = time_ms/1000;
	// printf("Time cost = %lf ms = %lf s\n",total_ms, total_ms/1000);

    /* Upload complete, tell server to sync data to SDN controller
     * Need a alg(by dedu ratio) to control this sync
     */
    end_backup(directory);
    backup_log();

	close(sockfd4pox);
	close(sockfd2server);

	return 0;
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

int isExistsFile(char *fp) {
	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen = sizeof(cliaddr);
	TransferUnit unit; // buffer
	memset(&unit, 0, sizeof(unit));

	int len  = strlen(fp);
	unit.type = FILE_FP_QUERY;
	unit.len = len;
	strcpy(unit.value, fp);
	// lookup SDN controller cache
	if(sdna_on){
		// TODO : use UDP communicate with POX pupu
		Sendto(sockfd2pox, fp, strlen(fp), 0, (SA *) &servaddr_pox, sizeof(servaddr_pox));
		memset(recvline, 0, sizeof(recvline));
		Recvfrom(sockfd2pox, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen);
		pox_reply_count ++;

		if(strcmp(recvline, "new_file") == 0){ // Bloom filter ,Quick decision
			// printf("**File(%s) new, so start transfer(POX).**\n", filename);
			files_new_pox ++;
			return FILE_QUERY_STATE_NEW_POX;
		}else if(strcmp(recvline, "duplicated") == 0){// cache hit, Quick decision
			// printf("**File(%s) duplicated, so transfer complete(POX)..**\n", filename);
			files_deduplicated_pox ++;
			return FILE_QUERY_STATE_DUPLICATED_POX;
		}
	}


	// Else: Go to server
	Write(sockfd2server, &unit, len + 8);
	memset(recvline, 0, sizeof(recvline));
	Read(sockfd2server, recvline, MAXLINE);

	if(strcmp(recvline, "duplicated") == 0){
		// printf("**File duplicated(%s), so transfer complete..**\n", fp);
		files_deduplicated ++ ;
		return FILE_QUERY_STATE_DUPLICATED;
	}else{
		files_new ++;
		// printf("**File new(%s), so start transfer..**\n", fp);
		return FILE_QUERY_STATE_NEW;
	}

	return -1; // cannot get here
}

/*
 * Transfer file data to server
 */
void transfer_file(char *filepath, char *fp){
	char buf[CHUNK_SIZE];
	char *sendline = "file chunk data";
	int fd, res, len;

	TransferUnit unit; // buffer
	memset(&unit, 0, sizeof(unit));

	FILE *fs = fopen(filepath, "r");
	// printf("[*] transfer %s, fp=%s\n", filepath, fp);

	unit.type = FILE_FIRST_PACKET;
	unit.len = strlen(fp);
	strcpy(unit.value, fp);
	Write(sockfd2server, &unit, unit.len + 8);  //DEBUG:  use unit.len NOT len (undefined)
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

/* transfer file one by one, can be optimized by batch
 * line = file path + fingerprint
 */
int upload(char *filepath_fp){
	char *space = strchr(filepath_fp, ' ');
	*space = '\0';
	char *fp = space + 1;
	char *filepath = filepath_fp;

	int res = isExistsFile(fp);

	switch(res){
		case FILE_QUERY_STATE_DUPLICATED_POX:
			// yes fall through
		case FILE_QUERY_STATE_DUPLICATED:
			break;
		case FILE_QUERY_STATE_NEW_POX:
			// yes fall through
		case FILE_QUERY_STATE_NEW:
			transfer_file(filepath, fp);
			break;
		default:
			printf("ERROR!!!\n");
	}

	return 0;
}

// upload a folder
void uploadDir(const char *name){
	FILE *configfile = fopen(CONFIG_FILE, "w+");
	if(configfile == NULL){
		printf("error opening file\n");
		exit(-1);
	}

	create_config_file(name, configfile);
	fflush(configfile);
	
	fseek(configfile, 0L, SEEK_SET);

	char *line = NULL;
	size_t len = 0;
	ssize_t nread = -1;
	while((nread = getline(&line, &len, configfile)) != -1) {
		line[nread-1] = '\0';  // BUG: strip last \n
		// printf("read:%s", line);
		upload(line);

		files_count ++;
	}
	
	// printf("File Count : %d\n", files_count);

	if(fclose(configfile)){
		printf("error closing file\n");
		exit(-1);
	}
	if(line)
		free(line);
}
/*
 * Config file: file path  -> fingerprint
 */
void create_config_file(const char *name, FILE *outfile){
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
      		create_config_file(path, outfile);
      	}
      	else{
			char path[MAXLINE];
			unsigned char fp[20];
			char hash[41];
			int res = 0;

			int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
			path[len] = 0;

			res = SHA1File(path, fp);
			if(res){
				printf("Get file sha1 hash failed.\n");
				exit(-1);
			}
			digestToHash(fp, hash); // rabin.h

			fprintf(outfile, "%s %s\n", path, hash);
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

void end_backup(char *dir){
	char *msg = "##end_backup##";
	TransferUnit unit; // buffer
	memset(&unit, 0, sizeof(unit));

	// TODO Send Config File To Server, Now Ignore It
	// transfer_config_file(dir);

	int len  = strlen(msg);
	unit.type = END_BACKUP;
	unit.len = len;
	strcpy(unit.value, msg);
	Write(sockfd2server, &unit, len + 8);
}

void transfer_config_file(char *dir){
	char buf[CHUNK_SIZE];
	char *sendline = "config file chunk data";
	int fd, res, len;

	TransferUnit unit; // buffer
	memset(&unit, 0, sizeof(unit));

	FILE *fs = fopen(CONFIG_FILE, "r");
	// printf("[*] transfer %s, fp=%s\n", filepath, fp);

	unit.type = BACKUP_CONFIG_FILE_START;
	unit.len = strlen(dir);
	strcpy(unit.value, dir);
	Write(sockfd2server, &unit, unit.len + 8);

	memset(&unit, 0, sizeof(unit));
	while((res = fread(unit.value, sizeof(char), CHUNK_SIZE, fs)) > 0){
		unit.type = BACKUP_CONFIG_FILE;
		unit.len = res;
		Write(sockfd2server, &unit, unit.len + 8);
		memset(&unit, 0, sizeof(unit));
	}

	sendline = "##file_eof##";
	unit.type = BACKUP_CONFIG_FILE_END;
	unit.len = strlen(sendline);
	Write(sockfd2server, &unit, unit.len + 8);

	fclose(fs);
}
