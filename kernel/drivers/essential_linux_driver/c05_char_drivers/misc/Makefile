obj-m := mymisc.o

KERN := /lib/modules/4.15.0-29-generic/build
PWD := $(shell pwd)

all:
	make -C $(KERN) M=$(PWD) modules
	cp mymisc.ko ~/qemu_shared/
