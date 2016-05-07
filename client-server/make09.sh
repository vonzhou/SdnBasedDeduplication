rm *.o
rm *.a

gcc -c error.c readn.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c bloom.c

ar -rcs lib10.a *.o

gcc -g -o client09 client09.c lib10.a

gcc -g -o server10 server10.c lib10.a


rm ../mininet/server10
rm  ../mininet/client09
cp server10 ../mininet/.
cp client09 ../mininet/.

rm *.o

