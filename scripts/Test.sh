#!/bin/bash
if [ "$1" == '' ]; then
   echo "Using script: sudo ./Test.sh size"
   exit
fi
if [ -f ../dist/dmp.ko ]; then
	size=$1
	sudo insmod ../dist/dmp.ko
	sudo dmsetup create zero1 --table "0 $size zero"
	sudo ls -al /dev/mapper/*
	sudo dmsetup create dmp1 --table "0 $size proxy /dev/mapper/zero1 0"
	sudo ls -al /dev/mapper/*
	sudo dd if=/dev/random of=/dev/mapper/dmp1 bs=4k count=1
	sudo dd of=/dev/null if=/dev/mapper/dmp1 bs=4k count=1
	sudo cat /sys/module/dmp/stat/volumes
	sudo dmsetup remove dmp1
	sudo dmsetup remove zero1
	sudo rmmod dmp
	exit
else
	echo "[ERROR]:   Build the solution first"
	exit
fi
