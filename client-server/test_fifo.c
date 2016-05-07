#include "global.h"

int main(){
	int     readfifo, writefifo;
    size_t  len;
    ssize_t n;
    char    *ptr, fifoname[MAXLINE], buff[MAXLINE];
    pid_t   pid;
    int i;

    /* create FIFO with our PID as part of name */
    pid = getpid();
    for(i=0; i<10; i++){
    	snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
   	 	if ((mkfifo(fifoname, FILE_MODE) < 0) && (errno != EEXIST))
        	err_sys("can't create %s", fifoname);
    }
    
}