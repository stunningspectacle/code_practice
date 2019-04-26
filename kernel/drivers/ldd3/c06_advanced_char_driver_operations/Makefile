obj-m := blocking_io.o hello.o

PWD := $(shell pwd)
KERN := /lib/modules/4.15.0-29-generic/build

all:
	make -C $(KERN) M=$(PWD) modules
	cp blocking_io.ko ~/vmshare
