obj-m := cmos.o

KERN := /lib/modules/4.15.0-29-generic/build
PWD := $(shell pwd)

all:
	make -C $(KERN) M=$(PWD) modules
	cp cmos.ko ~/qemu_shared/

clean:
	@rm *.o *.ko Module.* modules.* *.mod.c
