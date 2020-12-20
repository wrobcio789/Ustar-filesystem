obj-m := ustar.o
ustar-objs := init.o super.o inode.o ustar_base.o

CFLAGS_init.o := -DDEBUG
CFLAGS_super.o := -DDEBUG
CFLAGS_ustar_base.o := -DDEBUG
CFLAGS_inode.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean