# DOS needs variable MAKESHELL. Linux does not care.
export MAKESHELL :=	bash

CROSS_COMPILE :=	i486-elf-

export CC :=		$(CROSS_COMPILE)gcc
export LD :=		$(CROSS_COMPILE)ld
export OBJCOPY :=	$(CROSS_COMPILE)objcopy
export OBJDUMP :=	$(CROSS_COMPILE)objdump
export NM :=		$(CROSS_COMPILE)nm
export READELF :=	$(CROSS_COMPILE)readelf
export AR :=	$(CROSS_COMPILE)ar
export RM :=		del
export MD :=		md