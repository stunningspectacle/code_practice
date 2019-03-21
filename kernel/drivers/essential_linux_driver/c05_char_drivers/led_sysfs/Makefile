obj-m := led_sysfs.o

KERN := /lib/modules/4.15.0-29-generic/build
PWD := $(shell pwd)

all:
	make -C $(KERN) M=$(PWD) modules
	cp led_sysfs.ko ~/qemu_shared

clean:
	rm Module.* modules.* *.o *.ko

