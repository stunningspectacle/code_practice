obj-m := c14.o c14_kobj.o

DIR := $(shell pwd)
KERN := /lib/modules/4.15.0-29-generic/build

all:
	make -C $(KERN) M=$(DIR) modules
	cp c14.ko ~/vmshare/
	cp c14_kobj.ko ~/vmshare/

.PHONY: clean
clean:
	rm -rf *.ko *.mod.* modules.* Module* *.o

