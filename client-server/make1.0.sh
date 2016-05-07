rm *.o
rm *.a

gcc -c error.c readn.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c bloom.c

ar -rcs lib10.a *.o

gcc -g -o client client1.0.c lib10.a

gcc -g -o server server1.0.c lib10.a


rm ../mininet/server
rm  ../mininet/client
cp server ../mininet/.
cp client ../mininet/.

rm *.o

