/*
 *
 * (C) COPYRIGHT 2013-2015 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#include <drm/drm_crtc.h>
#include <sw_sync.h>
#include <video/adf.h>
#include <video/adf_client.h>

#include <uapi/video/malidp_adf.h>

#include "malidp_adf.h"
#include "malidp_hw.h"

#define to_malidp_interface(adf_intf) container_of(adf_intf, \
			struct malidp_interface, intf)

#define MALIDP_INTF_TIMEOUT_MS 250

struct malidp_interface {
	char name[ADF_NAME_LEN];
	u32 idx;
	struct adf_interface intf;
	enum adf_interface_type adf_type;
	u32 adf_flags;
	const struct adf_interface_ops *ops;
	const struct malidp_intf_hw_info *hw_info;
	struct notifier_block nb;
	struct video_tx_display_info disp_info;
	/* these fields should only be used for the memory interface */
	struct sw_sync_timeline *mw_timeline;
	int mw_timeline_max;
	struct sync_fence *mw_latest_fence;
	struct malidp_intf_memory_cfg *mw_latest_cfg;
	struct malidp_intf_memory_cfg *mw_prev_cfg;
	/* This spinlock protects access to per-post state */
	struct mutex post_lock;
	atomic_t flip_occurred;
	atomic_t report_vsync;
	atomic_t report_flip;
	wait_queue_head_t wq;
	struct video_tx_device *tx;
	ktime_t prepare_ts;
};

int malidp_adf_intf_get_dp_type(struct adf_interface *adf_intf)
{
	struct malidp_interface *intf = to_malidp_interface(adf_intf);

	return intf->hw_info->type;
};

static int malidp_primary_blank(struct adf_interface *intf, u8 state)
{
	struct adf_device *dev = adf_interface_parent(intf);
	struct malidp_device *dp_dev = to_malidp_device(dev);
	struct malidp_interface *dp_intf = to_malidp_interface(intf);
	struct malidp_driver_state *driver_state;
	int i, res;

	dev_dbg(&intf->base.dev, "Blank (%i) on %s\n", state, intf->base.name);

	res = video_tx_dpms(dp_intf->tx, state);
	if (res < 0) {
		dev_dbg(dp_dev->device, "%s: transmitter reported error\n",
			__func__);
		return res;
	}

	switch (state) {
	case DRM_MODE_DPMS_ON:
		/* Refuse if the current mode doesn't look sensible */
		if (!intf->current_mode.hdisplay || !intf->current_mode.vdisplay) {
			dev_err(dp_dev->device, "%s: No mode currently set - cannot unblank", __func__);
			return -EINVAL;
		}

		/*
		 * Flush all pending posts. Then dev->onscreen will contain
		 * the last submitted post or NULL if no posts were made.
		 *
		 * There is no possible race when accessing dev->onscreen
		 * because we have just flushed the worker thread and the client
		 * lock is being held by blank() so no new posts can be queued
		 * in the meantime.
		 */
		flush_kthread_worker(&dev->post_worker);
		malidp_hw_display_switch(dp_dev->hw_dev, true);
		/*
		 * If we "dropped" any frames whilst we were blanked, then
		 * display the most recent one now
		 */
		if (dev->onscreen) {
			driver_state = dev->onscreen->state;
			malidp_hw_commit(dp_dev->hw_dev, &driver_state->hw_state);
			for (i = 0; i < driver_state->n_intfs; i++)
				malidp_intf_wait(driver_state->intf_list[i], dp_dev);
		}

		break;
	case DRM_MODE_DPMS_STANDBY:
	case DRM_MODE_DPMS_SUSPEND:
	case DRM_MODE_DPMS_OFF:
		if (intf->dpms_state == DRM_MODE_DPMS_ON)
			malidp_hw_display_switch(dp_dev->hw_dev, false);

		/*
		 * The last flushed post will be considered onscreen by ADF.
		 * We need to clean that one up too.
		 */
		if (dev->onscreen) {
			struct adf_pending_post *post;

			post = dev->onscreen;
			dev->timeline_max++;
			dev->ops->advance_timeline(dev, &post->config,
				post->state);
			malidp_adf_post_cleanup(dev, post);
			/*
			 * Set it to NULL and ADF won't try to clean it up again
			 * when we resume normal service
			 */
			dev->onscreen = NULL;
		}

		break;
	default:
		BUG();
	}

	dp_dev->current_dpms = state;
	/*
	 * This should propagate the video_tx return value if nothing more
	 * important happened. Note that the ADF blank implementation assumes
	 * that we did nothing if we return < 0 here.
	 */
	return res;
}

static bool malidp_adf_intf_validate_mode(struct malidp_device *dev,
	struct drm_mode_modeinfo *mode)
{
	u32 max_w, max_h, rate_diff, full_frame;

	/* Interlaced modes not supported */
	if (mode->flags & DRM_MODE_FLAG_INTERLACE) {
		dev_dbg(dev->device, "%s: mode '%.*s' failed flag check!\n",
			__func__, DRM_DISPLAY_MODE_LEN, mode->name);
		return false;
	}

	/* Sanity check dimensions */
	if ((mode->hdisplay == 0) || (mode->htotal == 0) ||
	    (mode->hsync_start < mode->hdisplay) ||
	    (mode->hsync_end < mode->hsync_start) ||
	    (mode->htotal < mode->hsync_end) ||
	    (mode->vdisplay == 0) || (mode->vtotal == 0) ||
	    (mode->vsync_start < mode->vdisplay) ||
	    (mode->vsync_end < mode->vsync_start) ||
	    (mode->vtotal < mode->vsync_end) ||
	    (mode->clock == 0)) {
		dev_dbg(dev->device, "%s: mode '%.*s' failed timing check!\n",
			__func__, DRM_DISPLAY_MODE_LEN, mode->name);
		return false;
	}

	/*
	 * Check refresh rate/clock. We allow for up to 1 Hz of error
	 * full_frame == pixels per frame
	 * full_frame * mode->vrefresh == pixels per second
	 * mode->clock * 1000 == clocks per second
	 */
	full_frame = (u32)mode->htotal * (u32)mode->vtotal;
	rate_diff = abs((full_frame * mode->vrefresh) - (mode->clock * 1000));
	if (rate_diff > full_frame) {
		dev_dbg(dev->device, "%s: mode '%.*s' failed rate check!\n",
			__func__, DRM_DISPLAY_MODE_LEN, mode->name);
		return false;
	}

	/* Check against max supported dimensions */
	malidp_hw_supported_dimensions_get(dev->hw_dev, NULL, NULL,
					   &max_w, &max_h);
	if ((mode->hdisplay > max_w) || (mode->vdisplay > max_h)) {
		dev_dbg(dev->device, "%s: mode '%.*s' failed dimension check!\n",
			__func__, DRM_DISPLAY_MODE_LEN, mode->name);
		return false;
	}

	return true;
}

static int malidp_primary_modeset(struct adf_interface *intf,
		struct drm_mode_modeinfo *mode)
{
	struct malidp_device *dp_dev = to_malidp_device(intf->base.parent);
	struct malidp_interface *dp_intf = to_malidp_interface(intf);
	int ret;

	dev_dbg(dp_dev->device, "%s : (%ix%i) on %s\n", __func__,
			mode->hdisplay, mode->vdisplay, intf->base.name);

	if (intf->dpms_state == DRM_MODE_DPMS_ON) {
		dev_err(dp_dev->device, "%s : Can't set mode with display turned on\n",
				__func__);
		return -EBUSY;
	}

	if (!malidp_adf_intf_validate_mode(dp_dev, mode))
		return -EINVAL;

	ret = video_tx_set_mode(dp_intf->tx, mode);
	if (ret == 0)
		ret = malidp_hw_modeset(dp_dev->hw_dev, mode);

	/* If modeset failed, we should attempt to restore the old one */
	if (ret) {
		struct drm_mode_modeinfo *fallback = &intf->current_mode;
		if (fallback->hdisplay && fallback->vdisplay) {
			int err;
			dev_err(dp_dev->device, "%s: Modeset failed. Attempting fallback\n",
				__func__);

			err = video_tx_set_mode(dp_intf->tx, fallback);
			if (malidp_hw_modeset(dp_dev->hw_dev, fallback) || err)
				dev_err(dp_dev->device, "%s: Modeset and fallback failed. Display state undefined!\n",
					__func__);
		}
	}

	return ret;
}

static int malidp_primary_describe_simple_post(struct adf_interface *intf,
		struct adf_buffer *buf, void *data, size_t *size)
{
	struct malidp_custom_data *blob_p;
	struct drm_mode_modeinfo mode;
	struct malidp_custom_data blob = {
		.n_malidp_buffer_configs = 1,
		.sizeof_malidp_buffer_config = sizeof(struct malidp_buffer_config),
	};
	struct malidp_buffer_config malidp_buf = {
		.adf_buffer_index = 0,
		.adf_intf_id = intf->base.id,
		.display_width = buf->w,
		.display_height = buf->h,
		.transform = 0,
		.flags = MALIDP_FLAG_BUFFER_INPUT,
		.alpha_mode = MALIDP_ALPHA_MODE_NONE
	};

	/* Get the current mode and use its dimensions to center the image */
	adf_interface_current_mode(intf, &mode);
	malidp_buf.display_top = (mode.vdisplay - buf->h) / 2,
	malidp_buf.display_left = (mode.hdisplay - buf->w) / 2,

	*size = sizeof(blob) + sizeof(malidp_buf);
	if (ADF_MAX_CUSTOM_DATA_SIZE < *size)
		return -ENOMEM;

	blob_p = (struct malidp_custom_data *)data;
	memcpy(blob_p, &blob, sizeof(blob));
	memcpy(&blob_p->buffers, &malidp_buf, sizeof(malidp_buf));

	return 0;
}

static bool malidp_interface_supports_event(struct adf_obj *obj,
				     enum adf_event_type type)
{
	bool supported;

	switch ((int)type) {
	case ADF_EVENT_VSYNC:
	case MALIDP_ADF_EVENT_FLIP:
	case ADF_EVENT_HOTPLUG:
		supported = true;
		break;
	default:
		supported = false;
	}
	return supported;
}

static void malidp_interface_set_event(struct adf_obj *obj,
				    enum adf_event_type type, bool enabled)
{
	struct adf_interface *adf_intf = container_of(obj,
				struct adf_interface, base);
	struct malidp_interface *malidp_intf = to_malidp_interface(adf_intf);
	int val = (enabled == true) ? 1 : 0;

	switch ((int)type) {
	case ADF_EVENT_VSYNC:
		atomic_set(&malidp_intf->report_vsync, val);
		break;
	case MALIDP_ADF_EVENT_FLIP:
		atomic_set(&malidp_intf->report_flip, val);
		break;
	default:
		break;
	}
}

enum adf_interface_type malidp_drm_conn_type_to_adf(u32 drm_type)
{
	switch (drm_type) {
	case DRM_MODE_CONNECTOR_VGA:
		return ADF_INTF_VGA;
	case DRM_MODE_CONNECTOR_DVII:
	case DRM_MODE_CONNECTOR_DVID:
	case DRM_MODE_CONNECTOR_DVIA:
		return ADF_INTF_DVI;
	case DRM_MODE_CONNECTOR_LVDS:
		return MALIDP_INTF_LVDS;
	case DRM_MODE_CONNECTOR_DisplayPort:
		return ADF_INTF_DPI;
	case DRM_MODE_CONNECTOR_HDMIA:
	case DRM_MODE_CONNECTOR_HDMIB:
		return ADF_INTF_HDMI;
	case DRM_MODE_CONNECTOR_eDP:
		return ADF_INTF_eDP;
	case DRM_MODE_CONNECTOR_VIRTUAL:
		return MALIDP_INTF_VIRTUAL;
	default:
		return MALIDP_INTF_UNKNOWN;
	}
}

static const char *malidp_interface_type_str(struct adf_interface *intf)
{
	switch ((u32)intf->type) {
	case MALIDP_INTF_LVDS:
		return "LVDS";
	case MALIDP_INTF_VIRTUAL:
		return "virtual";
	case MALIDP_INTF_MEMORY:
		return "memory";
	case MALIDP_INTF_UNKNOWN:
	default:
		return "unknown";
	}
}

static long malidp_memory_ioctl(struct adf_obj *obj, unsigned int cmd,
			       unsigned long arg)
{
	struct malidp_adf_get_output_fence data;
	struct adf_interface *adf_intf = container_of(obj,
				struct adf_interface, base);
	struct malidp_interface *malidp_intf = to_malidp_interface(adf_intf);
	int fence_fd;

	switch (cmd) {
	case MALIDP_ADF_IOCTL_GET_OUTPUT_FENCE:
		if (malidp_adf_intf_get_dp_type(adf_intf) != MALIDP_HW_INTF_MEMORY)
			return -EINVAL;

		mutex_lock(&malidp_intf->post_lock);
		if (malidp_intf->mw_latest_fence == NULL) {
			mutex_unlock(&malidp_intf->post_lock);
			return -ENOENT;
		}

		fence_fd = get_unused_fd();
		if (fence_fd < 0) {
			mutex_unlock(&malidp_intf->post_lock);
			return fence_fd;
		}

		sync_fence_install(malidp_intf->mw_latest_fence, fence_fd);
		data.output_fence = (s64)fence_fd;
		/* Increase reference count of sync fence.
		 * The fence is not only used by userland but also used
		 * by system pm notifier.
		 */
		sync_fence_fdget(fence_fd);

		/* Destroying this fence is not our problem anymore */
		malidp_intf->mw_latest_fence = NULL;
		mutex_unlock(&malidp_intf->post_lock);

		if (copy_to_user((void *)arg, &data, sizeof(data)))
			return -EFAULT;

		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int malidp_adf_ioctl_set_gamma(struct malidp_device *dev,
			       struct malidp_adf_set_gamma *gamma)
{
	bool gamma_enabled;
	u32 *gamma_coeffs;

	if (get_user(gamma_enabled, &gamma->enable))
		return -EFAULT;

	gamma_coeffs = kzalloc(sizeof(u32) * MALIDP_N_GAMMA_COEFFS, GFP_KERNEL);
	if (!gamma_coeffs)
		return -ENOMEM;

	if (copy_from_user(gamma_coeffs, gamma->coefficient,
			   sizeof(u32) * MALIDP_N_GAMMA_COEFFS)) {
		kfree(gamma_coeffs);
		return -EFAULT;
	}

	mutex_lock(&dev->adf_dev.client_lock);

	malidp_hw_update_gamma_settings(dev->hw_dev,
					gamma_enabled,
					gamma_coeffs);

	mutex_unlock(&dev->adf_dev.client_lock);

	kfree(gamma_coeffs);

	return 0;
}

static long malidp_primary_ioctl(struct adf_obj *obj, unsigned int cmd,
			       unsigned long arg)
{
	struct adf_interface *adf_intf = container_of(obj,
			struct adf_interface, base);
	struct malidp_device *dp_dev = to_malidp_device(adf_intf->base.parent);

	if (malidp_adf_intf_get_dp_type(adf_intf) != MALIDP_HW_INTF_PRIMARY)
		return -EINVAL;

	switch (cmd) {
	case MALIDP_ADF_IOCTL_SET_GAMMA:
		return malidp_adf_ioctl_set_gamma(dp_dev,
					(struct malidp_adf_set_gamma *)arg);
	default:
		return -EINVAL;
	}

	return 0;
}

static int malidp_adf_intf_custom_data(struct adf_obj *obj,
				       void *data,
				       size_t *size)
{
	struct malidp_adf_intf_custom_data *custom_data = data;
	struct adf_interface *adf_intf = container_of(obj,
				struct adf_interface, base);
	struct malidp_interface *malidp_intf = to_malidp_interface(adf_intf);
	struct malidp_device *dp_dev =
			to_malidp_device(malidp_intf->intf.base.parent);

	memset(custom_data, 0, sizeof(*custom_data));

	custom_data->gamma = malidp_intf->disp_info.gamma;
	custom_data->downscaling_threshold =
		malidp_hw_downscaling_threshold(dp_dev->hw_dev);

	*size = sizeof(*custom_data);

	return 0;
}

static int malidp_primary_screen_size(struct adf_interface *intf,
				      u16 *width_mm, u16 *height_mm)
{
	struct malidp_interface *malidp_intf = to_malidp_interface(intf);

	*width_mm = malidp_intf->disp_info.width_mm;
	*height_mm = malidp_intf->disp_info.height_mm;

	return 0;
}

static const struct adf_interface_ops malidp_intf_ops[] = {
	[MALIDP_HW_INTF_PRIMARY] = {
		.base = {
			.ioctl = malidp_primary_ioctl,
			.supports_event = malidp_interface_supports_event,
			.set_event = malidp_interface_set_event,
			.custom_data = malidp_adf_intf_custom_data,
		},
		.blank = malidp_primary_blank,
		.describe_simple_post =
			malidp_primary_describe_simple_post,
		.modeset = malidp_primary_modeset,
		.type_str = malidp_interface_type_str,
		.screen_size = malidp_primary_screen_size,
	},
	[MALIDP_HW_INTF_MEMORY] = {
		.base = {
			.ioctl = malidp_memory_ioctl,
		},
		.type_str = malidp_interface_type_str,
	},
};

static void malidp_adf_flip_notify(struct adf_interface *intf, ktime_t timestamp)
{
	struct malidp_adf_flip_event flip_event;

	flip_event.base.type = MALIDP_ADF_EVENT_FLIP;
	flip_event.base.length = sizeof(flip_event);
	flip_event.timestamp = ktime_to_ns(timestamp);
	adf_event_notify(&intf->base,  &flip_event.base);
}

static void malidp_adf_intf_flip_de_cb(struct device *dev, void *opaque,
				    struct malidp_hw_event_queue *queue)
{
	struct malidp_interface *dp_intf = opaque;
	struct malidp_hw_event event;
	char errstring[50];

	mutex_lock(&dp_intf->post_lock);

	malidp_hw_event_queue_dequeue(queue, &event);
	while (event.type != MALIDP_HW_EVENT_NONE) {
		if (event.type & MALIDP_HW_EVENT_FLIP) {
			if (atomic_read(&dp_intf->report_flip))
				malidp_adf_flip_notify(&dp_intf->intf, event.timestamp);

			/* Only wake up if this flip isn't "stale" */
			if (ktime_compare(dp_intf->prepare_ts, event.timestamp) < 0) {
				atomic_set(&dp_intf->flip_occurred, 1);
				wake_up(&dp_intf->wq);
			}
		}
		if (event.type & MALIDP_HW_EVENT_VSYNC) {
			if (atomic_read(&dp_intf->report_vsync))
				adf_vsync_notify(&dp_intf->intf, event.timestamp);
		}

		if (event.type & MALIDP_HW_EVENT_ERROR) {
			dev_err(dev, "%s: %s interface returned (%s) event",
				__func__, dp_intf->intf.base.name,
				malidp_hw_get_event_string(errstring, 50, &event));
		} else if (event.type != MALIDP_HW_EVENT_VSYNC) {
			dev_dbg(dev, "%s: %s interface returned (%s) event",
				__func__, dp_intf->intf.base.name,
				malidp_hw_get_event_string(errstring, 50, &event));
		}
		malidp_hw_event_queue_dequeue(queue, &event);
	}

	mutex_unlock(&dp_intf->post_lock);
}

/*
 * This function needs to be called with the post_lock mutex held in order to
 * protect mw_cf->signaled.
 */
static void malidp_adf_int_cleanup_output_buffer(struct malidp_interface *dp_intf,
				struct malidp_intf_memory_cfg *mw_cfg)
{
	struct malidp_device *dp_dev = to_malidp_device(dp_intf->intf.base.parent);
	struct adf_buffer *buf;
	struct adf_buffer_mapping *map;
	int i;

	buf = mw_cfg->mw_buf;
	map = mw_cfg->mw_map;

	if (mw_cfg->signaled) {
		dev_dbg(&dp_intf->intf.base.dev, "%s: %p already signaled\n", __func__, mw_cfg);
		return;
	}

	BUG_ON(!buf | !map);

	for (i = 0; i < buf->n_planes; i++) {
		if (dp_dev->iommu_domain) {
			struct malidp_iommu_mapping *iommu_map =
				*(mw_cfg->iommu_map + i);
			dev_dbg(&dp_intf->intf.base.dev, "%s: unmap sgt = %p\n",
				__func__, map->sg_tables[i]);
			malidp_iommu_unmap_sgt(dp_dev->iommu_domain, iommu_map);
		}
		dma_buf_unmap_attachment(map->attachments[i],
			map->sg_tables[i], DMA_FROM_DEVICE);
		dma_buf_detach(buf->dma_bufs[i],
			       map->attachments[i]);
		if (buf->dma_bufs[i])
			dma_buf_put(buf->dma_bufs[i]);
		if (buf->acquire_fence)
			sync_fence_put(buf->acquire_fence);

		/*
		* The ADF framework will want to do this too when the post is
		* no longer on screen. This trick will prevent double clean up.
		*/
		map->sg_tables[i] = NULL;
		map->attachments[i] = NULL;
		buf->dma_bufs[i] = NULL;
		buf->acquire_fence = NULL;
	}

	sw_sync_timeline_inc(dp_intf->mw_timeline, 1);

	mw_cfg->signaled = true;

	dev_dbg(&dp_intf->intf.base.dev, "%s: signal mw_cfg = %p\n", __func__, mw_cfg);
}

static void malidp_adf_intf_flip_se_cb(struct device *dev, void *opaque,
				    struct malidp_hw_event_queue *queue)
{
	struct malidp_interface *dp_intf = opaque;
	struct malidp_hw_event event;
	char errstring[50];

	mutex_lock(&dp_intf->post_lock);

	dev_dbg(dev, "%s: start\n", __func__);
	malidp_hw_event_queue_dequeue(queue, &event);
	while (event.type != MALIDP_HW_EVENT_NONE) {
		/*
		 * Ignore stale events which will be/will have been correctly
		 * handled by state free.
		 */
		if (ktime_compare(event.timestamp, dp_intf->prepare_ts) < 0) {
			malidp_hw_event_queue_dequeue(queue, &event);
			continue;
		}

		if (event.type & MALIDP_HW_EVENT_FLIP) {
			dev_dbg(dev, "%s: wake memory wq\n", __func__);
			atomic_set(&dp_intf->flip_occurred, 1);
			wake_up(&dp_intf->wq);
		}

		if (event.type & MALIDP_HW_EVENT_STOP) {
			if (dp_intf->mw_prev_cfg) {
				dev_dbg(dev, "%s: releasing (prev) mw_cfg = %p\n",
					__func__, dp_intf->mw_prev_cfg);
				malidp_adf_int_cleanup_output_buffer(dp_intf,
					dp_intf->mw_prev_cfg);
				dp_intf->mw_prev_cfg = NULL;
			} else if (dp_intf->mw_latest_cfg) {
				dev_dbg(dev, "%s: releasing (latest) mw_cfg = %p\n",
					__func__, dp_intf->mw_latest_cfg);
				malidp_adf_int_cleanup_output_buffer(dp_intf,
					dp_intf->mw_latest_cfg);
				dp_intf->mw_latest_cfg = NULL;
			}
		}

		if (event.type & MALIDP_HW_EVENT_ERROR) {
			dev_err(dev, "%s: %s interface returned (%s) event",
				__func__, dp_intf->intf.base.name,
				malidp_hw_get_event_string(errstring, 50, &event));
		} else {
			dev_dbg(dev, "%s: %s interface returned (%s) event",
				__func__, dp_intf->intf.base.name,
				malidp_hw_get_event_string(errstring, 50, &event));
		}

		malidp_hw_event_queue_dequeue(queue, &event);
	}
	dev_dbg(dev, "%s: done\n", __func__);

	mutex_unlock(&dp_intf->post_lock);
}

void malidp_intf_prepare(struct adf_interface *adf_intf,
			 struct malidp_intf_memory_cfg *mw_cfg)
{
	struct malidp_interface *dp_intf = to_malidp_interface(adf_intf);

	mutex_lock(&dp_intf->post_lock);

	if (malidp_adf_intf_get_dp_type(adf_intf) == MALIDP_HW_INTF_MEMORY) {
		if (dp_intf->mw_latest_cfg) {
			dev_dbg(&adf_intf->base.dev,
				"%s: moving %p to 'mw_prev_cfg'\n",
				__func__, dp_intf->mw_latest_cfg);
			dp_intf->mw_prev_cfg = dp_intf->mw_latest_cfg;
		}
		dp_intf->mw_latest_cfg = mw_cfg;
		dev_dbg(&adf_intf->base.dev, "%s: add %p to 'mw_latest_cfg'\n",
			__func__, mw_cfg);
	}

	dp_intf->prepare_ts = ktime_get();
	atomic_set(&dp_intf->flip_occurred, 0);
	mutex_unlock(&dp_intf->post_lock);
}

void malidp_intf_wait(struct adf_interface *adf_intf,
			      struct malidp_device *dp_dev)
{
	int ret;
	struct malidp_interface *dp_intf = to_malidp_interface(adf_intf);
	const char *intf_name = dp_intf->hw_info->name;

	dev_dbg(dp_dev->device, "%s: waiting on interface \"%s\"\n",
		__func__, intf_name);

	ret = wait_event_interruptible_timeout(dp_intf->wq,
			(atomic_read(&dp_intf->flip_occurred) == 1),
			msecs_to_jiffies(MALIDP_INTF_TIMEOUT_MS));
	if (ret == 0) {
		dev_err(dp_dev->device, "%s: timeout on interface \"%s\"\n",
			__func__, intf_name);
	} else if (ret == -ERESTARTSYS) {
		dev_err(dp_dev->device, "%s: signal received on interface \"%s\"\n",
			__func__, intf_name);
	} else {
		dev_dbg(dp_dev->device, "%s: flip on interface \"%s\"\n",
			__func__, intf_name);
	}
}

/*
 * Create a fence for the interface timeline that will be used to signal
 * output buffer when the memory write out interface has finished writting.
 */
struct malidp_intf_memory_cfg *malidp_intf_create_mw_cfg(struct adf_interface *adf_intf,
				     struct malidp_device *dp_dev,
				     struct adf_buffer *buf, int buf_index,
				     struct adf_buffer_mapping *map,
				     struct malidp_iommu_mapping **iommu_map)
{
	struct malidp_interface *dp_intf = to_malidp_interface(adf_intf);
	struct malidp_intf_memory_cfg *mw_cfg = NULL;
	struct sync_pt *pt;

	if (malidp_adf_intf_get_dp_type(adf_intf) == MALIDP_HW_INTF_MEMORY) {
		struct sync_fence *fence;

		mw_cfg = kzalloc(sizeof(struct malidp_intf_memory_cfg),
				 GFP_KERNEL);
		if (!mw_cfg)
			return NULL;

		mutex_lock(&dp_intf->post_lock);

		dp_intf->mw_timeline_max++;
		pt = sw_sync_pt_create(dp_intf->mw_timeline,
				       dp_intf->mw_timeline_max);
		if (!pt)
			goto err_pt_create;

		fence = sync_fence_create(adf_intf->base.name, pt);
		if (!fence)
			goto err_fence_create;

		mw_cfg->signaled = false;
		mw_cfg->mw_fence = fence;

		mw_cfg->mw_buf = buf;
		mw_cfg->mw_map = map;
		mw_cfg->iommu_map = iommu_map;
		mw_cfg->mw_buf_index = buf_index;

		dp_intf->mw_latest_fence = fence;

		dev_dbg(dp_dev->device, "%s: mw_cfg = %p\n", __func__, mw_cfg);

		mutex_unlock(&dp_intf->post_lock);
	}

	return mw_cfg;

err_fence_create:
	sync_pt_free(pt);
err_pt_create:
	dp_intf->mw_timeline_max--;
	mutex_unlock(&dp_intf->post_lock);
	kfree(mw_cfg);

	return NULL;
}

void malidp_adf_intf_destroy_mw_cfg(struct adf_interface *adf_intf,
				struct malidp_intf_memory_cfg *mw_cfg)
{
	struct malidp_interface *dp_intf = to_malidp_interface(adf_intf);
	struct device *dev = &adf_intf->base.dev;

	if (malidp_adf_intf_get_dp_type(adf_intf) != MALIDP_HW_INTF_MEMORY)
		return;
	dev_dbg(&adf_intf->base.dev, "%s: trying to get mutex mw_cfg = %p\n",
		__func__, mw_cfg);

	mutex_lock(&dp_intf->post_lock);
	dev_dbg(&adf_intf->base.dev, "%s: got mutex mw_cfg = %p\n",
		__func__, mw_cfg);


	/* If a STOP event hasn't already, we need to free the buffer */
	if (!mw_cfg->signaled) {
		dev_dbg(dev, "%s: clean up mw_cfg = %p\n", __func__, mw_cfg);
		malidp_adf_int_cleanup_output_buffer(dp_intf, mw_cfg);
		if (mw_cfg == dp_intf->mw_latest_cfg)
			dp_intf->mw_latest_cfg = NULL;
		else if (mw_cfg == dp_intf->mw_prev_cfg)
			dp_intf->mw_prev_cfg = NULL;
	} else {
		dev_dbg(dev, "%s: mw_cfg = %p already cleaned\n", __func__, mw_cfg);
	}

	sync_fence_put(mw_cfg->mw_fence);

	kfree(mw_cfg);
	mutex_unlock(&dp_intf->post_lock);
}

/*
 * Remove any invalid modes by consolidating all the valid ones at the start
 * of the list.
 * Returns the number of valid modes in the list.
 */
static int malidp_adf_intf_prune_modes(struct malidp_device *dp_dev,
	struct drm_mode_modeinfo *modes, int n_modes)
{
	struct drm_mode_modeinfo *gap = NULL;
	int i, n_valid = n_modes;

	for (i = 0; i < n_modes; i++) {
		struct drm_mode_modeinfo *mode = &modes[i];
		bool mode_valid = true;

		if (!malidp_hw_pxclk_ok(dp_dev->hw_dev, mode->clock * 1000))
			mode_valid = false;
		else if (!malidp_adf_intf_validate_mode(dp_dev, mode))
			mode_valid = false;

		if (mode_valid) {
			if (gap) {
				memcpy(gap, mode, sizeof(*mode));
				gap++;
			}
		} else {
			if (!gap)
				gap = mode;
			n_valid--;
		}
	}

	return n_valid;
}

static int malidp_primary_set_modes(struct malidp_device *dp_dev,
	struct malidp_interface *malidp_intf)
{
	struct drm_mode_modeinfo *modes;
	int n, ret;
	bool done = false;

	do {
		n = video_tx_get_modes(malidp_intf->tx, NULL, 0);
		if (n < 0) {
			dev_err(dp_dev->device, "could not get number of modes\n");
			return n;
		}

		modes = kzalloc(n * sizeof(struct drm_mode_modeinfo),
				GFP_KERNEL);
		if (!modes) {
			dev_err(dp_dev->device, "could not allocate the modes\n");
			return -ENOMEM;
		}

		ret = video_tx_get_modes(malidp_intf->tx, modes, n);
		if (ret < 0) {
			dev_err(dp_dev->device, "could not retrieve the modes\n");
			kfree(modes);
			return ret;
		} else if (ret != n) {
			dev_err(dp_dev->device, "mode list changed!\n");
			kfree(modes);
		} else {
		    done = true;
		}
	} while (!done);

	n = malidp_adf_intf_prune_modes(dp_dev, modes, n);

	ret = adf_hotplug_notify_connected(&malidp_intf->intf, modes, n);

	/* The ADF framework takes a copy of the modes so we can free it */
	kfree(modes);

	return ret;
}

static int malidp_adf_notifier_fn(struct notifier_block *nb,
			unsigned long cmd, void *ptr)
{
	struct video_tx_device *tx = ptr;
	struct malidp_interface *malidp_intf =
			container_of(nb, struct malidp_interface, nb);
	struct malidp_device *dp_dev =
			to_malidp_device(malidp_intf->intf.base.parent);
	struct video_tx_display_info display_info;
	int ret;

	switch (cmd) {
	case connector_status_connected:
		ret = video_tx_get_display_info(tx, &display_info);
		if (ret != 0) {
			dev_err(dp_dev->device, "get display info error\n");
			return NOTIFY_BAD;
		}

		malidp_hw_update_color_adjustment(dp_dev->hw_dev,
				display_info.red_x, display_info.red_y,
				display_info.green_x, display_info.green_y,
				display_info.blue_x, display_info.blue_y,
				display_info.white_x, display_info.white_y);

		malidp_intf->disp_info = display_info;

		malidp_primary_set_modes(dp_dev, malidp_intf);

		dev_dbg(dp_dev->device, "display is connected.\n");
		break;
	case connector_status_disconnected:
		adf_hotplug_notify_disconnected(&malidp_intf->intf);
		dev_dbg(dp_dev->device, "display is disconnected.\n");
		break;
	case connector_status_unknown:
		break;
	}
	return NOTIFY_DONE;
}

static int malidp_interface_init(struct malidp_device *dp_dev,
			struct malidp_interface *malidp_intf,
			const struct malidp_intf_hw_info *hw_info,
			struct video_tx_device *tx)
{
	int ret;
	struct video_tx_info info;

	init_waitqueue_head(&malidp_intf->wq);
	atomic_set(&malidp_intf->report_vsync, 0);
	atomic_set(&malidp_intf->report_flip, 0);

	mutex_init(&malidp_intf->post_lock);

	malidp_intf->mw_latest_cfg = NULL;
	malidp_intf->mw_prev_cfg = NULL;
	malidp_intf->mw_latest_fence = NULL;
	malidp_intf->ops = &malidp_intf_ops[hw_info->type];
	malidp_intf->hw_info = hw_info;
	malidp_intf->tx = tx;
	malidp_intf->prepare_ts = ktime_get();

	switch (hw_info->type) {
	case MALIDP_HW_INTF_PRIMARY:
		if (!malidp_intf->tx) {
			dev_err(dp_dev->device, "%s: primary interface should have a transmitter\n",
				__func__);
			return -EINVAL;
		}

		ret = video_tx_get_info(malidp_intf->tx, &info);
		if (ret) {
			dev_err(dp_dev->device, "%s: couldn't query transmitter info\n",
				__func__);
			return ret;
		}

		snprintf(malidp_intf->name, ADF_NAME_LEN, "%s", info.name);
		malidp_intf->adf_type =
			malidp_drm_conn_type_to_adf(info.connector_type);
		malidp_intf->adf_flags = ADF_INTF_FLAG_PRIMARY;
		malidp_intf->idx = info.idx;

		malidp_hw_set_de_output_depth(dp_dev->hw_dev, info.red_bits,
					      info.green_bits, info.blue_bits);

		malidp_hw_set_callback(dp_dev->hw_dev, malidp_intf->hw_info,
				       &malidp_adf_intf_flip_de_cb,
				       malidp_intf);

		malidp_intf->nb.notifier_call = malidp_adf_notifier_fn;
		video_tx_hotplug_notifier_register(malidp_intf->tx,
				&malidp_intf->nb);
		break;
	case MALIDP_HW_INTF_MEMORY:
		snprintf(malidp_intf->name, ADF_NAME_LEN, "%s", hw_info->name);
		malidp_intf->adf_type = MALIDP_INTF_MEMORY;
		malidp_intf->adf_flags = ADF_INTF_FLAG_EXTERNAL;
		malidp_intf->idx = hw_info->idx;

		/* Create a sync timeline for this interface */
		malidp_intf->mw_timeline =
			sw_sync_timeline_create(malidp_intf->hw_info->name);
		if (!malidp_intf->mw_timeline) {
			dev_err(dp_dev->device,
				"could not create timeline for interface %s",
				malidp_intf->hw_info->name);
			return -ENOMEM;
		}
		malidp_intf->mw_timeline_max = 0;

		malidp_hw_set_callback(dp_dev->hw_dev, malidp_intf->hw_info,
				       &malidp_adf_intf_flip_se_cb,
				       malidp_intf);
		break;
	default:
		BUG();
	}

	return 0;
}

/*
 * Add an interface to the adf_device. Allocates an interface with the
 * parameters specified in info, and initialises it on the given adf
 * device.
 *
 * @dp_dev The adf device which the interface will belong to
 * @hw_info Describes the interface to be instantiated
 * @tx (optional) A transmitter which should be associated with the interface,
 *     or NULL if not required.
 *
 * Returns the adf object id if successful, an error code (<0) on error.
 */
static int malidp_add_interface(struct malidp_device *dp_dev,
		const struct malidp_intf_hw_info *hw_info,
		struct video_tx_device *tx)
{
	int ret;
	struct adf_device *dev = &dp_dev->adf_dev;
	struct malidp_interface *malidp_intf;
	struct adf_interface *adf_intf;

	malidp_intf = kzalloc(sizeof(struct malidp_interface), GFP_KERNEL);
	if (NULL == malidp_intf)
		return -ENOMEM;
	adf_intf = &malidp_intf->intf;

	ret = malidp_interface_init(dp_dev, malidp_intf, hw_info, tx);
	if (ret < 0)
		goto fail;

	ret = adf_interface_init(&malidp_intf->intf, dev,
			malidp_intf->adf_type, malidp_intf->idx,
			malidp_intf->adf_flags, malidp_intf->ops,
			"%s", malidp_intf->name);
	if (ret)
		goto fail;

	if (hw_info->type == MALIDP_HW_INTF_PRIMARY &&
		video_tx_detect(tx) == connector_status_connected)
		malidp_adf_notifier_fn(&malidp_intf->nb,
					connector_status_connected,
					tx);

	return malidp_intf->intf.base.id;

fail:
	kfree(malidp_intf);
	return ret;
}

int malidp_adf_intf_add_interfaces(struct malidp_device *dp_dev,
		const struct malidp_intf_hw_info *interfaces, u32 n_interfaces,
		struct video_tx_device *tx)
{
	int i, ret = 0;

	for (i = 0; i < n_interfaces; i++) {
		const struct malidp_intf_hw_info *hw_info = &interfaces[i];
		switch (hw_info->type) {
		case MALIDP_HW_INTF_PRIMARY:
			ret = malidp_add_interface(dp_dev, hw_info, tx);
			break;
		case MALIDP_HW_INTF_MEMORY:
			ret = malidp_add_interface(dp_dev, hw_info, NULL);
			break;
		default:
			BUG();
		}
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int malidp_intf_destroy_cb(int id, void *p, void *opaque)
{
	struct adf_interface *intf = (struct adf_interface *)p;
	struct malidp_interface *malidp_intf =
			to_malidp_interface(intf);

	dev_dbg(&intf->base.dev, "%s : Destroying interface %s-%i\n",
			__func__, adf_interface_type_str(intf),
			intf->idx);

	adf_interface_blank(intf, DRM_MODE_DPMS_OFF);

	if (malidp_adf_intf_get_dp_type(intf) == MALIDP_HW_INTF_MEMORY) {
		/*
		* sw_sync implementation should export something to destroy
		* a sw fence instead.
		*/
		sync_timeline_destroy(&malidp_intf->mw_timeline->obj);
	} else
		video_tx_hotplug_notifier_unregister(malidp_intf->tx,
			&malidp_intf->nb);

	adf_interface_destroy(intf);
	kfree(malidp_intf);

	return 0;
}

int malidp_adf_intf_destroy_interfaces(struct malidp_device *dp_dev)
{
	struct adf_device *adf_dev = &dp_dev->adf_dev;

	return idr_for_each(&adf_dev->interfaces, malidp_intf_destroy_cb,
			NULL);
}

static int is_interface_primary(int id, void *p, void *data)
{
	struct adf_interface *intf = (struct adf_interface *)p;

	if (malidp_adf_intf_get_dp_type(intf) == MALIDP_HW_INTF_PRIMARY) {
		*((struct adf_interface **)data) = intf;
		return 1;
	}
	return 0;
}

static struct adf_interface *find_primary_intf(struct malidp_device *dev)
{
	struct adf_interface *intf = NULL;
	idr_for_each(&dev->adf_dev.interfaces, is_interface_primary, &intf);
	return intf;
}

void malidp_adf_intf_restore_drmmode(struct malidp_device *dev)
{
	struct adf_interface *intf = find_primary_intf(dev);

	if (malidp_adf_intf_validate_mode(dev, &intf->current_mode) == true)
		malidp_hw_modeset(dev->hw_dev, &intf->current_mode);
}
