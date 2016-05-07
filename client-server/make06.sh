rm *.o

gcc -c error.c index05.c jwHash.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c bloom.c

ar -rcs lib06.a *.o

gcc -g -o client06sdn client06sdn.c lib06.a
gcc -g -o client06 client06.c lib06.a

gcc -g -o server06 server06.c lib06.a


rm ../mininet/server06
rm  ../mininet/client06sdn
rm  ../mininet/client06
cp server06 ../mininet/.
cp client06sdn ../mininet/.
cp client06 ../mininet/.

rm *.o

