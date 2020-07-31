Пример использования прокси-устройства с устройством loop:
# insmod path/to/module/dmp.ko
# dd if=/dev/zero of=/path/to/file-disk bs=4k count=20000
# losetup /dev/loop0 /tmp/disk
# dmsetup create disk_mapper --table "0 160000 proxy /dev/loop0 0"
# mkfs -t ext4 /dev/mapper/disk_mapper 80000
# mount -t ext4 /dev/mapper/disk_mapper /path/to/mnt
# Работа с подключенным устройством в файловой системе ext4
# Получение текущей статистики прокси-устройства
# cat /sys/module/dmp/stat/volumes
# umount /mnt
# dmsetup remove disk_mapper
# losetup -d /dev/loop9
# rm /tmp/disk
# rmmod dmp
