rm *.o

gcc -c error.c index05.c jwHash.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c

ar -rcs lib05.a *.o

gcc -o client05 client05.c lib05.a

gcc -o server05 server05.c lib05.a

cp server05 ../mininet/.
cp client05 ../mininet/.

