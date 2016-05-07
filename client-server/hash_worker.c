#include "global.h"

#define WORKERDEBUG     1

#ifdef WORKERDEBUG
# define WORKER_DEBUG(fmt,args...) printf(fmt, ## args)
#else
# define WORKER_DEBUG(fmt,args...) do {} while (0);
#endif

// use hashtable worker 
static jwHashTable *table = NULL;
static int flag = 0;

void hash_worker_init(int buckets){
    int     readfifo, writefifo, dummyfd, fd;
    char    *ptr, buff[MAXLINE], fifoname[MAXLINE];
    pid_t   pid;
    ssize_t n;

    table = create_hash(buckets);
    if(!table) {
        // fail
        return ;
    }

    /* create server's well-known FIFO; OK if already exists */
    if ((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST))
        err_sys("can't create %s", SERV_FIFO);

    /* open server's well-known FIFO for reading and writing */
    readfifo = Open(SERV_FIFO, O_RDONLY, 0);
    dummyfd = Open(SERV_FIFO, O_WRONLY, 0);     /* never used */

    while ( (n = Readline(readfifo, buff, MAXLINE)) > 0) {
        if (buff[n-1] == '\n')
            n--;            /* delete newline from readline() */
        buff[n] = '\0';     /* null terminate <pid + cmd_str + key + value> */

        printf("%s\n", buff);

        if ( (ptr = strchr(buff, ',')) == NULL) {
            err_msg("bogus request: %s", buff);
            continue;
        }

        *ptr++ = 0;         /* null terminate PID, ptr = <cmd_str + key + value> */
        pid = atol(buff);
        WORKER_DEBUG("**pid=%ld**\n", pid);
        snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
        if ( (writefifo = open(fifoname, O_WRONLY, 0)) < 0) {
            err_msg("cannot open: %s", fifoname);
            continue;
        }

        char *tmp = strchr(ptr, ',');
        *tmp++ = 0;
        char *cmd_str = ptr;
        WORKER_DEBUG("**command string=%s**\n", cmd_str);

        ptr = tmp;
        tmp = strchr(ptr, ',');
        *tmp++ = 0;
        char *key = ptr;
        WORKER_DEBUG("**key=%s**\n", key);

        char *val = tmp;
        WORKER_DEBUG("**value=%s**\n", val);

        // hash table op and send the reply to the client by writing to pipe
        char reply[MAXLINE];
        if(strcmp(cmd_str, "add_int_by_str") == 0){
            add_int_by_str(table, key, atoi(val));
            // return 0
            snprintf(reply, sizeof(reply), "0");

        }else if(strcmp(cmd_str, "get_int_by_str") == 0){
            int r = 0;
            get_int_by_str(table, key, &r);
            WORKER_DEBUG("get_int_by_str key=%s, val=%d\n", key, r);
            // return a int
            snprintf(reply, sizeof(reply), "%d", r);

        }else{
            printf("**Unsupported hash table command**\n");
        }

        Write(writefifo, reply, strlen(reply));
        Close(writefifo);
    }
}

int isExistsFP(char *fp){
    int res = exec_hashtable_cmd("get_int_by_str", fp, -1);
    WORKER_DEBUG("**isExistsFP : %d**\n", res);
    return res;
    // todo
}

void insertFP(char *fp){
    exec_hashtable_cmd("add_int_by_str", fp, 1);
}

// file name -> fp
void addFileFP(char *fp, char *filename){
   // todo 
}

// fp -> {file name 1, file name 2.....}
void addFilename(char *fp, char *filename){
   //
}
// key is string, val is int,
int exec_hashtable_cmd(char *cmd, char *key, int val){
    int     readfifo, writefifo;
    size_t  len;
    ssize_t n;
    char    *ptr, fifoname[MAXLINE], buff[MAXLINE];
    pid_t   pid;

    /* create FIFO with our PID as part of name */
    if(flag == 0){
    pid = getpid();
    snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
    if ((mkfifo(fifoname, FILE_MODE) < 0) && (errno != EEXIST))
        err_sys("can't create %s", fifoname);
}
    // readline need a \n 
    snprintf(buff, sizeof(buff), "%ld,%s,%s,%d\n", (long) pid, cmd, key, val);
    len = strlen(buff);

    /* open FIFO to server and write hashtable command to FIFO */
    writefifo = Open(SERV_FIFO, O_WRONLY, 0);
    Write(writefifo, buff, len);
if(flag == 0){
    /* now open our FIFO; blocks until server opens for writing */
    readfifo = Open(fifoname, O_RDONLY, 0);
    flag = 1;
}
    /* read from IPC, get hashtable op result */
    memset(buff, 0, sizeof(buff));
    n = Read(readfifo, buff, MAXLINE);
    WORKER_DEBUG("**reply from server : %s\n", buff);

    Close(readfifo);
    Unlink(fifoname);

    return atoi(buff);
}







