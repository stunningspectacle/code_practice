/*
 * Copyright (C) 2011-2013 Intel Mobile Communications GmbH
 * Copyright (C) 2010 ARM Limited. All rights reserved.
 *
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

#include <sys/mman.h>

#include <dlfcn.h>

#include <cutils/ashmem.h>
#include <cutils/log.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>

#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#if HAVE_ANDROID_OS
#include <linux/fb.h>
#endif
#include <video/xgold-dcc.h>
#include <pixfmt_custom.h>
#include "gralloc_priv.h"
#include "gr.h"

#include <GLES/gl.h>

/*****************************************************************************/

// numbers of buffers for page flipping
#define NUM_BUFFERS NUM_FB_BUFFERS

enum {
    PAGE_FLIP = 0x00000001,
    LOCKED = 0x00000002
};

struct fb_context_t {
    framebuffer_device_t  device;
    struct framebuffer_device_public_api_t pub;
};

/*****************************************************************************/

static int fb_set_swap_interval(struct framebuffer_device_t* dev, int interval)
{
	fb_context_t* ctx = (fb_context_t*)dev;
	ALOGD("%s [%d,%d]interval = %d", __FUNCTION__,dev->minSwapInterval,dev->maxSwapInterval, interval);
	if (interval < dev->minSwapInterval || interval > dev->maxSwapInterval)
		return -EINVAL;

	// Currently not implemented
	return 0;
}

static int fb_post(struct framebuffer_device_t* dev, buffer_handle_t buffer)
{
	int ret;
	if (private_handle_t::validate(buffer) < 0)
		return -EINVAL;

	private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(buffer);
	private_module_t* m = reinterpret_cast<private_module_t*>(dev->common.module);

	if (m->currentBuffer){
		m->base.unlock(&m->base, m->currentBuffer);
		m->currentBuffer = 0;
	}

	print_private_handle(const_cast<private_handle_t*>(hnd), "fb_post" );
	if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {

		m->base.lock(&m->base, buffer, private_module_t::PRIV_USAGE_LOCKED_FOR_POST,
				0, 0, m->info.xres, m->info.yres, NULL);

		m->info.activate = FB_ACTIVATE_VBL;
		m->info.yoffset = hnd->offset / m->finfo.line_length;

		/*Standard Android way*/
#ifdef MALI_VSYNC_EVENT_REPORT_ENABLE
		gralloc_mali_vsync_report(MALI_VSYNC_EVENT_BEGIN_WAIT);
#endif
		if (ioctl(m->fbfd, FBIOPUT_VSCREENINFO, &m->info) == -1) {
			ALOGE("FBIOPUT_VSCREENINFO failed %s %d", __FUNCTION__,__LINE__);
#ifdef MALI_VSYNC_EVENT_REPORT_ENABLE
			gralloc_mali_vsync_report(MALI_VSYNC_EVENT_END_WAIT);
#endif
			m->base.unlock(&m->base, buffer);
			return -errno;
		}
#ifdef MALI_VSYNC_EVENT_REPORT_ENABLE
		gralloc_mali_vsync_report(MALI_VSYNC_EVENT_END_WAIT);
#endif
		m->currentBuffer = buffer;

	} else {
		void* fb_vaddr;
		void* buffer_vaddr;

		m->base.lock(&m->base, m->framebuffer,
				GRALLOC_USAGE_SW_WRITE_RARELY,
				0, 0, m->info.xres, m->info.yres,
				&fb_vaddr);

		m->base.lock(&m->base, buffer,
				GRALLOC_USAGE_SW_READ_RARELY,
				0, 0, m->info.xres, m->info.yres,
				&buffer_vaddr);

		memcpy(fb_vaddr, buffer_vaddr, m->finfo.line_length * m->info.yres);

		m->base.unlock(&m->base, buffer);
		m->base.unlock(&m->base, m->framebuffer);
	}

	return 0;
}

int fb_post_external(hw_module_t* module, buffer_handle_t buffer)
{
	int ret=0;
	if (private_handle_t::validate(buffer) < 0)
		return -EINVAL;

	private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(buffer);
	private_module_t * m = reinterpret_cast<private_module_t *>(module);
	struct framebuffer_device_t* dev = m->pub.FbDev;

	print_private_handle(const_cast<private_handle_t*>(hnd), "fb_post" );
    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER) {
        m->info.activate = FB_ACTIVATE_VBL;
        m->info.yoffset = hnd->offset / m->finfo.line_length;

        if (ioctl(m->fbfd, FBIOPUT_VSCREENINFO, &m->info) == -1) {
            ALOGE("FBIOPUT_VSCREENINFO failed %s %d", __FUNCTION__,__LINE__);
            m->base.unlock(&m->base, buffer);
            return -errno;
        }
        m->currentBuffer = buffer;

    } else {
        // If we can't do the page_flip, just copy the buffer to the front
        // FIXME: use copybit HAL instead of memcpy

        ALOGV("%s not a framebuffer", __FUNCTION__);
        void* fb_vaddr;
        void* buffer_vaddr;

        m->base.lock(&m->base, m->framebuffer,
                GRALLOC_USAGE_SW_WRITE_RARELY,
                0, 0, m->info.xres, m->info.yres,
                &fb_vaddr);

        m->base.lock(&m->base, buffer,
                GRALLOC_USAGE_SW_READ_RARELY,
                0, 0, m->info.xres, m->info.yres,
                &buffer_vaddr);

        memcpy(fb_vaddr, buffer_vaddr, m->finfo.line_length * m->info.yres);

        m->base.unlock(&m->base, buffer);
        m->base.unlock(&m->base, m->framebuffer);
    }

	    return ret;
}


/**
 *  Get Display informations
 */
static int dcc_getdispinfo(struct gra_display_cfg_t  *display_info)
{
	int fd;
	int ret=-EINVAL;

	fd = open(DCC_DEVICE_NAME, 0);
	if (fd < 0) {
		ALOGE("Can't open device file: %s", DCC_DEVICE_NAME);
	}

	ret = ioctl( fd, GRA_IOR_DISPLAY_INFO, display_info );
	if (ret){
		ALOGE("GRA_IOR_DISPLAY_INFO failed !\n");
		return ret;
	}else{
		ALOGI("Display found %dx%d @0x%x HW(0x%x) DRV(%d)\n",
				display_info->width, display_info->height, display_info->base,
				display_info->hwid, display_info->drvid);
		ret = 0;
	}

	return fd;
}

int init_frame_buffer_locked(alloc_device_t* dev, struct private_module_t* module)
{
	int status = 0;
	int DisplayBytes = 0;
	gra_display_cfg_t dispinfo;

	// already initialized...
	if (module->framebuffer) {
		ALOGW("already initialized\n");
		return 0;
	}

	char const * const device_template[] = {
		"/dev/graphics/fb%u",
		"/dev/fb%u",
		0 };

	int fd = -1;
	int i=0;
	char name[64];

	module->gpufd = dcc_getdispinfo(&dispinfo);
	if (module->gpufd < 0){
		ALOGE("failed to open dcc device node");
		return -1;
	}

	module->width = dispinfo.width;
	module->height = dispinfo.height;
	module->format = dispinfo.format;

	while ((fd==-1) && device_template[i]) {
		snprintf(name, 64, device_template[i], 0);
		fd = open(name, O_RDWR, 0);
		i++;
	}
	if (fd < 0){
		ALOGE("failed to open fb device node");
		return -errno;
	}

	struct fb_fix_screeninfo finfo;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1){
		ALOGE("FBIOGET_FSCREENINFO failed %s l.%d", __FUNCTION__, __LINE__);
		return -errno;
	}
	struct fb_var_screeninfo info;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1){
		ALOGE("FBIOGET_VSCREENINFO failed %s l.%d", __FUNCTION__, __LINE__);
		return -errno;
	}
	info.reserved[0] = 0;
	info.reserved[1] = 0;
	info.reserved[2] = 0;
	info.xoffset = 0;
	info.yoffset = 0;
	info.activate = FB_ACTIVATE_NOW;

	/*
	 * Request NUM_BUFFERS screens (at lest 2 for page flipping)
	 */
	info.yres_virtual = info.yres * NUM_BUFFERS;


	uint32_t flags = PAGE_FLIP;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &info) == -1) {
		info.yres_virtual = info.yres;
		flags &= ~PAGE_FLIP;
		ALOGW("FBIOPUT_VSCREENINFO failed, page flipping not supported");
	}

	if (info.yres_virtual < info.yres * 2) {
		// we need at least 2 for page-flipping
		info.yres_virtual = info.yres;
		flags &= ~PAGE_FLIP;
		ALOGW("page flipping not supported (yres_virtual=%d, requested=%d)",
				info.yres_virtual, info.yres*2);
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &info) == -1){
		ALOGE("FBIOGET_VSCREENINFO failed %s l.%d", __FUNCTION__, __LINE__);
		return -errno;
	}

	int refreshRate = 0;
	if ( info.pixclock > 0 )
	{
		refreshRate = 1000000000000000LLU /
		(
			uint64_t( info.upper_margin + info.lower_margin + info.yres + info.hsync_len )
			* ( info.left_margin  + info.right_margin + info.xres + info.vsync_len )
			* info.pixclock
		);
	}else{
		ALOGW( "fbdev pixclock is zero for fd: %d", fd );
	}

	if (refreshRate == 0) {
		// bleagh, bad info from the driver
		refreshRate = 60*1000;  // 60 Hz
	}

	if (int(info.width) <= 0 || int(info.height) <= 0) {
		// the driver doesn't return that information
		// default to 160 dpi
		info.width  = ((info.xres * 25.4f)/160.0f + 0.5f);
		info.height = ((info.yres * 25.4f)/160.0f + 0.5f);
	}

	float xdpi = (info.xres * 25.4f) / info.width;
	float ydpi = (info.yres * 25.4f) / info.height;
	float fps  = refreshRate / 1000.0f;

	ALOGI(  "using (fd=%d)\n"
		"id           = %s\n"
		"xres         = %d px\n"
		"yres         = %d px\n"
		"xres_virtual = %d px\n"
		"yres_virtual = %d px\n"
		"bpp          = %d\n"
		"r            = %2u:%u\n"
		"g            = %2u:%u\n"
		"b            = %2u:%u\n",
		fd,
		finfo.id,
		info.xres,
		info.yres,
		info.xres_virtual,
		info.yres_virtual,
		info.bits_per_pixel,
		info.red.offset, info.red.length,
		info.green.offset, info.green.length,
		info.blue.offset, info.blue.length
	     );

	ALOGI(  "width        = %d mm (%f dpi)\n"
		"height       = %d mm (%f dpi)\n"
		"refresh rate = %.2f Hz\n",
		info.width,  xdpi,
		info.height, ydpi,
		fps
	     );

	if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		ALOGE("FBIOGET_FSCREENINFO failed");
		return -errno;
	}

	if (finfo.smem_len <= 0){
		ALOGW("smem_len error");
		return -errno;
	}

	module->flags = flags;
	module->info = info;
	module->finfo = finfo;
	module->xdpi = xdpi;
	module->ydpi = ydpi;
	module->fps = fps;

	/*
	 * map the framebuffer
	 */
	int err;
	size_t fbSize = roundUpToPageSize(finfo.line_length * info.yres_virtual);

	module->numBuffers = info.yres_virtual / info.yres;
	module->bufferMask = 0;
	status = gralloc_alloc_hwbuffer(dev, fbSize, GRALLOC_USAGE_HW_FB,
			(buffer_handle_t*) &module->framebuffer, private_handle_t::PRIV_FLAGS_FRAMEBUFFER);
	if (status != 0){
		ALOGE("gralloc_alloc_hwbuffer failed %s l.%d", __FUNCTION__, __LINE__);
		return status;
	}

	ALOGI("Frame Buffer mmapped %d bytes @ 0x%x (from 0x%x)",
			module->framebuffer->size,
			module->framebuffer->base,
			module->framebuffer->phys);
	module->fbfd = fd;

	return 0;
}

static int init_frame_buffer(alloc_device_t* dev, struct private_module_t* module)
{
    pthread_mutex_lock(&module->lock);
    int err = init_frame_buffer_locked(dev, module);
    pthread_mutex_unlock(&module->lock);
    return err;
}

/*****************************************************************************/

static int fb_close(struct hw_device_t *dev)
{
    fb_context_t* ctx = (fb_context_t*)dev;
	ALOGI("%s", __FUNCTION__);
    if (ctx) {
        free(ctx);
    }
    return 0;
}

static buffer_handle_t nextFb(private_module_t* m)
{
	int numBuffersUsed = 0;
	// get active buffers number
	for (uint32_t i=0 ; i<m->numBuffers ; i++) {
		if (m->bufferMask & (1LU<<i)) {
			numBuffersUsed++;
		}
	}

	for (int i=0 ; i<numBuffersUsed ; i++) {
		if(m->currentBuffer == m->frameBufferList[i]){
			int inext = i+1;
			if(inext == numBuffersUsed)
				inext=0;
			//ALOGV("%s current fb:%d next:%d max:%d used:%d mask 0x%x", __FUNCTION__,
			//		i, inext,
			//		m->numBuffers, numBuffersUsed, m->bufferMask);
			return m->frameBufferList[inext];
		}

	}
	ALOGE("%s next Fb not found !!!", __FUNCTION__);
	return 0;
}

static int getbackfb(struct framebuffer_device_t const* dev, buffer_handle_t *buffer)
{
	private_module_t* m = reinterpret_cast<private_module_t*>(
			dev->common.module);

	*buffer = nextFb(m);
	return 0;
}

int compositionComplete(struct framebuffer_device_t* dev)
{
	/* By doing a finish here we force the GL driver to start rendering
	   all the drawcalls up to this point, and to wait for the rendering to be complete.*/
	glFinish();
	/* The rendering of the backbuffer is now completed.
	   When SurfaceFlinger later does a call to eglSwapBuffer(), the swap will be done
	   synchronously in the same thread, and not asynchronoulsy in a background thread later.
	   The SurfaceFlinger requires this behaviour since it releases the lock on all the
	   SourceBuffers (Layers) after the compositionComplete() function returns.
	   However this "bad" behaviour by SurfaceFlinger should not affect performance,
	   since the Applications that render the SourceBuffers (Layers) still get the
	   full renderpipeline using asynchronous rendering. So they perform at maximum speed,
	   and because of their complexity compared to the Surface flinger jobs, the Surface flinger
	   is normally faster even if it does everyhing synchronous and serial.
	   */
	return 0;
}

int fb_device_open(hw_module_t const* module, const char* name,
		hw_device_t** device)
{
	int status = -EINVAL;
	ALOGI("%s(%s)", __FUNCTION__, name);
	if (!strcmp(name, GRALLOC_HARDWARE_FB0)) {
		alloc_device_t* gralloc_device;
		status = gralloc_open(module, &gralloc_device);
		if (status < 0)
			return status;

		/* initialize our state here */
		fb_context_t *dev = (fb_context_t*)malloc(sizeof(*dev));
		memset(dev, 0, sizeof(*dev));

		/* initialize the procs */
		dev->device.common.tag = HARDWARE_DEVICE_TAG;
		dev->device.common.version = 0;
		dev->device.common.module = const_cast<hw_module_t*>(module);
		dev->device.common.close = fb_close;
		dev->device.setSwapInterval = fb_set_swap_interval;
		dev->device.post = fb_post;
		dev->device.setUpdateRect = 0;
		dev->device.compositionComplete = &compositionComplete;
		dev->pub.GetBackFb = getbackfb;
		dev->pub.PostFb = fb_post_external;

		private_module_t* m = (private_module_t*)module;
		status = init_frame_buffer(gralloc_device, m);
		if (status < 0){
			ALOGE("%s(l.%d) mapFrameBuffer failed", __FUNCTION__, __LINE__);
			gralloc_close(gralloc_device);
			return status;
		}

		int stride = m->finfo.line_length / (m->info.bits_per_pixel >> 3);
		int format = (m->info.bits_per_pixel == 32)
			? HAL_PIXEL_FORMAT_RGBX_8888
			: HAL_PIXEL_FORMAT_RGB_565;
		const_cast<uint32_t&>(dev->device.flags) = 0;
		const_cast<uint32_t&>(dev->device.width) = m->info.xres;
		const_cast<uint32_t&>(dev->device.height) = m->info.yres;
		const_cast<int&>(dev->device.stride) = stride;
		const_cast<int&>(dev->device.format) = format;
		const_cast<float&>(dev->device.xdpi) = m->xdpi;
		const_cast<float&>(dev->device.ydpi) = m->ydpi;
		const_cast<float&>(dev->device.fps) = m->fps;
		const_cast<int&>(dev->device.minSwapInterval) = 1;
		const_cast<int&>(dev->device.maxSwapInterval) = 1;
		*device = &dev->device.common;
	}else{
		ALOGE("%s(%s) unknown !!", __FUNCTION__, name);
	}
	return status;
}
