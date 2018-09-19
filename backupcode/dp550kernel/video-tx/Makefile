#
# (C) COPYRIGHT 2014-2015 ARM Limited. All rights reserved.
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


# default to building for the host
ARCH ?= $(shell uname -m)

ifeq ($(KDIR),)
$(error Must specify KDIR to point to the kernel to target))
endif

all: video-tx test_video_tx

video-tx:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) EXTRA_CFLAGS="-I$(CURDIR)/../../../include" CONFIG_VIDEO_TX=m

test_video_tx:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) EXTRA_CFLAGS="-I$(CURDIR)/../../../include" CONFIG_TEST_VIDEO_TX=m

sii902x:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) EXTRA_CFLAGS="-I$(CURDIR)/../../../include" CONFIG_SII902X=m

senc_tx:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) EXTRA_CFLAGS="-I$(CURDIR)/../../../include" CONFIG_SENC_TX=m

clean:
	$(MAKE) ARCH=$(ARCH) -C $(KDIR) M=$(CURDIR) clean
