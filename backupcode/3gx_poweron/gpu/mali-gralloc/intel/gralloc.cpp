/*
 * Copyright (C) 2011-2015 Intel Mobile Communications GmbH
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

#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <ion/ion.h>
#include <linux/xgold_ion.h>
#include "ion-hal.h"
#include "gralloc_priv.h"
#include "gr.h"

#include <utils/CallStack.h>

#include <stdio.h>

#define DBG_GRALLOC_ION_HEAP

/*****************************************************************************/

struct gralloc_context_t {
    alloc_device_t  device;
    /* our private data here */
};

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle);

static int gralloc_reserve_fb(alloc_device_t *p_alloc_dev, int fb_w, int fb_h, int format);

/*****************************************************************************/

int fb_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

static int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device);

int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr);


int gralloc_lock_ycbcr(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        struct android_ycbcr *ycbcr);


int gralloc_unlock(gralloc_module_t const* module,
        buffer_handle_t handle);

int gralloc_flush_cache(gralloc_module_t const* module, buffer_handle_t handle);

extern int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

extern int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle);

extern int gralloc_getphysaddr(struct gralloc_module_t const* module,
		    buffer_handle_t handle, unsigned int *PhyAddr);
/*****************************************************************************/

static struct hw_module_methods_t gralloc_module_methods = {
        open: gralloc_device_open
};

private_module_t::private_module_t()
{
#define INIT_ZERO(obj) (memset(&(obj),0,sizeof((obj))))

	base.common.tag = HARDWARE_MODULE_TAG;
	base.common.version_major = 1;
	base.common.version_minor = 0;
	base.common.id = GRALLOC_HARDWARE_MODULE_ID;
	base.common.name = "Graphics Memory Allocator Module";
	base.common.author = "Intel MCG";
	base.common.methods = &gralloc_module_methods;
	base.common.dso = NULL;
	INIT_ZERO(base.common.reserved);

	base.registerBuffer = gralloc_register_buffer;
	base.unregisterBuffer = gralloc_unregister_buffer;
	base.lock = gralloc_lock;
	base.lock_ycbcr = gralloc_lock_ycbcr;
	base.unlock = gralloc_unlock;
	//base.perform = NULL;
	INIT_ZERO(base.reserved_proc);
	base.reserved_proc[0] = (void*)gralloc_flush_cache;
	framebuffer = NULL;
	//pub.FbDev = 0;
	flags = 0;
	numBuffers = 0;
	bufferMask = 0;
	pthread_mutex_init(&(lock), NULL);
	currentBuffer = NULL;
	INIT_ZERO(frameBufferList);
	ion_client = -1;
	INIT_ZERO(info);
	INIT_ZERO(finfo);
	fb_list = NULL;
	reserve_fb = gralloc_reserve_fb;
	xdpi = 0.0f;
	ydpi = 0.0f;
	fps = 0.0f;
	width = 0;
	height = 0;
	format = 0;
	gpufd = -1;
	bpp = 0;
	line_length = 0;

#undef INIT_ZERO
};

/*
 * HAL_MODULE_INFO_SYM will be initialized using the default constructor
 * implemented above
 */
struct private_module_t HAL_MODULE_INFO_SYM;

/*****************************************************************************/
#ifdef DBG_GRALLOC_ION_HEAP
/* print ion cm-heap content into log
   uses for this the ion_debug_heap_show() via proc fs
   in order to access with out root rights.
   And trace out ion buffer pfns if enabled by property
 */
#include <cutils/properties.h>
#define DBG_GRALLOC_TRC_PFN_PROP "debug.gralloc.trcPfn"
static int dbg_ion_pfn = 0;
char dbg_property[PROPERTY_VALUE_MAX];
/* DBG_GRALLOC trace macros */
/* switch on / off gralloc ion pfn tracing via property */
#define  DBG_GRALLOC_PROP_CHECK(p) \
    { if(property_get(DBG_GRALLOC_TRC_PFN_PROP, p, "0") > 0) {\
          dbg_ion_pfn = strtoul(p, NULL, 10); \
          ALOGD("DBG-ION-PFN: %s",(dbg_ion_pfn)?"on":"off"); \
      } \
      else \
          dbg_ion_pfn = 0; }
/* trace gralloc ion pfn for alloc and free if enabled by property
    IF for CTS trace should be eabled always comment out "if(dbg_ion_pfn)" */
#define DBG_GRALLOC_PFN(f,p,s) \
    { if(dbg_ion_pfn) \
          ALOGD("[%s] pfn:0x%08x sz:0x%08x", f, (p>>12), s); \
    }
#define DBG_CMA_HEAP_PATH "/sys/kernel/debug/ion/heaps/cma-heap"
#define DBG_LINE_LG 100
#define DBG_GRALLOC_ION(s) (gralloc_dbg_trc_cma(s))
static void gralloc_dbg_trc_cma(const char*func)
{
	FILE *fp;
	char line[DBG_LINE_LG];
	const char *fl_path = DBG_CMA_HEAP_PATH;

	fp = fopen(fl_path , "r");
	if(NULL != fp) {
		ALOGE("[%s] dump %s:", func, fl_path);
		while( NULL != fgets (line, DBG_LINE_LG, fp) ) {
			ALOGE("%s",line);
		}
		fclose(fp);
	}
	else
		ALOGW("[%s] cannot open %s!", func, fl_path);
}
#else
#define DBG_GRALLOC_PROP_CHECK(p)
#define DBG_GRALLOC_ION(s)
#define DBG_GRALLOC_PFN(f,p)
#endif

static int gralloc_alloc_framebuffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
	int indx = 0, err = 0, ret = 0;
	private_handle_t *hnd = NULL;
  (void)size;
  (void)usage;

	private_module_t* m = reinterpret_cast<private_module_t*>(
		dev->common.module);
	pthread_mutex_lock(&m->lock);

	if (m->fb_list && m->fb_list->avail_buffers) {
		indx = NUM_FB_BUFFERS - m->fb_list->avail_buffers;
		hnd = m->fb_list->framebuffer[indx];
		private_handle_t *new_hnd = new private_handle_t(hnd->fd, hnd->usage, hnd->flags, hnd->size);
		if ( NULL != new_hnd ) {
			new_hnd->ion_fd = m->ion_client;
			new_hnd->ion_hdl = hnd->ion_hdl;

			new_hnd->base = hnd->base;
			new_hnd->phys = hnd->phys;
			*pHandle = new_hnd;
			m->fb_list->avail_buffers--;
		} else {
			ALOGE( "[%s] FB alloc failed\n", __func__ );
			err = -EINVAL;
		}

	} else {
		err = -EINVAL;
	}

exit:
	pthread_mutex_unlock(&m->lock);
	return err;
}

static int gralloc_alloc_buffer(alloc_device_t* dev,
        size_t size, int usage, buffer_handle_t* pHandle)
{
    int err = 0;
    int fd = -1;

    size = roundUpToPageSize(size);

    fd = ashmem_create_region("gralloc-buffer", size);
    if (fd < 0) {
        ALOGE("couldn't create ashmem (%s)", strerror(-errno));
        err = -errno;
    }

    if (err == 0) {
        private_handle_t* hnd = new private_handle_t(fd, usage, 0, size);
        gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(
                dev->common.module);
		*pHandle = hnd;
		hnd->phys = 0;
		ALOGD("%s(%d bytes @ 0x%08x (offset:0x%x)) usage 0x%x, fd=%d", __FUNCTION__, hnd->size, hnd->base, hnd->offset, usage, hnd->fd);
    }

    ALOGE_IF(err, "gralloc failed err=%s", strerror(-err));

    return err;
}

ion_user_handle_t ion_alloc_buffer(int fd, size_t len, size_t align, unsigned int heap_mask,
        unsigned int flags, int *shared_fd, unsigned int *phys) {
    ion_user_handle_t handle;
    int ret;

    ret = ion_alloc(fd, len, align, heap_mask, flags, &handle);
    if (ret < 0)
        handle = 0;
    else {
        ret = ion_share(fd, handle, shared_fd);
        ion_get_param(fd, (int)handle, phys, &len);
    }
    ALOGI("ion_alloc_buffer size=%d, heap_mask=%x, flags=%x, phys=0x%8x, ret=%d",
            len, heap_mask, flags, *phys, ret);
    return handle;
}

static int gralloc_alloc_hwbuffer(alloc_device_t* dev, size_t size, int usage, buffer_handle_t* pHandle, int flags)
{
	private_module_t* m = reinterpret_cast<private_module_t*>(dev->common.module);
	ion_user_handle_t ion_hnd;
	private_handle_t *hnd = NULL;

	unsigned char *cpu_ptr = 0;
	unsigned int      phys = 0;
	int                 fd = 0;
	int              error = 0;
	int                ret = 0;
	int         cache_flag = 0;
	int ion_heap_mask = XGOLD_ION_HEAP_ID_CMA_MASK;


	if(!dev){
		ALOGE("%s:dev param is null", __FUNCTION__);

		ret = -1;
		goto end;
	}

	if(!dev->common.module){
		ALOGE("%s:dev->common.module param is null", __FUNCTION__);

		ret = -1;
		goto end;
	}

    /* Keeping page size rounding */
    size = roundUpToPageSize(size);

    /* select heap based on usage flags */
    do {
        if (usage & GRALLOC_USAGE_PROTECTED) {
            ion_heap_mask = XGOLD_ION_HEAP_ID_PROTECTED_MASK;
            ALOGV("[%s] VPU buffers routed to protected heap\n", __func__);
        } else if (usage & GRALLOC_USAGE_PRIVATE_2) {
            ion_heap_mask = XGOLD_ION_HEAP_ID_VIDEO_MASK;
            ALOGV("[%s] VPU buffers routed to secured heap\n", __func__);
        }

        if ((usage & GRALLOC_USAGE_HW_MASK) && (usage & GRALLOC_USAGE_PRIVATE_3)) {
            cache_flag = ION_FLAG_CACHED | ION_FLAG_CACHED_NEEDS_SYNC;
            ALOGV("[%s] enable ION_FLAG_CACHED|ION_FLAG_CACHED_NEEDS_SYNC (usage=0x%x)\n", __func__, usage);
        }

        ion_hnd = ion_alloc_buffer(m->ion_client, size, 0, ion_heap_mask, cache_flag,
                &fd, &phys);
        DBG_GRALLOC_PFN(__FUNCTION__, phys, size);
        if(ion_hnd == 0) {
            /* check if a second try on a different heap is needed: */
            /* both CAMERA_MASK and USAGE_PRIVATE2 must be set */
            if((usage & GRALLOC_USAGE_HW_CAMERA_MASK) &&
               (usage & GRALLOC_USAGE_PRIVATE_2))
            {
                usage &= ~GRALLOC_USAGE_PRIVATE_2;
                ALOGW("Camera buffer second try with usage 0x%x",usage);
                ret = 0;
            } else {
                ALOGE("ion_alloc_buffer(size %u) = 0", size);
                DBG_GRALLOC_ION(__FUNCTION__);
                ret = -1;
            }
        }
        /* loop until we get a valid handle or an error */
    } while (ion_hnd == 0 && ret == 0);

    if (ion_hnd == 0)
        goto end;

    if(usage & (GRALLOC_USAGE_HW_MASK|GRALLOC_USAGE_PROTECTED)) {
        cpu_ptr = (unsigned char*)mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );

        if ( MAP_FAILED == cpu_ptr )
        {
            ALOGE( "ion_map( %d ) failed", m->ion_client );
            if ( 0 != ion_free( m->ion_client, ion_hnd ) ) ALOGE( "ion_free( %d ) failed", m->ion_client );
            close(fd);

            ret = -1;
            goto end;
        }

        memset(cpu_ptr, 0, size);
    }

	/* allocate private handle */
	hnd = new private_handle_t(fd, usage, flags, size);

	if ( NULL != hnd ){
		hnd->ion_fd  = m->ion_client;
		hnd->ion_hdl = ion_hnd;
		hnd->phys    = phys;
		*pHandle     = hnd;

		if(usage & GRALLOC_USAGE_HW_MASK)
			hnd->base = intptr_t(cpu_ptr);

		print_private_handle(hnd, "ion_buffer_alloc" );

		ret = 0;
	} else {
		ALOGE( "Gralloc: out of mem for private handle, ion_client:%d", m->ion_client );

		/* clean resources */

		if(usage & GRALLOC_USAGE_HW_MASK && cpu_ptr != NULL) {
			if(fd >= 0) {
				close(fd);
			}

			error = munmap( cpu_ptr, size );
			if (error != 0) {
				ALOGE( "munmap failed for base:%p size: %d", cpu_ptr, size );
			}

			error = ion_free( m->ion_client, ion_hnd );
			if (error != 0) {
				ALOGE( "ion_free( %d ) failed", m->ion_client );
			}
		} else {
			error = ion_free_secure(m->ion_client, size, phys);
		}

		ret = -1;
	}

end:
	ALOGV("%s = %d, size %u, usage 0x%08x, flags %08x", __FUNCTION__,
			ret, size, usage, flags);

	return ret;
}

/*****************************************************************************/

#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))
static int gralloc_alloc(alloc_device_t* dev,
        int w, int h, int format, int usage,
        buffer_handle_t* pHandle, int* pStride)
{
	if (!dev || !pHandle || !pStride)
		return -EINVAL;

	size_t size, pixel_stride, bpr, y_size, c_size, c_stride;
	int bpp = 0;
	int err;
	char* s;

	if (HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED == format) {
		if (GRALLOC_USAGE_HW_VIDEO_ENCODER & usage) {
			if (GRALLOC_USAGE_HW_COMPOSER & usage)
				format = HAL_PIXEL_FORMAT_RGBA_8888; // miracast (RGB)
			else
				format = HAL_PIXEL_FORMAT_YCbCr_420_SP; // video recording
		} else if (GRALLOC_USAGE_HW_CAMERA_WRITE & usage) {
			format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
		} else if (GRALLOC_USAGE_PRIVATE_MASK & usage) {
			/* TODO */
		} else if ((GRALLOC_USAGE_HW_COMPOSER & usage) &&
                            (GRALLOC_USAGE_HW_TEXTURE & usage)) {
                                format = HAL_PIXEL_FORMAT_RGBA_8888;
                }
		ALOGV("[%s], usage 0x%08x, format (0x%x)\n", __func__, usage, format);
	} else if (HAL_PIXEL_FORMAT_YCbCr_420_888 == format) {
		format = HAL_PIXEL_FORMAT_YCrCb_420_SP;
		ALOGV("[%s] HAL_PIXEL_FORMAT_YCbCr_420_888\n", __func__);
	}
	if(( GRALLOC_USAGE_HW_VIDEO_ENCODER & usage) && (format== HAL_PIXEL_FORMAT_RGBA_8888)) {
		usage |= GRALLOC_USAGE_PRIVATE_3;
		ALOGV("[%s] enable ION_FLAG_CACHED for VPU Encoder and RGBA 32bit\n", __func__);
	}
	switch (format) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
		bpp = 4;
		bpr = GRALLOC_ALIGN(w * bpp, 8);
		size = bpr * h;
		pixel_stride = bpr / bpp;
		break;
	/* TODO: review this; bpp=3 is not possible with DCC */
        case HAL_PIXEL_FORMAT_RGB_888:
		bpp = 3;
		bpr = GRALLOC_ALIGN(w * bpp, 8);
		size = bpr * h;
		pixel_stride = bpr / bpp;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
	case HAL_PIXEL_FORMAT_RAW16:
		bpp = 2;
		bpr = GRALLOC_ALIGN(w * bpp, 8);
		size = bpr * h;
		pixel_stride = bpr / bpp;
		break;

	/* YCbCr 4:2:0 Planar : 3 Planes Co-sited*/
	/* This format assumes
	 * - an even width
	 * - an even height
	 * - a horizontal stride multiple of 16 pixels
	 * - a vertical stride equal to the height
	 *
	 *   y_size = stride * height
	 *   c_stride = ALIGN(stride/2, 16)
	 *   c_size = c_stride * height/2
	 *   size = y_size + c_size * 2
	 *   cr_offset = y_size
	 *   cb_offset = y_size + c_size
	 */
	case HAL_PIXEL_FORMAT_YV12:
		if ((w % 16) || (h % 2)) {
			ALOGW("%s - wrong WXH for format (0x%x)!", __FUNCTION__, format);
			//return -EINVAL;
		}
		/* y=1 cb=1 cr=1 bytes */
		bpp = 2; /* this is wrong, but we are not using for yuv. so ok for now */
		bpr = GRALLOC_ALIGN(w, 16);
		y_size = bpr * h;
		c_stride = GRALLOC_ALIGN(bpr/2, 16);
		c_size = c_stride * h/2;
		size = y_size + (c_size * 2);
		pixel_stride = bpr;
		break;

	/* YCbCr 4:2:0 Semi Planar: 2 Planes Co-sited*/
	/* This format assumes
	 * - an even width
	 * - an even height
	 * - a horizontal stride multiple of 16 pixels
	 * - a vertical stride equal to the height
	 */
	/* Support for VPU specific color formats */
	case HAL_PIXEL_FORMAT_YUV_420_SP:
		/* HAL_PIXEL_FORMAT_YUV_420_SP (0x15) requested but VPU does only
		recognize HAL_PIXEL_FORMAT_YCbCr_420_SP (0x12).
		Make sure correct format is stored in the gralloc handle */
		/* FALL THROUGH */
		format = HAL_PIXEL_FORMAT_YCbCr_420_SP;
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
	case HAL_PIXEL_FORMAT_YCbCr_420_SP:
		if ((w % 16) || (h % 2)) {
			ALOGW("%s - wrong WXH for format (0x%x)!", __FUNCTION__, format);
			//return -EINVAL;
		}
		/* y=1 cb=1 cr=1 bytes */
		bpp = 2; /* this is wrong, but we are not using for yuv. so ok for now */
		bpr = GRALLOC_ALIGN(w, 16);
		y_size = bpr * h;
		c_size = bpr * h/2;
		size = y_size + c_size;
		pixel_stride = bpr;
		break;

	case HAL_PIXEL_FORMAT_YCbCr_422_SP:
	case HAL_PIXEL_FORMAT_YCbCr_422_I:
		bpp = 2;
		bpr = GRALLOC_ALIGN(w, 16);
		size = bpr * h * 2;
		pixel_stride = bpr;
		break;

	case HAL_PIXEL_FORMAT_BLOB:
		bpp = 1;
		bpr = w;
		size = h * w;
		pixel_stride = bpr;
		break;

	default:
		ALOGW("%s - unknown format(0x%x) usage(0x%x)", __FUNCTION__, format, usage);
		return -EINVAL;
	}

   if (GRALLOC_USAGE_HW_CAMERA_WRITE & usage) {
      /* take count from private usage flags */
      int count = (usage & GRALLOC_USAGE_PRIVATE_MASK ) >> 28;
      size += count * GRALLOC_EXTRA_ALLOCATION_UNIT_SIZE;
   }

	if (usage & GRALLOC_USAGE_HW_FB) {
		err = gralloc_alloc_framebuffer(dev, size, usage, pHandle);
		s = (char*) "gralloc_alloc_framebuffer";
	} else if ((usage & GRALLOC_USAGE_HW_MASK)) {
		if (usage & GRALLOC_USAGE_HW_CAMERA_MASK) {
			usage |= GRALLOC_USAGE_PRIVATE_2;
		}
		err = gralloc_alloc_hwbuffer(dev, size, usage, pHandle,
			(private_handle_t::PRIV_FLAGS_CONTIGUOUS | private_handle_t::PRIV_FLAGS_USES_ION));
		s = (char*) "gralloc_alloc_hwbuffer";
	} else {
#if __ANDROID_API__ >= 21
        usage |= GRALLOC_USAGE_HW_CAMERA_MASK | GRALLOC_USAGE_PRIVATE_2;

		err = gralloc_alloc_hwbuffer(dev, size, usage, pHandle,
			(private_handle_t::PRIV_FLAGS_CONTIGUOUS | private_handle_t::PRIV_FLAGS_USES_ION));
		s = (char*) "gralloc_alloc_hwbuffer";
#else /* TODO: Find out why it doesn't work with ashmem */
		err = gralloc_alloc_buffer(dev, size, usage, pHandle);
		s = (char*) "gralloc_alloc_buffer";
#endif
	}

  /* in case of error we must not access the private handle */
	if (err < 0) {
		ALOGE("%s failed; %d bytes usage:0x%08x", __FUNCTION__, size, usage);
		return err;
	}

	private_handle_t *hnd = (private_handle_t *)*pHandle;
	int private_usage = usage & (GRALLOC_USAGE_PRIVATE_0 |
	                                  GRALLOC_USAGE_PRIVATE_1);

	switch (private_usage)
	{
		case 0:
			hnd->yuv_info = MALI_YUV_BT601_NARROW;
			break;

		case GRALLOC_USAGE_PRIVATE_1:
			hnd->yuv_info = MALI_YUV_BT601_WIDE;
			break;

		case GRALLOC_USAGE_PRIVATE_0:
			hnd->yuv_info = MALI_YUV_BT709_NARROW;
			break;

		case (GRALLOC_USAGE_PRIVATE_0 | GRALLOC_USAGE_PRIVATE_1):
			hnd->yuv_info = MALI_YUV_BT709_WIDE;
			break;
	}

	hnd->fmt = format;
	hnd->w = w;
	hnd->h = h;
	hnd->internalWidth = w;
	hnd->internalHeight = h;
	hnd->bpp = bpp;
	hnd->byte_stride = bpr;

	print_private_handle(hnd, s);

	*pStride = pixel_stride;

	return 0;
}


extern int getIonFd(gralloc_module_t const *module);

static int gralloc_free(alloc_device_t* dev,
		buffer_handle_t handle)
{
	if (!dev) {
		ALOGE("[%s] dev is NULL!!\n", __func__);
	} else {
		gralloc_module_t* module = reinterpret_cast<gralloc_module_t*>(dev->common.module);

		private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(handle);

        if (hnd != NULL) {
            if (hnd->base)
                grallocUnmap(module, const_cast<private_handle_t*>(hnd));
            close(hnd->fd);
            if (hnd->ion_hdl)
                ion_free(getIonFd(module), hnd->ion_hdl);
            DBG_GRALLOC_PFN(__FUNCTION__, hnd->phys, hnd->size);
            delete hnd;
		}
	}

	return 0;
}


static int gralloc_reserve_fb(alloc_device_t *p_alloc_dev, int fb_w, int fb_h, int format)
{

	size_t bpp = 0, fbsize = 0;
	int i = 0, status = 0;

	if (!p_alloc_dev) {
		ALOGE("[%s] dev is NULL!!\n", __func__);
		return -EINVAL;
	}

	private_module_t* m = reinterpret_cast<private_module_t*>(p_alloc_dev->common.module);
	if (m->fb_list) {
		ALOGE("[%s] FB buffers already reserved!!\n", __func__);
		return -EINVAL;
	}


	m->fb_list = (struct fb_list_t*)malloc(sizeof(struct fb_list_t));
	if (!(m->fb_list)) {
		ALOGE("[%s] malloc failed\n", __func__);
		return -EINVAL;
	}

	switch (format) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
	case HAL_PIXEL_FORMAT_BGRA_8888:
		bpp = 4;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		bpp = 2;
		break;
	default:
		ALOGW("%s - FB format not supported (0x%x)", __func__, format);
		return -EINVAL;
	}

	size_t bpr = GRALLOC_ALIGN(fb_w * bpp, 8);
	fbsize = bpr * fb_h;

	for (i = 0; i < NUM_FB_BUFFERS; i++) {
		status = gralloc_alloc_hwbuffer(p_alloc_dev, fbsize, GRALLOC_USAGE_HW_FB,
					(buffer_handle_t*) &(m->fb_list->framebuffer[i]),
					(/* private_handle_t::PRIV_FLAGS_FRAMEBUFFER | */ /* Do NOT use this flag */
					private_handle_t::PRIV_FLAGS_CONTIGUOUS |
					private_handle_t::PRIV_FLAGS_USES_ION));
		if (status != 0){
			ALOGE("gralloc_alloc_hwbuffer failed %s l.%d", __func__, __LINE__);
			return status;
		}
		m->fb_list->framebuffer[i]->fmt = format;
		m->fb_list->framebuffer[i]->w = fb_w;
		m->fb_list->framebuffer[i]->h = fb_h;
		m->fb_list->framebuffer[i]->internalWidth = fb_w;
		m->fb_list->framebuffer[i]->internalHeight = fb_h;
		m->fb_list->framebuffer[i]->bpp = bpp;
		m->fb_list->framebuffer[i]->byte_stride = bpr;

		print_private_handle(m->fb_list->framebuffer[i], __func__ );
	}

	m->fb_list->avail_buffers = NUM_FB_BUFFERS;

	return 0;
}


/*****************************************************************************/

static int gralloc_close(struct hw_device_t *dev)
{
    gralloc_context_t* ctx = reinterpret_cast<gralloc_context_t*>(dev);
	ALOGI("%s", __FUNCTION__);
    if (ctx) {
        /* TODO: keep a list of all buffer_handle_t created, and free them
         * all here.
         */
        free(ctx);
    }
    return 0;
}

int gralloc_device_open(const hw_module_t* module, const char* name,
        hw_device_t** device)
{
	int status = -EINVAL;
	ALOGI("%s(%s)", __FUNCTION__, name);

	if (!strcmp(name, GRALLOC_HARDWARE_GPU0)) {
		gralloc_context_t *dev;
		dev = (gralloc_context_t*)malloc(sizeof(*dev));
		if (!dev) {
			ALOGE( "gralloc device creation failed\n");
			return status;
		}

		/* initialize our state here */
		memset(dev, 0, sizeof(*dev));

		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module = const_cast<hw_module_t*>(module);
		dev->device.common.close = gralloc_close;

		dev->device.alloc   = gralloc_alloc;
		dev->device.free    = gralloc_free;

		private_module_t *m = reinterpret_cast<private_module_t *>(dev->device.common.module);

		/* Make sure module init is done only once */
		if (m->fb_list)
			ALOGV("[%s] FB buffers already reserved!!\n", __func__);

		if ( m->ion_client < 0 ) {
			m->ion_client = ion_open();
			if ( m->ion_client < 0 )
			{
				ALOGE( "ion device open failed with %s", strerror(errno) );
				free(dev);
				return status;
			}
			ALOGD("%s: ion device open success! fd = %d", __func__, m->ion_client);
		} else
			ALOGD("%s: ion device already opened! fd = %d", __func__, m->ion_client);

		*device = &dev->device.common;
		status = 0;
#if 0
    android::CallStack stack;
    stack.update();
    stack.log(LOG_TAG);
#endif
	} else {

#if 0
        status = fb_device_open(module, name, device);
	//private_module_t* m = reinterpret_cast<private_module_t*>(module);
	private_module_t* m = (private_module_t*)module;
	m->pub.FbDev = reinterpret_cast<struct framebuffer_device_t *>(*device);
#endif
	}
	DBG_GRALLOC_PROP_CHECK(dbg_property);
	return status;
}
