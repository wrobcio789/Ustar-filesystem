dmesg --clear

make
insmod ustar.ko

mkdir -p testdir
touch image
mount -o loop -t ustar ./sample.tar ./testdir

umount ./testdir
rmmod ustar

dmesg