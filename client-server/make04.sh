gcc -c error.c hash_worker.c jwHash.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c

ar -rcs lib04.a *.o

gcc -o client05 client05.c lib04.a

gcc -o server04 server04.c lib04.a
