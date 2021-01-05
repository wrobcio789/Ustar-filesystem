make
insmod ustar.ko

mount -o loop --read-only -t ustar ./sample.tar ./testdir