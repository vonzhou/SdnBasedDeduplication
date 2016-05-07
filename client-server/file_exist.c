#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


// TODO
// access vs stat performance test

int exist_file(char *fname){
	if((access(fname, F_OK)) != -1){
		return 1;  // file exist
	}

	return 0; // file not exist

}


int main(int argc, char **argv){
	printf("File exist = %d\n", exist_file(argv[1]));
	return 0;
}

