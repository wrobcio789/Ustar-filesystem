obj-m := ustar.o
ustar-objs := init.o

CFLAGS_init.o := -DDEBUG

all:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M="$(PWD)" clean