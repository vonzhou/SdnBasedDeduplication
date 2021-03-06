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
 */

#define CHUNK_SIZE 1400
#define QUICK_PORT 9999

 int get_tcp_socket(char *ip, int port);
 int get_udp_socket(int port);

// use for file chunk and sha1
FileInfo *fi;

int isExistsFile(int sockfd, int udpfd, char *fp_filename) {
	char	recvline[MAXLINE];
	struct sockaddr_in	cliaddr;
	socklen_t	cliaddrlen;
	
	Write(sockfd, fp_filename, strlen(fp_filename));

	// recv reply from POX 
	// channel to get quick decision from POX 
	// memset(recvline, 0, sizeof(recvline));  // bug:
	// Recvfrom(udpfd, recvline, MAXLINE, 0,  (SA *) &cliaddr, &cliaddrlen); // TODO timeout
	// printf("----%s----\n", recvline);

	if(strcmp(recvline, "new_file") == 0){ // Bloom filter ,Quick decision
		printf("**File new, so start transfer(POX).**\n");
		return 0;
	}else if(strcmp(recvline, "duplicated") == 0){// cache hit, Quick decision
		printf("**File duplicated, so transfer complete(POX)..**\n");
		return 1;
	}else{// go to server , wait for the reply from server
		memset(recvline, 0, sizeof(recvline)); 

		if (Read(sockfd, recvline, MAXLINE) == 0)
			err_quit("client: server terminated prematurely");
		// printf("=============%s\n", recvline);
		if(strcmp(recvline, "duplicated") == 0){
			printf("**File duplicated, so transfer complete..**\n");
			return 1;
		}else{
			printf("**File new, so start transfer..**\n");
			return 0;
		}
	}
	
	return -1; // cannot get here
}

void
transfer_file(int sockfd, char *fp_filename, char *filename){
	char	recvline[MAXLINE];
	char buf[CHUNK_SIZE];
	char *sendline = "file chunk data";
	int fd, res;
	

	int chunk_count = 4, i = 0;
	FILE *fs = fopen(filename, "r");

	// 1. transfer fp+file name
	printf("-------%s\n", fp_filename);
	Write(sockfd, fp_filename, strlen(fp_filename));
	// fp+file name should be a separate packet
	if (Read(sockfd, recvline, MAXLINE) == 0)
			err_quit("transfer_file: server terminated prematurely");

	// 2. then transfer the whole file chunkly
	while((res = fread(buf, sizeof(char), CHUNK_SIZE, fs)) > 0){
		Write(sockfd, buf, res);
		// we DO NOT need reply
		memset(buf, 0, CHUNK_SIZE);
	}

	// 3. eof it
	sendline = "exit";
	Write(sockfd, sendline, strlen(sendline));
	
	fclose(fs);
}

int
main(int argc, char **argv)
{
	int					sockfd, sockfd2;
	struct sockaddr_in	servaddr;
	// For simple, I use 40B hex string instead
	char hash[41];
	int res = 0;
	char fp_filename[MAXLINE];

	if (argc != 3)
		err_quit("usage: client <IPaddress> <file>");

	// 1. inquiry the server if the file is duplicated by fingerprint (a RTT)
	fi = file_new();
	strcpy(fi->file_path, argv[2]);
	// file's sha1 
	res = SHA1File(fi->file_path, fi->file_hash);
	if(res){
		printf("Get file sha1 hash failed.\n");
		exit(-1);
	}
	digestToHash(fi->file_hash, hash); // rabin.h 
	fp_filename[0] = '\0';
	strcat(fp_filename, hash);
	strcat(fp_filename, argv[2]);

	int udpfd = get_udp_socket(QUICK_PORT);// channel with POX
	
	sockfd = get_tcp_socket(argv[1], SERV_PORT+1);
	res = isExistsFile(sockfd, udpfd, fp_filename);		/* fp match with server */

	close(sockfd); // yes close sockfd

	if(res){ // file existed , stop
		exit(0);
	}

	// 2. transfer not duplicated chunks to server 
	sockfd2 = get_tcp_socket(argv[1], SERV_PORT);
	transfer_file(sockfd2, fp_filename, argv[2]);
	close(sockfd2);

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
