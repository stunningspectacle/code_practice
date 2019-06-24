obj-m := hid-sensor-custom.o

KERN := /lib/modules/4.15.0-29-generic/build
DIR:= $(shell pwd)

all:
	make -C $(KERN) M=$(DIR) modules
