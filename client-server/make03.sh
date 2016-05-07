gcc -c error.c hash_worker.c jwHash.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c

ar -rcs lib03.a *.o

gcc -o client03 client03.c lib03.a

gcc -o server03 server03.c lib03.a
