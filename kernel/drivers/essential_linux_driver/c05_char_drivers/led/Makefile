obj-m := led.o

KERN := /lib/modules/4.15.0-29-generic/build
PWD := $(shell pwd)

all:
	make -C $(KERN) M=$(PWD) modules
