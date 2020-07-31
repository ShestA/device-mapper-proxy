#!/bin/bash
if [ -f ../dist/dmp.ko ]; then
	sudo insmod ../dist/dmp.ko
	sudo dd if=/dev/zero of=/tmp/disk bs=4k count=10000
	sudo losetup /dev/loop9 /tmp/disk
	sudo dmsetup create disk_mapper --table "0 80000 proxy /dev/loop9 0"
	sudo cat /sys/module/dmp/stat/volumes
	sudo dd if=/dev/random of=/dev/mapper/disk_mapper bs=4k count=1
	for i in {0..10}
	do
		sudo dd of=/dev/null if=/dev/mapper/disk_mapper bs=4k count=1
	done
	sudo cat /sys/module/dmp/stat/volumes
	sudo dmsetup remove disk_mapper
	sudo losetup -d /dev/loop9
	sudo rm /tmp/disk
	sudo rmmod dmp
	exit
else
	echo "[ERROR]:   Build the solution first"
	exit
fi