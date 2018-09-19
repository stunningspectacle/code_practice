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
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <asm/uaccess.h>
#include <uapi/drm/drm_mode.h>
#include <uapi/drm/drm_fourcc.h>
#include <uapi/video/malidp_adf.h>

#include "malidp_hw_types.h"
#include "malidp_product_api.h"
#include "malidp_de_device.h"

#define DE_N_QUEUE_EVENTS 24

#define DE_N_YUV2RGB_COEFFS 12
#define DE_MAX_OUT_DEPTH 12
#define DE_MIN_OUT_DEPTH 5

const char *const op_mode_name[] = {
	"MODE_NORMAL",
	"MODE_CONFIG",
	"MODE_POWERSAVE",
	"MODE_TEST",
	"MODE_UNKNOWN",
};

static const s32 malidp_de_bt601_narrow_coeffs[DE_N_YUV2RGB_COEFFS] = {
	1192,    0, 1634,
	1192, -401, -832,
	1192, 2066,    0,

	  64,  512,  512
};

static const s32 malidp_de_bt601_wide_coeffs[DE_N_YUV2RGB_COEFFS] = {
	1024,    0, 1436,
	1024, -352, -731,
	1024, 1815,    0,

	   0,  512,  512
};

static const s32 malidp_de_bt709_narrow_coeffs[DE_N_YUV2RGB_COEFFS] = {
	1192,    0, 1836,
	1192, -218, -546,
	1192, 2163,    0,

	  64,  512,  512
};

static const s32 malidp_de_bt709_wide_coeffs[DE_N_YUV2RGB_COEFFS] = {
	1024,    0, 1613,
	1024, -192, -479,
	1024, 1900,    0,

	   0,  512,  512
};


void malidp_de_write(struct malidp_de_device *dev,
					  u32 value, u32 reg)
{
	writel(value, dev->regs + reg);
}

u32 malidp_de_read(struct malidp_de_device *dev, u32 reg)
{
	return readl(dev->regs + reg);
}

void malidp_de_setbits(struct malidp_de_device *dev, u32 mask,
				     u32 reg)
{
	u32 data = malidp_de_read(dev, reg);
	data |= mask;
	malidp_de_write(dev, data, reg);
}

void malidp_de_clearbits(struct malidp_de_device *dev,
				       u32 mask, u32 reg)
{
	u32 data = malidp_de_read(dev, reg);
	data &= ~mask;
	malidp_de_write(dev, data, reg);
}

static const struct malidp_layer_hw_info *
malidp_de_get_layers(struct malidp_de_device *dev, int *n_layer)
{
	if (n_layer != NULL)
		*n_layer = dev->hwdev->topology->n_layers;

	return dev->hwdev->topology->layers;
}

/* Write the alpha lookup tables providing a linear interpolation */
void malidp_de_write_alpha_lookup(struct malidp_de_device *dev)
{
	/* Write the alpha lookup table for all the layers */
	const struct malidp_layer_hw_info *hw_layers;
	int n_layers, i;

	hw_layers = malidp_de_get_layers(dev, &n_layers);
	for (i = 0; i < n_layers; i++) {
		if (hw_layers[i].type != MALIDP_HW_LAYER_SMART) {
			malidp_de_write(dev, DE_L_ALPHA3(255) | DE_L_ALPHA2(170) |
					DE_L_ALPHA1(85) | DE_L_ALPHA0(0),
					hw_layers[i].regs_base + DE_REG_L_COMPOSE);
		}
	}
}


irqreturn_t malidp_de_irq_thread_handler(int irq, void *data)
{
	struct malidp_de_device *dev = data;
	void (*callback)(struct device *, void *, struct malidp_hw_event_queue *);
	void *callback_opaque;
	unsigned long flags;

	spin_lock_irqsave(dev->hw_lock, flags);
	callback = dev->flip_callback;
	callback_opaque = dev->callback_opaque;
	spin_unlock_irqrestore(dev->hw_lock, flags);

	if (callback)
		callback(dev->device, callback_opaque, dev->ev_queue);

	return IRQ_HANDLED;
}

static int malidp_de_mode_drm2hw(struct malidp_de_device *dev,
					 struct drm_mode_modeinfo *mode,
					 struct malidp_de_hwmode *hwmode)
{
	/* Initialize the structure */
	memset(hwmode, 0, sizeof(*hwmode));

	/* Sanity checks */
	if (mode->flags & DRM_MODE_FLAG_INTERLACE) {
		dev_err(dev->device, "interlace mode not supported\n");
		return -1;
	}

	/* drm_mode_modeinfo clocks are specified in kHz */
	hwmode->clock = mode->clock * 1000;

	hwmode->h_active = mode->hdisplay;
	hwmode->hfp = mode->hsync_start - mode->hdisplay;
	hwmode->hsw = mode->hsync_end - mode->hsync_start;
	hwmode->hbp = mode->htotal - mode->hsync_end;

	hwmode->v_active = mode->vdisplay;
	hwmode->vfp = mode->vsync_start - mode->vdisplay;
	hwmode->vsw = mode->vsync_end - mode->vsync_start;
	hwmode->vbp = mode->vtotal - mode->vsync_end;

	if (mode->flags & DRM_MODE_FLAG_PHSYNC)
		hwmode->hsync_pol_pos = 1;

	if (mode->flags & DRM_MODE_FLAG_PVSYNC)
		hwmode->vsync_pol_pos = 1;

	return 0;
}

int malidp_de_fmt_drm2hw(struct malidp_de_device *dev,
		struct malidp_hw_buffer *buf)
{
	int i, idx;
	int n_fmts = buf->hw_layer->n_supported_formats;
	const u32 *fmts = buf->hw_layer->supported_formats;
	const u32 *ids = buf->hw_layer->format_ids;
	u32 drm_fmt = dev->hwdev->dp_api->de_api.fmt_fixup(buf->fmt, buf->flags);

	for (i = 0; i < n_fmts; i++) {
		if (ids)
			idx = ids[i];
		else
			idx = i;

		if (fmts[i] == drm_fmt)
			return idx;
	}

	return -1;
}

void malidp_de_set_flip_callback(struct malidp_de_device *dev,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque)
{
	dev->flip_callback = callback;
	dev->callback_opaque = opaque;
}

void malidp_de_set_coeftab(struct malidp_de_device *dev,
		u32 table, const u32 *coeffs)
{
	u32 i;
	u16 coloradj = dev->de_regmap->coloradj_coeff;

	malidp_de_write(dev, table | DE_COEFTAB_INTAB_ADDR(0),
			coloradj + DE_REG_COEFTAB_ADDR);

	for (i = 0; i < DE_N_COEFTAB_COEFS; i++)
		malidp_de_write(dev, coeffs[i], coloradj + DE_REG_COEFTAB_DATA);
}

static void malidp_de_set_yuv2rgb_coeffs(struct malidp_de_device *dev,
		struct malidp_hw_buffer *buf)
{
	int i;
	u32 coeff_off = 0;
	const s32 *coeffs;
	const struct malidp_layer_hw_info *layer = buf->hw_layer;

	dev_dbg(dev->device, "%s", __func__);

	switch (buf->flags & MALIDP_FLAG_YUV_MASK) {
	case (MALIDP_FLAG_YUV_BT601 | MALIDP_FLAG_YUV_NARROW):
		coeffs = malidp_de_bt601_narrow_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT601 | MALIDP_FLAG_YUV_WIDE):
		coeffs = malidp_de_bt601_wide_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT709 | MALIDP_FLAG_YUV_NARROW):
		coeffs = malidp_de_bt709_narrow_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT709 | MALIDP_FLAG_YUV_WIDE):
		coeffs = malidp_de_bt709_wide_coeffs;
		break;
	default:
		BUG();
	}

	coeff_off = layer->yuv2rgb_reg_offset;
	BUG_ON(coeff_off == 0);

	if (coeffs != dev->yuv2rgb_coeffs[layer->index]) {
		dev->yuv2rgb_coeffs[layer->index] = coeffs;
		dev_dbg(dev->device, "%s : changing coefficients", __func__);
		/*
		 * This assumes the coefficient registers are adjacent in the
		 * register map
		 */
		for (i = 0; i < DE_N_YUV2RGB_COEFFS; i++) {
			malidp_de_write(dev, coeffs[i],
					coeff_off + (i * 4));
		}
	}
}

void malidp_de_cfg_cmp_flow(struct malidp_de_device *dev,
			enum malidp_de_flow_cmp_cfg cfg)
{
	const struct malidp_de_regmap *reg = dev->de_regmap;

	malidp_de_clearbits(dev, DE_SET_FLOWCFG(DE_SET_FLOWCFG_MASK),
			    reg->disp_func);
	malidp_de_setbits(dev, DE_SET_FLOWCFG(cfg), reg->disp_func);
}

void malidp_de_cfg_layer_flow(struct malidp_de_device *dev,
			const struct malidp_layer_hw_info *hw_layer,
			enum malidp_de_flow_layer_cfg cfg)
{
	int loff = hw_layer->regs_base;

	if (!(hw_layer->features & MALIDP_LAYER_FEATURE_SCALING))
		return;

	malidp_de_clearbits(dev, DE_L_FCFG(DE_L_FCFG_MASK),
			    loff + DE_REG_L_CONTROL);
	malidp_de_setbits(dev, DE_L_FCFG(cfg), loff + DE_REG_L_CONTROL);
}

enum malidp_de_flow_layer_cfg malidp_de_get_layer_flow(struct malidp_de_device *dev,
							const struct malidp_layer_hw_info *hw_layer)
{
	int loff = hw_layer->regs_base;
	u32 data;

	if (!(hw_layer->features & MALIDP_LAYER_FEATURE_SCALING))
		return MALIDP_DE_LAYER_FLOW_LOCAL;

	data = malidp_de_read(dev, loff + DE_REG_L_CONTROL);

	return DE_GET_L_FCFG(data);
}

static int malidp_de_set_alpha(struct malidp_de_device *dev,
		struct malidp_hw_buffer *buf)
{
	int loff = buf->hw_layer->regs_base;
	uint32_t mask = DE_L_PREMULT | DE_L_COMPOSE_BG | DE_L_COMPOSE_PIXEL | DE_L_ALPHA(0xFF);
	uint32_t value = DE_L_ALPHA(buf->layer_alpha);

	if (buf->alpha_mode & MALIDP_ALPHA_MODE_NONE) {
		value = DE_L_COMPOSE_BG | DE_L_ALPHA(0xFF);
	} else {
		if (buf->alpha_mode & MALIDP_ALPHA_MODE_PIXEL)
			value |= DE_L_COMPOSE_PIXEL;
		if (buf->alpha_mode & MALIDP_ALPHA_MODE_W_BG)
			value |= DE_L_COMPOSE_BG;
		if (buf->alpha_mode & MALIDP_ALPHA_MODE_PREMULT)
			value |= DE_L_PREMULT;
	}

	malidp_de_clearbits(dev, mask, loff + DE_REG_L_CONTROL);
	dev_dbg(dev->device, "%s : Setting alpha bits: 0x%08x", __func__, value);
	malidp_de_setbits(dev, value, loff + DE_REG_L_CONTROL);

	return 0;
}

void malidp_de_cfg_smart_state(struct malidp_de_device *dev,
			const struct malidp_hw_smart_layer_state *ls_state)
{
	int loff = ls_state->ls_hw_layer->regs_base;
	u16 bbox_width, bbox_height;

	bbox_width = ls_state->ls_bbox_right - ls_state->ls_bbox_left;
	bbox_height = ls_state->ls_bbox_bottom - ls_state->ls_bbox_top;

	malidp_de_write(dev, (DE_L_SIZE_V(bbox_height) | DE_L_SIZE_H(bbox_width)),
			loff + DE_REG_L_SIZE);
	malidp_de_write(dev, (DE_L_SIZE_V(bbox_height) | DE_L_SIZE_H(bbox_width)),
			loff + DE_REG_L_SIZE_CMP);
	malidp_de_write(dev, (DE_L_OFFSET_V(ls_state->ls_bbox_top) | DE_L_OFFSET_H(ls_state->ls_bbox_left)),
			loff + DE_REG_L_OFFSET);
	malidp_de_write(dev, ls_state->ls_bbox_argb, loff + DE_REG_LS_BBOX_ARGB);

	/* Enable layer */
	malidp_de_write(dev, DE_LS_EN_NUM(ls_state->n_smart_layers),
			  loff + DE_REG_LS_ENABLE);
	malidp_de_setbits(dev, DE_L_EN, loff + DE_REG_L_CONTROL);

	dev->scene_changing = true;
}

static int malidp_de_cfg_smart_layer(struct malidp_de_device *dev,
				     struct malidp_hw_buffer *buf)
{
	const struct malidp_layer_hw_info *hw_layer = buf->hw_layer;
	const u32 ls_rect_reg_offset = buf->ls_rect_idx * DE_REG_LS_Rn_ADDR_DELTA;
	const u32 ls_rn_in_size = hw_layer->ls_r1_in_size + ls_rect_reg_offset;
	const u32 ls_rn_offset  = hw_layer->ls_r1_offset + ls_rect_reg_offset;
	const u32 ls_rn_stride  = hw_layer->ls_r1_stride + ls_rect_reg_offset;
	const u32 ls_rn_ptr_low = hw_layer->ls_r1_ptr_low + ls_rect_reg_offset;
	const u32 ls_rn_ptr_high = hw_layer->ls_r1_ptr_high + ls_rect_reg_offset;
	int loff = hw_layer->regs_base;

	/* Sizes and offset */
	malidp_de_write(dev, (DE_L_SIZE_V(buf->natural_h) | DE_L_SIZE_H(buf->natural_w)),
			loff + ls_rn_in_size);
	malidp_de_write(dev, (DE_L_OFFSET_V(buf->v_offset) | DE_L_OFFSET_H(buf->h_offset)),
			loff + ls_rn_offset);

	/* Only need set format and alpha of the first buffer because all the smart
	 * layers are sharing a same format and alpha. */
	if (buf->ls_rect_idx == 0) {
		malidp_de_set_alpha(dev, buf);
		malidp_de_clearbits(dev, DE_L_IGEN, loff + DE_REG_L_CONTROL);
		if (buf->flags & MALIDP_FLAG_SRGB) {
			malidp_de_clearbits(dev, DE_L_IGSEL_MASK,
				loff + DE_REG_L_CONTROL);
			malidp_de_setbits(dev, DE_L_IGEN | DE_L_IGSEL_SRGB,
				loff + DE_REG_L_CONTROL);
		}
		malidp_de_clearbits(dev, DE_L_SET_FMT(DE_L_FMT_MASK),
				    loff + DE_REG_L_FORMAT);
		malidp_de_setbits(dev, DE_L_SET_FMT(buf->hw_fmt),
				  loff + DE_REG_L_FORMAT);
	}

	/* Set HW pointers */
	if (buf->n_planes > buf->hw_layer->n_max_planes)
		return -EINVAL;

	malidp_de_write(dev, buf->pitch[0], loff + ls_rn_stride);
	malidp_de_write(dev, lower_32_bits(buf->addr[0]), loff + ls_rn_ptr_low);
	malidp_de_write(dev, upper_32_bits(buf->addr[0]), loff + ls_rn_ptr_high);

	return 0;
}

int malidp_de_cfg_layer(struct malidp_de_device *dev,
			struct malidp_hw_buffer *buf)
{
	int loff = buf->hw_layer->regs_base;
	bool fmt_is_yuv;
	u32 w, h;
	u32 comp_w, comp_h;

	if (buf->hw_layer->type == MALIDP_HW_LAYER_SMART) {
		return malidp_de_cfg_smart_layer(dev, buf);
	}

	comp_w = buf->cmp_rect.dest_w;
	comp_h = buf->cmp_rect.dest_h;
	w = buf->natural_w;
	h = buf->natural_h;

	malidp_de_clearbits(dev, DE_AD_EN | DE_AD_YTR | DE_AD_BS, buf->hw_layer->ad_ctrl_reg);
	if (buf->flags & MALIDP_FLAG_AFBC) {
		u32 ytr = buf->flags & MALIDP_FLAG_AFBC_YTR ? DE_AD_YTR : 0;
		u32 bs = buf->flags & MALIDP_FLAG_AFBC_SPLITBLK ? DE_AD_BS : 0;
		u32 crop_h, crop_v;

		w = buf->natural_w + buf->afbc_crop_l + buf->afbc_crop_r;
		h = buf->natural_h + buf->afbc_crop_t + buf->afbc_crop_b;

		dev_dbg(dev->device, "%s : configuring for AFBC buffer", __func__);

		crop_h = DE_AD_CROP_RIGHT(buf->afbc_crop_r) | DE_AD_CROP_LEFT(buf->afbc_crop_l);
		crop_v = DE_AD_CROP_BOTTOM(buf->afbc_crop_b) | DE_AD_CROP_TOP(buf->afbc_crop_t);

		malidp_de_write(dev, crop_h, buf->hw_layer->ad_crop_h_reg);
		malidp_de_write(dev, crop_v, buf->hw_layer->ad_crop_v_reg);
		malidp_de_setbits(dev, DE_AD_EN | ytr | bs, buf->hw_layer->ad_ctrl_reg);

		dev_dbg(dev->device, "%s : dimensions: %ix%i, crop: 0x%08x, 0x%08x",
				__func__, w, h, crop_h, crop_v);
	}

	/* Transform */
	malidp_de_clearbits(dev, DE_L_TRANS_MASK, loff + DE_REG_L_CONTROL);
	malidp_de_setbits(dev, DE_L_SET_TRANS(buf->transform), loff + DE_REG_L_CONTROL);

	/* Sizes and offset */
	malidp_de_write(dev, (DE_L_SIZE_V(h) | DE_L_SIZE_H(w)),
			loff + DE_REG_L_SIZE);
	malidp_de_write(dev, (DE_L_SIZE_V(comp_h) | DE_L_SIZE_H(comp_w)),
			loff + DE_REG_L_SIZE_CMP);
	malidp_de_write(dev, (DE_L_OFFSET_V(buf->v_offset) | DE_L_OFFSET_H(buf->h_offset)),
			loff + DE_REG_L_OFFSET);

	/* Set format of the buffer */
	malidp_de_clearbits(dev, DE_L_SET_FMT(DE_L_FMT_MASK),
			    loff + DE_REG_L_FORMAT);
	malidp_de_setbits(dev, DE_L_SET_FMT(buf->hw_fmt),
			  loff + DE_REG_L_FORMAT);

	/* Set up alpha blending */
	if (malidp_de_set_alpha(dev, buf) < 0)
		return -EINVAL;

	/* Set YUV coefficients if necessary */
	fmt_is_yuv = malidp_hw_format_is_yuv(buf->fmt);
	if (fmt_is_yuv)
		malidp_de_set_yuv2rgb_coeffs(dev, buf);

	/* Set inverse gamma/sRGB */
	if (buf->flags & MALIDP_FLAG_SRGB) {
		malidp_de_clearbits(dev, DE_L_IGSEL_MASK,
			loff + DE_REG_L_CONTROL);
		malidp_de_setbits(dev, DE_L_IGEN | DE_L_IGSEL_SRGB,
			loff + DE_REG_L_CONTROL);
	} else if (fmt_is_yuv && !(buf->flags & MALIDP_FLAG_FORCE_NO_IGAMMA)) {
		dev_dbg(dev->device, "%s : enabling inverse gamma\n",
			__func__);
		if (malidp_hw_buf_support_srgb(buf) == true)
			malidp_de_clearbits(dev, DE_L_IGSEL_MASK,
				loff + DE_REG_L_CONTROL);
		malidp_de_setbits(dev, DE_L_IGEN, loff + DE_REG_L_CONTROL);
	} else {
		dev_dbg(dev->device, "%s : disabling inverse gamma\n",
			__func__);
		malidp_de_clearbits(dev, DE_L_IGEN, loff + DE_REG_L_CONTROL);
	}

	/* Set HW pointers */
	if (buf->n_planes > buf->hw_layer->n_max_planes)
		return -EINVAL;

	switch (buf->n_planes) {
	case 3:
		if (buf->hw_layer->p3_stride_offset)
			malidp_de_write(dev, buf->pitch[2], loff + buf->hw_layer->p3_stride_offset);
		malidp_de_write(dev, lower_32_bits(buf->addr[2]), loff + DE_REG_LV3_PTR0_LOW);
		malidp_de_write(dev, upper_32_bits(buf->addr[2]), loff + DE_REG_LV3_PTR0_HIGH);
		/* Fallthrough */
	case 2:
		malidp_de_write(dev, buf->pitch[1], loff + DE_REG_LV2_STRIDE);
		malidp_de_write(dev, lower_32_bits(buf->addr[1]), loff + DE_REG_LV2_PTR0_LOW);
		malidp_de_write(dev, upper_32_bits(buf->addr[1]), loff + DE_REG_LV2_PTR0_HIGH);
		/* Fallthrough */
	case 1:
		malidp_de_write(dev, buf->pitch[0],
				loff + buf->hw_layer->stride_offset);
		malidp_de_write(dev, lower_32_bits(buf->addr[0]),
				loff + buf->hw_layer->ptr0_low_offset);
		malidp_de_write(dev, upper_32_bits(buf->addr[0]),
				loff + buf->hw_layer->ptr0_high_offset);
	}

	/* Enable layer */
	malidp_de_setbits(dev, DE_L_EN, loff + DE_REG_L_CONTROL);

	dev->scene_changing = true;

	return 0;
}

void malidp_de_disable_all_layers(struct malidp_de_device *dev)
{
	const struct malidp_layer_hw_info *hw_layers;
	int n_layers, i;

	hw_layers = malidp_de_get_layers(dev, &n_layers);
	for (i = 0; i < n_layers; i++)
		malidp_de_clearbits(dev, DE_L_EN,
			hw_layers[i].regs_base + DE_REG_L_CONTROL);
}

void malidp_de_cleanup_yuv2rgb_coeffs(struct malidp_de_device *dev)
{
	int i;

	for (i = 0; i < MALIDP_MAX_LAYERS; i++)
		dev->yuv2rgb_coeffs[i] = NULL;
}

void malidp_de_store_output_depth(struct malidp_de_device *dev,
	u8 red, u8 green, u8 blue)
{
	if ((red > DE_MAX_OUT_DEPTH) || (green > DE_MAX_OUT_DEPTH) ||
	    (blue > DE_MAX_OUT_DEPTH)) {
		dev_warn(dev->device, "%s : depth exceeds maximum (%d), values will be truncated\n",
			 __func__, DE_MAX_OUT_DEPTH);
		if (red > DE_MAX_OUT_DEPTH)
			red = DE_MAX_OUT_DEPTH;
		if (green > DE_MAX_OUT_DEPTH)
			green = DE_MAX_OUT_DEPTH;
		if (blue > DE_MAX_OUT_DEPTH)
			blue = DE_MAX_OUT_DEPTH;
	}
	if ((red < DE_MIN_OUT_DEPTH) || (green < DE_MIN_OUT_DEPTH) ||
	    (blue < DE_MIN_OUT_DEPTH)) {
		dev_warn(dev->device, "%s : depth is less than minimum (%d), values will be increased\n",
			 __func__, DE_MIN_OUT_DEPTH);
		if (red < DE_MIN_OUT_DEPTH)
			red = DE_MIN_OUT_DEPTH;
		if (green < DE_MIN_OUT_DEPTH)
			green = DE_MIN_OUT_DEPTH;
		if (blue < DE_MIN_OUT_DEPTH)
			blue = DE_MIN_OUT_DEPTH;
	}

	dev->red_bits = red;
	dev->green_bits = green;
	dev->blue_bits = blue;
}

static void malidp_de_set_output_depth(struct malidp_de_device *dev)
{
	const struct malidp_de_regmap *reg = dev->de_regmap;

	malidp_de_write(dev, DE_OUT_DEPTH_R(dev->red_bits) |
			DE_OUT_DEPTH_G(dev->green_bits) |
			DE_OUT_DEPTH_B(dev->blue_bits),
			reg->output_depth);
}

int malidp_de_modeset(struct malidp_de_device *dev,
			     struct drm_mode_modeinfo *mode)
{
	struct malidp_de_hwmode hwmode;
	int ret;
	u32 reg_addr;
	const struct malidp_de_regmap *reg = dev->de_regmap;
	const struct malidp_de_product_api *de_api =
				&dev->hwdev->dp_api->de_api;

	ret = malidp_de_mode_drm2hw(dev, mode, &hwmode);
	if (ret < 0)
		return ret;

	de_api->modeset(dev, &hwmode);
	memcpy(&dev->current_mode, mode, sizeof(struct drm_mode_modeinfo));

	/* Set up dithering */
	malidp_de_set_output_depth(dev);

	/* Program the gamma coefficients table */
	if (dev->gamma_enabled) {
		malidp_de_clearbits(dev, DE_GAMMA_EN, reg->disp_func);
		de_api->set_gamma_coeff(dev, dev->gamma_coeffs);
		malidp_de_setbits(dev, DE_GAMMA_EN, reg->disp_func);
	} else {
		malidp_de_clearbits(dev, DE_GAMMA_EN, reg->disp_func);
	}

	/* Writing color adjustments coefficients */
	malidp_de_clearbits(dev, DE_CADJ_EN, reg->disp_func);
	reg_addr = reg->coloradj_coeff;
	for (ret = 0; ret < DE_N_COLORADJ_COEFS; ret++) {
		malidp_de_write(dev, dev->color_adjustment_coeffs[ret],
			reg_addr);
		reg_addr += 4;
	}
	malidp_de_setbits(dev, DE_CADJ_EN, reg->disp_func);

	/* Disable the layers to make sure we don't try to show a bad scene */
	malidp_de_disable_all_layers(dev);

	return 0;
}

void malidp_de_modeget(struct malidp_de_device *dev,
		       struct drm_mode_modeinfo *mode)
{
	memcpy(mode, &dev->current_mode, sizeof(struct drm_mode_modeinfo));
}

enum malidp_op_mode malidp_de_get_op_mode(struct malidp_de_device *dev)
{
	return dev->op_mode;
}

bool malidp_de_attr_valid(struct malidp_de_device *dev,
				 u32 attr, u32 val)
{
	u32 fifo_size = malidp_hw_get_fifo_size(dev->hwdev);

	switch (attr) {
	case MALIDP_ATTR_DE_RQOS_LOW:
		if ((val < 1) || (val > dev->arqos_threshold_high))
			return false;
		break;
	case MALIDP_ATTR_DE_RQOS_HIGH:
		if ((val < dev->arqos_threshold_low) || (val > (fifo_size - 1)))
			return false;
		break;
	case MALIDP_ATTR_DE_RQOS_RED:
	case MALIDP_ATTR_DE_RQOS_GREEN:
		if (val > 0xF)
			return false;
		break;
	default:
		return dev->hwdev->dp_api->de_api.axi_valid(attr, val);
	}

	return true;
}

void malidp_de_set_axi_cfg(struct malidp_de_device *dev, u32 outstran,
				  u32 poutstdcab, u32 burstlen)
{
	/*TODO (LEOSW-312): need new object to handle AXI stuff */
	const struct malidp_de_regmap *reg = dev->de_regmap;

	dev_dbg(dev->device, "%s: outstran: %i, burstlen: %i\n",
		__func__, outstran, burstlen);

	dev->outstran = outstran;
	dev->burstlen = burstlen;
	dev->poutstdcab = poutstdcab;
	malidp_de_write(dev, DE_AXI_OUTSTDCAPB(dev->outstran) |
			DE_AXI_POUTSTDCAB(poutstdcab) |
			DE_AXI_BURSTLEN(dev->burstlen - 1),
			reg->axi_control);
}

void malidp_de_init_axi_qos(struct malidp_de_device *dev,
				   u32 low, u32 high,
				   u32 red_code, u32 green_code)
{
	u32 fifo_size = malidp_hw_get_fifo_size(dev->hwdev);
	u32 qos_reg_val = 0;

	dev_dbg(dev->device,
		"%s: low: 0x%X, high: 0x%X, red: 0x%X, green: 0x%X\n",
		__func__, low, high, red_code, green_code);

	/* trim the threshold values */
	low = low > 0 ? low : DE_DEFAULT_AXI_ARQOS_LOW;
	high = high > 0 ? high : DE_DEFAULT_AXI_ARQOS_HIGH;
	high = high < fifo_size ? high : fifo_size;
	low = low < high ? low : high;

	/* program the qos register */
	qos_reg_val |= DE_RQOS_LOW(low);
	qos_reg_val |= DE_RQOS_HIGH(high);
	qos_reg_val |= DE_RQOS_RED(red_code);
	qos_reg_val |= DE_RQOS_GREEN(green_code);
	malidp_de_write(dev, qos_reg_val, dev->de_regmap->qos_control);

	/* save the rqos settings for sysfs */
	dev->arqos_threshold_low = low;
	dev->arqos_threshold_high = high;
	dev->arqos_red = red_code;
	dev->arqos_green = green_code;
}

struct malidp_de_device  *malidp_de_hw_init(struct malidp_hw_device *hwdev,
			     struct platform_device *pdev,
			     struct malidp_hw_pdata *pdata,
			     spinlock_t *hw_lock)
{
	int res;
	struct malidp_de_device *dev = devm_kzalloc(&pdev->dev,
					sizeof(struct malidp_de_device),
						    GFP_KERNEL);
	if (!dev)
		return NULL;

	dev->hw_lock = hw_lock;
	dev->hwdev = hwdev;
	dev->regs = pdata->regs + hwdev->hw_regmap->de_base;
	dev->de_regmap = &hwdev->hw_regmap->de_regmap;
	dev->scene_changing = false;
	dev->gamma_enabled = false;

	dev->device = &pdev->dev;

	res = devm_request_threaded_irq(dev->device,
			pdata->de_irq,
			hwdev->dp_api->de_api.irq_handler,
			malidp_de_irq_thread_handler,
			IRQF_SHARED, "malidp-de", dev);
	if (res < 0) {
		dev_err(&pdev->dev, "%s: failed to request 'de' irq\n", __func__);
		return NULL;
	}

	dev->ev_queue = malidp_hw_event_queue_create(DE_N_QUEUE_EVENTS);
	if (!dev->ev_queue)
		return NULL;

	/* Set identity matrix to coefficients,
	* no conversion as default.
	*/
	dev->color_adjustment_coeffs[0] = 4096;
	dev->color_adjustment_coeffs[4] = 4096;
	dev->color_adjustment_coeffs[8] = 4096;

	dev->op_mode = MALIDP_OP_MODE_UNKNOWN;

	dev->outstran = pdata->de_axi_outstran;
	dev->poutstdcab = pdata->de_axi_poutstdcab;
	dev->burstlen = pdata->de_axi_burstlen;

	dev->arqos_threshold_low = pdata->de_axi_arqos_low;
	dev->arqos_threshold_high = pdata->de_axi_arqos_high;
	dev->arqos_red = pdata->de_axi_arqos_red;
	dev->arqos_green = pdata->de_axi_arqos_green;

	dev_dbg(dev->device, "%s : success!\n", __func__);

	return dev;
}

void malidp_de_hw_exit(struct malidp_de_device *dev)
{
	malidp_hw_event_queue_destroy(dev->ev_queue);
	return;
}

int malidp_de_get_attr(struct malidp_de_device *dev, u32 attr, u32 *val)
{
	u32 fifo_size = malidp_hw_get_fifo_size(dev->hwdev);

	switch (attr) {
	case MALIDP_ATTR_DE_BURSTLEN:
		*val = dev->burstlen;
		break;
	case MALIDP_ATTR_DE_POUTSTDCAB:
		*val = dev->poutstdcab;
		break;
	case MALIDP_ATTR_DE_OUTSTRAN:
		*val = dev->outstran;
		break;
	case MALIDP_ATTR_DE_RQOS_LOW:
		*val = dev->arqos_threshold_low;
		break;
	case MALIDP_ATTR_DE_RQOS_HIGH:
		*val = dev->arqos_threshold_high;
		break;
	case MALIDP_ATTR_DE_RQOS_RED:
		*val = dev->arqos_red;
		break;
	case MALIDP_ATTR_DE_RQOS_GREEN:
		*val = dev->arqos_green;
		break;
	case MALIDP_ATTR_DE_FIFO_SIZE:
		*val = fifo_size;
		break;
	default:
		dev_err(dev->device, "%s: unkown DE attribute %i\n",
			__func__, attr);
		return -EINVAL;
	}

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n",
		__func__, attr, *val);

	return 0;
}

int malidp_de_set_attr(struct malidp_de_device *dev, u32 attr, u32 val)
{
	int ret = 0;

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n", __func__, attr, val);

	if (!malidp_de_attr_valid(dev, attr, val)) {
		dev_dbg(dev->device, "%s: invalid value %u for attr %u\n",
			__func__, val, attr);
		return -EINVAL;
	}

	switch (attr) {
	case MALIDP_ATTR_DE_BURSTLEN:
		malidp_de_set_axi_cfg(dev, dev->outstran,
					dev->poutstdcab, val);
		break;
	case MALIDP_ATTR_DE_POUTSTDCAB:
		malidp_de_set_axi_cfg(dev, dev->outstran, val,
					dev->burstlen);
		break;
	case MALIDP_ATTR_DE_OUTSTRAN:
		malidp_de_set_axi_cfg(dev, val, dev->poutstdcab,
					dev->burstlen);
		break;
	case MALIDP_ATTR_DE_RQOS_LOW:
		malidp_de_init_axi_qos(dev, val, dev->arqos_threshold_high,
				       dev->arqos_red, dev->arqos_green);
		break;
	case MALIDP_ATTR_DE_RQOS_HIGH:
		malidp_de_init_axi_qos(dev, dev->arqos_threshold_low, val,
				       dev->arqos_red, dev->arqos_green);
		break;
	case MALIDP_ATTR_DE_RQOS_RED:
		malidp_de_init_axi_qos(dev, dev->arqos_threshold_low,
				       dev->arqos_threshold_high,
				       val, dev->arqos_green);
		break;
	case MALIDP_ATTR_DE_RQOS_GREEN:
		malidp_de_init_axi_qos(dev, dev->arqos_threshold_low,
				       dev->arqos_threshold_high,
				       dev->arqos_red, val);
		break;
	default:
		dev_err(dev->device, "%s: unkown DE attribute %i\n",
			__func__, attr);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int malidp_de_save_attr(struct malidp_de_device *dev, u32 attr, u32 val)
{
	int ret = 0;

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n", __func__, attr, val);

	if (!malidp_de_attr_valid(dev, attr, val)) {
		dev_dbg(dev->device, "%s: invalid value %u for attr %u\n",
			__func__, val, attr);
		return -EINVAL;
	}

	switch (attr) {
	case MALIDP_ATTR_DE_BURSTLEN:
		dev->burstlen = val;
		break;
	case MALIDP_ATTR_DE_POUTSTDCAB:
		dev->poutstdcab = val;
		break;
	case MALIDP_ATTR_DE_OUTSTRAN:
		dev->outstran = val;
		break;
	case MALIDP_ATTR_DE_RQOS_LOW:
		dev->arqos_threshold_low = val;
		break;
	case MALIDP_ATTR_DE_RQOS_HIGH:
		dev->arqos_threshold_high = val;
		break;
	case MALIDP_ATTR_DE_RQOS_RED:
		dev->arqos_red = val;
		break;
	case MALIDP_ATTR_DE_RQOS_GREEN:
		dev->arqos_green = val;
		break;
	default:
		dev_err(dev->device, "%s: unkown DE attribute %i\n",
			__func__, attr);
		ret = -EINVAL;
		break;
	}

	return ret;
}

/*
 * Update the gamma settings
 *
 * @enable: enable/disable the gamma correction.
 * @coeffs: the gamma coeffs table, it is omitted if @enable = false.
 */
void malidp_de_update_gamma_settings(struct malidp_de_device *dev,
			bool enable, u32 *coeffs)
{
	dev_dbg(dev->device, "%s: gamma enabled: %d\n", __func__, enable);

	dev->gamma_enabled = enable;
	if (dev->gamma_enabled) {
		if (coeffs != NULL) {
			memcpy(dev->gamma_coeffs, coeffs,
			       sizeof(u32) * DE_N_COEFTAB_COEFS);
		} else {
			dev_warn(dev->device, "%s: the gamma coeffs table is null\n",
				 __func__);
		}
	}
}

#define MANTISSA	12
#define FP12_1_0	(1 << MANTISSA)

static s32 fp_div(s32 dividend, s32 divisor)
{
	u64 a, b;
	int sign = 0;

	WARN_ON(divisor == 0);

	if (dividend < 0) {
		a = -dividend;
		sign = !sign;
	} else {
		a = dividend;
	}

	if (divisor < 0) {
		b = -divisor;
		sign = !sign;
	} else {
		b = divisor;
	}

	a <<= MANTISSA;
	do_div(a, b);
	dividend = a;

	return (sign == 0) ? dividend : -dividend;
}

static s32 fp_mul(s32 faciend, s32 multiplier)
{
	s64 t = faciend;

	t *= multiplier;
	faciend = (t >> MANTISSA);
	return faciend;
}

/*
*	Calculation of the inverse matrix
*		| a00 a01 a02 |
*		| a10 a11 a12 |
*		| a20 a21 a22 |
*	The result is stored into inverse[3][3]
*/
static int matrix_inverse(s32 inverse[3][3],
		s32 a00, s32 a01, s32 a02,
		s32 a10, s32 a11, s32 a12,
		s32 a20, s32 a21, s32 a22)
{
	s32 a_1 = fp_mul(a11, a22) - fp_mul(a12, a21);
	s32 a_2 = fp_mul(a12, a20) - fp_mul(a10, a22);
	s32 a_3 = fp_mul(a10, a21) - fp_mul(a11, a20);

	s32 determinant = fp_mul(a00, a_1) + fp_mul(a01, a_2) + fp_mul(a02, a_3);
	if (determinant == 0)
		return -EINVAL;

	inverse[0][0] = fp_div(a_1, determinant);
	inverse[1][0] = fp_div(a_2, determinant);
	inverse[2][0] = fp_div(a_3, determinant);

	a_1 = fp_mul(a02, a21) - fp_mul(a01, a22);
	a_2 = fp_mul(a00, a22) - fp_mul(a02, a20);
	a_3 = fp_mul(a01, a20) - fp_mul(a00, a21);

	inverse[0][1] = fp_div(a_1, determinant);
	inverse[1][1] = fp_div(a_2, determinant);
	inverse[2][1] = fp_div(a_3, determinant);

	a_1 = fp_mul(a01, a12) - fp_mul(a02, a11);
	a_2 = fp_mul(a02, a10) - fp_mul(a00, a12);
	a_3 = fp_mul(a00, a11) - fp_mul(a01, a10);

	inverse[0][2] = fp_div(a_1, determinant);
	inverse[1][2] = fp_div(a_2, determinant);
	inverse[2][2] = fp_div(a_3, determinant);
	return 0;
}

/* This routing makes the sum of every row in matrix not bigger than 1 */
static void normalize_matrix(s32 m[3][3])
{
	s32 sum_line[3], max_sum;
	int i, j;

	for (i = 0; i < 3; i++) {
		sum_line[i] = 0;
		for (j = 0; j < 3; j++)
			sum_line[i] += m[i][j];
	}

	max_sum = sum_line[0];
	for (i = 1; i < 3; i++) {
		if (max_sum < sum_line[i])
			max_sum = sum_line[i];
	}

	if (max_sum <= FP12_1_0)
		return;
	/* If max_sum is larger than 4096, we need get 4096/max_sum,
	* then, every element in the matrix multiple 4096/max_sum
	*/
	max_sum = fp_div(FP12_1_0, max_sum);
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++)
			m[i][j] = fp_mul(m[i][j], max_sum);
	}
}

/*
* This matrix is used for transforming from Rec. 709 RGB
* into CIE XYZ.
*/
static const s32 matrix709[3][3] = {
	{1689, 1465,  739},
	{ 871, 2929,  296},
	{  79,  488, 3892}
};

/*
 * Update color adjustment coefficients
 *	All the calculation is base on fix point number which format is
 *	S19.12 (32bit, signed)
 *	So, integer 4096 is actually for fix pointer number 1.0
 *	The xy coordinates should be in 10bits representing values
 *	from 0 to 1023/1024 as reported by the EDID standard.
 * For result:
 *	Coefficients are 15bit two's complement code.
 */
int malidp_de_update_cadj_coeffs(struct malidp_de_device *dev,
	u16 red_x, u16 red_y, u16 green_x, u16 green_y,
	u16 blue_x, u16 blue_y, u16 white_x, u16 white_y)
{
	s32 w_x, w_y, w_z;
	s32 r_x, r_y, r_z;
	s32 g_x, g_y, g_z;
	s32 b_x, b_y, b_z;
	s32 inverse[3][3], coeffs[3][3];
	s32 r_xyz, g_xyz, b_xyz;
	unsigned long flags;

	r_x = red_x << 2;
	r_y = red_y << 2;
	g_x = green_x << 2;
	g_y = green_y << 2;
	b_x = blue_x << 2;
	b_y = blue_y << 2;

	/*
	*	Algorithm for calculating M:
	*	    | r_x  g_x b_x |   |r_xyz   0     0   |
	*	M = | r_y  g_y b_y | * |  0   g_xyz   0   |
	*	    | r_z  g_z b_z |   |  0     0   b_xyz |
	*
	*	|r_xyz|          | r_x g_x b_x |   | w_x |
	*	|g_xyz| = inverse| r_y g_y b_y | * | w_y |
	*	|b_xyz|          | r_z g_z b_z |   | w_z |
	*
	*	w_x = white_x/white_y, w_y = 1,
	*	w_z = (1 - white_x - white_y)/white_y
	*/

	if (white_y == 0)
		return -EINVAL;

	w_x = white_x << 2;
	w_y = white_y << 2;
	w_z = FP12_1_0 - w_x - w_y;

	w_x = fp_div(w_x, w_y);
	w_z = fp_div(w_z, w_y);
	w_y = FP12_1_0;

	r_z = FP12_1_0 - r_x - r_y;
	g_z = FP12_1_0 - g_x - g_y;
	b_z = FP12_1_0 - b_x - b_y;

	if (matrix_inverse(inverse,
		r_x, g_x, b_x, r_y, g_y, b_y, r_z, g_z, b_z) != 0)
		return -EINVAL;

	r_xyz = fp_mul(inverse[0][0], w_x)
			+fp_mul(inverse[0][1], w_y)
			+fp_mul(inverse[0][2], w_z);

	g_xyz = fp_mul(inverse[1][0], w_x)
			+fp_mul(inverse[1][1], w_y)
			+fp_mul(inverse[1][2], w_z);

	b_xyz = fp_mul(inverse[2][0], w_x)
			+fp_mul(inverse[2][1], w_y)
			+fp_mul(inverse[2][2], w_z);

	if (matrix_inverse(inverse,
		fp_mul(r_x, r_xyz), fp_mul(g_x, g_xyz), fp_mul(b_x, b_xyz),
		fp_mul(r_y, r_xyz), fp_mul(g_y, g_xyz), fp_mul(b_y, b_xyz),
		fp_mul(r_z, r_xyz), fp_mul(g_z, g_xyz), fp_mul(b_z, b_xyz))
		!= 0)
		return -EINVAL;

	/* inverse(M) * Matrix 709 */
	for (r_x = 0; r_x < 3; r_x++) {
		for (r_y = 0; r_y < 3; r_y++) {
			coeffs[r_x][r_y] = 0;
			for (g_x = 0; g_x < 3; g_x++)
				coeffs[r_x][r_y] +=
					fp_mul(inverse[r_x][g_x], matrix709[g_x][r_y]);
		}
	}

	normalize_matrix(coeffs);

	spin_lock_irqsave(dev->hw_lock, flags);

	for (r_x = 0; r_x < 3; r_x++)
		for (r_y = 0; r_y < 3; r_y++)
			dev->color_adjustment_coeffs[r_x * 3 + r_y] =
				coeffs[r_x][r_y] & 0x7FFF;

	spin_unlock_irqrestore(dev->hw_lock, flags);

	return 0;
}

static int malidp_de_dbg_dump_cadj(struct seq_file *dump_file,
		void *v)
{
	struct malidp_de_device *dev = dump_file->private;
	int i, ret;

	for (i = 0; i < DE_N_COLORADJ_COEFS; i++) {
		ret = seq_printf(dump_file,
				"K%d = %u (0x%X)\n", i + 1,
				dev->color_adjustment_coeffs[i],
				(u32)dev->color_adjustment_coeffs[i]);
		if (ret != 0)
			return -EINVAL;
	}

	return 0;
}

static int malidp_de_dbg_open(struct inode *inode, struct file *pfile)
{
	return single_open(pfile, malidp_de_dbg_dump_cadj,
				inode->i_private);
}

static const struct file_operations f_ops_cadj = {
	.owner = THIS_MODULE,
	.open = malidp_de_dbg_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void malidp_de_debugfs_init(struct malidp_de_device *dev,
	struct dentry *folder)
{
	struct dentry *dbg_file;

	dbg_file = debugfs_create_file("de_color_adj",
			S_IROTH, folder, dev, &f_ops_cadj);
	if (dbg_file == NULL)
		dev_err(dev->device,
			"debugfs de_color_adj is not created!\n");
	/* More dumping */
}
