obj-m := hello.o

PWD := $(shell pwd)
KERN := /lib/modules/$(shell uname -r)/build
all:
	make -C $(KERN) M=$(PWD) modules
	#cp hello.ko ~/vmshare/

.PHONY: clean
clean:
	@rm -rf *.ko *.o Module.* *.mod.c modules.*
