#
# (C) COPYRIGHT 2013-2014 ARM Limited. All rights reserved.
#
# This program is free software and is provided to you under the terms of the
# GNU General Public License version 2 as published by the Free Software
# Foundation, and any use by you of this program is subject to the terms
# of such GNU licence.
#
# A copy of the licence is included with the program, and can also be obtained
# from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA  02110-1301, USA.
#
#


# linux build system bootstrap for out-of-tree module

# default to building for the host
ARCH ?= $(shell uname -m)

ifeq ($(KDIR),)
$(error Must specify KDIR to point to the kernel to target))
endif

all: mali-dp

debug:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) KBUILD_EXTRA_SYMBOLS=$(CURDIR)/../../video-tx/Module.symvers EXTRA_CFLAGS="-I$(CURDIR)/../../../../include -I$(KDIR)/drivers/staging/android" CONFIG_MALI_DP=m CONFIG_MALI_DP_DEBUG=y

mali-dp:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) KBUILD_EXTRA_SYMBOLS=$(CURDIR)/../../video-tx/Module.symvers EXTRA_CFLAGS="-I$(CURDIR)/../../../../include -I$(KDIR)/drivers/staging/android" CONFIG_MALI_DP=m

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) clean

