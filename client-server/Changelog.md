# Client/Server change log

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

## Backup Server

### 0.1

 * Get file fingerprint from client
 * see if the file is duplicated
 * reply with yes/no

### 0.2

 * recv file by fixed chunk, redis store index
 *

### 0.3

 * use hashtable not redis
 * IPC use mmap
 * a worker process to maintain the hashtable (command by fifo pipe)

### 0.4

 * reconstruct use UDP
 * recv and write chunk in a directory
 * bug : open error for /tmp/fifo.20093: Too many open files
 *

### 0.5

 * do not need hash table worker, bcs this is a single process
 * All the fp are in the memory(hash table)
 *


### 0.6

 * Add bloom filter all fps (NOT USED YET)
 * All unique fps append to a file  (Index File on Disk)
 * Delta updated fps will sync to POX for updating BloomFilter and Cache
 * bug: some chunk begin with '#', so error interpreted as a new file fp
 *

### 0.7

 * UDP port for fp and control flow, and TCP port file data flow
 * Single process model
 * Need readn not read to get the size you want
 * Try to handle UDP packet loss, but too hard !
 * 

### 0.8

 * Oops return to USE TCP to send fp and file, bcs UDP unreliable
 * Implement a reliable UDP is not easy(tried!)
 * Not update fp cache and fp file when check dedu, BUT when we recv the file 

### 0.9

 * Use a config file to index the files 
 * Not use hash table to cache fingerprint, file exists by syscall ! 
 * Only need to record the latest fp into a file, afer backup, sync to the sdn controller, not need file containing all fps
 * store folder: all files named by fp
 * index folder: store the config file of every backup

### 1.0

use option to choose different deduplication method, same as client1.0





