/*
 ******************************************************************************
 * Copyright (C) 2013-2015 Intel Mobile Communications GmbH
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
 ******************************************************************************
 */


/**
 *	dcc_hal.cpp
 *	The idea is to have a DCC capability point to which
 *	the upper layer requirements can be mapped.
 */

#include <fcntl.h>
#include <errno.h>
#include <hardware/hwcomposer.h>
#include "gralloc_priv.h"
#include "dcc-hal.h"

// number of overlays actually used by HWC, must be <= DCC_OVERLAY_NUM

static int dcc_fd = -1;

/**
 *  HAL init function
 */
int dcc_hal_init(void)
{
	dcc_fd = open(DCC_DEVICE_NAME, 0);
	if (0 > dcc_fd) {
		dcc_hal_err("Can't open device file: %s\n", DCC_DEVICE_NAME);
		return -1;
	}

	return dcc_fd;
}

/**
 * HAL closure
 */
void dcc_hal_deinit(void)
{
	if (0 > dcc_fd) {
		dcc_hal_err("%s not yet opened\n", DCC_DEVICE_NAME);
		return;
	}
	close(dcc_fd);
	return;
}

/**
 * Get dcc attr needed for framework
 */
int dcc_hal_get_attr(int attr_type, struct dcc_hal_attr_t *p_attr)
{
	int ret = -1;

	if ((0 > dcc_fd) || !p_attr) {
		dcc_hal_err("Wrong arguments OR device not ready\n");
		return ret;
	}

	switch (attr_type) {
	case DCC_HAL_ATTR_DISPLAY_INFO:
		ret = ioctl(dcc_fd, DCC_IOR_DISPLAY_INFO, &p_attr->display_info);
		break;

	default:
		ret = -1;
		dcc_hal_err("[%s]Unknown attribute!!\n", __func__);
		break;
	}

	return ret;
}

/**
 * Set dcc attr exported to framework
 */
int dcc_hal_set_attr(int attr_type, struct dcc_hal_attr_t *p_attr)
{

	int ret = -1;
	if ((0 > dcc_fd) || !p_attr) {
		dcc_hal_err("[%s] Wrong arguments OR device not ready\n", __func__);
		return -1;
	}

	switch (attr_type) {
/*	case DCC_HAL_ATTR_VSYNC:
		ret = ioctl(dcc_fd, DCC_IOW_VSYNC, &p_attr->vsync);
		break;*/
	default:
		ret = -1;
		dcc_hal_err("[%s]Unknown attribute!!\n", __func__);
		break;
	}

	if (0 > ret)
		dcc_hal_err("[%s] setting attribute failed\n", __func__);

	return ret;
}


#define DCC_IOCTL(_err_, _fd_, _key_, _param_) \
      _err_ = ioctl( _fd_, _key_, _param_ ); \
      if (_err_){ \
    	  ALOGE("%s failed !\n",#_key_); \
      }

static int hal_to_dcc_fmt(int hal_spr_fmt, int* isOpaque ) {
	if (isOpaque)
		*isOpaque = 0;
    switch (hal_spr_fmt) {
    case HAL_PIXEL_FORMAT_RGBX_8888:
        if (isOpaque)
            *isOpaque = 1;
        /* FALLTHROUGH */
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return DCC_FMT_ARGB8888;
    case HAL_PIXEL_FORMAT_RGB_565:
        return DCC_FMT_RGB565;
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return DCC_FMT_ABGR8888;
    case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        return DCC_FMT_YUV420SP;
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        return DCC_FMT_YUV422PACKED;
    default:
        return -1;
    }
}

// TODO: remove this copy (from gralloc) and store instead the stride
// in the buffer handle
#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))
static int dcc_hal_compute_stride(int w, int format)
{
    size_t stride, bpr;
	int bpp = 0;

    switch (format) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        bpp = 4;
        bpr = GRALLOC_ALIGN(w * bpp, 8);
        stride = bpr / bpp;
        break;
    /* TODO: review this; bpp=3 is not possible with DCC */
    case HAL_PIXEL_FORMAT_RGB_888:
        bpp = 3;
        bpr = GRALLOC_ALIGN(w * bpp, 8);
        stride = bpr / bpp;
        break;
    case HAL_PIXEL_FORMAT_RGB_565:
    case HAL_PIXEL_FORMAT_RAW16:
        bpp = 2;
        bpr = GRALLOC_ALIGN(w * bpp, 8);
        stride = bpr / bpp;
        break;

    case HAL_PIXEL_FORMAT_YV12:
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
    case HAL_PIXEL_FORMAT_YCbCr_420_SP:
    case HAL_PIXEL_FORMAT_YCbCr_422_SP:
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        stride = GRALLOC_ALIGN(w, 16);
        break;
    default:
        ALOGE("%s - unknown format (0x%x)", __FUNCTION__, format);
        return w;
    }

    return stride;
}

static void hal_to_dcc_boundsi(const hwc_rect_t* halr, struct dcc_bounds_t* dccr) {
    dccr->x = halr->left;
    dccr->y = halr->top;
    dccr->w = halr->right - halr->left;
    dccr->h = halr->bottom - halr->top;
}

static void hal_to_dcc_boundsf(const hwc_frect_t* halr, struct dcc_bounds_t* dccr) {
    dccr->x = (int)halr->left;
    dccr->y = (int)halr->top;
    dccr->w = (int)(halr->right - halr->left);
    dccr->h = (int)(halr->bottom - halr->top);
}

static void hal_to_dcc_layer_back(const hwc_layer_1_t* hal_layer,
        struct dcc_layer_back* dcc_layer) {
    struct private_handle_t const * hnd;
    hnd = ((struct private_handle_t const *)hal_layer->handle);

    hal_to_dcc_boundsf(&hal_layer->sourceCropf, &dcc_layer->src);
    // TODO: need to support a dest rectangle for background?
    //hal_to_dcc_bounds(&hal_layer->displayFrame, &dcc_layer->dst);
    dcc_layer->fence_acquire = hal_layer->acquireFenceFd;
    if (hnd != NULL) {
        dcc_layer->fd = hnd->fd;
        dcc_layer->phys = hnd->phys;
        dcc_layer->fmt = hal_to_dcc_fmt(hnd->fmt, NULL);
        // TODO: stride in private handle, for now recompute
        dcc_layer->stride = dcc_hal_compute_stride(hnd->w, hnd->fmt);
    } else {
        ALOGE("%s - layer handle is NULL!\n", __FUNCTION__);
    }
}

static void hal_to_dcc_layer_ovl(const hwc_layer_1_t* hal_layer,
        struct dcc_layer_ovl* dcc_layer) {
    struct private_handle_t const * hnd;
    hnd = ((struct private_handle_t const *)hal_layer->handle);
    int isOpaque;

    hal_to_dcc_boundsf(&hal_layer->sourceCropf, &dcc_layer->src);
    // TODO: need to support a dest rectangle for background?
    hal_to_dcc_boundsi(&hal_layer->displayFrame, &dcc_layer->dst);
    dcc_layer->fence_acquire = hal_layer->acquireFenceFd;
    if (hnd != NULL) {
        dcc_layer->fd = hnd->fd;
        dcc_layer->phys = hnd->phys;
        dcc_layer->fmt = hal_to_dcc_fmt(hnd->fmt, &isOpaque);
        if (isOpaque) {
            dcc_layer->global = 1;
            dcc_layer->alpha = 0xFF;
        }
        // TODO: stride in private handle, for now recompute
        // TODO: DCC does not support stride for overlays
        //dcc_layer-> = dcc_hal_compute_stride(hnd->w, hnd->fmt);
    } else {
        ALOGE("%s - layer handle is NULL!\n", __FUNCTION__);
    }
}

/*
Post an update request to the kernel driver.

The update will be performed asynchronously at the next display refresh
event.

The function will set the releaseFenceFd field of the given layer to
a fence that will be signaled when the buffer is no longer in use by the
display.
*/
int dcc_hal_compose(hwc_layer_1_t* fb_layer, hwc_layer_1_t** ov_layers,
        size_t ov_count, int fb_enabled, int *retireFenceFd, int *wb_release_fence,
        unsigned int wb_outbuf_phys, pthread_mutex_t *wbLock, pthread_cond_t *wbCondition,
        int *wb_flag, unsigned int *wb_buff_addr_phys) {
    int err = 0;
    struct dcc_update_layers_wb updt_wb;
    unsigned int ov_id, l_id;

    // check that we do not have more overlay than supported by HW
    if (ov_count > HWC_OVERLAY_NUM) {
        ALOGE("Too many overlays: %d (max is %d)", ov_count, HWC_OVERLAY_NUM);
        ov_count = HWC_OVERLAY_NUM;
    }
    // a memset to 0 will set everything as disabled
    memset(&updt_wb.updt, 0, sizeof(updt_wb.updt));
    updt_wb.updt.flags = DCC_UPDATE_ONESHOT_ASYNC;
    // but fences need to be set to -1
    updt_wb.updt.fence_retire = -1;
    updt_wb.updt.back.fence_release = -1;
    for (ov_id = 0 ; ov_id < DCC_OVERLAY_NUM ; ov_id++)
        updt_wb.updt.ovls[ov_id].fence_release = -1;

    /* Set up FB */
    hal_to_dcc_layer_back(fb_layer, &updt_wb.updt.back);
    if (!fb_enabled) {
        /* disable FB if requested */
        updt_wb.updt.back.fd   = -1;
        updt_wb.updt.flags |= DCC_UPDATE_NOBG_MASK;
        updt_wb.updt.back.phys = 0;
        updt_wb.updt.back.fmt = DCC_FMT_ARGB8888;
    }

    /* Set up overlays */
    for (l_id = 0, ov_id = 0; l_id < ov_count ; l_id++) {
        /* the DCC overlay order must be reversed because there
         * lower id means higher priority (i.e. on top) */
        ov_id = ov_count - 1 - l_id;
        hal_to_dcc_layer_ovl(ov_layers[l_id], &updt_wb.updt.ovls[ov_id]);
    }

    /* Post the update to the kernel driver */
    if (wb_outbuf_phys) {
        DCC_UPDATE_WB_SET(updt_wb.updt.flags, 1);
        updt_wb.wb.phys = *wb_buff_addr_phys;
        updt_wb.wb.fence_release = -1;
        DCC_IOCTL(err, dcc_fd, DCC_IOW_COMPOSE_WITH_WB, &updt_wb);
    }
    else {
        DCC_IOCTL(err, dcc_fd, DCC_IOW_COMPOSE, &updt_wb.updt);
    }

    // DCC may report an error for updates if the panel is off
    if (err < 0)
        ALOGW("DCC compose error: err= %d", err);

    /* Retrieve the fence fd returned by the driver*/
    *retireFenceFd = updt_wb.updt.fence_retire;
    fb_layer->releaseFenceFd = updt_wb.updt.back.fence_release;
    if (wb_outbuf_phys) {
        if (updt_wb.wb.fence_release >= 0) {
            if (*wb_flag == 1) {
                *wb_release_fence = updt_wb.wb.fence_release;
                pthread_cond_signal(wbCondition);
            }
            else {
                close(updt_wb.wb.fence_release);
                updt_wb.wb.fence_release = -1;
            }
        }
    }
    /* Make sure we do not get a release fence when the back is disabled */
    if (!fb_enabled && fb_layer->releaseFenceFd >= 0) {
        ALOGE("Error, got a release fence fd for a disabled background.");
        close(fb_layer->releaseFenceFd);
        fb_layer->releaseFenceFd = -1;
    }
    for (l_id = 0, ov_id = 0; l_id < ov_count ; l_id++) {
        /* the DCC overlay order is reversed */
        ov_id = ov_count - 1 - l_id;
        ov_layers[l_id]->releaseFenceFd = updt_wb.updt.ovls[ov_id].fence_release;
    }
    return err;
}
