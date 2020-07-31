#!/bin/bash
if [ -f ../dist/dmp.ko ]; then
	sudo insmod ../dist/dmp.ko
	sudo dd if=/dev/zero of=/tmp/disk bs=4k count=20000
	sudo losetup /dev/loop9 /tmp/disk
	sudo dmsetup create disk_mapper --table "0 160000 proxy /dev/loop9 0"
	sudo mkfs -t ext4 /dev/mapper/disk_mapper 80000
	sudo mount -t ext4 /dev/mapper/disk_mapper /mnt
	sudo cat /sys/module/dmp/stat/volumes
	sudo cp /bin/gdb /mnt/gdb
	sudo cp /bin/gdb /mnt/gdb1
	sudo cp /bin/gdb /mnt/gdb2
	sudo cp /bin/gdb /mnt/gdb3
	sudo cp /bin/gdb /mnt/gdb4
	sudo cp /bin/gdb /mnt/gdb5
	echo ls /mnt/*
	sudo cat /sys/module/dmp/stat/volumes
	sudo umount /mnt
	sudo dmsetup remove disk_mapper
	sudo losetup -d /dev/loop9
	sudo rm /tmp/disk
	sudo rmmod dmp
	exit
else
	echo "[ERROR]:   Build the solution first"
	exit
fi