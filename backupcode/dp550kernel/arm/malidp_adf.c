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



#include <linux/device.h>
#include <linux/slab.h>
#include <video/adf.h>
#include <sw_sync.h>

#include <uapi/video/malidp_adf.h>

#include "malidp_adf.h"
#include "malidp_adf_format.h"
#include "malidp_hw.h"
#include "malidp_iommu.h"

/*
 * Fuzzyness applied to the common scaling factor to deal with cases where
 * some dimensions are not divisible.
 * The value has been calculated for the worst possible scaling factor, which
 * is 7.5 (8 was used to make it more simple).
 *
 */
#define SF_FRACT_FUZZYNESS 0x00006189 /* 0.381 in 16,16 binary fixed point */

static int malidp_allow_attachments(struct malidp_device *dp_dev);
static const struct adf_device_ops *malidp_get_dev_ops(
		struct malidp_device *dp_dev);
static void malidp_adf_state_free(struct adf_device *dev, void *driver_state);

static void malidp_dump_malidp_buffer_config(struct malidp_device *dp_dev,
		struct malidp_buffer_config *buf)
{
	dev_dbg(dp_dev->device, "malidp_buffer_config:\n");
	dev_dbg(dp_dev->device, "  adf_buffer_index: %i\n",
		buf->adf_buffer_index);
	dev_dbg(dp_dev->device, "  adf_intf_id: %i\n", buf->adf_intf_id);
	dev_dbg(dp_dev->device, "  display rect: (%i, %i) %ix%i\n",
			buf->display_left, buf->display_top,
			buf->display_width, buf->display_height);
	dev_dbg(dp_dev->device, "  transform: %i\n", buf->transform);
	dev_dbg(dp_dev->device, "  flags: 0x%08x\n", buf->flags);
	dev_dbg(dp_dev->device, "  alpha_mode: %i\n", buf->alpha_mode);
	dev_dbg(dp_dev->device, "  layer_alpha: %i\n", buf->layer_alpha);
}

static void malidp_dump_adf_buffer(struct malidp_device *dp_dev,
		struct adf_buffer *buf)
{
	int i;

	dev_dbg(dp_dev->device, "adf_buffer:\n");
	dev_dbg(dp_dev->device, "  overlay_engine: %i (%s)\n",
			buf->overlay_engine->base.id,
			buf->overlay_engine->base.name);
	dev_dbg(dp_dev->device, "  size: %ix%i\n", buf->w, buf->h);
	dev_dbg(dp_dev->device, "  format: 0x%08x\n", buf->format);
	dev_dbg(dp_dev->device, "  n_planes: %i\n", buf->n_planes);

	dev_dbg(dp_dev->device, "  dma_bufs:");
	for (i = 0; i < buf->n_planes; i++) {
		dev_dbg(dp_dev->device, "    %p", buf->dma_bufs[i]);
	}

	dev_dbg(dp_dev->device, "  offset: ");
	for (i = 0; i < buf->n_planes; i++) {
		dev_dbg(dp_dev->device, "    %i", buf->offset[i]);
	}

	dev_dbg(dp_dev->device, "  pitch: ");
	for (i = 0; i < buf->n_planes; i++) {
		dev_dbg(dp_dev->device, "    %i", buf->pitch[i]);
	}

	dev_dbg(dp_dev->device, "  acquire_fence: %p\n", buf->acquire_fence);
}

/*
 * These cleanup functions are copied from ADF core so that we can use
 * them here. Some modifications have been made so that if we cleanup
 * within our driver the ADF framework won't clean up again.
 */
static void malidp_adf_buffer_cleanup(struct adf_buffer *buf)
{
	size_t i;
	for (i = 0; i < ARRAY_SIZE(buf->dma_bufs); i++)
		if (buf->dma_bufs[i]) {
			dma_buf_put(buf->dma_bufs[i]);
			buf->dma_bufs[i] = NULL;
		}

	if (buf->acquire_fence) {
		sync_fence_put(buf->acquire_fence);
		buf->acquire_fence = NULL;
	}
}

static void malidp_adf_buffer_mapping_cleanup(struct adf_buffer_mapping *mapping,
		struct adf_buffer *buf, enum dma_data_direction dir)
{
	/*
	 * calling adf_buffer_mapping_cleanup() is safe even if mapping is
	 * uninitialized or partially-initialized, as long as it was
	 * zeroed on allocation
	 */
	size_t i;

	for (i = 0; i < ARRAY_SIZE(mapping->sg_tables); i++) {
		if (mapping->sg_tables[i]) {
			dma_buf_unmap_attachment(mapping->attachments[i],
					mapping->sg_tables[i], dir);
			mapping->sg_tables[i] = NULL;
		}
		if (mapping->attachments[i]) {
			dma_buf_detach(buf->dma_bufs[i],
					mapping->attachments[i]);
			mapping->attachments[i] = NULL;
		}
	}
}

static void malidp_unmap_all_buffers(struct adf_device *dev,
		struct adf_pending_post *post)
{
	struct malidp_driver_state *state = post->state;
	struct malidp_device *dp_dev = to_malidp_device(dev);
	int output_buffer = -1;
	size_t i, j;

	if (state->mw_cfg)
		output_buffer = state->mw_cfg->mw_buf_index;

	for (i = 0; i < post->config.n_bufs; i++) {
		struct adf_buffer *buf = &post->config.bufs[i];
		enum dma_data_direction dir;

		if (i == output_buffer)
			dir = DMA_FROM_DEVICE;
		else
			dir = DMA_TO_DEVICE;

		if (dp_dev->iommu_domain) {
			for (j = 0; j < buf->n_planes; j++) {
				struct sg_table *sgt =
					post->config.mappings[i].sg_tables[j];
				if (sgt) {
					dev_dbg(dp_dev->device, "%s: unmap sgt = %p\n", __func__, sgt);
					malidp_iommu_unmap_sgt(dp_dev->iommu_domain,
						       state->iommu_maps[i][j]);
				}
			}
		}

		malidp_adf_buffer_mapping_cleanup(&post->config.mappings[i],
				&post->config.bufs[i], dir);
		malidp_adf_buffer_cleanup(&post->config.bufs[i]);
	}
}

void malidp_adf_post_cleanup(struct adf_device *dev,
		struct adf_pending_post *post)
{
	BUG_ON(!post->state);

	dev->ops->state_free(dev, post->state);

	kfree(post->config.custom_data);
	kfree(post->config.mappings);
	kfree(post->config.bufs);
	kfree(post);
}

static void malidp_adf_post(struct adf_device *dev, struct adf_post *cfg,
		void *driver_state)
{
	struct malidp_device *dp_dev = to_malidp_device(dev);
	struct malidp_driver_state *state = driver_state;
	u8 primary_dpms_state = DRM_MODE_DPMS_ON;
	int res;
	int i;

	dev_dbg(dp_dev->device, "%s : %s post\n", __func__, dev->base.name);

	for (i = 0; i < state->n_intfs; i++) {
		malidp_intf_prepare(state->intf_list[i], state->mw_cfg);
		/*
		 * Retrieve the state of the primary interface. This field
		 * doesn't need to be protected because the blank()
		 * implementation always flushes the worker thread for both
		 * blank and unblank.
		 */
		if (malidp_adf_intf_get_dp_type(state->intf_list[i]) ==
		    MALIDP_HW_INTF_PRIMARY)
			primary_dpms_state = state->intf_list[i]->dpms_state;
	}

	/*
	 * If the interface is off we have to just drop the frame.
	 * We will commit the most recent one on un-blank
	 */
	if (primary_dpms_state == DRM_MODE_DPMS_ON) {
		res = malidp_hw_commit(dp_dev->hw_dev, &state->hw_state);
		if (res < 0) {
			dev_err(dp_dev->device, "%s : post failed\n", __func__);
			return;
		}

		for (i = 0; i < state->n_intfs; i++)
			malidp_intf_wait(state->intf_list[i], dp_dev);
	}
}

static void expand_smart_layer_bbox(struct malidp_hw_smart_layer_state *ls_state,
		const struct malidp_hw_buffer *hw_buf)
{
	u16 top    = hw_buf->v_offset;
	u16 left   = hw_buf->h_offset;
	u16 bottom = top + hw_buf->cmp_rect.src_h;
	u16 right  = left + hw_buf->cmp_rect.src_w;

	if (ls_state->ls_bbox_top > top)
		ls_state->ls_bbox_top = top;

	if (ls_state->ls_bbox_left > left)
		ls_state->ls_bbox_left = left;

	if (ls_state->ls_bbox_bottom < bottom)
		ls_state->ls_bbox_bottom = bottom;

	if (ls_state->ls_bbox_right < right)
		ls_state->ls_bbox_right = right;
}

static int malidp_adf_dp_buf_to_hw_buf(struct malidp_device *dp_dev,
		struct malidp_buffer_config *dp_buf, struct malidp_hw_buffer *hw_buf,
		struct malidp_hw_smart_layer_state *ls_state,
		int verify)
{
	if (verify) {

		/* Ignore the special buffer used to set the bbox */
		if (dp_buf->flags == MALIDP_FLAG_SMART_BBOX) {
			return 0;
		}

		if (dp_buf->flags != hw_buf->flags) {
			dev_err(dp_dev->device, "%s : Buffer flags do not match, 0x%08x != 0x%08x\n",
					__func__, dp_buf->flags, hw_buf->flags);
			return -EINVAL;
		}

		if (dp_buf->transform != hw_buf->transform) {
			dev_err(dp_dev->device, "%s : Buffer transforms do not match, 0x%08x != 0x%08x\n",
					__func__, dp_buf->transform, hw_buf->transform);
			return -EINVAL;
		}

		if (hw_buf->flags & MALIDP_FLAG_AFBC) {
			if ((hw_buf->afbc_crop_l != dp_buf->afbc_crop_l) ||
				(hw_buf->afbc_crop_r != dp_buf->afbc_crop_r) ||
				(hw_buf->afbc_crop_t != dp_buf->afbc_crop_t) ||
				(hw_buf->afbc_crop_b != dp_buf->afbc_crop_b)) {
				dev_err(dp_dev->device, "%s: DP buffers disagree about AFBC crop\n",
						__func__);
				return -EINVAL;
			}
		}

		if (hw_buf->alpha_mode != dp_buf->alpha_mode) {
			dev_err(dp_dev->device, "%s: DP buffers disagree about alpha mode\n",
					__func__);
			return -EINVAL;
		}

		if (hw_buf->layer_alpha != dp_buf->layer_alpha) {
			dev_err(dp_dev->device, "%s: DP buffers disagree about layer alpha\n",
					__func__);
			return -EINVAL;
		}

	} else {
		hw_buf->flags = dp_buf->flags;
		hw_buf->transform = dp_buf->transform;
		if (dp_buf->transform != MALIDP_TRANSFORM_R0)
			hw_buf->requirements |= MALIDP_LAYER_FEATURE_TRANSFORM;

		if (dp_buf->flags & MALIDP_FLAG_AFBC) {
			hw_buf->afbc_crop_l = dp_buf->afbc_crop_l;
			hw_buf->afbc_crop_r = dp_buf->afbc_crop_r;
			hw_buf->afbc_crop_t = dp_buf->afbc_crop_t;
			hw_buf->afbc_crop_b = dp_buf->afbc_crop_b;
			hw_buf->requirements |= MALIDP_LAYER_FEATURE_AFBC;
		}

		hw_buf->alpha_mode = dp_buf->alpha_mode;
		hw_buf->layer_alpha = dp_buf->layer_alpha;

		hw_buf->h_offset = dp_buf->display_left;
		hw_buf->v_offset = dp_buf->display_top;

		if (hw_buf->hw_layer->type == MALIDP_HW_LAYER_SMART) {
			hw_buf->ls_rect_idx = ls_state->n_smart_layers;
			ls_state->n_smart_layers++;

			if (!ls_state->ls_bbox_from_user)
				expand_smart_layer_bbox(ls_state, hw_buf);

			ls_state->ls_hw_layer = hw_buf->hw_layer;
		}
	}

	return 0;
}

static void malidp_adf_remap_mw_buf(struct adf_buffer_mapping *map,
		struct adf_buffer *adf_buf, struct malidp_hw_buffer *hw_buf)
{
	int i;

	for (i = 0; i < adf_buf->n_planes; i++) {
		dma_buf_unmap_attachment(map->attachments[i],
			map->sg_tables[i], DMA_TO_DEVICE);
		map->sg_tables[i] = dma_buf_map_attachment(map->attachments[i],
					DMA_FROM_DEVICE);
	}
	for (i = 0; i < adf_buf->n_planes; i++) {
		hw_buf->addr[i] = sg_dma_address(map->sg_tables[i]->sgl) + adf_buf->offset[i];
		hw_buf->pitch[i] = adf_buf->pitch[i];
	}
}

struct malidp_adf_transposed_config {
	int *n_dp_bufs;
	struct malidp_buffer_config ***buf_lists;
};

/* We get the scaling factor in a binary fixed point 16,16 representation */
static inline u32 get_fp_sf(u16 src, u16 dst)
{
	u32 tmp = dst << 16;

	return (tmp / src);
}

/* Is fp1 greater than fp2 ? */
static inline bool check_fp_gt(u32 fp1, u32 fp2)
{
	if (fp1 > fp2)
		return true;
	else
		return false;
}

static inline u32 get_fp_diff(u32 big, u32 small)
{
	return (big - small);
}

static inline u32 fp_get_integer(u32 fp)
{
	return (fp >> 16);
}

static inline u32 fp_get_fractional(u32 fp)
{
	return (fp & 0xffff);
}

static bool sf_compare_fuzzyness(u32 sf1, u32 sf2)
{
	u32 diff;

	/* Find the difference between the highest and the smallest sf */
	if (check_fp_gt(sf2, sf1))
		diff = get_fp_diff(sf2, sf1);
	else
		diff = get_fp_diff(sf1, sf2);

	if (check_fp_gt(SF_FRACT_FUZZYNESS, diff))
		return true;
	else
		return false;
}

static bool check_scaling_in_dp_buf(struct malidp_device *dp_dev,
				    struct malidp_buffer_config **dp_bufs,
				    int n_dp_bufs, u32 csf_w,
				    u32 csf_h)
{
	int i;
	int dp_buf_disp = -1, dp_buf_mem = -1;
	enum malidp_hw_intf_type dp_type;
	u32 tmp_sf;

	/*
	 * Every buffer has to have 2 DP buffers, one must target the display
	 * and the other one the memory. The top and left values need to be
	 * scaled by the same scaling factor as the width and the height.
	 */

	if (n_dp_bufs != 2)
		return false;

	for (i = 0; i < n_dp_bufs; i++) {
		struct adf_interface *adf_intf;

		adf_intf = idr_find(&dp_dev->adf_dev.interfaces,
				    dp_bufs[i]->adf_intf_id);
		dp_type = malidp_adf_intf_get_dp_type(adf_intf);
		if (dp_type == MALIDP_HW_INTF_MEMORY)
			dp_buf_mem = i;
		else
			dp_buf_disp = i;
	}

	/*
	 * Something very wrong has happened if we don't get a memory and
	 * a display buffer at this point.
	 */
	BUG_ON(dp_buf_mem == -1 ||  dp_buf_disp == -1);

	if ((dp_bufs[dp_buf_disp]->display_top == 0) &&
	    (dp_bufs[dp_buf_mem]->display_top != 0)) {
		return false;
	} else if (dp_bufs[dp_buf_disp]->display_top != 0) {
		tmp_sf = get_fp_sf(dp_bufs[dp_buf_disp]->display_top,
					dp_bufs[dp_buf_mem]->display_top);
		if (!sf_compare_fuzzyness(tmp_sf, csf_h))
			return false;
	}

	if ((dp_bufs[dp_buf_disp]->display_left == 0) &&
	    (dp_bufs[dp_buf_mem]->display_left != 0)) {
		return false;
	} else if (dp_bufs[dp_buf_disp]->display_left != 0) {
		tmp_sf = get_fp_sf(dp_bufs[dp_buf_disp]->display_left,
					dp_bufs[dp_buf_mem]->display_left);
		if (!sf_compare_fuzzyness(tmp_sf, csf_w))
			return false;
	}

	return true;
}

static bool composition_is_scaled(struct malidp_device *dp_dev,
			struct malidp_hw_buffer *hw_bufs,
			struct malidp_adf_transposed_config *transpose,
			int nbufs, int output_buffer, u32 mode_h,
			u32 mode_w)
{
	struct malidp_hw_buffer *hw_buf;
	/* common scaling factor (csf) */
	u32 csf_w_max = 0, csf_h_max = 0;
	u32 csf_w_min = 0, csf_h_min = 0;
	u32 tmp_csf_h, tmp_csf_w;
	bool first_sf = true;
	int i;

	/*
	 * Make sure all the layers need to be scaled to memory and not to
	 * display, and they share the same scaling factor.
	 *
	 * In this loop we only get the minimum and the maximum.
	 */
	for (i = 0; i < nbufs; i++) {
		hw_buf = &hw_bufs[i];

		if (i == output_buffer)
			continue;

		if (hw_buf->mw_scaling_enable && !hw_buf->cmp_scaling_enable) {
			tmp_csf_h = get_fp_sf(hw_buf->mw_rect.src_h,
					    hw_buf->mw_rect.dest_h);
			tmp_csf_w = get_fp_sf(hw_buf->mw_rect.src_w,
					    hw_buf->mw_rect.dest_w);

			if (first_sf) {
				csf_w_max = tmp_csf_w;
				csf_w_min = tmp_csf_w;
				csf_h_max = tmp_csf_h;
				csf_h_min = tmp_csf_h;
			} else {
				if (check_fp_gt(tmp_csf_w, csf_w_max))
					csf_w_max = tmp_csf_w;
				if (check_fp_gt(csf_w_min, tmp_csf_w))
					csf_w_min = tmp_csf_w;
				if (check_fp_gt(tmp_csf_h, csf_h_max))
					csf_h_max = tmp_csf_h;
				if (check_fp_gt(csf_h_min, tmp_csf_h))
					csf_h_min = tmp_csf_h;
			}

			if (!check_scaling_in_dp_buf(dp_dev,
			      transpose->buf_lists[i], transpose->n_dp_bufs[i],
			      tmp_csf_w, tmp_csf_h))
				return false;

			first_sf = false;
		} else {
			return false;
		}
	}

	/* Check the fuzzyness of the max and min sf for each dimensions */
	if (!sf_compare_fuzzyness(csf_w_max, csf_w_min))
		return false;

	if (!sf_compare_fuzzyness(csf_h_max, csf_h_min))
		return false;

	/*
	 * We still have to validate that the output buffer has the dimensions
	 * suitable to fit the scaled composition.
	 */
	BUG_ON(output_buffer == -1);

	tmp_csf_w = get_fp_sf(mode_w, hw_bufs[output_buffer].natural_w);
	if (!sf_compare_fuzzyness(tmp_csf_w, csf_w_max) &&
	    !sf_compare_fuzzyness(tmp_csf_w, csf_w_min))
		return false;

	tmp_csf_h = get_fp_sf(mode_h, hw_bufs[output_buffer].natural_h);
	if (!sf_compare_fuzzyness(tmp_csf_h, csf_h_max) &&
	    !sf_compare_fuzzyness(tmp_csf_h, csf_h_min))
		return false;

	dev_dbg(dp_dev->device,
		"found scaling factors: w: 0x%04x.%04x, h: 0x%04x.%04x\n",
		fp_get_integer(csf_w_max), fp_get_fractional(csf_w_max),
		fp_get_integer(csf_h_max), fp_get_fractional(csf_h_max));

	return true;
}

static bool composition_is_written_out(struct malidp_hw_buffer *hw_bufs,
				     int nbufs, int output_buffer, u32 mode_h,
				     u32 mode_w)
{
	bool write_out_composition = false;
	int i;

	/*
	 * If only 2 ADF buffers are used, one input buffer and one output
	 * buffer, we need to tell the difference between writing-out/scaling
	 * the result of the composition and writing-out/scaling only the
	 * input buffer by comparing the display sizes:
	 *  - the sizes do not match -> write the result of the composition
	 *  - the sizes match the mode -> write the result of the composition
	 *  - the sizes match (but not the mode) -> write only the layer
	 *    matching the input buffer
	 */
	if ((nbufs == 2) && (output_buffer != -1)) {
		struct malidp_hw_buffer *out_hw_buf = &hw_bufs[output_buffer];
		struct malidp_hw_buffer *in_hw_buf = NULL;
		int input_buffer;
		write_out_composition = false;

		input_buffer = !output_buffer;

		in_hw_buf = &hw_bufs[input_buffer];

		/* Buffer sizes don't match */
		if ((in_hw_buf->mw_rect.dest_w != out_hw_buf->natural_w) ||
		    (in_hw_buf->mw_rect.dest_h != out_hw_buf->natural_h)) {
			write_out_composition = true;

		/* Buffer sizes match the mode */
		} else if ((in_hw_buf->mw_rect.dest_w == out_hw_buf->natural_w) &&
				(in_hw_buf->mw_rect.dest_h == out_hw_buf->natural_h) &&
				(out_hw_buf->cmp_rect.src_w == mode_w) &&
				(out_hw_buf->cmp_rect.src_h == mode_h)) {
			write_out_composition = true;
		}
	} else if (output_buffer != -1) {
		struct malidp_hw_buffer *hw_buf;
		write_out_composition = true;

		for (i = 0; i < nbufs; i++) {
			hw_buf = &hw_bufs[i];
			if (!(hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) &&
			    !(hw_buf->write_out_enable))
				write_out_composition = false;
		}
	}

	return write_out_composition;
}

static bool check_rect_sizes(struct malidp_hw_scale_rect *rect)
{
	if (rect->src_w == 0 || rect->src_h == 0 || rect->dest_w == 0 ||
	    rect->dest_h == 0)
		return false;

	return true;
}

static void reset_smart_layer_state(struct malidp_hw_smart_layer_state *ls_state)
{
	memset(ls_state, 0, sizeof(struct malidp_hw_smart_layer_state));
	ls_state->ls_bbox_top  = USHRT_MAX;
	ls_state->ls_bbox_left = USHRT_MAX;
}

static int malidp_adf_build_hw_bufs(struct malidp_device *dp_dev,
		struct adf_post *cfg, struct malidp_adf_transposed_config *transpose,
		struct malidp_driver_state *state)
{
	struct malidp_hw_buffer *hw_bufs = state->hw_state.bufs;
	struct malidp_hw_smart_layer_state *ls_state = &state->hw_state.ls_state;
	enum malidp_hw_intf_type dp_type;
	struct malidp_buffer_config *dp_buf;
	struct malidp_buffer_config **list;
	struct malidp_hw_buffer *hw_buf;
	struct adf_interface *adf_intf;
	struct adf_buffer *adf_buf;
	struct adf_buffer_mapping *map;
	u32 mode_w = 0;
	u32 mode_h = 0;
	int output_buffer = -1;
	int i, j = 0;
	int res = -EINVAL;

	/*
	 * We make no guarantees that this configuration is possible,
	 * we just set the parameters userspace asked for.
	 * hw_validate will make the decision about whether we can
	 * fulfil it
	 */

	/* Reset the smart layer state */
	reset_smart_layer_state(ls_state);

	/* First do the common things for each ADF buffer */
	for (i = 0; i < cfg->n_bufs; i++) {
		list = transpose->buf_lists[i];
		adf_buf = &cfg->bufs[i];
		map = &cfg->mappings[i];
		hw_buf = &hw_bufs[i];

		malidp_dump_adf_buffer(dp_dev, adf_buf);

		hw_buf->fmt = adf_buf->format;
		hw_buf->n_planes = adf_buf->n_planes;

		/* These may be ignored/overriden for output buffers */
		hw_buf->hw_layer = malidp_adf_ovr_get_hw_layer(adf_buf->overlay_engine);
		if (!hw_buf->hw_layer) {
			dev_err(dp_dev->device, "%s : Couldn't get hw layer for ADF buffer %i\n",
					__func__, i);
			res = -EINVAL;
			goto error;
		}

		hw_buf->natural_w = adf_buf->w;
		hw_buf->natural_h = adf_buf->h;

		/* The source size for the scaling rectangles is always the
		 * size of the buffers in memory.
		 */
		hw_buf->cmp_rect.src_w = adf_buf->w;
		hw_buf->cmp_rect.src_h = adf_buf->h;
		hw_buf->mw_rect.src_w = adf_buf->w;
		hw_buf->mw_rect.src_h = adf_buf->h;

		/* Default values for the destination rectangles. The DP buffer
		 * is not yet available here.
		 */
		hw_buf->cmp_rect.dest_w = adf_buf->w;
		hw_buf->cmp_rect.dest_h = adf_buf->h;
		hw_buf->mw_rect.dest_w = adf_buf->w;
		hw_buf->mw_rect.dest_h = adf_buf->h;

		/* Normally, an ADF has less than 3 DP buffers */
		if (transpose->n_dp_bufs[i] > 2) {
			for (res = 0; res < transpose->n_dp_bufs[i]; res++)
				if (list[res]->flags ==
						MALIDP_FLAG_SMART_BBOX) {
					res = 0;
					break;
				}
			/* For bindingbox, it can have 3 DP buffers */
			if (res || transpose->n_dp_bufs[i] > 3) {
				dev_err(dp_dev->device, "%s : A maximum of %d DP buffers are allowed for this ADF buffer\n",
						__func__, (res == 0) ? 3 : 2);
				res = -EINVAL;
				goto error;
			}
		}

		/* Then combine the list of DP bufs into a single hw buf */
		for (j = 0; j < transpose->n_dp_bufs[i]; j++) {
			dp_buf = list[j];

			malidp_dump_malidp_buffer_config(dp_dev, dp_buf);

			if (dp_buf->flags == MALIDP_FLAG_SMART_BBOX) {
				/* These fields of dp_buf are used for bbox test */
				ls_state->ls_bbox_argb = dp_buf->alpha_mode;
				if (dp_buf->display_top > 0 ||
				    dp_buf->display_left > 0 ||
				    dp_buf->display_height > 0 ||
				    dp_buf->display_width > 0) {
					ls_state->ls_bbox_top = dp_buf->display_top;
					ls_state->ls_bbox_left = dp_buf->display_left;
					ls_state->ls_bbox_bottom = dp_buf->display_top + dp_buf->display_height;
					ls_state->ls_bbox_right = dp_buf->display_left + dp_buf->display_width;
					ls_state->ls_bbox_from_user = true;
				}
				continue;
			}

			/*
			 * If this is the first time we hit this ADF buffer, then
			 * translate the dp_buf to a hw_buf.
			 * If not, verify that this dp_buf is compatible with the
			 * other one
			 */
			res = malidp_adf_dp_buf_to_hw_buf(dp_dev, dp_buf, hw_buf, ls_state, j);
			if (res)
				goto error;

			if (hw_buf->hw_layer->type == MALIDP_HW_LAYER_SMART)
				ls_state->ls_hw_buf_idx[hw_buf->ls_rect_idx] = i;

			adf_intf = idr_find(&dp_dev->adf_dev.interfaces, dp_buf->adf_intf_id);
			dp_type = malidp_adf_intf_get_dp_type(adf_intf);

			switch (dp_type) {
			case MALIDP_HW_INTF_MEMORY:
			{
				if (hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {

					if (transpose->n_dp_bufs[i] > 1) {
						dev_err(dp_dev->device, "%s : More than one DP buffer uses the output ADF buffer\n",
								__func__);
						res = -EINVAL;
						goto error;
					}

					if (output_buffer >= 0) {
						dev_err(dp_dev->device, "%s : More than one output buffer specified\n", __func__);
						res = -EINVAL;
						goto error;
					}

					/*
					 * ADF will map all buffers using DMA_TO_DEVICE but
					 * output buffers need to be mapped as DMA_FROM_DEVICE.
					 */
					malidp_adf_remap_mw_buf(map, adf_buf, hw_buf);

					output_buffer = i;
					hw_buf->hw_layer = NULL;
				} else {
					hw_buf->write_out_enable = true;
					if (hw_buf->transform & MALIDP_TRANSFORM_R90) {
						hw_buf->mw_rect.src_w = hw_buf->natural_h;
						hw_buf->mw_rect.src_h = hw_buf->natural_w;
						hw_buf->mw_rect.dest_w = dp_buf->display_height;
						hw_buf->mw_rect.dest_h = dp_buf->display_width;
					} else {
						hw_buf->mw_rect.dest_w = dp_buf->display_width;
						hw_buf->mw_rect.dest_h = dp_buf->display_height;
					}
					if ((hw_buf->mw_rect.dest_h != hw_buf->mw_rect.src_h) ||
					    (hw_buf->mw_rect.dest_w != hw_buf->mw_rect.src_w)) {
						hw_buf->mw_scaling_enable = true;
						if (!check_rect_sizes(&hw_buf->mw_rect)) {
							dev_err(dp_dev->device, "wrong mw rect sizes\n");
							res = -EINVAL;
							goto error;
						}
					}
				}

				break;
			}
			case MALIDP_HW_INTF_PRIMARY:
			{
				if (hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {
					dev_err(dp_dev->device, "%s : Output buffers are only supported for the memory interface\n",
							__func__);
					res = -EINVAL;
					goto error;
				}

				if (j && !hw_buf->write_out_enable) {
					dev_err(dp_dev->device, "%s : Multiple DP buffer configs provided for ADF buffer %i, interface %i\n",
							__func__, i, dp_buf->adf_intf_id);
					res = -EINVAL;
					goto error;
				}

				if (hw_buf->transform & MALIDP_TRANSFORM_R90) {
					hw_buf->cmp_rect.src_w = hw_buf->natural_h;
					hw_buf->cmp_rect.src_h = hw_buf->natural_w;
					hw_buf->cmp_rect.dest_w = dp_buf->display_height;
					hw_buf->cmp_rect.dest_h = dp_buf->display_width;
				} else {
					hw_buf->cmp_rect.src_w = hw_buf->natural_w;
					hw_buf->cmp_rect.src_h = hw_buf->natural_h;
					hw_buf->cmp_rect.dest_w = dp_buf->display_width;
					hw_buf->cmp_rect.dest_h = dp_buf->display_height;
				}

				if ((hw_buf->cmp_rect.src_w != hw_buf->cmp_rect.dest_w) ||
						(hw_buf->cmp_rect.src_h != hw_buf->cmp_rect.dest_h)) {
					/*
					 * With the current HW, a layer that needs to be scaled
					 * for the composition will have to be scaled for the
					 * memory as well.
					 */
					hw_buf->cmp_scaling_enable = true;
					hw_buf->mw_scaling_enable = true;
					hw_buf->requirements |= MALIDP_LAYER_FEATURE_SCALING;
					hw_buf->mw_rect.dest_w = hw_buf->cmp_rect.dest_w;
					hw_buf->mw_rect.dest_h = hw_buf->cmp_rect.dest_h;
					if (!check_rect_sizes(&hw_buf->cmp_rect)) {
						dev_err(dp_dev->device, "wrong cmp rect sizes\n");
						res = -EINVAL;
						goto error;
					}
				}

				mode_w = adf_intf->current_mode.hdisplay;
				mode_h = adf_intf->current_mode.vdisplay;
				if (((hw_buf->cmp_rect.dest_w + hw_buf->h_offset) > mode_w) ||
						((hw_buf->cmp_rect.dest_h + hw_buf->v_offset) > mode_h)) {
					dev_err(dp_dev->device, "%s : Buffer falls outside of display region\n", __func__);
					res = -ENOSPC;
					goto error;
				}

				break;
			}
			default:
				BUG();
			}
		}

		for (j = 0; j < adf_buf->n_planes; j++) {
			if (dp_dev->iommu_domain) {
				enum dma_data_direction dir;
				if (output_buffer == i)
					dir = DMA_FROM_DEVICE;
				else
					dir = DMA_TO_DEVICE;

				state->iommu_maps[i][j] = malidp_iommu_map_sgt(dp_dev->iommu_domain,
									       map->sg_tables[j], dir);
				if (!state->iommu_maps[i][j]) {
					dev_err(dp_dev->device, "could not map sg table in the iommu");
					res = -ENOMEM;
					goto error;
				}
				hw_buf->addr[j] = malidp_iommu_dma_addr(dp_dev->iommu_domain,
					state->iommu_maps[i][j]) + adf_buf->offset[j];
			} else {
				hw_buf->addr[j] = sg_dma_address(map->sg_tables[j]->sgl) +
						adf_buf->offset[j];
			}
			hw_buf->pitch[j] = adf_buf->pitch[j];
		}
	}

	/*
	 * There is no output buffer. We cannot scale to memory or write out to
	 * memory.
	 */
	if (output_buffer == -1) {
		for (i = 0; i < cfg->n_bufs; i++) {
			if (hw_bufs[i].write_out_enable) {
				dev_err(dp_dev->device,
				  "cannot write out to memory with no output buffer");
				res = -EINVAL;
				goto error;
			}
		}
	} else {
		hw_bufs[output_buffer].mw_scaling_enable =
			composition_is_scaled(dp_dev, hw_bufs, transpose,
				cfg->n_bufs, output_buffer, mode_h, mode_w);
		if (hw_bufs[output_buffer].mw_scaling_enable) {
			/* If the composition is scaled the src dimensions
			 * always have to match the mode
			 */
			hw_bufs[output_buffer].mw_rect.src_h = mode_h;
			hw_bufs[output_buffer].mw_rect.src_w = mode_w;
		}

		hw_bufs[output_buffer].write_out_enable =
			composition_is_written_out(hw_bufs, cfg->n_bufs,
					output_buffer, mode_h, mode_w);

		/*
		 * Check that if a layer is written to memory it fits in the
		 * output buffer.
		 */
		if (!hw_bufs[output_buffer].write_out_enable) {
			u16 out_w = hw_bufs[output_buffer].natural_w;
			u16 out_h = hw_bufs[output_buffer].natural_h;
			for (i = 0; i < cfg->n_bufs; i++) {
				u16 in_dst_w = hw_bufs[i].mw_rect.dest_w;
				u16 in_dst_h = hw_bufs[i].mw_rect.dest_h;

				if (i == output_buffer)
					continue;

				if (hw_bufs[i].mw_scaling_enable)
					hw_bufs[i].requirements |=
						MALIDP_LAYER_FEATURE_SCALING;

				if (hw_bufs[i].write_out_enable &&
					((in_dst_w != out_w) ||
					(in_dst_h != out_h))) {
					dev_err(dp_dev->device,
						"output buffer is too small");
					res = -EINVAL;
					goto error;
				}
			}
		}
	}

	/* Clipping the smart layers' v/h offset with the bbox */
	if (ls_state->ls_hw_layer) {
		for (i = 0; i < cfg->n_bufs; i++) {
			if (hw_bufs[i].hw_layer &&
			    hw_bufs[i].hw_layer->type == MALIDP_HW_LAYER_SMART) {
				if (hw_bufs[i].v_offset >= ls_state->ls_bbox_top)
					hw_bufs[i].v_offset -= ls_state->ls_bbox_top;
				else {
					dev_err(dp_dev->device, "%s : Invalid bbox top %d for active rect top %d\n",
						__func__, ls_state->ls_bbox_top, hw_bufs[i].v_offset);
					res = -EINVAL;
					goto error;
				}
				if (hw_bufs[i].h_offset >= ls_state->ls_bbox_left)
					hw_bufs[i].h_offset -= ls_state->ls_bbox_left;
				else {
					dev_err(dp_dev->device, "%s : Invalid bbox left %d for active rect left %d\n",
						__func__, ls_state->ls_bbox_left, hw_bufs[i].h_offset);
					res = -EINVAL;
					goto error;
				}
			}
		}
	}

	return 0;

error:
	if (dp_dev->iommu_domain) {
		for (i = 0; i < cfg->n_bufs; i++) {
			adf_buf = &cfg->bufs[i];
			for (j = 0; j < adf_buf->n_planes; j++)
				malidp_iommu_unmap_sgt(dp_dev->iommu_domain,
						state->iommu_maps[i][j]);
		}
	}
	dev_err(dp_dev->device, "%s : Validation failed for ADF buffer %i (DP buffer %i)\n",
			__func__, i, j);
	return res;
}

static int malidp_adf_non_afbc_validate(struct malidp_device *dp_dev,
					struct adf_buffer *adf_buf,
					struct malidp_buffer_config *dp_buf)
{
	int res = 0;
	uint32_t i;

	/* Pitch alignment matters for non-AFBC buffers */
	for (i = 0; i < adf_buf->n_planes; i++) {
		if (adf_buf->pitch[i] % 8) {
			dev_err(dp_dev->device, "%s : all buffer pitches must be 64-bit aligned\n", __func__);
			res = -EINVAL;
		}
	}

	if (dp_buf->flags & MALIDP_FLAG_AFBC_YTR) {
		dev_err(dp_dev->device, "%s : YTR is not valid for non-AFBC buffers\n",
			__func__);
		res = -EINVAL;
	}

	if (dp_buf->flags & MALIDP_FLAG_AFBC_SPLITBLK) {
		dev_err(dp_dev->device, "%s : Split block is not valid for non-AFBC buffers\n",
			__func__);
		res = -EINVAL;
	}

	if (malidp_adf_format_is_afbc_only(adf_buf->format)) {
		dev_err(dp_dev->device, "%s : format 0x%08x is only valid for AFBC buffers\n",
			__func__, adf_buf->format);
		res = -EINVAL;
	}

	return res;
}

static bool malidp_custom_data_size_valid(struct malidp_custom_data *blob,
		size_t blob_size)
{
	size_t expected_size = sizeof(struct malidp_custom_data);

	if (!blob)
		return false;

	/* First make sure there's space to access the other fields */
	if (blob_size < expected_size)
		return false;

	/* Add on space for the buffers */
	expected_size += blob->sizeof_malidp_buffer_config *
		blob->n_malidp_buffer_configs;

	return blob_size == expected_size;
}

static int malidp_adf_validate(struct adf_device *dev, struct adf_post *cfg,
			void **driver_state)
{
	struct malidp_device *dp_dev = to_malidp_device(dev);
	struct malidp_custom_data *blob =
		(struct malidp_custom_data *)cfg->custom_data;
	struct malidp_adf_transposed_config transpose = {0};
	struct adf_interface *intf;
	struct adf_buffer *adf_buf;
	struct malidp_driver_state *state;
	size_t state_size;
	uint32_t n_input_buffers = 0;
	int i, j;
	int res = 0;

	dev_dbg(dp_dev->device, "%s : %s validate\n", __func__,
			dev->base.name);

#ifdef CONFIG_PM_SLEEP
	if (atomic_read(&dp_dev->suspending)) {
		dev_dbg(dp_dev->device, "%s: Pending system suspend\n", __func__);
		return -EAGAIN;
	}
#endif /* CONFIG_PM_SLEEP */

	if (!malidp_custom_data_size_valid(blob, cfg->custom_data_size)) {
		dev_err(dp_dev->device, "%s : invalid custom data size: %zu\n",
			__func__, cfg->custom_data_size);
		dev_err(dp_dev->device, "%s : could be user/kernel header mismatch?\n",
			__func__);
		return -EINVAL;
	}

	if (cfg->n_bufs == 0) {
		dev_err(dp_dev->device, "%s: no ADF buffers found\n",
			__func__);
		return -EINVAL;
	}

	if (blob->n_malidp_buffer_configs == 0) {
		dev_err(dp_dev->device, "%s: no DP buffers found\n",
			__func__);
		return -EINVAL;
	}

	*driver_state = NULL;

	state_size = sizeof(*state) + sizeof(*state->hw_state.bufs) * cfg->n_bufs;
	state = kzalloc(state_size, GFP_KERNEL);
	if (!state)
		return -ENOMEM;
	state->hw_state.n_bufs = cfg->n_bufs;
	state->hw_state.bufs = (struct malidp_hw_buffer *)((u8 *)state + sizeof(*state));
	state->n_intfs = 0;

	transpose.n_dp_bufs = kzalloc(sizeof(*transpose.n_dp_bufs) * cfg->n_bufs, GFP_KERNEL);
	if (transpose.n_dp_bufs == NULL) {
		res = -ENOMEM;
		goto exit_state;
	}

	/* Independent checks per-buffer */
	for (i = 0; i < blob->n_malidp_buffer_configs; i++) {
		struct malidp_buffer_config *dp_buf = &blob->buffers[i];
		bool new_intf = true;

		/* Skip the special dp_buf for smart layer bounding box */
		if (dp_buf->flags == MALIDP_FLAG_SMART_BBOX)
			continue;

		intf = idr_find(&dev->interfaces, dp_buf->adf_intf_id);
		if (intf == NULL) {
			dev_err(dp_dev->device, "%s : invalid interface %i\n",
					__func__, dp_buf->adf_intf_id);
			res = -EINVAL;
			goto exit_n_bufs;
		}

		/* Construct the interfaces list */
		for (j = 0; j < state->n_intfs + 1; j++) {
			if (state->intf_list[j] == intf)
				new_intf = false;
		}
		if (new_intf) {
			state->intf_list[state->n_intfs] = intf;
			state->n_intfs++;
		}

		/* Basic field sanity checks */
		if (dp_buf->adf_buffer_index >= cfg->n_bufs) {
			dev_err(dp_dev->device,	"malidp_buf[%i] adf buffer index (%d) out of range\n",
				i, dp_buf->adf_buffer_index);
			res = -EINVAL;
			goto exit_n_bufs;
		}
		adf_buf = &cfg->bufs[dp_buf->adf_buffer_index];

		/* Is this an input buffer? */
		if (!(dp_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT))
			n_input_buffers++;

		/* Add one to the buffer count for this ADF buffer */
		transpose.n_dp_bufs[dp_buf->adf_buffer_index]++;

		/* Format/memory-layout Validation */
		if (dp_buf->flags & MALIDP_FLAG_AFBC) {
			res = malidp_adf_format_afbc_validate(dev, adf_buf,
							      dp_buf);
			if (res)
				goto exit_n_bufs;
		} else {
			res = malidp_adf_non_afbc_validate(dp_dev, adf_buf,
							   dp_buf);
			if (res)
				goto exit_n_bufs;
		}
	}

	/* Check for output buffer with no input content */
	if (!n_input_buffers && cfg->n_bufs) {
		dev_err(dp_dev->device, "%s : no input buffers provided, but write-out requested\n",
				__func__);
		res = -EINVAL;
		goto exit_n_bufs;
	}

	/* Pointers to lists of DP buffers per ADF buffer */
	transpose.buf_lists =
		kzalloc(sizeof(struct malidp_buffer_config **) * cfg->n_bufs, GFP_KERNEL);
	if (transpose.buf_lists == NULL) {
		res = -ENOMEM;
		goto exit_n_bufs;
	}

	/* The lists for each ADF buffer */
	for (i = 0; i < cfg->n_bufs; i++) {
		transpose.buf_lists[i] =
			kzalloc(sizeof(struct malidp_buffer_config *) * transpose.n_dp_bufs[i], GFP_KERNEL);
		if (transpose.buf_lists[i] == NULL) {
			res = -ENOMEM;
			goto exit_lists;
		}
		/* Reset the counter so we can use it for insertions */
		transpose.n_dp_bufs[i] = 0;
	}

	/* Finally populate the lists */
	for (i = 0; i < blob->n_malidp_buffer_configs; i++) {
		struct malidp_buffer_config *dp_buf = &blob->buffers[i];

		for (j = 0; j < cfg->n_bufs; j++) {

			if (j == dp_buf->adf_buffer_index) {
				int index = transpose.n_dp_bufs[j];

				transpose.buf_lists[j][index] = dp_buf;
				transpose.n_dp_bufs[j]++;
			}
		}
	}

	/* Generate HW buffers */
	res = malidp_adf_build_hw_bufs(dp_dev, cfg, &transpose, state);
	if (res)
		goto exit_lists;

	/* Check against hardware constraints */
	res = malidp_hw_validate(dp_dev->hw_dev, &state->hw_state);
	if (res) {
		dev_err(dp_dev->device, "%s : HW validate failed\n", __func__);
		goto exit_map;
	}

	/* Nothing wrong, so save state for later */
	*driver_state = state;

	/* Clean up temporal memory */
	for (i = 0; i < cfg->n_bufs; i++) {
		kfree(transpose.buf_lists[i]);
	}
	kfree(transpose.buf_lists);
	kfree(transpose.n_dp_bufs);

	return 0;

exit_map:
	if (dp_dev->iommu_domain && *driver_state == NULL) {
		for (i = 0; i < cfg->n_bufs; i++) {
			adf_buf = &cfg->bufs[i];
			for (j = 0; j < adf_buf->n_planes; j++)
				malidp_iommu_unmap_sgt(dp_dev->iommu_domain,
						state->iommu_maps[i][j]);
		}
	}
exit_lists:
	for (i = 0; i < cfg->n_bufs; i++) {
		kfree(transpose.buf_lists[i]);
	}
	kfree(transpose.buf_lists);
exit_n_bufs:
	kfree(transpose.n_dp_bufs);
exit_state:
	kfree(state);

	return res;
}

/*
 * Copied from adf_sw_complete_fence() and adf_sw_advance_timeline() in the ADF
 * core. Remove them if they are exported by the framework in the future.
 */

static struct sync_fence *malidp_adf_sw_complete_fence(struct adf_device *dev)
{
	struct sync_pt *pt;
	struct sync_fence *complete_fence;

	if (!dev->timeline) {
		dev->timeline = sw_sync_timeline_create(dev->base.name);
		if (!dev->timeline)
			return ERR_PTR(-ENOMEM);
		dev->timeline_max = 1;
	}

	dev->timeline_max++;
	pt = sw_sync_pt_create(dev->timeline, dev->timeline_max);
	if (!pt)
		goto err_pt_create;
	complete_fence = sync_fence_create(dev->base.name, pt);
	if (!complete_fence)
		goto err_fence_create;

	return complete_fence;

err_fence_create:
	sync_pt_free(pt);
err_pt_create:
	dev->timeline_max--;
	return ERR_PTR(-ENOSYS);
}

static void adf_sw_advance_timeline(struct adf_device *dev)
{
#ifdef CONFIG_SW_SYNC
	sw_sync_timeline_inc(dev->timeline, 1);
#else
	BUG();
#endif
}
/* End of sync functions copied from the ADF core */

static struct sync_fence *malidp_adf_complete_fence(struct adf_device *dev,
				struct adf_post *cfg, void *driver_state)
{
	struct malidp_custom_data *blob =
		(struct malidp_custom_data *)cfg->custom_data;
	struct malidp_driver_state *state = driver_state;
	int i;

	/*
	 * For output buffers we need to set up an additional fence, which is
	 * handled internally in the adf interface.
	 */
	for (i = 0; i < blob->n_malidp_buffer_configs; i++) {
		struct malidp_buffer_config *dp_buf = &blob->buffers[i];
		struct adf_buffer *buf = &cfg->bufs[dp_buf->adf_buffer_index];
		struct adf_buffer_mapping *map = &cfg->mappings[dp_buf->adf_buffer_index];

		if (dp_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {
			struct malidp_device *dp_dev = to_malidp_device(dev);
			struct adf_interface *adf_intf;

			adf_intf = idr_find(&dev->interfaces, dp_buf->adf_intf_id);
			state->mw_cfg = malidp_intf_create_mw_cfg(adf_intf,
				dp_dev, buf, dp_buf->adf_buffer_index, map,
				state->iommu_maps[dp_buf->adf_buffer_index]);
			if (!state->mw_cfg) {
				dev_err(dp_dev->device, "could not create mw_cfg");
				return ERR_PTR(-EINVAL);
			}
		}
	}

	return malidp_adf_sw_complete_fence(dev);
}

/* This has to be implemented so that ADF core does not complain */
static void malidp_adf_advance_timeline(struct adf_device *dev,
				struct adf_post *cfg, void *driver_state)
{
	return adf_sw_advance_timeline(dev);
}

static void malidp_adf_state_free(struct adf_device *dev, void *driver_state)
{
	struct malidp_device *dp_dev = to_malidp_device(dev);
	struct malidp_driver_state *state = driver_state;
	int i;

	if (state->mw_cfg) {
		for (i = 0; i < state->n_intfs; i++) {
			if (malidp_adf_intf_get_dp_type(state->intf_list[i]) == MALIDP_HW_INTF_MEMORY) {
				dev_dbg(dp_dev->device, "%s: clean output fence if needed\n", __func__);
				malidp_adf_intf_destroy_mw_cfg(state->intf_list[i],
							       state->mw_cfg);
			}
		}
	}

	/*
	 * Release the buffers uisng the right direction parameters for output
	 * buffers and unmapping the iommu maps if required.
	 */
	if (dev->onscreen)
		malidp_unmap_all_buffers(dev, dev->onscreen);

	malidp_hw_state_free(dp_dev->hw_dev, &state->hw_state);
	kfree(state);
}

static int malidp_adf_custom_data(struct adf_obj *obj, void *data, size_t *size)
{
	struct malidp_adf_device_custom_data custom_data;
	struct adf_device *adf_dev = adf_obj_to_device(obj);
	struct malidp_device *malidp_dev = to_malidp_device(adf_dev);
	const struct malidp_hw_topology *topology =
		malidp_hw_get_topology(malidp_dev->hw_dev);
	enum malidp_hw_partition_type type;
	uint32_t array_size;

	memset(&custom_data, 0, sizeof(custom_data));

	/* Obtain values from the device */

	/* Scaler information */
	custom_data.n_scalers = topology->n_scalers;

	/* AFBC information */
	BUG_ON(topology->n_supported_afbc_formats > MALIDP_MAX_N_FORMATS);

	array_size = sizeof(topology->supported_afbc_formats[0]) *
		topology->n_supported_afbc_formats;
	custom_data.n_supported_afbc_formats =
		topology->n_supported_afbc_formats;
	memcpy(&custom_data.supported_afbc_formats,
	       topology->supported_afbc_formats,
	       array_size);
	custom_data.supported_afbc_splitblk = topology->supported_afbc_splitblk;

	/* MW information */
	BUG_ON(topology->n_mw_formats > MALIDP_MAX_N_FORMATS);

	array_size = sizeof(topology->mw_formats[0]) * topology->n_mw_formats;
	custom_data.n_supported_mw_formats = topology->n_mw_formats;
	memcpy(&custom_data.supported_mw_formats, topology->mw_formats,
	       array_size);

	/* Rotation memory information */
	custom_data.rotation_memory_size =
		malidp_hw_rotmem_size_get(malidp_dev->hw_dev);
	type = malidp_hw_rotmem_type_get(malidp_dev->hw_dev);

	/* Min/max supported dimensions */
	malidp_hw_supported_dimensions_get(malidp_dev->hw_dev,
		&custom_data.min_width, &custom_data.min_height,
		&custom_data.max_width, &custom_data.max_height);

	/* Unsupported rotation formats */
	custom_data.n_xform_invalid_formats =
		topology->n_xform_invalid_formats;
	array_size = sizeof(topology->xform_invalid_formats[0]) *
		topology->n_xform_invalid_formats;
	memcpy(&custom_data.xform_invalid_formats,
	       topology->xform_invalid_formats,
	       array_size);

	/* Convert to user-facing value */
	switch (type) {
	case MALIDP_HW_PARTITION_FIXED: {
		custom_data.rotation_memory_strategy =
			MALIDP_ROTMEM_PARTITION_FIXED;
		break;
	};

	default:
		BUG();
	}

	memcpy(data, &custom_data, sizeof(custom_data));

	*size = sizeof(custom_data);

	return 0;
}

static const struct adf_device_ops malidp_adf_ops = {
	.owner = THIS_MODULE,
	.base = {
		.custom_data = malidp_adf_custom_data,
	},
	.quirks = {
		.buffer_padding = ADF_BUFFER_UNPADDED,
	},
	.validate_custom_format = malidp_adf_format_validate,
	.validate = malidp_adf_validate,
	.post = malidp_adf_post,
	.complete_fence = malidp_adf_complete_fence,
	.advance_timeline = malidp_adf_advance_timeline,
	.state_free = malidp_adf_state_free,
};

/*
 * Initialises (and allocates) the ADF parts of a malidp_device.
 *
 * @dp_dev The malidp_device which contains the adf_device to be initialised.
 * @hw_desc The description of the hardware for which ADF representations
 *          should be made
 * Returns 0 on success, an error (<0) on failure
 */
int malidp_adf_init(struct malidp_device *dp_dev,
		struct malidp_hw_description *hw_desc,
		struct video_tx_device *tx)
{
	const struct adf_device_ops *dev_ops;
	const struct malidp_hw_topology *topo = hw_desc->topology;
	int ret;

	dev_ops = malidp_get_dev_ops(dp_dev);
	if (NULL == dev_ops)
		return -EINVAL;

	ret = adf_device_init(&dp_dev->adf_dev, dp_dev->device, dev_ops,
			"%s%u", dp_dev->name, dp_dev->id);
	if (ret)
		return ret;

	ret = malidp_adf_ovr_add_layers(dp_dev, topo->layers, topo->n_layers);
	if (ret)
		goto destroy_device;

	ret = malidp_adf_intf_add_interfaces(dp_dev, topo->interfaces,
			topo->n_interfaces, tx);
	if (ret)
		goto destroy_device;

	ret = malidp_allow_attachments(dp_dev);
	if (ret)
		goto destroy_device;

	return ret;

destroy_device:
	malidp_adf_destroy(dp_dev);
	return ret;
}

static int attach_overlay_engine(int id, void *p, void *opaque)
{
	struct adf_interface *intf = (struct adf_interface *)opaque;
	struct adf_overlay_engine *eng = (struct adf_overlay_engine *)p;
	struct adf_device *dev = intf->base.parent;

	dev_dbg(&dev->base.dev, "Allowing attachment %s->%s\n", intf->base.name,
			eng->base.name);
	return adf_attachment_allow(dev, eng, intf);
}

static int attach_interface(int id, void *p, void *opaque)
{
	struct adf_interface *intf = (struct adf_interface *)p;
	struct adf_device *dev = intf->base.parent;
	return idr_for_each(&dev->overlay_engines, attach_overlay_engine,
				intf);
}

static int malidp_allow_attachments(struct malidp_device *dp_dev)
{
	struct adf_device *adf_dev = &dp_dev->adf_dev;

	/*
	 * For now we attach all interfaces to all overlays, so we will just
	 * iterate the interface and overlay lists and attach them all.
	 */
	return idr_for_each(&adf_dev->interfaces, attach_interface, NULL);
}

int malidp_adf_destroy(struct malidp_device *dp_dev)
{
	struct adf_device *adf_dev = &dp_dev->adf_dev;

	malidp_adf_intf_destroy_interfaces(dp_dev);
	malidp_adf_ovr_destroy_layers(dp_dev);

	adf_device_destroy(adf_dev);
	return 0;
}

static const struct adf_device_ops *malidp_get_dev_ops(
		struct malidp_device *dp_dev)
{
	/* LEOSW-410: Return operations appropriate for dp_dev->hwver */
	return &malidp_adf_ops;
}

int malidp_adf_runtime_resume(struct malidp_device *dp_dev)
{
	malidp_adf_intf_restore_drmmode(dp_dev);
	return 0;
}

void malidp_adf_cleanup_signaled_mw(struct malidp_driver_state *state)
{
	if (state->mw_cfg && state->mw_cfg->signaled == true)
		malidp_hw_clear_mw(&state->hw_state);
}

void malidp_adf_waiting_for_mw(struct malidp_driver_state *state)
{
	if (state->mw_cfg == NULL || state->mw_cfg->signaled == true)
		return;
	sync_fence_wait(state->mw_cfg->mw_fence, 250);
}
