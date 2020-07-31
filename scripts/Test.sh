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


test:
	@echo ">>   Making module dmp"
	@make -s -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
	@sudo rm -r ../dist
	@mkdir ../dist
	@mv dmp.ko ../dist/dmp.ko
	@echo "**************************"
	@sudo ./../scripts/MountZERO.sh 16384 test
	@echo "**************************"
	@dd if=/dev/random of=/dev/mapper/testMapper bs=16k count=1
	@echo "**************************"
	@dd of=/dev/null if=/dev/mapper/testMapper bs=16k count=1
	@echo "**************************"
	@sudo ./../scripts/Info.sh
	@echo "**************************"
	@sudo ./../scripts/Unmount.sh test
	@echo "**************************"
	exit

echo "**************************"
sudo ./MountZERO.sh 16384 zero test
echo "**************************"
dd if=/dev/random of=/dev/mapper/testMapper bs=16k count=1
echo "**************************"
dd of=/dev/null if=/dev/mapper/testMapper bs=16k count=1
echo "**************************"
sudo ./Info.sh
echo "**************************"
sudo ./Unmount.sh test
echo "**************************"

sudo dd if=/dev/zero of=/tmp/disk bs=4k count=25000
sudo losetup /dev/loop9 /tmp/disk
sudo dmsetup create disk_mapper --table "0 25000 proxy /dev/loop9 0"
sudo mkfs -t ext4 /dev/mapper/disk_mapper 25000
sudo mount -t ext4 /dev/mapper/disk_mapper /mnt


sudo dmsetup remove disk_mapper
sudo losetup -d /dev/loop9
sudo rm /tmp/disk
sudo rmmod dmp