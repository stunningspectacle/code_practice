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



#include <linux/clk.h>
#include <linux/of.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/pm_runtime.h>

#include <uapi/drm/drm_fourcc.h>
#include <uapi/video/malidp_adf.h>

#include "malidp_product_api.h"
#include "malidp_de_device.h"
#include "malidp_se_device.h"
#include "malidp_sysfs.h"

#define MALIDP_AFBC_ALIGN_MASK 0xf

#define MALIDP_MAX_EVENT_STRING 100

/* Global register offset definitions */
#define MALIDP_REG_DP500_CORE_ID	0x18
#define MALIDP_REG_CORE_ID		0xC018
#define		MALIDP_CFG_VALID		0x1
#define MALIDP_REG_PER_ID_4	0x00
#define MALIDP_REG_CONFIG_0 0x04
#define		MALIDP_CONFIG_0_GET_DISP(x)	(((x) >> 0) & 0x1)
#define		MALIDP_CONFIG_0_GET_LS(x)	(((x) >> 4) & 0x3)

#define MALIDP_DEFAULT_ROTMEM_SIZE 0

struct rotmem_partition {
	int dividers[3];
};

struct malidp_hw_event_queue {
	spinlock_t lock;
	size_t n_events;
	struct malidp_hw_event *queue;
	/*
	 * head == NULL when the queue is empty
	 * head == tail when the queue is full
	 */
	struct malidp_hw_event *head;
	struct malidp_hw_event *tail;
};

struct rotmem_partition fixed_partition_table[] = {
	/* 1 layer */
	{
		.dividers = { 1, 0, 0 }
	},
	/* 2 layers */
	{
		.dividers = { 2, 2, 0 }
	},
	/* 3 layers */
	{
		.dividers = { 2, 4, 4 }
	},
};

/* Start of HW description */

const struct malidp_intf_hw_info dp_interfaces[] = {
	{
		.name = "Panel",
		.type = MALIDP_HW_INTF_PRIMARY,
		.idx = 0,
	},
	{
		.name = "Memory",
		.type = MALIDP_HW_INTF_MEMORY,
		.idx = 0,
	}
};

bool malidp_hw_format_is_yuv(u32 format)
{
	switch (format) {
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		return false;
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case MALIDP_FORMAT_XYUV:
	case MALIDP_FORMAT_VYU30:
	case MALIDP_FORMAT_YUV10_420AFBC:
	case MALIDP_FORMAT_NV12AFBC:
	case MALIDP_FORMAT_NV16AFBC:
	case MALIDP_FORMAT_Y0L2:
	case MALIDP_FORMAT_P010:
		return true;
	default:
		BUG();
	}
}

bool malidp_hw_format_has_alpha(u32 format)
{
	switch (format) {
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_YUYV:
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case MALIDP_FORMAT_XYUV:
	case MALIDP_FORMAT_VYU30:
	case MALIDP_FORMAT_YUV10_420AFBC:
	case MALIDP_FORMAT_NV12AFBC:
	case MALIDP_FORMAT_NV16AFBC:
	case MALIDP_FORMAT_P010:
	/* Y0L2 does have alpha, but Mali-DP will ignore it */
	case MALIDP_FORMAT_Y0L2:
		return false;
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_ABGR1555:
		return true;
	default:
		BUG();
	}
}

u32 malidp_hw_format_bpp(u32 format)
{
	switch (format)	{
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
	case DRM_FORMAT_ARGB8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_XRGB8888:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_BGRX8888:
	case MALIDP_FORMAT_VYU30:
	case MALIDP_FORMAT_XYUV:
	case MALIDP_FORMAT_Y0L2:
	case MALIDP_FORMAT_P010:
		return 32;

	case DRM_FORMAT_RGB888:
	case DRM_FORMAT_BGR888:
		return 24;

	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_YUYV:
	case MALIDP_FORMAT_NV16AFBC:
		return 16;

	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case MALIDP_FORMAT_NV12AFBC:
		return 12;

	case MALIDP_FORMAT_YUV10_420AFBC:
		return 15;

	default:
		BUG();
	}
}

static void malidp_dump_hw_buf(struct malidp_hw_device *hwdev,
		struct malidp_hw_buffer *buf)
{
	int i;
	struct device *dev = hwdev->device;

	dev_dbg(dev, "hw_buffer:\n");
	if (buf->hw_layer) {
		dev_dbg(dev, "  layer: %s\n", buf->hw_layer->name);
	}
	dev_dbg(dev, "  natural size: %ix%i\n", buf->natural_w, buf->natural_h);
	dev_dbg(dev, "  cmp_scaling_enable = %s\n",
		buf->cmp_scaling_enable ? "true" : "false");
	dev_dbg(dev, "    cmp_rect src: %ix%i\n", buf->cmp_rect.src_w,
			buf->cmp_rect.src_h);
	dev_dbg(dev, "    cmp_rect dest: %ix%i\n", buf->cmp_rect.dest_w,
			buf->cmp_rect.dest_h);
	dev_dbg(dev, "  mw_scaling_enable = %s\n",
		buf->mw_scaling_enable ? "true" : "false");
	dev_dbg(dev, "    mw_rect src: %ix%i\n", buf->mw_rect.src_w,
			buf->mw_rect.src_h);
	dev_dbg(dev, "    mw_rect dest: %ix%i\n", buf->mw_rect.dest_w,
			buf->mw_rect.dest_h);
	dev_dbg(dev, "  offset: %ix%i\n",
			buf->h_offset, buf->v_offset);
	dev_dbg(dev, "  format: 0x%08x\n", buf->fmt);
	dev_dbg(dev, "  HW format: 0x%x\n", buf->hw_fmt);

	dev_dbg(dev, "  n_planes: %i\n", buf->n_planes);
	dev_dbg(dev, "  address: ");
	for (i = 0; i < buf->n_planes; i++) {
		dev_dbg(dev, "    0x%llx", (unsigned long long)buf->addr[i]);
	}
	dev_dbg(dev, "  pitch: ");
	for (i = 0; i < buf->n_planes; i++) {
		dev_dbg(dev, "    %i", buf->pitch[i]);
	}
	dev_dbg(dev, "  flags: 0x%08x\n", buf->flags);
	dev_dbg(dev, "  transform: 0x%08x\n", buf->transform);
	dev_dbg(dev, "  alpha_mode: %i\n", buf->alpha_mode);
	dev_dbg(dev, "  layer_alpha: %i\n", buf->layer_alpha);
	dev_dbg(dev, "  write_out: %s\n", buf->write_out_enable ? "True" : "False");
}

struct malidp_hw_state_priv {
	struct malidp_se_mw_conf mw;
	enum malidp_de_flow_cmp_cfg cmp_flow;
	/* LEOSW-409: layer_flow is product specific */
	enum malidp_de_flow_layer_cfg layer_flow[MALIDP_MAX_LAYERS];
	struct malidp_se_scaler_conf s0;
};

u32 malidp_hw_read(struct malidp_hw_device *hwdev, u32 reg)
{
	return readl(hwdev->regs + reg);
}

static const struct malidp_line_size_hw_info *malidp_hw_get_ls_info(
		struct malidp_hw_device *hwdev,
		struct malidp_hw_description *hw_desc)
{
	u32 config_raw;
	u32 ls_config_id;

	config_raw = malidp_hw_read(hwdev,
		hwdev->hw_regmap->id_registers + MALIDP_REG_CONFIG_0);

	ls_config_id = MALIDP_CONFIG_0_GET_LS(config_raw);

	/* The HW can't report a line size config we don't understand */
	if (ls_config_id >= hw_desc->config->n_configs) {
		return NULL;
	}

	return &hw_desc->config->ls_configs[ls_config_id];
}

void malidp_hw_enumerate(struct malidp_hw_description *hw_desc,
		enum malidp_hw_product product,
		struct malidp_hw_pdata *pdata)
{
	switch (product) {
	case MALI_DP500:
		malidp_dp500_get_hw_description(hw_desc);
		break;
	case MALI_DP550:
		malidp_dp550_get_hw_description(hw_desc);
		break;
	default:
		BUG();
	}

	hw_desc->pdata = pdata;
}

void malidp_hw_write(struct malidp_hw_device *hwdev,
					  u32 value, u32 reg)
{
	writel(value, hwdev->regs + reg);
}

void malidp_hw_setbits(struct malidp_hw_device *hwdev, u32 mask,
			u32 reg)
{
	u32 data = malidp_hw_read(hwdev, reg);
	data |= mask;
	malidp_hw_write(hwdev, data, reg);
}

void malidp_hw_clearbits(struct malidp_hw_device *hwdev, u32 mask,
			u32 reg)
{
	u32 data = malidp_hw_read(hwdev, reg);
	data &= ~mask;
	malidp_hw_write(hwdev, data, reg);
}

void malidp_hw_commit_scene_atomic(struct malidp_hw_device *hwdev, bool set)
{
	u32 val = 0;

	dev_dbg(hwdev->device, "%s: start: set = %d\n", __func__, set);

	if (set)
		val = MALIDP_CFG_VALID;
	malidp_hw_write(hwdev, val, hwdev->hw_regmap->config_valid);

	dev_dbg(hwdev->device, "%s: end: set = %d\n", __func__, set);
}

static bool malidp_hw_validate_rotmem_fixed(struct malidp_hw_device *hwdev,
				struct malidp_hw_state *state)
{
	struct rotmem_partition *table;
	u32 rotmem_idx = 0;
	u32 rotmem_count = 0;
	int i, j, k;
	struct malidp_hw_buffer *valid_hw_bufs[ARRAY_SIZE(fixed_partition_table)];

	memset(valid_hw_bufs, 0, sizeof(valid_hw_bufs));

	/*
	 * First pass:
	 * Find out the total number of buffers using rotation memory
	 * and sort them.
	 */
	for (i = 0; i < state->n_bufs; i++) {
		struct malidp_hw_buffer *hw_buf = &state->bufs[i];

		/* Only AFBC and 90/270 rotations use rotation memory */
		if ((hw_buf->flags & MALIDP_FLAG_AFBC) ||
		   (hw_buf->transform & MALIDP_TRANSFORM_R90) ||
		   (hw_buf->transform & MALIDP_TRANSFORM_R270)) {
			rotmem_count++;
			if (rotmem_count > ARRAY_SIZE(fixed_partition_table)) {
				dev_err(hwdev->device, "Too many hw buffers using rotation memory\n");
				return false;
			}
			for (j = 0; j < rotmem_count; j++) {
				if (valid_hw_bufs[j] == NULL)
					valid_hw_bufs[j] = hw_buf;
				else if (valid_hw_bufs[j]->hw_layer->index >
					    hw_buf->hw_layer->index) {
					k = rotmem_count - 1;
					valid_hw_bufs[k] = hw_buf;
					while (k-- > j) {
						struct malidp_hw_buffer *tmp;
						tmp = valid_hw_bufs[k];
						valid_hw_bufs[k] = valid_hw_bufs[k + 1];
						valid_hw_bufs[k + 1] = tmp;
					}
					break;
				}
			}
		}
	}

	/*
	 * If no buffers use rotation memory
	 * then there's nothing to validate
	 */
	if (!rotmem_count)
		return true;

	/* Get the correct set of dividers */
	table = &fixed_partition_table[rotmem_count - 1];

	/*
	 * Second pass:
	 * Validate each individual buffer that uses the memory
	 */
	for (i = 0; i < rotmem_count; i++) {
		struct malidp_hw_buffer *hw_buf = valid_hw_bufs[i];
		u32 layer_divider = table->dividers[rotmem_idx];
		u32 layer_size = hwdev->dp_api->rotmem_size_required(
							hw_buf);

		/* Does the layer fit in it's parition? */
		if (layer_size > (hwdev->rotmem_size / layer_divider)) {
			dev_err(hwdev->device, "hwbuf(%d) uses too much rotation memory\n"
				"\tAvailable: %d bytes\n"
				"\tNeeded: %d bytes\n"
				"\tCount: %d, Index: %d\n"
				"\tWidth: %d, Format: %d bpp %d\n",
				i, (hwdev->rotmem_size / layer_divider), layer_size,
				rotmem_count, rotmem_idx, hw_buf->cmp_rect.src_w,
				hw_buf->fmt,
				malidp_hw_format_bpp(hw_buf->fmt));
			return false;
		}
		rotmem_idx++;
	}
	return true;
}

static bool malidp_hw_validate_rotmem(struct malidp_hw_device *hwdev,
				struct malidp_hw_state *state)
{
	switch (hwdev->partition_type) {
	case MALIDP_HW_PARTITION_FIXED:
		return malidp_hw_validate_rotmem_fixed(hwdev, state);
	default:
		BUG();
	}
}

static int scale_f(u16 in, u16 out)
{
	int ret = 0;

	if (in < out)
		ret = 1; /* upscaling */
	else if (in > out)
		ret = -1; /* downscaling */

	return ret;	/* -1: downscaling, 0: no scaling, 1: upscaing */
}

/* Check limition for scaler, if no limitaion found return true, or return false */
static bool limitation_check(struct malidp_hw_device *hwdev,
				struct malidp_hw_state_priv *hw_priv)
{
	bool scaling_to_dp = false, scaling_layer_to_mw = false;
	int i;

	if (hw_priv->s0.scaling_enable == false)
		return true;

	for (i = 0; i < hwdev->topology->n_layers; i++) {
		if (hw_priv->layer_flow[i] == MALIDP_DE_LAYER_FLOW_SCALE_SE0)
			scaling_to_dp = true;
		else if (hw_priv->layer_flow[i] == MALIDP_DE_LAYER_FLOW_SIMULT_SE0)
			scaling_layer_to_mw = true;
	}

	if (scaling_to_dp == true) {
		if ((scale_f(hw_priv->s0.input_w, hw_priv->s0.output_w)) == -1 ||
			(scale_f(hw_priv->s0.input_h, hw_priv->s0.output_h) == -1)) {
			bool supported = hwdev->dp_api->se_api.limitation_check(hwdev->se_dev,
										&hw_priv->s0);
			if (!supported) {
				return false;
			}
		}
	}

	if (scaling_layer_to_mw ||
			((hw_priv->cmp_flow == MALIDP_DE_CMP_FLOW_SE0) && (hw_priv->mw.mode == MALIDP_SE_MW_L0))) {
		if ((scale_f(hw_priv->s0.input_w, hw_priv->s0.output_w)) == 1 ||
			(scale_f(hw_priv->s0.input_h, hw_priv->s0.output_h) == 1)) {
			dev_err(hwdev->device,
			"%s: Upscaling the layer or composition to mw only is not supported.",
			__func__);
			return false;
		}
	}

	return true;
}

int malidp_hw_buffer_set_hw_fmt(struct malidp_hw_device *hwdev,
	struct malidp_hw_buffer *hw_buf)
{
	const struct malidp_hw_topology *topo = hwdev->topology;
	int res = -1;
	unsigned int i;
	bool afbc_support = false;

	if (hw_buf->flags & MALIDP_FLAG_AFBC) {
		for (i = 0; i < topo->n_supported_afbc_formats; i++) {
			if (topo->supported_afbc_formats[i] == hw_buf->fmt) {
				afbc_support = true;
				if ((hw_buf->flags & MALIDP_FLAG_AFBC_SPLITBLK) &&
				    !((1 << i) & topo->supported_afbc_splitblk))
						afbc_support = false;
				break;
			}
		}

		if (!afbc_support) {
			dev_err(hwdev->device, "Format 0x%08x unsupported for AFBC flags 0x%08x\n",
				hw_buf->fmt, hw_buf->flags);
			return -1;
		}
	} else if (hw_buf->transform) {
		/*
		 * This needs to be in an "else" branch because the limitations
		 * don't apply for AFBC
		 */
		for (i = 0; i < topo->n_xform_invalid_formats; i++) {
			if (topo->xform_invalid_formats[i] == hw_buf->fmt) {
				dev_err(hwdev->device, "Transform not supported for format 0x%08x\n",
					hw_buf->fmt);
				return -1;
			}
		}
	}

	if (hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {
		res = malidp_se_fmt_drm2mw(hwdev->se_dev, hw_buf->fmt);
		if (res < 0) {
			dev_err(hwdev->device, "Format 0x%08x unsupported for MW\n",
				hw_buf->fmt);
			return -1;
		}
		hw_buf->hw_fmt = res;
	} else {
		res = malidp_de_fmt_drm2hw(hwdev->de_dev, hw_buf);
		if (res < 0) {
			dev_err(hwdev->device, "Format 0x%08x unsupported\n",
				hw_buf->fmt);
			return -1;
		}
		hw_buf->hw_fmt = res;
	}
	return 0;
}

static void malidp_dump_all_hw_bufs(struct malidp_hw_device *hwdev,
			struct malidp_hw_buffer *hw_bufs, int nbuf)
{
	int i;

	for (i = 0; i < nbuf; i++)
		malidp_dump_hw_buf(hwdev, &hw_bufs[i]);
}

bool malidp_hw_buf_support_srgb(struct malidp_hw_buffer *hw_buf)
{
	const struct malidp_layer_hw_info *layer = hw_buf->hw_layer;

	if (layer->features & MALIDP_LAYER_FEATURE_SRGB)
		return true;

	return false;
}

static int malidp_hw_validate_srgb(struct malidp_hw_device *hwdev,
	struct malidp_hw_buffer *hw_buf)
{
	if (!(hw_buf->flags & MALIDP_FLAG_SRGB))
		return 0;

	if (hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {
		dev_err(hwdev->device,
			"%s: buffer is sRGB but it is output buffer.\n",
			__func__);
		return -EINVAL;
	}

	if (malidp_hw_buf_support_srgb(hw_buf) == false) {
		dev_err(hwdev->device,
			"%s: buffer is sRGB but layer doesn't support.\n",
			__func__);
		return -EINVAL;
	}

	if (hw_buf->flags & MALIDP_FLAG_FORCE_NO_IGAMMA) {
		dev_err(hwdev->device,
			"%s: buffer is sRGB but IG block is disabled.\n",
			__func__);
		return -EINVAL;
	}

	if (malidp_hw_format_is_yuv(hw_buf->fmt) == true) {
		dev_err(hwdev->device,
			"%s: buffer is sRGB but its format (%u) is not supported\n",
			__func__, hw_buf->fmt);
		return -EINVAL;
	}

	return 0;
}

static bool malidp_hw_check_buf_overlap(const struct malidp_hw_buffer *buf_a,
					const struct malidp_hw_buffer *buf_b) {
	u16 top_a    = buf_a->v_offset;
	u16 bottom_a = buf_a->v_offset + buf_a->natural_h;
	u16 left_a   = buf_a->h_offset;
	u16 right_a  = buf_a->h_offset + buf_a->natural_w;
	u16 top_b    = buf_b->v_offset;
	u16 bottom_b = buf_b->v_offset + buf_b->natural_h;
	u16 left_b   = buf_b->h_offset;
	u16 right_b  = buf_b->h_offset + buf_b->natural_w;

	if (min(bottom_a, bottom_b) > max(top_a, top_b) &&
	    min(right_a, right_b) > max(left_a, left_b)) {
		return true;
	}

	return false;
}

static int malidp_hw_validate_smart_layer(struct malidp_hw_device *hwdev,
					  struct malidp_hw_state *state,
					  struct malidp_hw_buffer *ls_buf)
{
	u32 i;
	struct malidp_hw_buffer *hw_bufs = state->bufs;
	struct malidp_hw_smart_layer_state *ls_state = &state->ls_state;
	struct malidp_hw_buffer *buf = &hw_bufs[ls_state->ls_hw_buf_idx[0]];

	/* Check the "same format and same alpha" restriction and
	 * suppose same buffer flags
	 */
	if (ls_buf->fmt != buf->fmt ||
	    ls_buf->layer_alpha != buf->layer_alpha ||
	    ls_buf->alpha_mode != buf->alpha_mode ||
	    ls_buf->flags != buf->flags) {
		dev_err(hwdev->device, "%s : The smart layers have different format or alpha settings or flags\n",
				__func__);
		return -EINVAL;
	}

	/* Check the "no overlap" restriction */
	for (i = 0; i < ls_buf->ls_rect_idx; i++) {
		buf = &hw_bufs[ls_state->ls_hw_buf_idx[i]];
		if (malidp_hw_check_buf_overlap(ls_buf, buf)) {
			dev_err(hwdev->device, "%s : The smart layer overlapped with an existing smart layer\n",
					__func__);
			return -EINVAL;
		}
	}

	/* Check the "First sort on x offset, and then on y offset" restriction */
	i = ls_state->ls_hw_buf_idx[ls_buf->ls_rect_idx-1];
	buf = &hw_bufs[i];
	if (buf->h_offset > ls_buf->h_offset ||
	    (buf->h_offset == ls_buf->h_offset &&
	     buf->v_offset > ls_buf->v_offset)) {
		dev_err(hwdev->device, "%s : The smart layer x/y offset sort check failed\n",
					__func__);
		return -EINVAL;
	}

	return 0;
}

static bool malidp_hw_is_active_rect_in_bbox(struct malidp_hw_smart_layer_state *ls_state,
					     struct malidp_hw_buffer *ls_buf)
{
	u16 bbox_w = ls_state->ls_bbox_right - ls_state->ls_bbox_left;
	u16 bbox_h = ls_state->ls_bbox_bottom - ls_state->ls_bbox_top;

	if ((ls_buf->v_offset + ls_buf->natural_h) > bbox_h ||
	    (ls_buf->h_offset + ls_buf->natural_w) > bbox_w) {
		return false;
	}

	return true;
}

int malidp_hw_validate(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *state)
{
	struct drm_mode_modeinfo mode;
	struct malidp_hw_buffer *de_buffer_to_mw = NULL;
	struct malidp_hw_buffer *out_buffer = NULL;
	int n_write_out_buffers = 0, n_scaling_on_mw = 0;
	u32 n_hw_layers = hwdev->topology->n_layers;
	u32 *n_bufs_per_layer;
	unsigned long flags;
	u32 min_size = hwdev->ls_info->min_line_size;
	u32 max_size = hwdev->ls_info->max_line_size;
	u32 smart_layer_idx = 0;
	int i, ret = -EINVAL;

	struct malidp_hw_state_priv *hw_priv =
		kzalloc(sizeof(*hw_priv), GFP_KERNEL);
	if (!hw_priv)
		return -ENOMEM;

	n_bufs_per_layer = kzalloc(sizeof(*n_bufs_per_layer) * n_hw_layers,
				   GFP_KERNEL);
	if (!n_bufs_per_layer) {
		ret = -ENOMEM;
		goto error;
	}

	for (i = 0; i < n_hw_layers; i++)
		hw_priv->layer_flow[i] = MALIDP_DE_LAYER_FLOW_LOCAL;

	malidp_dump_all_hw_bufs(hwdev, state->bufs, state->n_bufs);

	for (i = 0; i < state->n_bufs; i++) {
		struct malidp_hw_buffer *hw_buf = &state->bufs[i];

		if (hw_buf->hw_layer &&
		    hw_buf->hw_layer->type == MALIDP_HW_LAYER_SMART) {
			struct malidp_hw_smart_layer_state *ls_state = &state->ls_state;

			BUG_ON(smart_layer_idx != hw_buf->ls_rect_idx);
			/* We don't need check the first smart layer */
			if (smart_layer_idx > 0) {
				if (malidp_hw_validate_smart_layer(hwdev, state, hw_buf)) {
					dev_err(hwdev->device, "%s : smart layer validation failed\n",
						__func__);
					goto error;
				}
			}

			if (ls_state->ls_bbox_from_user &&
			    !(malidp_hw_is_active_rect_in_bbox(ls_state, hw_buf))) {
				dev_err(hwdev->device, "%s : smart layer active rect %d is out of the bounding box\n",
					__func__, smart_layer_idx);
				goto error;
			}

			smart_layer_idx++;
		}

		if (malidp_hw_validate_srgb(hwdev, hw_buf) != 0)
			goto error;

		switch (hw_buf->n_planes) {
		case 3:
			/*
			 * The hardware behaviour is as follows:
			 * "For 3 planes the last bits of the cropping are
			 * determined by LV2PTR[2:0] and LV1PTR[2:0]/LV3PTR[2:0]
			 * are ignored."
			 *
			 * Compatibility with these constraints is enforced by
			 * the following checks:
			 * The plane 0 pointer must be even, as the hardware
			 * will ignore the least significant bit.
			 * The plane 2 pointer least-significant bits must match
			 * the plane 1 pointer, as the hardware will ignore the
			 * plane 2 value and use the plane 1 value directly
			 */
			if ((hw_buf->addr[0] & 0x1) ||
			    ((hw_buf->addr[1] & 0x7) != (hw_buf->addr[2] & 0x7))) {
				dev_err(hwdev->device, "%s : buffer %i does not meet alignment constraints\n",
					__func__, i);
				goto error;
			}

			/*
			 * The 3rd stride will be ignored on some hardware,
			 * so the 2nd and 3rd pitches of the buffer must be same.
			 */
			if ((hw_buf->pitch[2] != hw_buf->pitch[1]) &&
			    (!hw_buf->hw_layer->p3_stride_offset)) {
				dev_err(hwdev->device, "%s : the second and third strides are not same\n",
					__func__);
				goto error;
			}
			break;
		case 2:
			/*
			 * The hardware behaviour is as follows:
			 * "For 2 plane the last bits of the cropping are
			 *  determined by bits [2:0] of the LV1PTR register and
			 *  the [2:0] bits in LV2PTR are ignored."
			 *
			 * Compatibility with these constraints is enforced by
			 * the following checks:
			 * The plane 0 pointer must be even, as the hardware
			 * will ignore the least significant bit.
			 * The plane 1 pointer least-significant bits must match
			 * the plane 0 pointer, as the hardware will ignore the
			 * plane 1 value and use the plane 0 value directly
			 */
			if ((hw_buf->addr[0] & 0x1) ||
			    ((hw_buf->addr[0] & 0x7) != (hw_buf->addr[1] & 0x7))) {
				dev_err(hwdev->device, "%s : buffer %i does not meet alignment constraints\n",
					__func__, i);
				goto error;
			}
			break;
		case 1:
			/* AFBC buffers have to be 128-bit aligned */
			if ((hw_buf->flags & MALIDP_FLAG_AFBC) &&
				(hw_buf->addr[0] & MALIDP_AFBC_ALIGN_MASK)) {
				dev_err(hwdev->device, "%s : AFBC buffers must be aligned to %u bytes (including offset)\n",
					__func__, MALIDP_AFBC_ALIGN_MASK + 1);
				goto error;
			}
			break;
		default:
			/* Nothing to do */
			break;
		}

		if ((hw_buf->cmp_rect.src_w < min_size) || (hw_buf->cmp_rect.src_h < min_size) ||
		    (hw_buf->cmp_rect.dest_w < min_size) || (hw_buf->cmp_rect.dest_h < min_size)) {
			dev_err(hwdev->device, "%s : buffer dimensions too small. src: %ix%i, dest: %ix%i\n", __func__,
				hw_buf->cmp_rect.src_w, hw_buf->cmp_rect.src_h,
				hw_buf->cmp_rect.dest_w, hw_buf->cmp_rect.dest_h);
			goto error;
		}

		if ((hw_buf->mw_rect.src_w < min_size) || (hw_buf->mw_rect.src_h < min_size) ||
		    (hw_buf->mw_rect.dest_w < min_size) || (hw_buf->mw_rect.dest_h < min_size)) {
			dev_err(hwdev->device, "%s : buffer dimensions too small. src: %ix%i, dest: %ix%i\n", __func__,
				hw_buf->mw_rect.src_w, hw_buf->mw_rect.src_h,
				hw_buf->mw_rect.dest_w, hw_buf->mw_rect.dest_h);
			goto error;
		}

		if ((hw_buf->cmp_rect.src_w > max_size) || (hw_buf->cmp_rect.src_h > max_size) ||
		    (hw_buf->cmp_rect.dest_w > max_size) || (hw_buf->cmp_rect.dest_h > max_size)) {
			dev_err(hwdev->device, "%s : buffer dimensions too big. src: %ix%i, dest: %ix%i\n", __func__,
				hw_buf->cmp_rect.src_w, hw_buf->cmp_rect.src_h,
				hw_buf->cmp_rect.dest_w, hw_buf->cmp_rect.dest_h);
			goto error;
		}

		if ((hw_buf->mw_rect.src_w > max_size) || (hw_buf->mw_rect.src_h > max_size) ||
		    (hw_buf->mw_rect.dest_w > max_size) || (hw_buf->mw_rect.dest_h > max_size)) {
			dev_err(hwdev->device, "%s : buffer dimensions too big. src: %ix%i, dest: %ix%i\n", __func__,
				hw_buf->mw_rect.src_w, hw_buf->mw_rect.src_h,
				hw_buf->mw_rect.dest_w, hw_buf->mw_rect.dest_h);
			goto error;
		}

		if ((hw_buf->alpha_mode & MALIDP_ALPHA_MODE_PREMULT) &&
		    !(hw_buf->alpha_mode & MALIDP_ALPHA_MODE_PIXEL)) {
			dev_err(hwdev->device, "hwbuf(%d) pre-multiplied alpha "
				"only supported with pixel alpha blending\n", i);
			goto error;
		}

		if ((hw_buf->alpha_mode & MALIDP_ALPHA_MODE_PIXEL) &&
		    !malidp_hw_format_has_alpha(hw_buf->fmt)) {
			dev_err(hwdev->device, "hwbuf(%d) alpha pixel mode set "
				"for a pixel format without alpha\n", i);
			goto error;
		}

		if (malidp_hw_buffer_set_hw_fmt(hwdev, hw_buf)) {
			dev_err(hwdev->device,
				"Couldn't get HW pixel format for buffer %i\n",
				i);
			goto error;
		} else {
			dev_dbg(hwdev->device, "Set buffer %i hw_fmt: 0x%08x -> 0x%x\n",
				i, hw_buf->fmt, hw_buf->hw_fmt);
		}

		if (hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) {
			if (out_buffer != NULL) {
				dev_err(hwdev->device, "found more than one output buffer\n");
				goto error;
			}
			out_buffer = &state->bufs[i];

			/* The output buffer cannot be transformed */
			if (out_buffer->requirements & MALIDP_LAYER_FEATURE_TRANSFORM) {
				dev_err(hwdev->device, "output buffer cannot be transformed");
				goto error;
			}

			/* The output buffer cannot AFBC */
			if (out_buffer->requirements & MALIDP_LAYER_FEATURE_AFBC) {
				dev_err(hwdev->device, "output buffer cannot be compressed");
				goto error;
			}

			/* cmp_scaling_enable of output buffer should not be 'true' */
			if (out_buffer->cmp_scaling_enable == true) {
				dev_err(hwdev->device, "output buffer should not have cmp_scaling_enable set");
				goto error;
			}
		} else {
			if ((hw_buf->requirements & hw_buf->hw_layer->features) !=
			    hw_buf->requirements) {
				dev_err(hwdev->device,
					"%s: buffer requirements %08x not compatible with layer features %08x\n",
					__func__, hw_buf->requirements, hw_buf->hw_layer->features);
				goto error;
			}
			if (hw_buf->write_out_enable) {
				n_write_out_buffers++;
				de_buffer_to_mw = &state->bufs[i];
			}
		}

		if (hw_buf->cmp_scaling_enable) {

			if (hw_buf->mw_scaling_enable == false) {
				dev_err(hwdev->device, "Input buffer cannot be scaled to composition and not be scaled to MW interface");
				goto error;
			} else if (((hw_buf->cmp_rect.dest_w != hw_buf->mw_rect.dest_w) ||
							(hw_buf->cmp_rect.dest_h != hw_buf->mw_rect.dest_h)) && hw_buf->mw_scaling_enable == true) {
				dev_err(hwdev->device, "Input buffer cannot be scaled different to composition and MW interface");
				goto error;
			}

			if (hw_priv->s0.scaling_enable == true) {
				dev_err(hwdev->device, "%s: more than 1 input layer to be scaled is not support", __func__);
				goto error;
			}

			hw_priv->layer_flow[hw_buf->hw_layer->index] = MALIDP_DE_LAYER_FLOW_SCALE_SE0;
			hw_priv->s0.scaling_enable = true;
			hw_priv->s0.input_h = hw_buf->cmp_rect.src_h;
			hw_priv->s0.input_w = hw_buf->cmp_rect.src_w;
			hw_priv->s0.output_h = hw_buf->cmp_rect.dest_h;
			hw_priv->s0.output_w = hw_buf->cmp_rect.dest_w;
			hw_priv->s0.rgbo_enable = true;
			hw_priv->s0.scale_alpha = malidp_hw_format_has_alpha(hw_buf->fmt);
		} else if (!(hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT) && hw_buf->mw_scaling_enable) {
			dev_dbg(hwdev->device, "MW Scaling is detected\n");

			hw_priv->layer_flow[hw_buf->hw_layer->index] = MALIDP_DE_LAYER_FLOW_SIMULT_SE0;
			hw_priv->s0.input_h = hw_buf->mw_rect.src_h;
			hw_priv->s0.input_w = hw_buf->mw_rect.src_w;
			hw_priv->s0.output_h = hw_buf->mw_rect.dest_h;
			hw_priv->s0.output_w = hw_buf->mw_rect.dest_w;
			hw_priv->s0.scale_alpha = malidp_hw_format_has_alpha(hw_buf->fmt);
			hw_priv->s0.rgbo_enable = false;
			hw_priv->s0.scaling_enable = true;
			n_scaling_on_mw++;
		}

		if (hw_buf->hw_layer) {
			if (n_bufs_per_layer[hw_buf->hw_layer->index] >=
					hw_buf->hw_layer->n_supported_layers) {
				dev_err(hwdev->device, "%s: more than max %d buffers for layer %i\n",
					__func__, hw_buf->hw_layer->n_supported_layers,
					hw_buf->hw_layer->index);
				goto error;
			}
			n_bufs_per_layer[hw_buf->hw_layer->index]++;
		}

	}

	/* If all input layers are scaled to mw, then just scale composition */
	if ((n_scaling_on_mw == state->n_bufs -1) && (out_buffer != NULL && out_buffer->mw_scaling_enable == true)) {
		/* Disable it, then scaler will be configured to scale composition */
		hw_priv->s0.scaling_enable = false;
		/* All the layers will not be sent to SE */
		for (i = 0; i < n_hw_layers; i++)
			hw_priv->layer_flow[i] = MALIDP_DE_LAYER_FLOW_LOCAL;
	} else if (n_scaling_on_mw > 0 && out_buffer == NULL) {
		dev_err(hwdev->device, "%s: Output buffer is not found", __func__);
		goto error;
	} else if (n_scaling_on_mw >= 2) {
		dev_err(hwdev->device, "%s: More than one but not all layers are scaled to memory", __func__);
		goto error;
	}

	/*
	 * Validate that the scene uses rotation memory appropriately
	 */
	if (!malidp_hw_validate_rotmem(hwdev, state))
		goto error;

	/* Find flow for memory write-out, set scaler configuration for scaling composition to mw */
	hw_priv->cmp_flow = MALIDP_DE_CMP_FLOW_INTERNAL;
	if ((n_write_out_buffers == 0) && (!out_buffer)) {
		/* Do not use the memory write-out interface */
		hw_priv->mw.mode = MALIDP_SE_MW_DISABLE;
		hw_priv->mw.buf = NULL;
	} else if ((n_write_out_buffers == (state->n_bufs - 1)) &&
		    out_buffer && out_buffer->write_out_enable) {
		/* Write out the result of the composition.*/
		spin_lock_irqsave(&hwdev->hw_lock, flags);
		malidp_de_modeget(hwdev->de_dev, &mode);
		spin_unlock_irqrestore(&hwdev->hw_lock, flags);

		if ((out_buffer->mw_rect.src_w != mode.hdisplay) ||
				(out_buffer->mw_rect.src_h != mode.vdisplay)) {
			dev_err(hwdev->device, "output buffer src size did not match the mode");
			goto error;
		}
		/* check whether output buffer need be scaled */
		if (out_buffer->mw_scaling_enable == true) {
			if (hw_priv->s0.scaling_enable == true) {
				dev_err(hwdev->device, "no scaler for MW");
				goto error;
			}
			hw_priv->mw.mode = MALIDP_SE_MW_L0;

			/* Scale the composition to memory */
			hw_priv->s0.input_h = out_buffer->mw_rect.src_h;
			hw_priv->s0.input_w = out_buffer->mw_rect.src_w;
			hw_priv->s0.output_h = out_buffer->mw_rect.dest_h;
			hw_priv->s0.output_w = out_buffer->mw_rect.dest_w;
			hw_priv->s0.scale_alpha = malidp_hw_format_has_alpha(out_buffer->fmt);
			hw_priv->s0.rgbo_enable = false;
			hw_priv->s0.scaling_enable = true;
		} else {
			hw_priv->mw.mode = (hw_priv->s0.scaling_enable == true) ? MALIDP_SE_MW_L1 : MALIDP_SE_MW_L0;

			/* for L1, no alpha compoment */
			if (hw_priv->mw.mode == MALIDP_SE_MW_L1 && malidp_hw_format_has_alpha(out_buffer->fmt)) {
				dev_err(hwdev->device,
					"hwbuf(%d) alpha not supported when writing-out "
					"the result of the composition\n", i);
				goto error;
			}
		}

		hw_priv->mw.buf = out_buffer;
		hw_priv->cmp_flow = MALIDP_DE_CMP_FLOW_SE0;
	} else if ((n_write_out_buffers == 1) && out_buffer &&
		    !out_buffer->write_out_enable) {
		/* Write out the layer indicated by "de_buffer_to_mw" */

		if (!(de_buffer_to_mw->hw_layer->features & MALIDP_LAYER_FEATURE_SCALING)) {
			dev_err(hwdev->device, "writing layer %s to memory is not supported",
				de_buffer_to_mw->hw_layer->name);
			goto error;
		}

		hw_priv->mw.mode = MALIDP_SE_MW_L0;
		hw_priv->mw.buf = out_buffer;
		/* if the layer will be scaled, the size of cmp_rect.dest, should be same as the size of output */
		if (de_buffer_to_mw->cmp_scaling_enable) {
			if ((de_buffer_to_mw->cmp_rect.dest_w != out_buffer->mw_rect.dest_w) ||
				(de_buffer_to_mw->cmp_rect.dest_h != out_buffer->mw_rect.dest_h)) {
				dev_err(hwdev->device, "scaled input buffer size does not match output size");
				goto error;
			}
		}

		if (hw_priv->layer_flow[de_buffer_to_mw->hw_layer->index] == MALIDP_DE_LAYER_FLOW_LOCAL) {
			hw_priv->layer_flow[de_buffer_to_mw->hw_layer->index] = MALIDP_DE_LAYER_FLOW_SIMULT_SE0;
		}

	} else {
		/* Error condition */
		dev_err(hwdev->device, "couldn't determine memory write-out configuration");
		goto error;
	}

	if (!limitation_check(hwdev, hw_priv))
		goto error;

	malidp_se_set_scaling_dependent_state(hwdev->se_dev, &hw_priv->s0);
	state->hw_priv = hw_priv;
	kfree(n_bufs_per_layer);

	return 0;

error:
	kfree(n_bufs_per_layer);
	kfree(hw_priv);

	return ret;
};

void malidp_hw_state_free(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *state)
{
	kfree(state->hw_priv);
};

int malidp_hw_commit(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *state)
{
	int i, res = 0;
	unsigned long flags;
	struct malidp_hw_state_priv *hw_priv = state->hw_priv;

	spin_lock_irqsave(&hwdev->hw_lock, flags);

	dev_dbg(hwdev->device, "%s: start\n", __func__);

	malidp_hw_commit_scene_atomic(hwdev, false);

	malidp_de_disable_all_layers(hwdev->de_dev);

	for (i = 0; i < state->n_bufs; i++) {
		struct malidp_hw_buffer *hw_buf = &state->bufs[i];

		if (!(hw_buf->flags & MALIDP_FLAG_BUFFER_OUTPUT)) {
			res = malidp_de_cfg_layer(hwdev->de_dev, hw_buf);
			if (res)
				dev_err(hwdev->device, "%s : cfg layer returned %i",
						__func__, res);
		}
	}

	if (state->ls_state.ls_hw_layer) {
		dev_dbg(hwdev->device, "%s, smart layer state: bounding box (%d, %d, %d, %d), active smart layers (%d)\n",
			__func__,
			state->ls_state.ls_bbox_top,
			state->ls_state.ls_bbox_left,
			state->ls_state.ls_bbox_bottom,
			state->ls_state.ls_bbox_right,
			state->ls_state.n_smart_layers);
		malidp_de_cfg_smart_state(hwdev->de_dev, &state->ls_state);
	}

	/* Set flows for the different layers and composition result */
	malidp_de_cfg_cmp_flow(hwdev->de_dev, hw_priv->cmp_flow);
	for (i = 0; i < hwdev->topology->n_layers; i++) {
		malidp_de_cfg_layer_flow(hwdev->de_dev,
				 &hwdev->topology->layers[i],
				 hw_priv->layer_flow[i]);
	}

	malidp_se_cfg_processing(hwdev->se_dev, &hw_priv->mw, &hw_priv->s0);

	malidp_hw_commit_scene_atomic(hwdev, true);

	dev_dbg(hwdev->device, "%s: end\n", __func__);

	spin_unlock_irqrestore(&hwdev->hw_lock, flags);
	return res;
}

static void malidp_hw_pdata_dump(struct platform_device *pdev,
				struct malidp_hw_pdata *pdata)
{
	dev_dbg(&pdev->dev, "%s:\n", __func__);
	dev_dbg(&pdev->dev, "pxclk: %p, mclk: %p, aclk: %p, pclk: %p\n",
		pdata->pxclk, pdata->mclk, pdata->aclk, pdata->pclk);
	dev_dbg(&pdev->dev, "de axi burst length = %d\n", pdata->de_axi_burstlen);
	dev_dbg(&pdev->dev, "de axi outstanding transactions = %d\n",
		pdata->de_axi_outstran);
	dev_dbg(&pdev->dev, "de axi arqos threshold low = 0x%X\n",
		pdata->de_axi_arqos_low);
	dev_dbg(&pdev->dev, "de axi arqos threshold high = 0x%X\n",
		pdata->de_axi_arqos_high);
	dev_dbg(&pdev->dev, "de axi arqos red = 0x%X\n",
		pdata->de_axi_arqos_red);
	dev_dbg(&pdev->dev, "de axi arqos green = 0x%X\n",
		pdata->de_axi_arqos_green);
	dev_dbg(&pdev->dev, "se axi burst length = %d\n", pdata->se_axi_burstlen);
	dev_dbg(&pdev->dev, "se axi outstanding transactions = %d\n",
		pdata->se_axi_outstran);
	dev_dbg(&pdev->dev, "se axi awcache  %d\n", pdata->se_axi_awcache);
	dev_dbg(&pdev->dev, "se axi qos = %d\n", pdata->se_axi_awqos);
	dev_dbg(&pdev->dev, "rotmem size = %d\n", pdata->rotmem_size);
}

char *malidp_hw_get_event_string(char *string, int max, struct malidp_hw_event *event)
{
	int i = 0;

	if (!string)
		return NULL;

	if (event->type == MALIDP_HW_EVENT_NONE) {
		if (max <= 4)
			return NULL;
		sprintf(string, "NONE");
	} else {
		if (event->type & MALIDP_HW_EVENT_FLIP) {
			if (max <= i + 6)
				return NULL;
			i += sprintf(string + i, "FLIP |");
		}
		if (event->type & MALIDP_HW_EVENT_ERROR) {
			if (max <= i + 7)
				return NULL;
			i += sprintf(string + i, "ERROR |");
			if (event->type & MALIDP_HW_ERROR_URUN) {
				if (max <= i + 10)
					return NULL;
				i += sprintf(string + i, "UNDERRUN |");
			}
			if (event->type & MALIDP_HW_ERROR_ORUN) {
				if (max <= i + 9)
					return NULL;
				i += sprintf(string + i, "OVERRUN |");
			}
			if (event->type & MALIDP_HW_ERROR_AXI) {
				if (max <= i + 8)
					return NULL;
				i += sprintf(string + i, "AXIERR |");
			}
			if (event->type & MALIDP_HW_ERROR_QFULL) {
				if (max <= i + 7)
					return NULL;
				i += sprintf(string + i, "QFULL |");
			}
			if (event->type & MALIDP_HW_ERROR_IBUSY) {
				if (max <= i + 7)
					return NULL;
				i += sprintf(string + i, "IBUSY |");
			}
		}
		if (event->type & MALIDP_HW_EVENT_VSYNC) {
			if (max <= i + 7)
				return NULL;
			i += sprintf(string + i, "VSYNC |");
		}
		if (event->type & MALIDP_HW_EVENT_START) {
			if (max <= i + 7)
				return NULL;
			i += sprintf(string + i, "START |");
		}
		if (event->type & MALIDP_HW_EVENT_NEWCFG) {
			if (max <= i + 8)
				return NULL;
			i += sprintf(string + i, "NEWCFG |");
		}
		if (event->type & MALIDP_HW_EVENT_STOP) {
			if (max <= i + 6)
				return NULL;
			i += sprintf(string + i, "STOP |");
		}
	}

	return string;
}

/* Populate pdata structure using device tree description */
int malidp_hw_get_resources(struct platform_device *pdev,
			       struct device_node *nproot,
			       struct malidp_hw_pdata *pdata)
{
	struct resource *res;
	u32 props;
	int i, ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "Could not get registers resource\n");
		return -ENOMEM;
	}

	pdata->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pdata->regs))
		return -ENOMEM;

	pdata->de_irq = -1;
	pdata->se_irq = -1;
	for (i = 0; i < 2; i++) {
		res = platform_get_resource(pdev, IORESOURCE_IRQ, i);
		if (!res) {
			dev_err(&pdev->dev, "Could not get irq %d\n", i);
			return -EINVAL;
		}
		if (!strcmp("DE", res->name)) {
			if (pdata->de_irq < 0) {
				pdata->de_irq = res->start;
			} else {
				dev_err(&pdev->dev, "IRQ '%s' already assigned\n", res->name);
				return -EINVAL;
			}
		} else if (!strcmp("SE", res->name)) {
			if (pdata->se_irq < 0) {
				pdata->se_irq = res->start;
			} else {
				dev_err(&pdev->dev, "IRQ '%s' already assigned\n", res->name);
				return -EINVAL;
			}
		} else {
			dev_err(&pdev->dev, "Wrong name (%s) for irq %d\n",
				res->name, i);
			return -EINVAL;
		}
	}

	pdata->pxclk = devm_clk_get(&pdev->dev, "pxclk");
	if (!pdata->pxclk) {
		dev_err(&pdev->dev, "Could not get pxclk");
		return -EINVAL;
	}

	pdata->mclk = devm_clk_get(&pdev->dev, "mclk");
	if (!pdata->mclk) {
		dev_err(&pdev->dev, "Could not get mclk");
		return -EINVAL;
	}

	pdata->aclk = devm_clk_get(&pdev->dev, "aclk");
	if (!pdata->aclk) {
		dev_err(&pdev->dev, "Could not get aclk");
		return -EINVAL;
	}

	pdata->pclk = devm_clk_get(&pdev->dev, "pclk");
	if (!pdata->pclk) {
		dev_err(&pdev->dev, "Could not get pclk");
		return -EINVAL;
	}

	/*
	 * Optional, non-standard properties are retrieved parsing
	 * the device-tree description directly. If we wanted to add support
	 * for initializing using board files we would need to use a custom
	 * pdata structure in <linux/platform_data/malidp.h>.
	 */
	ret = of_property_read_u32(nproot, "de-axi-burst-length", &props);
	if (ret == 0)
		pdata->de_axi_burstlen = props;
	else
		pdata->de_axi_burstlen = DE_DEFAULT_AXI_BURSTLEN;

	ret = of_property_read_u32(nproot, "de-axi-poutstdcab", &props);
	if (ret == 0)
		pdata->de_axi_poutstdcab = props;
	else
		pdata->de_axi_poutstdcab = DE_DEFAULT_AXI_POUTSTDCAB;

	ret = of_property_read_u32(nproot,
				   "de-axi-outstanding-transactions", &props);
	if (ret == 0)
		pdata->de_axi_outstran = props;
	else
		pdata->de_axi_outstran = DE_DEFAULT_AXI_OUTSTRAN;

	ret = of_property_read_u32(nproot,
				   "de-axi-arqos-threshold-low", &props);
	if (ret == 0)
		pdata->de_axi_arqos_low = props;
	else
		pdata->de_axi_arqos_low = DE_DEFAULT_AXI_ARQOS_LOW;

	ret = of_property_read_u32(nproot,
				   "de-axi-arqos-threshold-high", &props);
	if (ret == 0)
		pdata->de_axi_arqos_high = props;
	else
		pdata->de_axi_arqos_high = DE_DEFAULT_AXI_ARQOS_HIGH;

	ret = of_property_read_u32(nproot, "de-axi-arqos-red", &props);
	if (ret == 0)
		pdata->de_axi_arqos_red = props;
	else
		pdata->de_axi_arqos_red = DE_DEFAULT_AXI_ARQOS_RED;

	ret = of_property_read_u32(nproot, "de-axi-arqos-green", &props);
	if (ret == 0)
		pdata->de_axi_arqos_green = props;
	else
		pdata->de_axi_arqos_green = DE_DEFAULT_AXI_ARQOS_GREEN;

	ret = of_property_read_u32(nproot, "se-axi-burst-length", &props);
	if (ret == 0)
		pdata->se_axi_burstlen = props;
	else
		pdata->se_axi_burstlen = SE_DEFAULT_AXI_BURSTLEN;

	ret = of_property_read_u32(nproot,
				   "se-axi-outstanding-transactions", &props);
	if (ret == 0)
		pdata->se_axi_outstran = props;
	else
		pdata->se_axi_outstran = SE_DEFAULT_AXI_OUTSTRAN;

	ret = of_property_read_u32(nproot, "se-axi-awcache", &props);
	if (ret == 0)
		pdata->se_axi_awcache = props;
	else
		pdata->se_axi_awcache = SE_DEFAULT_AXI_AWCACHE;

	ret = of_property_read_u32(nproot, "se-axi-awqos", &props);
	if (ret == 0)
		pdata->se_axi_awqos = props;
	else
		pdata->se_axi_awqos = SE_DEFAULT_AXI_AWQOS;

	ret = of_property_read_u32(nproot, "rotmem", &props);
	if (ret == 0)
		pdata->rotmem_size = props * SZ_1K;
	else
		pdata->rotmem_size = MALIDP_DEFAULT_ROTMEM_SIZE;

	ret = of_alias_get_id(nproot, "malidp");
	if (ret < 0) {
		dev_err(&pdev->dev, "Could not get alias id\n");

		/* Fallback to use the node phandle */
		pdata->dp_id = nproot->phandle;
	} else {
		pdata->dp_id = ret;
	}

	malidp_hw_pdata_dump(pdev, pdata);

	return 0;
}

static void malidp_hw_enable_clocks(struct malidp_hw_device *hwdev)
{
	clk_prepare_enable(hwdev->pclk);
	clk_prepare_enable(hwdev->aclk);
	clk_prepare_enable(hwdev->mclk);
	clk_prepare_enable(hwdev->pxclk);
}

static void malidp_hw_disable_clocks(struct malidp_hw_device *hwdev)
{
	clk_disable_unprepare(hwdev->aclk);
	clk_disable_unprepare(hwdev->pxclk);
	clk_disable_unprepare(hwdev->mclk);
	clk_disable_unprepare(hwdev->pclk);

}

/* Must be called with the power_mutex held */
static enum malidp_op_mode malidp_hw_change_op_mode(struct malidp_hw_device *hwdev,
		enum malidp_op_mode mode)
{
	enum malidp_op_mode old_mode_de;

	BUG_ON(mode == MALIDP_OP_MODE_UNKNOWN);

	/* We're protected by the power mutex, so just read the mode */
	old_mode_de = malidp_de_get_op_mode(hwdev->de_dev);
	if (old_mode_de == mode)
		return old_mode_de;

	old_mode_de = hwdev->dp_api->change_op_mode(hwdev, mode);

	return old_mode_de;
}

void malidp_hw_cfg_de_disable_mw_flows_atomic(struct malidp_hw_device *hwdev)
{
	enum malidp_de_flow_layer_cfg layer_flow;
	int i;

	for (i = 0; i < hwdev->topology->n_layers; i++) {
		layer_flow = malidp_de_get_layer_flow(hwdev->de_dev,
				&hwdev->topology->layers[i]);

		if (layer_flow == MALIDP_DE_LAYER_FLOW_SIMULT_SE0)
			malidp_de_cfg_layer_flow(hwdev->de_dev,
					&hwdev->topology->layers[i],
					MALIDP_DE_LAYER_FLOW_LOCAL);
	}

	malidp_de_cfg_cmp_flow(hwdev->de_dev, MALIDP_DE_CMP_FLOW_INTERNAL);
}

static void malidp_hw_update_downscaling_threshold(
	struct malidp_hw_device *hwdev,
	struct drm_mode_modeinfo *mode)
{
	u32 pxlclk;
	u32 mclk;

	mclk = clk_get_rate(hwdev->mclk) / 1000;
	pxlclk = clk_get_rate(hwdev->pxclk) / 1000;

	hwdev->downscaling_threshold =
			hwdev->dp_api->se_api.calc_downscaling_threshold(mclk,
					pxlclk, mode);
}

bool malidp_hw_pxclk_ok(struct malidp_hw_device *dev, long rate)
{
	long rounded_rate = clk_round_rate(dev->pxclk, rate);
	if (abs(rate - rounded_rate) >= 1000)
		return false;

	return true;
}

int malidp_hw_modeset(struct malidp_hw_device *dev, struct drm_mode_modeinfo *mode)
{
	struct malidp_se_mw_conf mw_conf;
	enum malidp_op_mode old_mode;
	unsigned long flags;
	long rate = mode->clock * 1000;
	int ret = 0;

	mutex_lock(&dev->power_mutex);

	/* Check if we can achieve the pixel clock before we try */
	if (!malidp_hw_pxclk_ok(dev, rate)) {
		dev_err(dev->device, "%s: pxclk rate couldn't be matched\n",
			__func__);
		ret = -ERANGE;
		goto unlock;
	}

	old_mode = malidp_de_get_op_mode(dev->de_dev);
	/* Skip the call if the hardware is suspended */
	if (old_mode == MALIDP_OP_MODE_POWERSAVE) {
		goto unlock;
	}

	old_mode = malidp_hw_change_op_mode(dev, MALIDP_OP_MODE_CONFIG);

	clk_set_rate(dev->pxclk, rate);
	ret = malidp_hw_set_mclk(dev, rate);
	if (ret < 0)
		goto exit;
	dev->clock_ratio = malidp_hw_clock_ratio_get(dev);
	dev_dbg(dev->device, "%s: New clock ratio: 0x%04x.%04x\n", __func__,
		(dev->clock_ratio & 0xFFFF0000) >> 16,
		dev->clock_ratio & 0xFFFF);

	/* Ratio must be >= 1 */
	if (dev->clock_ratio < (1 << 16)) {
		dev_dbg(dev->device, "%s: mclk rate must exceed pxclk rate\n",
			__func__);
		ret = -ERANGE;
		goto exit;
	}

	spin_lock_irqsave(&dev->hw_lock, flags);
	/*
	 * Modeset disables all the layers and memory interface in the system
	 * to avoid displaying an old configuration that doesn't match the same
	 * resultion.
	 */
	mw_conf.mode = MALIDP_SE_MW_DISABLE;
	malidp_se_cfg_processing(dev->se_dev, &mw_conf, NULL);
	ret = malidp_de_modeset(dev->de_dev, mode);
	if (ret == 0)
		malidp_hw_update_downscaling_threshold(dev, mode);
	spin_unlock_irqrestore(&dev->hw_lock, flags);

exit:
	malidp_hw_change_op_mode(dev, old_mode);
unlock:
	mutex_unlock(&dev->power_mutex);

	return ret;
}

int malidp_hw_set_callback(struct malidp_hw_device *dev,
		const struct malidp_intf_hw_info *hw_intf,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque)
{
	unsigned long flags;
	int ret = 0;

	spin_lock_irqsave(&dev->hw_lock, flags);
	if (hw_intf->type == MALIDP_HW_INTF_PRIMARY)
		malidp_de_set_flip_callback(dev->de_dev, callback, opaque);
	else if (MALIDP_HW_INTF_MEMORY)
		malidp_se_set_flip_callback(dev->se_dev, callback, opaque);
	else
		ret = -EINVAL;
	spin_unlock_irqrestore(&dev->hw_lock, flags);

	return ret;
}

inline u32 malidp_hw_get_core_id(struct malidp_hw_device *hwdev)
{
	BUG_ON(!hwdev->core_id);
	return hwdev->core_id;
}

static inline u32 malidp_hw_identify(struct malidp_hw_device *hwdev)
{
	u32 product_id = hwdev->topology->product_id;
	u32 core_id;

	/*
	 * If the old register identifies as DP500, then we can be sure that
	 * the *hardware* is DP500 (this requires that bits [27:24] of register
	 * MALIDP_REG_DP500_CORE_ID are reserved for all other hardware
	 * versions).
	 */
	core_id = malidp_hw_read(hwdev, MALIDP_REG_DP500_CORE_ID);
	switch (MALIDP_CORE_ID_PRODUCT_ID(core_id)) {
	case MALIDP_DP500_PRODUCT_ID:
		if (product_id == MALIDP_DP500_PRODUCT_ID)
			return core_id;
		else
			goto mismatch;
	default:
		if (product_id == MALIDP_DP500_PRODUCT_ID)
			goto mismatch;
	}

	/* For all other hardware version, use MALIDP_REG_CORE_ID */
	core_id = malidp_hw_read(hwdev, MALIDP_REG_CORE_ID);
	if (MALIDP_CORE_ID_PRODUCT_ID(core_id) == product_id)
		return core_id;

mismatch:
	dev_err(hwdev->device, "%s : device-tree expected hwver 0x%X but register suggested 0x%X\n",
		__func__, product_id, MALIDP_CORE_ID_PRODUCT_ID(core_id));

	return 0;
}

struct malidp_hw_device *malidp_hw_init(struct platform_device *pdev,
			       struct malidp_hw_description *hw_desc)
{
	struct malidp_hw_device *hwdev = devm_kzalloc(&pdev->dev,
			sizeof(struct malidp_hw_device),
			GFP_KERNEL);
	if (!hwdev)
		return hwdev;

	spin_lock_init(&hwdev->hw_lock);
	mutex_init(&hwdev->power_mutex);

	hwdev->device = &pdev->dev;
	hwdev->pclk = hw_desc->pdata->pclk;
	hwdev->mclk = hw_desc->pdata->mclk;
	hwdev->pxclk = hw_desc->pdata->pxclk;
	hwdev->aclk = hw_desc->pdata->aclk;
	hwdev->regs = hw_desc->pdata->regs;
	hwdev->topology = hw_desc->topology;
	hwdev->partition_type = hw_desc->config->partition_type;
	hwdev->clock_ratio = 0x10000;
	hwdev->dp_api = hw_desc->topology->dp_api;
	hwdev->hw_regmap = hw_desc->topology->regmap;

	/* We need the APB clock for register accesses */
	clk_prepare_enable(hwdev->pclk);

	hwdev->core_id = malidp_hw_identify(hwdev);
	if (!hwdev->core_id) {
		dev_err(hwdev->device, "%s : couldn't determine hardware version\n",
			__func__);
		goto fail_clk;
	}
	dev_info(hwdev->device, "Probed product ID: 0x%04x",
		 MALIDP_CORE_ID_PRODUCT_ID(hwdev->core_id));
	dev_info(hwdev->device, "  Major: %d, minor: %d, status: %d\n",
		 MALIDP_CORE_ID_MAJOR(hwdev->core_id),
		 MALIDP_CORE_ID_MINOR(hwdev->core_id),
		 MALIDP_CORE_ID_STATUS(hwdev->core_id));

	hwdev->ls_info = malidp_hw_get_ls_info(hwdev, hw_desc);
	if (!hwdev->ls_info) {
		dev_err(hwdev->device, "%s : couldn't determine linesize config\n",
			__func__);
		goto fail_clk;
	}

	/* Look for rotmem in order of priority: platform then HW config */
	if (hw_desc->pdata->rotmem_size)
		hwdev->rotmem_size = hw_desc->pdata->rotmem_size;
	else
		hwdev->rotmem_size = hwdev->ls_info->default_rotmem_size;

	hwdev->dp_api->disable_irq(hwdev);

	/* Set up the SE device structure */
	hwdev->se_dev = malidp_se_hw_init(hwdev, pdev,
			hw_desc->pdata, &hwdev->hw_lock);
	if (!hwdev->se_dev)
		goto fail_clk;

	/* Set up the DE device structure */
	hwdev->de_dev = malidp_de_hw_init(hwdev, pdev,
			hw_desc->pdata, &hwdev->hw_lock);
	if (!hwdev->de_dev)
		goto fail_clk;

	/* Enter config mode and do configuration reset */
	malidp_hw_runtime_resume(hwdev);

	/* Enter lowpower mode */
	malidp_hw_runtime_suspend(hwdev);
	/*
	 * Drop our reference on the APB clock.
	 */
	clk_disable_unprepare(hwdev->pclk);

	dev_dbg(&pdev->dev, "%s: success\n", __func__);

	return hwdev;

fail_clk:
	clk_disable_unprepare(hwdev->pclk);
	return ERR_PTR(-ENODEV);
}

void malidp_hw_exit(struct malidp_hw_device *hwdev)
{
	/* We need the APB clock for register accesses */
	clk_prepare_enable(hwdev->pclk);

	malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_CONFIG);

	hwdev->dp_api->disable_irq(hwdev);

	malidp_se_hw_exit(hwdev->se_dev);
	malidp_de_hw_exit(hwdev->de_dev);

	malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_POWERSAVE);

	clk_disable_unprepare(hwdev->pclk);
}

u32 malidp_hw_rotmem_size_get(struct malidp_hw_device *hwdev)
{
	return hwdev->rotmem_size;
}

u32 malidp_hw_get_fifo_size(struct malidp_hw_device *hwdev)
{
	return hwdev->ls_info->input_fifo_size;
}

void malidp_hw_supported_dimensions_get(struct malidp_hw_device *hwdev,
					u32 *min_width, u32 *min_height,
					u32 *max_width, u32 *max_height)
{
	if (min_width)
		*min_width = hwdev->ls_info->min_line_size;
	if (min_height)
		*min_height = hwdev->ls_info->min_line_size;
	if (max_width)
		*max_width = hwdev->ls_info->max_line_size;
	if (max_height)
		*max_height = hwdev->ls_info->max_line_size;
}

u32 malidp_hw_clock_ratio_get(struct malidp_hw_device *hwdev)
{
	u64 current_mclk;
	u32 clock_r;

	if (hwdev->mclk == hwdev->pxclk)
		return 1 << 16;

	current_mclk = clk_get_rate(hwdev->mclk) / 1000;
	clock_r = clk_get_rate(hwdev->pxclk) / 1000;

	current_mclk <<= 16;
	do_div(current_mclk, clock_r);
	clock_r = current_mclk;
	return clock_r;
}

int malidp_hw_clock_ratio_set(struct malidp_hw_device *hwdev,
		u32 new_clock_ratio)
{
	unsigned long flags;

	if (new_clock_ratio < 0x10000) {
		dev_err(hwdev->device, "clock ratio is less than 1.");
		return -EINVAL;
	}

	if ((hwdev->mclk == hwdev->pxclk) && new_clock_ratio != 0x10000) {
		dev_err(hwdev->device, "mclk and pxclk are shared. Ratio must be 1.\n");
		return -EINVAL;
	}

	spin_lock_irqsave(&hwdev->hw_lock, flags);
	hwdev->clock_ratio = new_clock_ratio;
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);
	return 0;
}

int malidp_hw_set_mclk(struct malidp_hw_device *hwdev, u32 pxclk)
{
	u32 mclk;
	u64 tmp64 = pxclk;

	if (pxclk == 0)
		return -EINVAL;

	if (hwdev->pxclk == hwdev->mclk)
		return 0;

	tmp64 *= hwdev->clock_ratio;
	tmp64 >>= 16;
	mclk = tmp64;

	/*
	 * This can fail if mclk is fixed, but we will check the actual
	 * rate later
	 */
	clk_set_rate(hwdev->mclk, mclk);

	return 0;
}

enum malidp_hw_partition_type malidp_hw_rotmem_type_get(struct malidp_hw_device *hwdev)
{
	return hwdev->partition_type;
}

const struct malidp_hw_topology *malidp_hw_get_topology(struct malidp_hw_device *hwdev)
{
	return hwdev->topology;
}

int malidp_hw_get_attr(struct malidp_hw_device *hwdev, u32 attr, u32 *val)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&hwdev->hw_lock, flags);

	switch (attr & (MALIDP_ATTR_FLAG_DE | MALIDP_ATTR_FLAG_SE)) {
	case MALIDP_ATTR_FLAG_DE:
		ret = malidp_de_get_attr(hwdev->de_dev, attr, val);
		break;
	case MALIDP_ATTR_FLAG_SE:
		ret = malidp_se_get_attr(hwdev->se_dev, attr, val);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	return ret;
}

static int malidp_hw_save_attr(struct malidp_hw_device *hwdev, u32 attr, u32 val)
{
	int ret;

	switch (attr & (MALIDP_ATTR_FLAG_DE | MALIDP_ATTR_FLAG_SE)) {
	case MALIDP_ATTR_FLAG_DE:
		ret = malidp_de_save_attr(hwdev->de_dev, attr, val);
		break;
	case MALIDP_ATTR_FLAG_SE:
		ret = malidp_se_save_attr(hwdev->se_dev, attr, val);
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

int malidp_hw_set_attr(struct malidp_hw_device *hwdev, u32 attr, u32 val)
{
	int ret;
	unsigned long flags;
	enum malidp_op_mode de_mode;

	mutex_lock(&hwdev->power_mutex);
	de_mode = malidp_de_get_op_mode(hwdev->de_dev);

	if (de_mode == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * If PM runtime is enabled, we just save the attributes
		 * to device for PSM, they will be reset when the device
		 * is resumed.
		 */
		ret = malidp_hw_save_attr(hwdev, attr, val);
		goto exit;
	}

	if (de_mode == MALIDP_OP_MODE_NORMAL) {
		if ((attr & MALIDP_ATTR_FLAG_CM)) {
			dev_err(hwdev->device, "%s: can't set attr %u with display on\n", __func__, attr);
			ret = -EBUSY;
			goto exit;
		}
		/* If attr doesn't need CFM, then we don't need to change */
	} else {
		/*
		 * We need to make sure we aren't in PSM so we can access
		 * registers, even if the attr doesn't specifically need CFM.
		 */
		malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_CONFIG);
	}

	spin_lock_irqsave(&hwdev->hw_lock, flags);

	switch (attr & (MALIDP_ATTR_FLAG_DE | MALIDP_ATTR_FLAG_SE)) {
	case MALIDP_ATTR_FLAG_DE:
		ret = malidp_de_set_attr(hwdev->de_dev, attr, val);
		break;
	case MALIDP_ATTR_FLAG_SE:
		ret = malidp_se_set_attr(hwdev->se_dev, attr, val);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	malidp_hw_change_op_mode(hwdev, de_mode);

exit:
	mutex_unlock(&hwdev->power_mutex);
	return ret;
}

/*
 * Pass-through the gamma settings to DE dev
 */
void malidp_hw_update_gamma_settings(struct malidp_hw_device *hwdev,
				     bool enable,
				     u32 *coeffs)
{
	unsigned long flags;

	spin_lock_irqsave(&hwdev->hw_lock, flags);
	malidp_de_update_gamma_settings(hwdev->de_dev, enable, coeffs);
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);
}

void malidp_hw_set_de_output_depth(struct malidp_hw_device *hwdev, u8 red,
		u8 green, u8 blue)
{
	unsigned long flags;
	spin_lock_irqsave(&hwdev->hw_lock, flags);
	malidp_de_store_output_depth(hwdev->de_dev, red, green, blue);
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);
}

/*
 * The xy CIE coordinates should be in 10bits representing values
 * from 0 to 1023/1024 as reported by the EDID standard.
 */
int malidp_hw_update_color_adjustment(struct malidp_hw_device *hwdev,
	u16 red_x, u16 red_y, u16 green_x, u16 green_y,
	u16 blue_x, u16 blue_y, u16 white_x, u16 white_y)
{
	return malidp_de_update_cadj_coeffs(hwdev->de_dev, red_x, red_y,
		green_x, green_y, blue_x, blue_y, white_x, white_y);
}

u32 malidp_hw_downscaling_threshold(struct malidp_hw_device *hwdev)
{
	return hwdev->downscaling_threshold;
}

static int malidp_hw_dbg_dump_clock(struct seq_file *dump_file,
		void *x)
{
	struct malidp_hw_device *hwdev = dump_file->private;

	return seq_printf(dump_file, "mclk=%luHz pxclk=%luHz\n",
				clk_get_rate(hwdev->mclk),
				clk_get_rate(hwdev->pxclk));
}

static int malidp_hw_dbg_open(struct inode *inode, struct file *pfile)
{
	return single_open(pfile, malidp_hw_dbg_dump_clock,
			inode->i_private);
}

static const struct file_operations f_ops_clock = {
	.owner = THIS_MODULE,
	.open = malidp_hw_dbg_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int malidp_hw_debugfs_init(struct malidp_hw_device *hwdev,
	struct dentry *parent)
{
	struct dentry *dbg_file;

	if (hwdev->dbg_folder != NULL)
		return 0;

	hwdev->dbg_folder = debugfs_create_dir("hardware", parent);
	if (hwdev->dbg_folder == NULL)
		return -EINVAL;

	malidp_de_debugfs_init(hwdev->de_dev, hwdev->dbg_folder);

	dbg_file = debugfs_create_file("clock", S_IROTH,
			hwdev->dbg_folder, hwdev, &f_ops_clock);
	if (dbg_file == NULL)
		dev_err(hwdev->device, "debugfs clock is created error!\n");

	dbg_file = debugfs_create_u32("rotmem", S_IRUSR | S_IWUSR,
			hwdev->dbg_folder, &hwdev->rotmem_size);
	if (dbg_file == NULL)
		dev_err(hwdev->device, "couldn't create rotmem debugfs file\n");

	dbg_file = debugfs_create_u32("maxline", S_IRUSR | S_IRGRP | S_IROTH,
			hwdev->dbg_folder, (u32 *)&hwdev->ls_info->max_line_size);
	if (dbg_file == NULL)
		dev_err(hwdev->device, "couldn't create maxline debugfs file\n");

	if (hwdev->dp_api->debugfs_func != NULL)
		hwdev->dp_api->debugfs_func(hwdev);

	return 0;
}

struct malidp_hw_event_queue *malidp_hw_event_queue_create(size_t n_events)
{
	struct malidp_hw_event_queue *queue;
	size_t size = sizeof(*queue) +
		(sizeof(struct malidp_hw_event) * n_events);

	queue = kzalloc(size, GFP_KERNEL);
	if (!queue)
		return NULL;

	spin_lock_init(&queue->lock);
	queue->n_events = n_events;
	queue->queue = (struct malidp_hw_event *)((char *)queue + sizeof(*queue));
	queue->head = NULL;
	queue->tail = queue->queue;
	return queue;
}

void malidp_hw_event_queue_destroy(struct malidp_hw_event_queue *queue)
{
	kfree(queue);
}

void malidp_hw_event_queue_enqueue(struct malidp_hw_event_queue *queue,
		struct malidp_hw_event *event)
{
	unsigned long flags;
	spin_lock_irqsave(&queue->lock, flags);

	*queue->tail = *event;

	/* If the queue gets full, log an error */
	if (queue->tail == queue->head)
		(*queue->tail).type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_QFULL;

	if (!queue->head)
		queue->head = queue->tail;

	queue->tail--;
	if (queue->tail < queue->queue)
		queue->tail = &queue->queue[queue->n_events - 1];

	spin_unlock_irqrestore(&queue->lock, flags);
}

void malidp_hw_event_queue_dequeue(struct malidp_hw_event_queue *queue,
		struct malidp_hw_event *event)
{
	unsigned long flags;
	spin_lock_irqsave(&queue->lock, flags);

	if (!queue->head) {
		event->type = MALIDP_HW_EVENT_NONE;
	} else {
		*event = *queue->head;

		queue->head--;
		if (queue->head < queue->queue)
			queue->head = &queue->queue[queue->n_events - 1];

		/* Are we empty? */
		if (queue->head == queue->tail)
			queue->head = NULL;
	}

	spin_unlock_irqrestore(&queue->lock, flags);
}

void malidp_hw_disable_all_layers_and_mw(struct malidp_hw_device *hwdev)
{
	struct malidp_se_mw_conf mw_conf = {
		.mode = MALIDP_SE_MW_DISABLE,
	};
	unsigned long flags;

	spin_lock_irqsave(&hwdev->hw_lock, flags);
	malidp_se_cfg_processing(hwdev->se_dev, &mw_conf, NULL);
	malidp_de_disable_all_layers(hwdev->de_dev);
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);
}

attr_visible_t malidp_hw_get_attr_visible_func(struct malidp_hw_device *hwdev)
{
	return hwdev->dp_api->attr_visible;
}

int malidp_hw_runtime_suspend(struct malidp_hw_device *hwdev)
{
	mutex_lock(&hwdev->power_mutex);
	malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_POWERSAVE);
	mutex_unlock(&hwdev->power_mutex);

	malidp_hw_disable_clocks(hwdev);
	return 0;
}

int malidp_hw_runtime_resume(struct malidp_hw_device *hwdev)
{
	int res;

	malidp_hw_enable_clocks(hwdev);

	mutex_lock(&hwdev->power_mutex);
	malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_CONFIG);
	mutex_unlock(&hwdev->power_mutex);

	res = hwdev->dp_api->se_api.hw_cfg(hwdev->se_dev);
	WARN_ON(res != 0);

	res = hwdev->dp_api->de_api.hw_cfg(hwdev->de_dev);
	WARN_ON(res != 0);

	return 0;
}

int malidp_hw_display_switch(struct malidp_hw_device *hwdev, bool power_on)
{

	if (power_on == true) {
		pm_runtime_get_sync(hwdev->device);

		mutex_lock(&hwdev->power_mutex);
		malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_NORMAL);
		mutex_unlock(&hwdev->power_mutex);
	} else {
		malidp_hw_disable_all_layers_and_mw(hwdev);

		mutex_lock(&hwdev->power_mutex);
		malidp_hw_change_op_mode(hwdev, MALIDP_OP_MODE_CONFIG);
		mutex_unlock(&hwdev->power_mutex);

		pm_runtime_put(hwdev->device);
	}

	return 0;
}

void malidp_hw_clear_mw(struct malidp_hw_state *hw_state)
{
	struct malidp_hw_state_priv *hw_priv = hw_state->hw_priv;
	int i;

	hw_priv->mw.mode = MALIDP_SE_MW_DISABLE;
	hw_priv->mw.buf = NULL;

	hw_priv->cmp_flow = MALIDP_DE_CMP_FLOW_INTERNAL;

	for (i = 0; i < MALIDP_MAX_LAYERS; i++) {
		if (hw_priv->layer_flow[i] == MALIDP_DE_LAYER_FLOW_SIMULT_SE0)
			hw_priv->layer_flow[i] = MALIDP_DE_LAYER_FLOW_LOCAL;
	}
}
