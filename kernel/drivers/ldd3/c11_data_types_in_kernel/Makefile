obj-m := c11.o

KERN := /lib/modules/$(shell uname -r)/build
MOD := $(shell pwd)

all:
	make -C $(KERN) modules M=$(MOD)
