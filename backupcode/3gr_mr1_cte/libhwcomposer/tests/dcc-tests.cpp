/*
 ************************************************************************
 * Copyright (C) 2013 Intel Mobile Communications GmbH
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
 ************************************************************************
 */


#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <linux/time.h>
#include <cutils/log.h>
#include <dcc-hal.h>
#include <ion-hal.h>
#include <unistd.h>


#include <linux/fb.h>
#include <linux/kd.h>


#define PIXEL_FORMAT_BGRA_8888 0x01
#define PIXEL_FORMAT_RGBX_8888 0x02
#define PIXEL_FORMAT PIXEL_FORMAT_BGRA_8888
#define PIXEL_SIZE   4
/*
#define PIXEL_FORMAT PIXEL_FORMAT_RGBX_8888
#define PIXEL_SIZE   4
#define PIXEL_FORMAT PIXEL_FORMAT_RGB_565
#define PIXEL_SIZE   2
*/


//Rainbow pattern
#define PATTERN_9_COLOR_1   0xFFFF0000
#define PATTERN_9_COLOR_2   0xFFFF7F00
#define PATTERN_9_COLOR_3   0xFFFFFF00
#define PATTERN_9_COLOR_4   0xFF00FF00
#define PATTERN_9_COLOR_5   0xFF0000FF
#define PATTERN_9_COLOR_6   0xFF4B0082
#define PATTERN_9_COLOR_7   0xFF8F00FF


struct ion_buf_handle_t {
	int alloc_fd;
	struct ion_handle *ion_hnd;
	void *buf_ptr;
	unsigned int phys;
};

struct fb_buf_handle_t {
	int fb_fd;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
	void *buf_ptr;
};

struct buf_handle_t {
	struct ion_buf_handle_t ion_buf;
	struct fb_buf_handle_t fb_buf;
};


static int alloc_fd = -1;
static int disp_fd = -1;
int open_alloc_device()
{
	alloc_fd = ion_open();
	if (alloc_fd < 0)
		return -1;

	return 0;
}

int open_display_device()
{
	disp_fd = open("/dev/dcc", 0);
	if(disp_fd < 0)
		return -1;

	return 0;
}


inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}


int alloc_hw_buf(struct ion_buf_handle_t *buf_hand, int width, int height, int bpp)
{
	int ret = -1;
	struct ion_handle *ion_hnd = NULL;
	unsigned char * buf_ptr = NULL;
	unsigned int phys = 0x00;
	int shared_fd;
	int align = 4;

	int bpr = ((width * bpp) + (align -1)) & ~(align-1);
	int size = bpr * height;
	
	size = roundUpToPageSize(size);
	
	ret = ion_alloc(alloc_fd, size, 0, ION_HEAP_CARVEOUT_MASK, 0, &ion_hnd);
	if ( ret != 0)
	{
		printf("Failed to ion_alloc from alloc_fd:%d size:0x%x", alloc_fd, size);
		return -1;
	}

	ret = ion_share(alloc_fd, ion_hnd, &shared_fd );
	if ( ret != 0 )
	{
		printf( "ion_share( %d ) failed", alloc_fd );
		if ( 0 != ion_free( alloc_fd, ion_hnd ) ) printf( "ion_free( %d ) failed", alloc_fd );
			return -1;
	}
	buf_ptr = (unsigned char*)mmap( NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shared_fd, 0 );
	if ( MAP_FAILED == buf_ptr )
	{
		printf( "ion_map( %d ) failed", alloc_fd );
		if ( 0 != ion_free( alloc_fd, ion_hnd ) ) printf( "ion_free( %d ) failed", alloc_fd );
		close( shared_fd );
		return -1;
	}

	if(ion_get_param(alloc_fd, ion_hnd, &phys, (size_t*)&size)){
		printf("%s ion_get_param failed: %s\n",__FUNCTION__, strerror(errno));
		if ( 0 != ion_free( alloc_fd, ion_hnd ) ) printf( "ion_free( %d ) failed", alloc_fd );
		close( shared_fd );
		return -1;
	}

	buf_hand->alloc_fd = alloc_fd;
	buf_hand->ion_hnd = ion_hnd;
	buf_hand->buf_ptr = buf_ptr;
	buf_hand->phys = phys;
	
	return 0;
}

int fill_pattern(int type, unsigned int *p_buf, int width, int height)
{

	int i, j;

	switch (type) {
	
	case 0:
		for(i = 0; i < width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_1;

		for(i = width/7; i < 2*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_2;

		for(i = 2*width/7; i < 3*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_3;

		for(i = 3*width/7; i < 4*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_4;

		for(i = 4*width/7; i < 5*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_5;

		for(i = 5*width/7; i < 6*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_6;

		for(i = 6*width/7; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_7;

		break;

	case 1:
		for(i = 0; i < width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_6;

		for(i = width/7; i < 2*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_7;

		for(i = 2*width/7; i < 3*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_1;

		for(i = 3*width/7; i < 4*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_2;

		for(i = 4*width/7; i < 5*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_3;

		for(i = 5*width/7; i < 6*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_4;

		for(i = 6*width/7; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_5;

		break;

	case 2:
		for(i = 0; i < width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_4;

		for(i = width/7; i < 2*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_5;

		for(i = 2*width/7; i < 3*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_6;

		for(i = 3*width/7; i < 4*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_7;

		for(i = 4*width/7; i < 5*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_1;

		for(i = 5*width/7; i < 6*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_2;

		for(i = 6*width/7; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_3;

		break;

	case 3:
		for(i = 0; i < width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_2;

		for(i = width/7; i < 2*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_3;

		for(i = 2*width/7; i < 3*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_4;

		for(i = 3*width/7; i < 4*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_5;

		for(i = 4*width/7; i < 5*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_6;

		for(i = 5*width/7; i < 6*width/7; i++)
			p_buf[i] = PATTERN_9_COLOR_7;

		for(i = 6*width/7; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_1;

		break;

	case 4:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_1;
		break;

	case 5:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_2;
		break;

	case 6:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_3;
		break;

	case 7:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_4;
		break;

	case 8:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_5;
		break;

	case 9:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_6;
		break;

	case 10:
		for(i = 0; i < width; i++)
			p_buf[i] = PATTERN_9_COLOR_7;
		break;
	}
  
	for(j = 1; j < height; j++)
	{
		//hard coded for 4 bpp
		memcpy(&p_buf[width*j], &p_buf[0], (width << 2));
	}
	
	return 0;
	
}


static struct fb_var_screeninfo vi;
static struct fb_fix_screeninfo fi;
static int get_dcc_framebuffer(struct buf_handle_t *p_buf_hnd)
{
	int fd;
	void *bits_0;

	fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd < 0) {
		ALOGE("cannot open fb0");
		return -1;
	}

	if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
		ALOGE("failed to get fb0 info");
		close(fd);
		return -1;
	}

	/* Dump the VSCREENINFO as read */
	ALOGE("DCC_FB VSCREENINFO as read::	\
		var.xres = %d,	\
		var.yres = %d,	\
		var.xres_virtual = %d,	\
		var.yres_virtual = %d,	\
		var.bits_per_pixel = %d,	\
		var.height = %d,	\
		var.width = %d,	\
		var.red.offset = %d,	\
		var.red.length = %d,	\
		var.green.offset = %d,	\
		var.green.length = %d,	\
		var.blue.offset = %d,	\
		var.blue.length = %d\n", 
		vi.xres, vi.yres, vi.xres_virtual, vi.yres_virtual,
		vi.bits_per_pixel, vi.height, vi.width, vi.red.offset,
		vi.red.length, vi.green.offset, vi.green.length,
		vi.blue.offset, vi.blue.length);


	vi.bits_per_pixel = PIXEL_SIZE * 8;
	if (PIXEL_FORMAT == PIXEL_FORMAT_BGRA_8888) {
		vi.red.offset     = 8;
		vi.red.length     = 8;
		vi.green.offset   = 16;
		vi.green.length   = 8;
		vi.blue.offset    = 24;
		vi.blue.length    = 8;
		vi.transp.offset  = 0;
		vi.transp.length  = 8;
	} else if (PIXEL_FORMAT == PIXEL_FORMAT_RGBX_8888) {
		vi.red.offset     = 24;
		vi.red.length     = 8;
		vi.green.offset   = 16;
		vi.green.length   = 8;
		vi.blue.offset    = 8;
		vi.blue.length    = 8;
		vi.transp.offset  = 0;
		vi.transp.length  = 8;
	} else { /* RGB565*/
		vi.red.offset     = 11;
		vi.red.length     = 5;
		vi.green.offset   = 5;
		vi.green.length   = 6;
		vi.blue.offset    = 0;
		vi.blue.length    = 5;
		vi.transp.offset  = 0;
		vi.transp.length  = 0;
	}

	/* Dump the VSCREENINFO as write */
	ALOGE("DCC_FB VSCREENINFO as write::	\
		var.xres = %d,	\
		var.yres = %d,	\
		var.xres_virtual = %d,	\
		var.yres_virtual = %d,	\
		var.bits_per_pixel = %d,	\
		var.height = %d,	\
		var.width = %d,	\
		var.red.offset = %d,	\
		var.red.length = %d,	\
		var.green.offset = %d,	\
		var.green.length = %d,	\
		var.blue.offset = %d,	\
		var.blue.length = %d\n", 
		vi.xres, vi.yres, vi.xres_virtual, vi.yres_virtual,
		vi.bits_per_pixel, vi.height, vi.width, vi.red.offset,
		vi.red.length, vi.green.offset, vi.green.length,
		vi.blue.offset, vi.blue.length);

	if (ioctl(fd, FBIOPUT_VSCREENINFO, &vi) < 0) {
		ALOGE("failed to put fb0 info");
		close(fd);
		return -1;
	}

	if (ioctl(fd, FBIOGET_FSCREENINFO, &fi) < 0) {
		ALOGE("failed to get fb0 info");
		close(fd);
		return -1;
	}
	
	/* Dump the FSCREENINFO as read */
	ALOGE("DCC_FB VSCREENINFO as read:	\
		fix.smem_start = 0x%08lx,	\
		fix.smem_len = 0x%08x,	\
		fix.line_length = %d\n",
		fi.smem_start,
		fi.smem_len,
		fi.line_length);

	bits_0 = mmap(0, fi.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (bits_0 == MAP_FAILED) {
		ALOGE("failed to mmap framebuffer");
		close(fd);
		return -1;
	}

	p_buf_hnd->fb_buf.fb_fd = fd;
	p_buf_hnd->fb_buf.vi = vi;
	p_buf_hnd->fb_buf.fi = fi;
	p_buf_hnd->fb_buf.buf_ptr = bits_0;


	return 0;
}

static int pan_display(struct fb_buf_handle_t *p_fb_buf, int n)
{

	p_fb_buf->vi.yoffset = n * p_fb_buf->vi.yres;
	p_fb_buf->vi.bits_per_pixel = PIXEL_SIZE * 8;
	if (ioctl(p_fb_buf->fb_fd, FBIOPUT_VSCREENINFO, &p_fb_buf->vi) < 0) {
		ALOGE("active fb set failed\n");
		return -1;
	}

	return 0;
}



static int test_dcc_framebuffer_apis()
{

	struct buf_handle_t buf_hnd;
	struct fb_buf_handle_t *p_fb_buf = NULL;
	void *buf_ptr[4];
	int status = -1, i = 0;
#if 0
	const char *cmds[4] = {
		"cat /dev/graphics/fb0 > /storage/sdcard1/dcc_test_fb_1",
		"cat /dev/graphics/fb0 > /storage/sdcard1/dcc_test_fb_2",
		"cat /dev/graphics/fb0 > /storage/sdcard1/dcc_test_fb_3",
		"cat /dev/graphics/fb0 > /storage/sdcard1/dcc_test_fb_4",};
#endif		

	/* Write and read */
	status = get_dcc_framebuffer(&buf_hnd);
	if (status < 0) {
		ALOGE("DCC framebuffer failed\n");
		return status;
	}


	p_fb_buf = &buf_hnd.fb_buf;
	for (i = 0; i < 4; i++) {
		buf_ptr[i] = (void *)(((unsigned int)p_fb_buf->buf_ptr) + (i * p_fb_buf->vi.yres * p_fb_buf->fi.line_length));
		fill_pattern(i, (unsigned int *)buf_ptr[i], p_fb_buf->vi.xres, p_fb_buf->vi.yres);
	}


	for(i = 0; i < 4; i++) {
		pan_display(p_fb_buf, i);
#if 0
		/* Read the buffer */
		system(cmds[i]);
#endif
	}

	return 0;
}

static int test_dcc_m2d_update(long long itr)
{
	int i = 0;
	struct buf_handle_t buf_hand[4];
	struct timespec mytime;
	uint64_t begin = 0, end = 0;
#if 0
	int trace_fd = -1, marker_fd = -1;
#endif
	dcc_hal_layer_list_t test_layer_list;
	dcc_hal_layer_t test_layers[5];

	memset((void*)&test_layer_list, 0x00, sizeof(dcc_hal_layer_list_t));
	memset((void*)&test_layers[0], 0x00, 5*sizeof(dcc_hal_layer_t));

	if(-1 == open_alloc_device())
	{
		ALOGE("Error\n");
		return -1;
	}
	
	if (0 > dcc_hal_init()) {
		ALOGE("Error DCC HAL failed\n");
		return -1;
	}
	/* TODO:
		1. Remove hard codings
		2. Add test cases for ovs (Now only BG tests)
	*/

	for(i = 0; i < 4; i ++)
	{
		if(-1 == alloc_hw_buf(&buf_hand[i].ion_buf, 480, 854, 4)) {
			ALOGE("Error in allocation\n");
			return -1;
		}
		
		fill_pattern(i, (unsigned int*)buf_hand[i].ion_buf.buf_ptr, 480, 854);

		test_layers[i].buff.phys_addr = buf_hand[i].ion_buf.phys;
		test_layers[i].buff.width = 480;
		test_layers[i].buff.height = 854;
		test_layers[i].buff.format = (DCC_COLOR_RGB | DCC_COLOR_ARGB8888);
		test_layers[i].src_rect.x = 0;
		test_layers[i].src_rect.y = 0;
		test_layers[i].src_rect.w = 480;
		test_layers[i].src_rect.h = 854;
		test_layers[i].dst_rect.x = 0;
		test_layers[i].dst_rect.y = 0;
		test_layers[i].dst_rect.w = 480;
		test_layers[i].dst_rect.h = 854;
		test_layers[i].rot_ops.ops = DCC_NO_TRANSFORM; 
		test_layers[i].draw_ops.ops = DCC_DRAW_OPS_NONE;
		test_layers[i].compose_ops.ops = DCC_NO_COMPOSITION;
	}
	
	test_layer_list.num_layers = 1;
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	
#if 0
	marker_fd = open("/sys/kernel/debug/tracing/trace_marker", O_WRONLY);
	if (marker_fd < 0) {
		ALOGE("Error\n");
		return -1;
	}
	trace_fd = open("/sys/kernel/debug/tracing/tracing_on", O_WRONLY);
	if (trace_fd >=0) {
		write(trace_fd, "1", 1);
	} else {
		ALOGE("Error\n");
		return -1;
	}
	write(marker_fd, "In critical area\n", 17);
#endif

	if (itr <= 0)
		itr = 1000;
	i = 0;
	while(itr >= 0) {
		test_layer_list.dcc_layer[0] = &test_layers[i];
		//ioctl( disp_fd, DCC_IOW_SET_FRAMEBUFFER, &r[i] );
		clock_gettime(CLOCK_REALTIME, &mytime);
		begin = mytime.tv_nsec;
		//ioctl( disp_fd, DCC_IOW_UPDATE, &r[i] );
		dcc_hal_bind_layers(&test_layer_list);
		/* TODO:
			check binding status
		 */
		dcc_hal_process_layers(&test_layer_list);
		clock_gettime(CLOCK_REALTIME, &mytime);
		end = mytime.tv_nsec;
		ALOGE("[%s] delay in update ---> %lld \n", __func__, (end - begin));
		i++;
		if(i >= 4)
			i = 0;
		itr--;
	}
#if 0
	write(marker_fd, "Out critical area\n", 18);
	if (trace_fd >= 0)
		write(trace_fd, "0", 1);

	sleep(1000);
#endif

	return 0;

}

static int test_dcc_m2d_update_with_vsync(long long itr)
{
	int i = 0;
	struct buf_handle_t buf_hand[4];
	struct timespec mytime;
	uint64_t begin = 0, end = 0;
	int vsync_fd = -1, err = -1;
	char buf[1024] = {0};
	uint64_t ts = 0;
	struct pollfd fds[1];
#if 0
	int trace_fd = -1, marker_fd = -1;
#endif
	dcc_hal_layer_list_t test_layer_list;
	dcc_hal_layer_t test_layers[5];
	struct dcc_hal_attr_t attr;

	memset((void*)&test_layer_list, 0x00, sizeof(dcc_hal_layer_list_t));
	memset((void*)&test_layers[0], 0x00, 5*sizeof(dcc_hal_layer_t));

	if(-1 == open_alloc_device())
	{
		ALOGE("Error\n");
		return -1;
	}
	
	if (0 > dcc_hal_init()) {
		ALOGE("Error DCC HAL failed\n");
		return -1;
	}
	/* TODO:
		1. Remove hard codings
		2. Add test cases for ovs (Now only BG tests)
	*/

	for(i = 0; i < 4; i ++)
	{
		if(-1 == alloc_hw_buf(&buf_hand[i].ion_buf, 480, 854, 4)) {
			ALOGE("Error in allocation\n");
			return -1;
		}
		
		fill_pattern(i, (unsigned int*)buf_hand[i].ion_buf.buf_ptr, 480, 854);

		test_layers[i].buff.phys_addr = buf_hand[i].ion_buf.phys;
		test_layers[i].buff.width = 480;
		test_layers[i].buff.height = 854;
		test_layers[i].buff.format = (DCC_COLOR_RGB | DCC_COLOR_ARGB8888);
		test_layers[i].src_rect.x = 0;
		test_layers[i].src_rect.y = 0;
		test_layers[i].src_rect.w = 480;
		test_layers[i].src_rect.h = 854;
		test_layers[i].dst_rect.x = 0;
		test_layers[i].dst_rect.y = 0;
		test_layers[i].dst_rect.w = 480;
		test_layers[i].dst_rect.h = 854;
		test_layers[i].rot_ops.ops = DCC_NO_TRANSFORM; 
		test_layers[i].draw_ops.ops = DCC_DRAW_OPS_NONE;
		test_layers[i].compose_ops.ops = DCC_NO_COMPOSITION;
	}
	
	test_layer_list.num_layers = 1;
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;

	vsync_fd = open("/sys/devices/system/dcc/vsyncts0", O_RDONLY);
	if (vsync_fd < 0) {
		ALOGE("failed to open vsync attr\n");
		return -1;
	}

	/* Clear existing */
	err = read(vsync_fd, buf, sizeof(buf));
	if (err < 0) {
		ALOGE("error in reading vsync timestamp: %s\n", strerror(errno));
		return -1;
	}

	fds[0].fd = vsync_fd;
	fds[0].events = POLLPRI;

	if (itr <= 0)
		itr = 1000;
	attr.vsync = VSYNC_ON;
	i = 0;
	ALOGE("[%s] in poll..\n", __func__);
	while (itr > 0) {
		dcc_hal_set_attr(DCC_HAL_ATTR_VSYNC, &attr);
		clock_gettime(CLOCK_REALTIME, &mytime);
		begin = mytime.tv_nsec;
		err = poll(fds, 1, -1);
		if (err > 0) {
			if (fds[0].revents & POLLPRI) {
				err = lseek(vsync_fd, 0, SEEK_SET);
				if (err < 0) {
					ALOGE("error in lseek: %s\n", strerror(errno));
					return -1;
				}
				err = read(vsync_fd, buf, sizeof(buf));
				if (err < 0) {
					ALOGE("error in reading vsync timestamp: %s\n", strerror(errno));
					return -1;
				}

				buf[sizeof(buf) - 1] = '\0';
				errno = 0;
				ts = strtoull(buf, NULL, 0);
				if (!errno) {
					ALOGE("[%s] ts: %llu\n", __func__, ts);
					test_layer_list.dcc_layer[0] = &test_layers[i];
					dcc_hal_bind_layers(&test_layer_list);
					dcc_hal_process_layers(&test_layer_list);
					clock_gettime(CLOCK_REALTIME, &mytime);
					end = mytime.tv_nsec;
					ALOGE("[%s] delay in (vsync + update) ---> %lld \n", __func__, (end - begin));
					i++;
					if(i >= 4)
						i = 0;
				} else
					ALOGE("[%s] strtoull returns error\n", __func__);
			}
		} else if (err == -1) {
			if (errno == EINTR)
				break;
			ALOGE("error in vsync polling: %s\n", strerror(errno));
		}
		itr--;
	};
	
	attr.vsync = VSYNC_OFF;
	dcc_hal_set_attr(DCC_HAL_ATTR_VSYNC, &attr);

	return 0;
}

#if 0
static int dcc_vsync_ts_read()
{
	int vsync_fd = -1, err = -1;
	char buf[1024] = {0};
	uint64_t ts = 0;
	struct pollfd fds[1];

	vsync_fd = open("/sys/devices/system/dcc/vsyncts0", O_RDONLY);
	if (vsync_fd < 0) {
		ALOGE("failed to open vsync attr\n");
		return -1;
	}

	/* Clear existing */
	err = read(vsync_fd, buf, sizeof(buf));
	if (err < 0) {
		ALOGE("error in reading vsync timestamp: %s\n", strerror(errno));
		return -1;
	}

	fds[0].fd = vsync_fd;
	fds[0].events = POLLPRI;
	
	while (true) {
		ALOGE("[%s] in poll..\n", __func__);
		err = poll(fds, 1, -1);
		if (err > 0) {
			if (fds[0].revents & POLLPRI) {
				err = lseek(vsync_fd, 0, SEEK_SET);
				if (err < 0) {
					ALOGE("error in lseek: %s\n", strerror(errno));
					return -1;
				}
				err = read(vsync_fd, buf, sizeof(buf));
				if (err < 0) {
					ALOGE("error in reading vsync timestamp: %s\n", strerror(errno));
					return -1;
				}

				buf[sizeof(buf) - 1] = '\0';
				errno = 0;
				ts = strtoull(buf, NULL, 0);
				if (!errno)
					ALOGE("[%s] ts: %llu\n", __func__, ts);
				else
					ALOGE("[%s] strtoull returns error\n", __func__);
			}
		} else if (err == -1) {
			if (errno == EINTR)
				break;
			ALOGE("error in vsync polling: %s\n", strerror(errno));
		}
	};

	close (vsync_fd);
	return 0;
}
#endif

static int test_dcc_m2d_overlays()
{
	int i = 0;
	struct buf_handle_t buf_hand[4];
	struct timespec mytime;
	uint64_t begin = 0, end = 0;

	dcc_hal_layer_list_t test_layer_list;
	dcc_hal_layer_t test_layers[5];

	memset((void*)&test_layer_list, 0x00, sizeof(dcc_hal_layer_list_t));
	memset((void*)&test_layers[0], 0x00, 5*sizeof(dcc_hal_layer_t));

	if(-1 == open_alloc_device())
	{
		ALOGE("Error\n");
		return -1;
	}
	if (0 > dcc_hal_init()) {
		ALOGE("Error DCC HAL failed\n");
		return -1;
	}
	/* TODO:
		1. Remove hard codings
		2. Add test cases for ovs (Now only BG tests)
	*/

	for(i = 0; i < 4; i ++)
	{
		if(-1 == alloc_hw_buf(&buf_hand[i].ion_buf, 480, 854, 4)) {
			ALOGE("Error in allocation\n");
			return -1;
		}

		fill_pattern((i + 4), (unsigned int*)buf_hand[i].ion_buf.buf_ptr, 480, 854);

		test_layers[i].buff.phys_addr = buf_hand[i].ion_buf.phys;
		test_layers[i].buff.width = 480;
		test_layers[i].buff.height = 854;
		test_layers[i].buff.format = (DCC_COLOR_RGB | DCC_COLOR_ARGB8888);
		test_layers[i].src_rect.x = 0;
		test_layers[i].src_rect.y = 0;
		test_layers[i].src_rect.w = 480;
		test_layers[i].src_rect.h = 854;
		test_layers[i].dst_rect.x = 0;
		test_layers[i].dst_rect.y = 0;
		test_layers[i].dst_rect.w = 480;
		test_layers[i].dst_rect.h = 854;
		test_layers[i].rot_ops.ops = DCC_NO_TRANSFORM;
		test_layers[i].draw_ops.ops = DCC_DRAW_OPS_NONE;
		test_layers[i].compose_ops.ops = DCC_NO_COMPOSITION;
	}


	/* Test case 1:
		BG: on
		ovs: off
	*/
	test_layer_list.num_layers = 1;
	test_layer_list.dcc_layer[0] = &test_layers[0];
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	test_layer_list.dcc_layer[0]->dcc_bind = DCC_BIND_NONE;
	dcc_hal_bind_layers(&test_layer_list);
	/* TODO:
		check binding status
	 */
	dcc_hal_process_layers(&test_layer_list);
	sleep(5);

	/* Test case 2:
		BG: on
		ovs:  1 active
	*/
	test_layer_list.num_layers = 2;
	test_layers[1].dst_rect.x = 25;
	test_layers[1].dst_rect.y = 25;
	test_layer_list.dcc_layer[0] = &test_layers[1];
	test_layer_list.dcc_layer[1] = &test_layers[0];
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	test_layer_list.dcc_layer[0]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[1]->dcc_bind = DCC_BIND_NONE;
	dcc_hal_bind_layers(&test_layer_list);
	/* TODO:
		check binding status
	 */
	dcc_hal_process_layers(&test_layer_list);
	test_layers[1].dst_rect.x = 0;
	test_layers[1].dst_rect.y = 0;
	sleep(5);

	/* Test case 3:
		BG: on
		ovs:  2 active
	*/
	test_layer_list.num_layers = 3;
	test_layers[1].dst_rect.x = 25;
	test_layers[1].dst_rect.y = 25;
	test_layers[2].dst_rect.x = 50;
	test_layers[2].dst_rect.y = 50;
	test_layer_list.dcc_layer[0] = &test_layers[2];
	test_layer_list.dcc_layer[1] = &test_layers[1];
	test_layer_list.dcc_layer[2] = &test_layers[0];
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	test_layer_list.dcc_layer[0]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[1]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[2]->dcc_bind = DCC_BIND_NONE;
	dcc_hal_bind_layers(&test_layer_list);
	/* TODO:
		check binding status
	 */
	dcc_hal_process_layers(&test_layer_list);
	test_layers[1].dst_rect.x = 0;
	test_layers[1].dst_rect.y = 0;
	test_layers[2].dst_rect.x = 0;
	test_layers[2].dst_rect.y = 0;
	sleep(5);

	/* Test case 4:
		BG: on
		ovs:  3 active
	*/
	test_layer_list.num_layers = 4;
	test_layers[1].dst_rect.x = 25;
	test_layers[1].dst_rect.y = 25;
	test_layers[2].dst_rect.x = 50;
	test_layers[2].dst_rect.y = 50;
	test_layers[3].dst_rect.x = 75;
	test_layers[3].dst_rect.y = 75;
	test_layer_list.dcc_layer[0] = &test_layers[3];
	test_layer_list.dcc_layer[1] = &test_layers[2];
	test_layer_list.dcc_layer[2] = &test_layers[1];
	test_layer_list.dcc_layer[3] = &test_layers[0];
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	test_layer_list.dcc_layer[0]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[1]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[2]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[3]->dcc_bind = DCC_BIND_NONE;
	dcc_hal_bind_layers(&test_layer_list);
	/* TODO:
		check binding status
	 */
	dcc_hal_process_layers(&test_layer_list);
	test_layers[1].dst_rect.x = 0;
	test_layers[1].dst_rect.y = 0;
	test_layers[2].dst_rect.x = 0;
	test_layers[2].dst_rect.y = 0;
	test_layers[3].dst_rect.x = 0;
	test_layers[3].dst_rect.y = 0;
	sleep(5);

	/* Test case 5:
		BG: off
		ovs:  3 active
	*/
	test_layer_list.num_layers = 4;
	test_layers[1].dst_rect.x = 25;
	test_layers[1].dst_rect.y = 25;
	test_layers[2].dst_rect.x = 50;
	test_layers[2].dst_rect.y = 50;
	test_layers[3].dst_rect.x = 75;
	test_layers[3].dst_rect.y = 75;
	test_layer_list.dcc_layer[0] = &test_layers[3];
	test_layer_list.dcc_layer[1] = &test_layers[2];
	test_layer_list.dcc_layer[2] = &test_layers[1];
	test_layer_list.dcc_layer[3] = NULL;
	test_layer_list.dcc_op_class =  DCC_M2D_CLASS;
	test_layer_list.dcc_layer[0]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[1]->dcc_bind = DCC_BIND_NONE;
	test_layer_list.dcc_layer[2]->dcc_bind = DCC_BIND_NONE;
	dcc_hal_bind_layers(&test_layer_list);
	/* TODO:
		check binding status
	 */
	dcc_hal_process_layers(&test_layer_list);
	test_layers[1].dst_rect.x = 0;
	test_layers[1].dst_rect.y = 0;
	test_layers[2].dst_rect.x = 0;
	test_layers[2].dst_rect.y = 0;
	test_layers[3].dst_rect.x = 0;
	test_layers[3].dst_rect.y = 0;
	sleep(5);


	return 0;
}

static void test_dcc_help()
{
	ALOGE("dcctests <testcase>\n");
	ALOGE("testcase <0>: test framebuffer apis\n");
	ALOGE("testcase <1>: test simple updates\n");
	ALOGE("testcase <2>: test updates with vsync\n");
	ALOGE("testcase <3>: test overlays\n");
	ALOGE("testcase <4>: test video windows\n");

	return;
}

int main(int argc, char *argv[])
{
	int arg1 = -1;
	long long arg2 = -1;
	if (argc < 2) {
		ALOGE("[%s] Wrong arguments\n", __func__);
		test_dcc_help();
		return -1;
	}

	arg1 = atoi(argv[1]);
	if (argc == 3)
		arg2 = atoll(argv[2]);

	switch (arg1) {
	case 0:
		test_dcc_framebuffer_apis();
		break;
	case 1:
		test_dcc_m2d_update(arg2);
		break;
	case 2:
		test_dcc_m2d_update_with_vsync(arg2);
		break;
	case 3:
		test_dcc_m2d_overlays();
		break;
#if 0
	case 4:
		test_dcc_m2d_videowindow();
		break;
#endif
	default:
		ALOGE("[%s] Wrong arguments\n", __func__);
		test_dcc_help();
		break;
	}

	return 0;
}
