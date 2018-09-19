/*
 * Copyright (C) 2011-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GRALLOC_PRIV_H_
#define GRALLOC_PRIV_H_

#include <stdint.h>
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

#include <cutils/log.h>
#include <cutils/native_handle.h>

#include <ion/ion.h>
#include <linux/fb.h>

/*
Here you can change the number of buffers used for rendering surfaces.
This must be aligned with NUM_BUFFERS in egl_platform_android.cpp of the MALI GPU driver!
NUM_BUFFERS = NUM_FB_BUFFERS - 1
*/
#define NUM_FB_BUFFERS 3

#define GRALLOC_ARM_DMA_BUF_MODULE 1

#define GRALLOC_EXTRA_ALLOCATION_UNIT_SIZE 32768

#define GRALLOC_SOFIA_IMC

/*****************************************************************************/
struct fb_dmabuf_export
{
	__u32 fd;
	__u32 flags;
};
/*#define FBIOGET_DMABUF	_IOR('F', 0x21, struct fb_dmabuf_export)*/



typedef enum
{
	MALI_YUV_NO_INFO,
	MALI_YUV_BT601_NARROW,
	MALI_YUV_BT601_WIDE,
	MALI_YUV_BT709_NARROW,
	MALI_YUV_BT709_WIDE,
} mali_gralloc_yuv_info;

struct private_module_t;
struct private_handle_t;


struct fb_list_t {
	uint32_t avail_buffers;
	struct private_handle_t* framebuffer[NUM_FB_BUFFERS];
};

struct private_module_t {
	/* default minimal definition */
	gralloc_module_t base;
	struct private_handle_t* framebuffer;
	/* public custom fields (see hal-public.h) */
	// struct gralloc_module_public_api_t pub;

	uint32_t flags;
	uint32_t numBuffers;
	uint32_t bufferMask;
	pthread_mutex_t lock;
	buffer_handle_t currentBuffer;
	buffer_handle_t frameBufferList[NUM_FB_BUFFERS];
	int ion_client;

	struct fb_var_screeninfo info;
	struct fb_fix_screeninfo finfo;
	/* fb_list support */
	struct fb_list_t *fb_list;
	int (*reserve_fb) (alloc_device_t *, int, int, int);
	float xdpi;
	float ydpi;
	float fps;

	int width;
	int height;
	unsigned int format;
	int gpufd;
	int bpp;
	int line_length;
	int fbfd;

	enum
	{
		// flag to indicate we'll post this buffer
		PRIV_USAGE_LOCKED_FOR_POST = 0x80000000
	};

#if defined(__cplusplus)
	/* default constructor */
	private_module_t();
#endif
};

/*****************************************************************************/

/* Temporary support for VPU specific color formats */
enum {
	HAL_PIXEL_FORMAT_YCbCr_420_SP = 0x12, //NV12
	HAL_PIXEL_FORMAT_YUV_420_SP = 0x15, //NV12
};


#ifdef __cplusplus
struct private_handle_t : public native_handle {
#else
struct private_handle_t {
    struct native_handle nativeHandle;
#endif

    enum {
	    PRIV_FLAGS_FRAMEBUFFER	= 0x00000001,
	    PRIV_FLAGS_CONTIGUOUS	= 0x00000002,
	    /* Needed for Mali */
	    PRIV_FLAGS_USES_ION	= 0x00000008,
	    PRIV_FLAGS_USES_UMP	= 0x08000000,
    };

    enum {
	    LOCK_STATE_WRITE     =   1<<31,
	    LOCK_STATE_MAPPED    =   1<<30,
	    LOCK_STATE_READ_MASK =   0x3FFFFFFF
    };
    // file-descriptors
    int     fd;
    // ints
    int     magic;
    int     usage;
    int     flags;
    int     size;
    int     offset;

    // FIXME: the attributes below should be out-of-line
    unsigned int	base;
    int     lockState;
    int     writeOwner;

    mali_gralloc_yuv_info yuv_info;

    unsigned int	fmt;
    unsigned int	phys;
    int			w;
    int			h;
    int			internalWidth;
    int			internalHeight;
    int			bpp;
    int			byte_stride;

    /* ION */
    ion_user_handle_t	ion_hdl;
    int			ion_fd;

#ifdef __cplusplus
    static const int sNumInts = 17;
    static const int sNumFds = 1;
    static const int sMagic = 0x3141592;

    private_handle_t(int fd, int usage, int flags, int size) :
        fd(fd),
	magic(sMagic),
	usage(usage),
	flags(flags),
	size(size),
	offset(0),
	base(0),
	lockState(LOCK_STATE_MAPPED),
	writeOwner(0),
	yuv_info(MALI_YUV_NO_INFO),
	phys(0),
	w(0),
	h(0),
	internalWidth(0),
	internalHeight(0),
	bpp(0),
	byte_stride(0),
	ion_hdl(0),
	ion_fd(-1)
    {
        version = sizeof(native_handle);
        numInts = sNumInts;
        numFds = sNumFds;
    }
    ~private_handle_t() {
        magic = 0;
    }

    static int validate(const native_handle* h) {
        const private_handle_t* hnd = (const private_handle_t*)h;
        if (!h || h->version != sizeof(native_handle) ||
                h->numInts != sNumInts || h->numFds != sNumFds ||
                hnd->magic != sMagic)
        {
            ALOGE("invalid gralloc handle (at %p)", h);
            return -EINVAL;
        }
        return 0;
    }
#endif
};

#endif /* GRALLOC_PRIV_H_ */
