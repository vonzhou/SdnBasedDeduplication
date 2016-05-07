rm *.o

gcc -c error.c index05.c jwHash.c readn.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c bloom.c

ar -rcs lib07.a *.o

gcc -g -o client07 client07.c lib07.a

gcc -g -o server07 server07.c lib07.a


rm ../mininet/server07
rm  ../mininet/client07
cp server07 ../mininet/.
cp client07 ../mininet/.

rm *.o

