#!/bin/bash
if [[ $(id -u) -ne 0 ]]; then
   echo "[ERROR]:   Please run as root for start"
   exit
fi

echo ">>   Check install: make"
stat=$(apt-cache policy make | grep -w none)
if [ "$stat" != '' ]; then
   echo ">>   Installing 'make'"
   sudo apt-get update
   sudo apt-get install make
fi

echo ">>   Check install: gcc"
stat=$(apt-cache policy gcc | grep -w none)
if [ "$stat" != '' ]; then
   echo ">>   Installing 'gcc'"
   sudo apt-get update
   sudo apt-get install gcc
fi

cd ../src

case $1 in
   "--clean")
      echo ">>   Start making dmp.ko with cleanup"
      make clean
      ;;
   "--test")
      echo ">>   Start making dmp.ko with test"
      make test
      ;;
   *)
      echo ">>   Start making dmp.ko file"
      make
      ;;
esac