#include	"global.h"

/* Dump fps to file
 */

void dump_fingerprints(const char *name);
void get_all_file_paths(const char *name, FILE *outfile);

int file_count = 0; // udp gram rate control


int main(int argc, char **argv){
	int res = 0;
	struct timeval tv1, tv2;
	char *directory = NULL;

	if (argc != 2)
		err_quit("usage: dump_fps <file dir>");
	directory = argv[1];



	dump_fingerprints(directory);
	
	exit(0);
}

// get all their fps to a file
void dump_fingerprints(const char *name){
	unsigned char fp[20];
	char hash[41];
	int res = 0;
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
		res = SHA1File(line, fp);
		if(res){
			printf("Get file sha1 hash failed.\n");
			exit(-1);
		}
		digestToHash(fp, hash); // rabin.h
		printf("%s\n", hash);
		file_count ++;
	}

	// printf("File Count : %d\n", file_count);
	
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

