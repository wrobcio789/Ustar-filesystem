dmesg --clear

make
insmod ustar.ko

mkdir -p testdir
touch image
mount -o loop -t ustar ./image ./testdir

umount ./testdir
rmmod ustar

dmesg