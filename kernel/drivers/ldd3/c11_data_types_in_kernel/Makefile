obj-m := c11.o

KERN := /lib/modules/$(shell uname -r)/build
MOD := $(shell pwd)

all:
	make -C $(KERN) modules M=$(MOD)

.PHONY := clean
clean:
	rm -rf *.ko *.mod.c *.mod.o modules.order Module.symvers

