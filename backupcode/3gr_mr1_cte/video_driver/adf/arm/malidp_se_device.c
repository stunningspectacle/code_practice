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
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

#include <uapi/drm/drm_fourcc.h>
#include <uapi/video/malidp_adf.h>

#include "malidp_hw_types.h"
#include "malidp_product_api.h"
#include "malidp_se_device.h"
#include "malidp_de_device.h"

#define SE_N_QUEUE_EVENTS 8
#define SE_N_RGB2YUV_COEFFS 12
#define SE_N_ENH_COEFFS 9

#define SE_N_PHASE	4	/* 2^SE_N_PHASE is real number */
#define SE_SHIFT_N_PHASE 12

/* pre-define fix-point numbers */
#define FP_1_00000		0x00010000			/* 1.0 */
#define FP_0_66667		0x0000AAAA			/* 0.6667 = 1/1.5 */
#define FP_0_50000		0x00008000			/* 0.5 = 1/2 */
#define FP_0_36363		0x00005D17			/* 0.36363 = 1/2.75 */
#define FP_0_25000		0x00004000			/* 0.25 = 1/4 */
#define FP_0_12610		0x00002048			/* 0.12610 = 1/7.93 */

extern const char *const op_mode_name[];

static const s32 malidp_se_bt601_narrow_coeffs[SE_N_RGB2YUV_COEFFS] = {
	 66, 129,  25,
	-38, -74, 112,
	112, -94, -18,

	 16, 128, 128
};

static const s32 malidp_se_bt601_wide_coeffs[SE_N_RGB2YUV_COEFFS] = {
	 77,  150,  29,
	-43,  -85, 128,
	128, -107, -21,

	  0,  128, 128
};

static const s32 malidp_se_bt709_narrow_coeffs[SE_N_RGB2YUV_COEFFS] = {
	  47,  157,   16,
	 -26,  -87,  112,
	 112, -102,  -10,

	  16,  128,  128
};

static const s32 malidp_se_bt709_wide_coeffs[SE_N_RGB2YUV_COEFFS] = {
	  54,  183,  19,
	 -29,  -99, 128,
	 128, -116, -12,

	   0,  128, 128
};

static const s32 malidp_se_enhancer_coeffs[3][SE_N_ENH_COEFFS] = {
	[MALIDP_SE_ENHANCER_HORZ] = {
		  0,   0,   0, -32, 128, -32,   0,   0,   0
	},
	[MALIDP_SE_ENHANCER_VERT] = {
		  0, -32,   0,   0, 128,   0,   0, -32,   0
	},
	[MALIDP_SE_ENHANCER_BOTH] = {
		 -8,  -8,  -8,  -8, 128,  -8,  -8,  -8,  -8
	},
};

int malidp_se_fmt_drm2mw(struct malidp_se_device *dev, u32 drm_fmt)
{
	int i, idx;
	int n_fmts = dev->n_mw_formats;
	const u32 *fmts = dev->mw_formats;
	const u32 *ids = dev->mw_format_ids;

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

static enum malidp_scaling_coeff_set
malidp_se_scaling_coeffstab_select(u32 factor)
{
	/* NOTE: factor should be upscaling factor */
	if (factor >= FP_1_00000)
		return MALIDP_UPSCALING_COEFFS;
	else if ((factor < FP_1_00000) && (factor >= FP_0_66667))
		return MALIDP_DOWNSCALING_1_5_COEFFS;
	else if ((factor < FP_0_66667) && (factor >= FP_0_50000))
		return MALIDP_DOWNSCALING_2_COEFFS;
	else if ((factor < FP_0_50000) && (factor >= FP_0_36363))
		return MALIDP_DOWNSCALING_2_75_COEFFS;

	return MALIDP_DOWNSCALING_4_COEFFS;
}

void malidp_se_write(struct malidp_se_device *dev,
					  u32 value, u32 reg)
{
	writel(value, dev->regs + reg);
}

u32 malidp_se_read(struct malidp_se_device *dev, u32 reg)
{
	return readl(dev->regs + reg);
}

void malidp_se_setbits(struct malidp_se_device *dev, u32 mask,
				     u32 reg)
{
	u32 data = malidp_se_read(dev, reg);
	data |= mask;
	malidp_se_write(dev, data, reg);
}

void malidp_se_clearbits(struct malidp_se_device *dev,
				       u32 mask, u32 reg)
{
	u32 data = malidp_se_read(dev, reg);
	data &= ~mask;
	malidp_se_write(dev, data, reg);
}

/*
 * Set a user callback that will be triggered when the memory write out
 * interface finishes writing the current buffer.
 * @dev: pointer to the private malidp_de_device structure.
 * @callback: the user callback we want to call. This will have the
 *   the following arguments:
 *      @device: low level device structure
 *      event: the kind of event that has occurred in the SE.
 */
void malidp_se_set_flip_callback(struct malidp_se_device *dev,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque)
{
	dev->flip_callback = callback;
	dev->callback_opaque = opaque;
}

static void malidp_se_cfg_enhancer(struct malidp_se_device *dev,
		enum malidp_se_enhancer_cfg cfg)
{
	u32 control;
	u32 i;
	const struct malidp_se_regmap *reg = dev->se_regmap;

	/* Don't update anything unless we need to */
	if (dev->enh_cfg == cfg)
		return;

	control = malidp_se_read(dev, reg->control);
	control &= ~(SE_ENH_H_EN | SE_ENH_V_EN);

	switch (cfg) {
	case MALIDP_SE_ENHANCER_HORZ:
		control |= SE_ENH_H_EN;
		break;
	case MALIDP_SE_ENHANCER_VERT:
		control |= SE_ENH_V_EN;
		break;
	case MALIDP_SE_ENHANCER_BOTH:
		control |= (SE_ENH_H_EN | SE_ENH_V_EN);
		break;
	case MALIDP_SE_ENHANCER_OFF:
		break;
	default:
		BUG();
	}

	malidp_se_write(dev, control, reg->control);
	dev->enh_cfg = cfg;

	if (cfg == MALIDP_SE_ENHANCER_OFF) {
		dev_dbg(dev->device, "%s : Turning enhancer off\n", __func__);
		return;
	}

	dev_dbg(dev->device, "%s : Changing enhancer coefficients\n", __func__);

	/* This assumes the coefficients are adjacent in the register map */
	for (i = 0; i < SE_N_ENH_COEFFS; i++)
		malidp_se_write(dev, malidp_se_enhancer_coeffs[cfg][i],
			reg->enhancer_control + (i * 4) + SE_REG_ENH_COEFF1);
}

static void malidp_se_set_rgb2yuv_coeffs(struct malidp_se_device *dev,
		struct malidp_hw_buffer *buf)
{
	int i;
	const s32 *coeffs;
	const struct malidp_se_regmap *reg = dev->se_regmap;

	dev_dbg(dev->device, "%s", __func__);

	switch (buf->flags & MALIDP_FLAG_YUV_MASK) {
	case (MALIDP_FLAG_YUV_BT601 | MALIDP_FLAG_YUV_NARROW):
		coeffs = malidp_se_bt601_narrow_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT601 | MALIDP_FLAG_YUV_WIDE):
		coeffs = malidp_se_bt601_wide_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT709 | MALIDP_FLAG_YUV_NARROW):
		coeffs = malidp_se_bt709_narrow_coeffs;
		break;
	case (MALIDP_FLAG_YUV_BT709 | MALIDP_FLAG_YUV_WIDE):
		coeffs = malidp_se_bt709_wide_coeffs;
		break;
	default:
		BUG();
	}

	if (coeffs != dev->rgb2yuv_coeffs) {
		dev->rgb2yuv_coeffs = coeffs;
		dev_dbg(dev->device, "%s : changing coefficients", __func__);
		/*
		 * This assumes the coefficient registers are adjacent in the
		 * register map
		 */
		for (i = 0; i < SE_N_RGB2YUV_COEFFS; i++)
			malidp_se_write(dev, dev->rgb2yuv_coeffs[i],
					reg->conv_control + SE_REG_CONV_COEFF1 +
					(i * 4));
	}
}

/*
 * Set the configuration for the memory write out interface and scaler.
 * @dev: private SE hw device.
 * @cfg: memory write out mode and buffer.
 * @s0: scaler configuration for scaler 0.
 */
void malidp_se_cfg_processing(struct malidp_se_device *dev,
				struct malidp_se_mw_conf *cfg,
				struct malidp_se_scaler_conf *s0)
{
	u32 control, mw_reg;
	const struct malidp_se_regmap *reg = dev->se_regmap;

	mw_reg = reg->mw_control;
	if (cfg->mode != MALIDP_SE_MW_DISABLE) {
		dev->scene_changing = true;

		if (cfg->buf->fmt == DRM_FORMAT_NV12)
			malidp_se_set_rgb2yuv_coeffs(dev, cfg->buf);

		switch (cfg->buf->n_planes) {
		case 2:
			malidp_se_write(dev, lower_32_bits(cfg->buf->addr[1]),
				SE_REG_WP2_PTR0_LOW + mw_reg);
			malidp_se_write(dev, upper_32_bits(cfg->buf->addr[1]),
				SE_REG_WP2_PTR0_HIGH + mw_reg);
			malidp_se_write(dev, cfg->buf->pitch[1],
				SE_REG_WP2_STRIDE + mw_reg);
			/* Fallthrough */
		case 1:
			malidp_se_write(dev, lower_32_bits(cfg->buf->addr[0]),
				SE_REG_WP1_PTR0_LOW + mw_reg);
			malidp_se_write(dev, upper_32_bits(cfg->buf->addr[0]),
				SE_REG_WP1_PTR0_HIGH + mw_reg);
			malidp_se_write(dev, cfg->buf->pitch[0],
				SE_REG_WP1_STRIDE + mw_reg);
			break;
		default:
			dev_err(dev->device, "%d plane output formats not supported\n",
				cfg->buf->n_planes);
		}

		malidp_se_write(dev, SE_SET_WFORMAT(cfg->buf->hw_fmt),
				SE_REG_FORMAT + mw_reg);

		if (malidp_hw_format_has_alpha(cfg->buf->fmt))
			malidp_se_setbits(dev, SE_ALPHA_EN, reg->control);
		else
			malidp_se_clearbits(dev, SE_ALPHA_EN, reg->control);
	}

	if (s0 != NULL)
		malidp_se_cfg_enhancer(dev, s0->enh_cfg);

	/* Set L0 size register and phase registers */
	if (s0 != NULL && s0->scaling_enable == true) {
		malidp_se_write(dev, SE_SET_H_SIZE(s0->input_w) |
			SE_SET_V_SIZE(s0->input_h),
			reg->layers_control + SE_REG_L0_IN_SIZE);
		malidp_se_write(dev, SE_SET_H_SIZE(s0->output_w) |
			SE_SET_V_SIZE(s0->output_h),
			reg->layers_control + SE_REG_L0_OUT_SIZE);

		malidp_se_write(dev, s0->h_init_phase,
			reg->scaling_control + SE_REG_H_INIT_PH);
		malidp_se_write(dev, s0->h_delta_phase,
			reg->scaling_control + SE_REG_H_DELTA_PH);

		malidp_se_write(dev, s0->v_init_phase,
			reg->scaling_control + SE_REG_V_INIT_PH);
		malidp_se_write(dev, s0->v_delta_phase,
			reg->scaling_control + SE_REG_V_DELTA_PH);

		if (s0->al == MALIDP_SE_ARGB_PP) {
			dev->hwdev->topology->dp_api->se_api.set_scaler_coeff(dev,
				s0->h_coeffs_set, s0->v_coeffs_set);
			dev_dbg(dev->device,
				"%s: PP is enabled v_set: %d h_set: %d\n",
				__func__, s0->v_coeffs_set, s0->h_coeffs_set);
		} else
			BUG();
	} else if (cfg->mode == MALIDP_SE_MW_L0) {
		malidp_se_write(dev, SE_SET_H_SIZE(cfg->buf->natural_w) |
				SE_SET_V_SIZE(cfg->buf->natural_h),
				reg->layers_control + SE_REG_L0_IN_SIZE);
		malidp_se_write(dev, SE_SET_H_SIZE(cfg->buf->natural_w) |
				SE_SET_V_SIZE(cfg->buf->natural_h),
				reg->layers_control + SE_REG_L0_OUT_SIZE);
	}

	/* set L1 size register */
	if (cfg->mode == MALIDP_SE_MW_L1) {
		malidp_se_write(dev, SE_SET_H_SIZE(cfg->buf->natural_w) |
				SE_SET_V_SIZE(cfg->buf->natural_h),
				reg->layers_control + SE_REG_L1_SIZE);
	}

	/* Set control register */
	control = malidp_se_read(dev, reg->control);
	/* set mw mode */
	control &= (~SE_MW_IF_SET(SE_MW_IF_MASK));
	control |= SE_MW_IF_SET(cfg->mode);
	/* set scaling flag to control register */
	if (s0 != NULL) {
		if (s0->scaling_enable == true) {
			control |= SE_SCALING_EN;

			control &= ~(SE_RGB_MTH_SEL | SE_ALPHA_MTH_SEL);
			control |= SE_ARGB_MTH_SET(s0->al);

			if (s0->rgbo_enable)
				control |= SE_RGBO_IF_EN;
			else
				control &= ~SE_RGBO_IF_EN;

			if (s0->scale_alpha)
				control |= SE_ALPHA_EN;
			else
				control &= ~SE_ALPHA_EN;
		} else
			control &= ~(SE_RGBO_IF_EN | SE_SCALING_EN);
	}

	/* if we disable mw and rgbo also is diabled, then the scaler will be disabled */
	if ((cfg->mode == MALIDP_SE_MW_DISABLE) && ((control & SE_RGBO_IF_EN) ==0))
		control &= ~SE_SCALING_EN;

	malidp_se_write(dev, control, reg->control);
}

struct se_fixed_point {
	u16 integer;
	u16 fract;
};

/*
 * Determine parameters for the filter/enhancer blocks (which depend on scaling
 * factor), to be written at commit
 */
void malidp_se_set_scaling_dependent_state(struct malidp_se_device *dev,
				struct malidp_se_scaler_conf *s0)
{
	u32 tmp_fp, phase;
	struct se_fixed_point sf_h = { 0, 0 };
	struct se_fixed_point sf_v = { 0, 0 };

	if (s0->scaling_enable) {
		/* 16.16 fixed point scaling factors */
		tmp_fp = (s0->output_w << 16) / (s0->input_w);
		sf_h.integer = (tmp_fp >> 16) & 0xFFFF;
		sf_h.fract = tmp_fp & 0xFFFF;

		tmp_fp = (s0->output_h << 16) / (s0->input_h);
		sf_v.integer = (tmp_fp >> 16) & 0xFFFF;
		sf_v.fract = tmp_fp & 0xFFFF;

		s0->v_coeffs_set = malidp_se_scaling_coeffstab_select(
				(sf_v.integer << 16) | sf_v.fract);
		s0->h_coeffs_set = malidp_se_scaling_coeffstab_select(
				(sf_h.integer << 16) | sf_h.fract);

		s0->al = MALIDP_SE_ARGB_PP;

		/* Calculate initial_phase and delta_phase
			for horizontal dimension */
		phase = s0->input_w;
		s0->h_init_phase =
				((phase << SE_N_PHASE) / s0->output_w + 1) / 2;

		phase = s0->input_w;
		phase <<= (SE_SHIFT_N_PHASE + SE_N_PHASE);
		s0->h_delta_phase = phase / s0->output_w;

		/* Calculate initial_phase and delta_phase
			for vertical dimension */
		phase = s0->input_h;
		s0->v_init_phase =
				((phase << SE_N_PHASE) / s0->output_h + 1) / 2;

		phase = s0->input_h;
		phase <<= (SE_SHIFT_N_PHASE + SE_N_PHASE);
		s0->v_delta_phase = phase / s0->output_h;
	}

	dev_dbg(dev->device, "%s : Scaling factors: 0x%04x.%04x 0x%04x.%04x\n",
		__func__, sf_h.integer, sf_h.fract, sf_v.integer, sf_v.fract);

	/* Determine Image Enhancer setting */
	if (sf_h.integer >= 2) {
		if (sf_v.integer >= 2)
			s0->enh_cfg = MALIDP_SE_ENHANCER_BOTH;
		else
			s0->enh_cfg = MALIDP_SE_ENHANCER_HORZ;
	} else if (sf_v.integer >= 2) {
		s0->enh_cfg = MALIDP_SE_ENHANCER_VERT;
	} else {
		s0->enh_cfg = MALIDP_SE_ENHANCER_OFF;
	}


}

irqreturn_t malidp_se_irq_thread_handler(int irq, void *data)
{
	struct malidp_se_device *dev = data;
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

bool malidp_se_attr_valid(struct malidp_se_device *dev, u32 attr, u32 val)
{
	switch (attr) {
	case MALIDP_ATTR_SE_WQOS:
		if (val > 15)
			return false;
		break;
	default:
		return dev->hwdev->dp_api->se_api.axi_valid(attr, val);
	}

	return true;
}

void malidp_se_set_axi_cfg(struct malidp_se_device *dev, u32 outstran,
		u32 burstlen, u32 wcache, u32 wqos)
{
	dev_dbg(dev->device, "%s: outstran: %i, burstlen: %i\n", __func__, outstran, burstlen);
	dev_dbg(dev->device, "%s: wcache: %i, wqos: %i\n", __func__, wcache, wqos);

	dev->outstran = outstran;
	dev->burstlen = burstlen;
	dev->wcache = wcache;
	dev->wqos = wqos;
	malidp_se_write(dev, SE_AXI_OUTSTDCAPB(dev->outstran) |
			SE_AXI_BURSTLEN(dev->burstlen - 1) |
			SE_AXI_WCACHE(dev->wcache) |
			SE_AXI_WQOS(dev->wqos),
			dev->se_regmap->axi_control);
}

struct malidp_se_device *malidp_se_hw_init(struct malidp_hw_device *hwdev,
			     struct platform_device *pdev,
			     struct malidp_hw_pdata *pdata,
			     spinlock_t *hw_lock)
{
	int res;
	struct malidp_se_device *dev = devm_kzalloc(&pdev->dev,
					sizeof(struct malidp_se_device),
						    GFP_KERNEL);

	if (!dev)
		return NULL;

	dev->hw_lock = hw_lock;
	dev->regs = pdata->regs + hwdev->hw_regmap->se_base;
	dev->se_regmap = &hwdev->hw_regmap->se_regmap;
	dev->hwdev = hwdev;
	dev->scene_changing = false;

	dev->n_mw_formats = hwdev->topology->n_mw_formats;
	dev->mw_formats = hwdev->topology->mw_formats;
	dev->mw_format_ids = hwdev->topology->mw_format_ids;

	dev->device = &pdev->dev;

	res = devm_request_threaded_irq(dev->device,
			pdata->se_irq,
			hwdev->dp_api->se_api.irq_handler,
			malidp_se_irq_thread_handler,
			IRQF_SHARED, "malidp-se", dev);
	if (res < 0) {
		dev_err(&pdev->dev, "%s: failed to request 'se' irq\n", __func__);
		return NULL;
	}

	dev->ev_queue = malidp_hw_event_queue_create(SE_N_QUEUE_EVENTS);
	if (!dev->ev_queue)
		return NULL;

	dev->op_mode = MALIDP_OP_MODE_UNKNOWN;

	dev->outstran = pdata->se_axi_outstran;
	dev->burstlen = pdata->se_axi_burstlen;
	dev->wcache = pdata->se_axi_awcache;
	dev->wqos = pdata->se_axi_awqos;

	dev_dbg(dev->device, "%s : success!\n", __func__);

	return dev;
}

void malidp_se_hw_exit(struct malidp_se_device *dev)
{
	malidp_hw_event_queue_destroy(dev->ev_queue);
	return;
}

int malidp_se_get_attr(struct malidp_se_device *dev, u32 attr, u32 *val)
{
	switch (attr) {
	case MALIDP_ATTR_SE_BURSTLEN:
		*val = dev->burstlen;
		break;
	case MALIDP_ATTR_SE_OUTSTRAN:
		*val = dev->outstran;
		break;
	case MALIDP_ATTR_SE_WQOS:
		*val = dev->wqos;
		break;
	case MALIDP_ATTR_SE_WCACHE:
		*val = dev->wcache;
		break;
	default:
		dev_err(dev->device, "%s: unkown SE attribute %i\n",
			__func__, attr);
		return -EINVAL;
	}

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n",
		__func__, attr, *val);

	return 0;
}

int malidp_se_set_attr(struct malidp_se_device *dev, u32 attr, u32 val)
{
	int ret = 0;

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n", __func__, attr, val);

	if (!malidp_se_attr_valid(dev, attr, val)) {
		dev_dbg(dev->device, "%s: invalid value %u for attr %u\n",
			__func__, val, attr);
		return -EINVAL;
	}

	switch (attr) {
	case MALIDP_ATTR_SE_BURSTLEN:
		malidp_se_set_axi_cfg(dev, dev->outstran, val,
				      dev->wqos, dev->wcache);
		break;
	case MALIDP_ATTR_SE_OUTSTRAN:
		malidp_se_set_axi_cfg(dev, val, dev->burstlen,
				      dev->wqos, dev->wcache);
		break;
	case MALIDP_ATTR_SE_WCACHE:
		malidp_se_set_axi_cfg(dev, dev->outstran, dev->burstlen,
				      val, dev->wqos);
		break;
	case MALIDP_ATTR_SE_WQOS:
		malidp_se_set_axi_cfg(dev, dev->outstran, dev->burstlen,
				      dev->wcache, val);
		break;
	default:
		dev_err(dev->device, "%s: unkown SE attribute %i\n",
			__func__, attr);
		ret = -EINVAL;
		break;
	}

	return ret;
}

int malidp_se_save_attr(struct malidp_se_device *dev, u32 attr, u32 val)
{
	int ret = 0;

	dev_dbg(dev->device, "%s: attr: %i, val: %u\n", __func__, attr, val);

	if (!malidp_se_attr_valid(dev, attr, val)) {
		dev_dbg(dev->device, "%s: invalid value %u for attr %u\n",
			__func__, val, attr);
		return -EINVAL;
	}

	switch (attr) {
	case MALIDP_ATTR_SE_BURSTLEN:
		dev->burstlen = val;
		break;
	case MALIDP_ATTR_SE_OUTSTRAN:
		dev->outstran = val;
		break;
	case MALIDP_ATTR_SE_WCACHE:
		dev->wcache = val;
		break;
	case MALIDP_ATTR_SE_WQOS:
		dev->wqos = val;
		break;
	default:
		dev_err(dev->device, "%s: unkown SE attribute %i\n",
			__func__, attr);
		ret = -EINVAL;
		break;
	}

	return ret;
}
