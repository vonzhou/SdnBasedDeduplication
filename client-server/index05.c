/*
 * Just contain all the fps int a hash table
 * TODO: hash table is cache , all the fps are on the disk
 */

#include "global.h"

#define WORKERDEBUG     1

#ifdef WORKERDEBUG
# define WORKER_DEBUG(fmt,args...) printf(fmt, ## args)
#else
# define WORKER_DEBUG(fmt,args...) do {} while (0);
#endif

// fp hash table
static jwHashTable *table = NULL;

void hash_init(int buckets){
    table = create_hash(buckets);
    if(!table) {        // fail
        printf("create hash table failed..");
        exit(-1) ;
    }
}

int isExistsFP(char *fp){
    int r = 0;
    get_int_by_str(table, fp, &r);
    return r;
}


// update index , cache is updated when miss
void insertFP(char *fp){
    add_int_by_str(table, fp, 1);
}

// file name -> fp
void addFileFP(char *fp, char *filename){
   // todo
}

// fp -> {file name 1, file name 2.....}
void addFilename(char *fp, char *filename){
   //
}
