#!/bin/bash

LIMIT=150

start=1

while [ "$start" -le $LIMIT ]
do 
	# echo "$start"
	rm -rf $start
	let "start+=1"
done


dir_num=1
for a in *.tar.gz
do 
	mkdir $dir_num
	echo $a:$dir_num
	tar -zxf $a -C $dir_num
	# echo $dir_num
	let "dir_num+=1"
done

