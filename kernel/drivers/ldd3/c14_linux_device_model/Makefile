MYSRC := $(filter-out %.mod.c, $(wildcard *.c))
MYTARGET := $(patsubst %.c, %.o, $(SRC))

obj-m := c14.o c14_kobj.o c14_bus.o c14_driver_intel.o c14_driver_apple.o c14_device.o

DIR := $(shell pwd)
KERN := /lib/modules/4.15.0-29-generic/build

all:
	make -C $(KERN) M=$(DIR) modules
	cp c14.ko ~/vmshare/
	cp c14_kobj.ko ~/vmshare/
	cp c14_bus.ko ~/vmshare/
	cp c14_driver_intel.ko ~/vmshare/
	cp c14_driver_apple.ko ~/vmshare/
	cp c14_device.ko ~/vmshare/

.PHONY: clean
clean:
	@rm -rf *.ko *.mod.* modules.* Module* *.o

