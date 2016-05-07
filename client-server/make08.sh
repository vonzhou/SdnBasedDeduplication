rm *.o

gcc -c error.c index05.c jwHash.c readn.c readline.c recipe.c sha1.c sigchldwaitpid.c signal.c wrapsock.c wrapunix.c bloom.c

ar -rcs lib08.a *.o

gcc -g -o client09 client09.c lib08.a

gcc -g -o server09 server09.c lib08.a


rm ../mininet/server09
rm  ../mininet/client09
cp server09 ../mininet/.
cp client09 ../mininet/.

rm *.o

