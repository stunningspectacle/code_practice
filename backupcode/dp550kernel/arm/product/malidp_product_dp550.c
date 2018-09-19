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



#include <linux/fs.h>

#include "malidp_product_api.h"
#include <uapi/video/malidp_adf.h>

#define MALI_DP550_REG_DE_STATUS	0x0000
#define		MALI_DP550_DE_IRQ_UNR	(1 << 0)
#define		MALI_DP550_DE_IRQ_SAT	(1 << 8)
#define		MALI_DP550_DE_IRQ_PL1	(1 << 12)
#define		MALI_DP550_DE_IRQ_PL2	(1 << 13)
#define		MALI_DP550_DE_AXIE	(1 << 16)

#define MALI_DP550_REG_DE_IRQ_MSK	0x0008
#define MALI_DP550_REG_DE_IRQ_CLR	0x000C
#define MALI_DP550_REG_DE_CTRL		0x0010
#define		DP550_PREFETCH_LINE_MASK	0x3ff
#define		DP550_PREFETCH_LINE_SET(x)	\
		((x) & DP550_PREFETCH_LINE_MASK)

#define MALI_DP550_REG_LINE_INT_CTRL	0x0014

#define MALI_DP550_REG_H_INTERVALS	0x0030
#define MALI_DP550_REG_V_INTERVALS	0x0034
#define MALI_DP550_REG_SYNC_CTRL	0x0038
#define		MALI_DP550_VSP	(1 << 28)
#define		MALI_DP550_HSP	(1 << 12)
#define MALI_DP550_REG_HV_ACT_SIZE	0x003C
#define MALI_DP550_REG_BG_COLOR		0x0044

#define MALI_DP550_REG_ID			0xFFD0
#define MALI_DP550_REG_CFG_VALID	0xC014
#define MALI_DP550_REG_DE_BASE		0x0000
#define MALI_DP550_REG_SE_BASE		0x8000

#define MALI_DP550_REG_DE_AXI_CTL	0x0018
#define MALI_DP550_REG_DE_QOS       0x001C
#define MALI_DP550_REG_DE_DISP_FUNC	0x0020
#define		MALI_DP550_COPROC_EN	(1 << 12)
#define MALI_DP550_REG_DE_OD		0x004C
#define MALI_DP550_REG_DE_COLORCOEFFS 0x0050
#define MALI_DP550_DE_COEFTAB_GAMMA	(1 << DE_COEFTAB_GAMMA_SHIFT)

#define MALI_DP550_REG_LV1_YUV2RGB	0x0184
#define MALI_DP550_REG_LV2_YUV2RGB	0x0284

#define MALI_DP550_REG_SE_IRQ_CLR	0x000C
#define MALI_DP550_REG_SE_CTL		0x0010
#define		MALI_DP550_SE_CTL_OFM	(1 << 7)
#define		MALI_DP550_SE_CTL_xSEL_MASK	7
#define		MALI_DP550_SE_CTL_VCSEL(x)	(((x) & MALI_DP550_SE_CTL_xSEL_MASK) << 20)
#define		MALI_DP550_SE_CTL_HCSEL(x)	(((x) & MALI_DP550_SE_CTL_xSEL_MASK) << 16)
#define MALI_DP550_REG_SE_PL		0x0014
#define		MALI_DP550_SE_PL_LINE_MASK	0x1fff
#define		MALI_DP550_SE_PL_LINE(x)	(((x) & MALI_DP550_SE_PL_LINE_MASK) << 0)
#define		MALI_DP550_SE_PL_INTERVAL_MASK	0x3ff
#define		MALI_DP550_SE_PL_INTERVAL(x)	(((x) & MALI_DP550_SE_PL_INTERVAL_MASK) << 16)
#define MALI_DP550_REG_SE_AXI_CRL	0x0018
#define MALI_DP550_REG_SE_L_CTL		0x0024
#define MALI_DP550_REG_SE_SCL_CTL	0x0034
#define MALI_DP550_REG_SE_ENH_CTL	0x004C
#define MALI_DP550_REG_SE_COV_CTL	0x0074
#define MALI_DP550_REG_SE_MW_CTL	0x0100

#define MALI_DP550_REG_DC_STATUS	0xC000
#define		MALI_DP550_DC_IRQ_CVAL	(1 << 0)
#define		MALI_DP550_DC_IRQ_CM	(1 << 4)
#define		MALI_DP550_DC_IRQDE	(1 << 20)
#define		MALI_DP550_DC_IRQSE	(1 << 24)
#define MALI_DP550_REG_DC_IRQ_SET	0xC004
#define MALI_DP550_REG_DC_IRQ_MSK	0xC008
#define MALI_DP550_REG_DC_IRQ_CLR	0xC00C
#define MALI_DP550_REG_DC_CTRL		0xC010
#define		MALI_DP550_DC_CM	(1 << 16)
#define		MALI_DP550_DC_CRST	(1 << 17)

#define MALI_DP550_LV1_BASE		0x0100
#define MALI_DP550_LV2_BASE		0x0200
#define MALI_DP550_LG1_BASE		0x0300
#define MALI_DP550_LS1_BASE		0x0400

#define MALI_DP550_LV1_AD_CTRL	0x01B8
#define MALI_DP550_LV1_AD_H_CROP	0x01BC
#define MALI_DP550_LV1_AD_V_CROP	0x01C0
#define MALI_DP550_LV2_AD_CTRL	0x02B8
#define MALI_DP550_LV2_AD_H_CROP	0x02BC
#define MALI_DP550_LV2_AD_V_CROP	0x02C0
#define MALI_DP550_LG1_AD_CTRL	0x0330
#define MALI_DP550_LG1_AD_H_CROP	0x0334
#define MALI_DP550_LG1_AD_V_CROP	0x0338

#define MALI_DP550_SE_IRQ_EOW	(1 << 0)
#define MALI_DP550_SE_IRQ_PL	(1 << 12)
#define MALI_DP550_SE_IRQ_PI	(1 << 13)

#define MALI_DP550_SE_AXIE	(1 << 16)
#define MALI_DP550_SE_OVR	(1 << 17)
#define MALI_DP550_SE_IBUSY	(1 << 18)

#define MALIDP_FORMAT_ID(__group, __format) ((((__group) & 0x7) << 3) | \
		(((__format) & 0x7) << 0))
#define N_RGB_INPUT_FORMATS 18

#define SMART_INPUT_FORMAT_START_IDX 4
#define N_SMART_INPUT_FORMATS        8
#define N_SUPPORTED_SMART_LAYERS     4

extern const char *const op_mode_name[];
extern const struct malidp_intf_hw_info dp_interfaces[];

const u32 malidp550_input_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_RGBA1010102,
	DRM_FORMAT_BGRA1010102,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBX8888,
	DRM_FORMAT_BGRX8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	/* Formats below here supported by Video layers only */
	MALIDP_FORMAT_XYUV, /* [31:0] X:Y:Cb:Cr 8:8:8:8 little endian */
	MALIDP_FORMAT_NV16AFBC, /* AFBC compressed YUV 4:2:2 */
	DRM_FORMAT_YUYV,  /* YUV 4:2:2 1-plane */
	DRM_FORMAT_UYVY,  /* YUV 4:2:2 1-plane */
	DRM_FORMAT_NV12,  /* YUV 4:2:0 2-plane */
	DRM_FORMAT_YUV420,/* YUV 4:2:0 3-plane */
	MALIDP_FORMAT_VYU30, /* [31:0] X:Cr:Y:Cb 2:10:10:10 little endian */
	MALIDP_FORMAT_Y0L2, /* ARM Linear 10-bit packed YUV 4:2:0 */
	MALIDP_FORMAT_P010, /* 10-bit YUV 4:2:0 2-plane */
	/* AFBC formats which map to non-compressed format IDs */
	MALIDP_FORMAT_NV12AFBC, /* Included for backwards-compatibility only. Maps to NV12 */
	MALIDP_FORMAT_YUV10_420AFBC, /* AFBC compressed YUV 4:2:0, 10 bits per component */
};

/* We can use the same list for all the input formats */
const u32 malidp550_input_format_ids[] = {
	MALIDP_FORMAT_ID(0, 0),
	MALIDP_FORMAT_ID(0, 1),
	MALIDP_FORMAT_ID(0, 2),
	MALIDP_FORMAT_ID(0, 3),
	MALIDP_FORMAT_ID(1, 0),
	MALIDP_FORMAT_ID(1, 1),
	MALIDP_FORMAT_ID(1, 2),
	MALIDP_FORMAT_ID(1, 3),
	MALIDP_FORMAT_ID(2, 0),
	MALIDP_FORMAT_ID(2, 1),
	MALIDP_FORMAT_ID(2, 2),
	MALIDP_FORMAT_ID(2, 3),
	MALIDP_FORMAT_ID(3, 0),
	MALIDP_FORMAT_ID(3, 1),
	MALIDP_FORMAT_ID(4, 0),
	MALIDP_FORMAT_ID(4, 1),
	MALIDP_FORMAT_ID(4, 2),
	MALIDP_FORMAT_ID(4, 3),
	/* Start YUV formats */
	MALIDP_FORMAT_ID(5, 0),
	MALIDP_FORMAT_ID(5, 1),
	MALIDP_FORMAT_ID(5, 2),
	MALIDP_FORMAT_ID(5, 3),
	MALIDP_FORMAT_ID(5, 6),
	MALIDP_FORMAT_ID(5, 7),
	MALIDP_FORMAT_ID(6, 0),
	MALIDP_FORMAT_ID(6, 6),
	MALIDP_FORMAT_ID(6, 7),
	/* AFBC formats which map to non-compressed format IDs */
	MALIDP_FORMAT_ID(5, 6),
	MALIDP_FORMAT_ID(6, 7),
};

const u32 malidp550_output_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_RGBA1010102,
	DRM_FORMAT_BGRA1010102,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGBX8888,
	DRM_FORMAT_BGRX8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_NV12,  /* YUV 4:2:0 2-plane */
};

const u32 malidp550_output_format_ids[] = {
	MALIDP_FORMAT_ID(0, 0),
	MALIDP_FORMAT_ID(0, 1),
	MALIDP_FORMAT_ID(0, 2),
	MALIDP_FORMAT_ID(0, 3),
	MALIDP_FORMAT_ID(1, 0),
	MALIDP_FORMAT_ID(1, 1),
	MALIDP_FORMAT_ID(1, 2),
	MALIDP_FORMAT_ID(1, 3),
	MALIDP_FORMAT_ID(2, 0),
	MALIDP_FORMAT_ID(2, 1),
	MALIDP_FORMAT_ID(2, 2),
	MALIDP_FORMAT_ID(2, 3),
	MALIDP_FORMAT_ID(3, 0),
	MALIDP_FORMAT_ID(3, 1),
	MALIDP_FORMAT_ID(5, 6),
};

const u32 malidp550_afbc_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_RGBA1010102,
	DRM_FORMAT_BGRA1010102,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGBA8888,
	DRM_FORMAT_BGRA8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	MALIDP_FORMAT_XYUV,
	MALIDP_FORMAT_NV16AFBC,
	DRM_FORMAT_YUYV,
	DRM_FORMAT_UYVY,
	MALIDP_FORMAT_VYU30,
	MALIDP_FORMAT_NV12AFBC,
	MALIDP_FORMAT_YUV10_420AFBC,
};

const u32 malidp550_xform_invalid_formats[] = {
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	MALIDP_FORMAT_Y0L2,
};

/*
 * Format name				Block split support
 * DRM_FORMAT_ABGR2101010		Yes
 * DRM_FORMAT_ABGR8888			Yes
 * DRM_FORMAT_BGR888			Yes
 * DRM_FORMAT_RGBA5551			No
 * DRM_FORMAT_RGB565			No
 * MALIDP_FORMAT_XYUV			Yes
 * MALIDP_FORMAT_NV16AFBC		No
 * MALIDP_FORMAT_NV12AFBC		No
 * MALIDP_FORMAT_VYU30			Yes
 * MALIDP_FORMAT_YUV10_420AFBC	Yes
*/
#define MALIDP_AFBC_SPLIBLK_BITS 0x1443FF

/* Define layer information */
static struct malidp_layer_hw_info dp550_hw_layers[] = {
	{
		.index = 0,
		.name = "Video-1",
		.type = MALIDP_HW_LAYER_VIDEO,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC |
			    MALIDP_LAYER_FEATURE_SCALING |
			    MALIDP_LAYER_FEATURE_SRGB,
		.n_supported_formats = ARRAY_SIZE(malidp550_input_formats),
		.supported_formats = malidp550_input_formats,
		.format_ids = malidp550_input_format_ids,
		.n_max_planes = 3,
		.n_supported_layers = 1,
		.regs_base = MALI_DP550_LV1_BASE,
		.ad_ctrl_reg = MALI_DP550_LV1_AD_CTRL,
		.ad_crop_h_reg = MALI_DP550_LV1_AD_H_CROP,
		.ad_crop_v_reg = MALI_DP550_LV1_AD_V_CROP,
		.stride_offset = DE_REG_LV1_STRIDE,
		.ptr0_low_offset = DE_REG_LV1_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LV1_PTR0_HIGH,
		 /* The stride register for plane 3 is unused on DP550 */
		.p3_stride_offset = 0,
		.yuv2rgb_reg_offset = MALI_DP550_REG_LV1_YUV2RGB,
	},
	{
		.index = 1,
		.name = "Graphics-1",
		.type = MALIDP_HW_LAYER_GRAPHICS,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC |
			    MALIDP_LAYER_FEATURE_SCALING |
			    MALIDP_LAYER_FEATURE_SRGB,
		.n_supported_formats = N_RGB_INPUT_FORMATS,
		.supported_formats = malidp550_input_formats,
		.format_ids = malidp550_input_format_ids,
		.n_max_planes = 1,
		.n_supported_layers = 1,
		.regs_base = MALI_DP550_LG1_BASE,
		.ad_ctrl_reg = MALI_DP550_LG1_AD_CTRL,
		.ad_crop_h_reg = MALI_DP550_LG1_AD_H_CROP,
		.ad_crop_v_reg = MALI_DP550_LG1_AD_V_CROP,
		.stride_offset = DE_REG_LG_STRIDE,
		.ptr0_low_offset = DE_REG_LG_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LG_PTR0_HIGH,
	},
	{
		.index = 2,
		.name = "Video-2",
		.type = MALIDP_HW_LAYER_VIDEO,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC |
			    MALIDP_LAYER_FEATURE_SCALING |
			    MALIDP_LAYER_FEATURE_SRGB,
		.n_supported_formats = ARRAY_SIZE(malidp550_input_formats),
		.supported_formats = malidp550_input_formats,
		.format_ids = malidp550_input_format_ids,
		.n_max_planes = 3,
		.n_supported_layers = 1,
		.regs_base = MALI_DP550_LV2_BASE,
		.ad_ctrl_reg = MALI_DP550_LV2_AD_CTRL,
		.ad_crop_h_reg = MALI_DP550_LV2_AD_H_CROP,
		.ad_crop_v_reg = MALI_DP550_LV2_AD_V_CROP,
		.stride_offset = DE_REG_LV1_STRIDE,
		.ptr0_low_offset = DE_REG_LV1_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LV1_PTR0_HIGH,
		 /* The stride register for plane 3 is unused on DP550 */
		.p3_stride_offset = 0,
		.yuv2rgb_reg_offset = MALI_DP550_REG_LV2_YUV2RGB,
	},
	{
		.index = 3,
		.name = "Smart-1",
		.type = MALIDP_HW_LAYER_SMART,
		.features = MALIDP_LAYER_FEATURE_SRGB,
		.n_supported_formats = N_SMART_INPUT_FORMATS,
		.supported_formats = &malidp550_input_formats[SMART_INPUT_FORMAT_START_IDX],
		.format_ids = &malidp550_input_format_ids[SMART_INPUT_FORMAT_START_IDX],
		.n_max_planes = 1,
		.n_supported_layers = N_SUPPORTED_SMART_LAYERS,
		.regs_base = MALI_DP550_LS1_BASE,
		.ls_r1_in_size = DE_REG_LS_R1_IN_SIZE,
		.ls_r1_offset = DE_REG_LS_R1_OFFSET,
		.ls_r1_stride = DE_REG_LS_R1_STRIDE,
		.ls_r1_ptr_low = DE_REG_LS_R1_PTR_LOW,
		.ls_r1_ptr_high = DE_REG_LS_R1_PTR_HIGH,
	},
};

static const struct malidp_hw_regmap malidp550_regmap = {
	.id_registers = MALI_DP550_REG_ID,
	.config_valid = MALI_DP550_REG_CFG_VALID,
	.de_base = MALI_DP550_REG_DE_BASE,
	.se_base = MALI_DP550_REG_SE_BASE,
	.de_regmap = {
		.axi_control = MALI_DP550_REG_DE_AXI_CTL,
		.disp_func = MALI_DP550_REG_DE_DISP_FUNC,
		.output_depth = MALI_DP550_REG_DE_OD,
		.coloradj_coeff = MALI_DP550_REG_DE_COLORCOEFFS,
		.qos_control = MALI_DP550_REG_DE_QOS
	},
	.se_regmap = {
		.control = MALI_DP550_REG_SE_CTL,
		.axi_control = MALI_DP550_REG_SE_AXI_CRL,
		.layers_control = MALI_DP550_REG_SE_L_CTL,
		.scaling_control = MALI_DP550_REG_SE_SCL_CTL,
		.enhancer_control = MALI_DP550_REG_SE_ENH_CTL,
		.conv_control = MALI_DP550_REG_SE_COV_CTL,
		.mw_control = MALI_DP550_REG_SE_MW_CTL
	},
};

/* Declare of DP550 API */
static void dp550_disable_irq(struct malidp_hw_device *);
static enum malidp_op_mode dp550_change_op_mode(
		struct malidp_hw_device *,
		enum malidp_op_mode);
static int dp550_de_hw_cfg(struct malidp_de_device *);
static void dp550_de_modeset(struct malidp_de_device *,
		struct malidp_de_hwmode *);
static void dp550_de_set_gamma_coeff(struct malidp_de_device *,
		u32 *coeffs);
static u32 dp550_de_fmt_fixup(u32 drm_format, u32 flags);
static irqreturn_t dp550_de_irq_handler(int irq, void *data);
static int dp550_se_hw_cfg(struct malidp_se_device *);
static void dp550_se_set_scaler_coeff(struct malidp_se_device *,
		enum malidp_scaling_coeff_set hcoeff,
		enum malidp_scaling_coeff_set vcoeff);
static bool dp550_se_limitation_check(struct malidp_se_device *,
		struct malidp_se_scaler_conf *);
static u32 dp550_se_calc_downscaling_threshold(u32, u32,
		struct drm_mode_modeinfo *);
static umode_t dp550_attr_visible(struct kobject *obj,
		struct attribute *attr, int n);
static void dp550_debugfs(struct malidp_hw_device *);
static bool dp550_de_axi_valid(u32 attr, u32 val);
static bool dp550_se_axi_valid(u32 attr, u32 val);
static irqreturn_t dp550_se_irq_handler(int irq, void *data);
static u32 dp550_rotmem_required(const struct malidp_hw_buffer *hw_buf);

static const struct malidp_product_api dp550_api = {
	.change_op_mode = dp550_change_op_mode,
	.disable_irq = dp550_disable_irq,
	.attr_visible = dp550_attr_visible,
	.debugfs_func = dp550_debugfs,
	.rotmem_size_required = dp550_rotmem_required,
	.de_api = {
		.hw_cfg = dp550_de_hw_cfg,
		.modeset = dp550_de_modeset,
		.set_gamma_coeff = dp550_de_set_gamma_coeff,
		.fmt_fixup = dp550_de_fmt_fixup,
		.axi_valid = dp550_de_axi_valid,
		.irq_handler = dp550_de_irq_handler,
	},
	.se_api = {
		.hw_cfg = dp550_se_hw_cfg,
		.set_scaler_coeff = dp550_se_set_scaler_coeff,
		.limitation_check = dp550_se_limitation_check,
		.calc_downscaling_threshold = dp550_se_calc_downscaling_threshold,
		.axi_valid = dp550_se_axi_valid,
		.irq_handler = dp550_se_irq_handler,
	}
};

struct malidp_hw_topology malidp_dp550_topology = {
	.product_id = MALIDP_DP550_PRODUCT_ID,
	.interfaces = dp_interfaces,
	.n_interfaces = 2,
	.layers = dp550_hw_layers,
	.n_layers = ARRAY_SIZE(dp550_hw_layers),
	.n_scalers = 1,
	.n_supported_afbc_formats = ARRAY_SIZE(malidp550_afbc_formats),
	.supported_afbc_formats = malidp550_afbc_formats,
	.supported_afbc_splitblk = MALIDP_AFBC_SPLIBLK_BITS,
	.n_mw_formats = ARRAY_SIZE(malidp550_output_formats),
	.mw_formats = malidp550_output_formats,
	.mw_format_ids = malidp550_output_format_ids,
	.n_xform_invalid_formats = ARRAY_SIZE(malidp550_xform_invalid_formats),
	.xform_invalid_formats = malidp550_xform_invalid_formats,
	.dp_api = &dp550_api,
	.regmap = &malidp550_regmap,
};

static const struct malidp_line_size_hw_info dp550_ls_configs[] = {
	{
		.max_line_size = 2048,
		.min_line_size = 2,
		.input_fifo_size = 4096,
		.default_rotmem_size = 128 * SZ_1K,
	},
	{
		.max_line_size = 4096,
		.min_line_size = 2,
		.input_fifo_size = 8192,
		.default_rotmem_size = 256 * SZ_1K,
	},
	{
		.max_line_size = 1280,
		.min_line_size = 2,
		.input_fifo_size = 2560,
		.default_rotmem_size = 80 * SZ_1K,
	},
};

static struct malidp_hw_configuration malidp_hw_dp550_cf = {
	.ls_configs = dp550_ls_configs,
	.n_configs = ARRAY_SIZE(dp550_ls_configs),
	.partition_type = MALIDP_HW_PARTITION_FIXED,
};

umode_t dp550_attr_visible(struct kobject *obj,
	struct attribute *attr, int n)
{
	/* so far DP550 shows all attributes */
	return attr->mode;
}

void malidp_dp550_get_hw_description(struct malidp_hw_description *hwdes)
{
	hwdes->topology = &malidp_dp550_topology;
	hwdes->config = &malidp_hw_dp550_cf;
}

static int coproc_set(void *data, u64 val)
{
	struct malidp_hw_device *hwdev = data;
	bool b = (val != 0) ? true : false;
	enum malidp_op_mode curr_mode;

	if (hwdev->cproc_en == b)
		return 0;

	curr_mode = malidp_de_get_op_mode(hwdev->de_dev);
	if (curr_mode == MALIDP_OP_MODE_POWERSAVE) {
		dev_err(hwdev->device,
			"%s:Couldn't set co-processor enable bit in power save mode",
			__func__);
		return -EFAULT;
	}

	hwdev->cproc_en = b;
	if (b)
		malidp_hw_setbits(hwdev,
				  MALI_DP550_COPROC_EN,
				  MALI_DP550_REG_DE_DISP_FUNC);
	else
		malidp_hw_clearbits(hwdev,
				    MALI_DP550_COPROC_EN,
				    MALI_DP550_REG_DE_DISP_FUNC);
	return 0;
}

static int coproc_get(void *data, u64 *val)
{
	struct malidp_hw_device *hwdev = data;

	*val = (u8)hwdev->cproc_en;
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(f_ops_coproc, coproc_get, coproc_set, "0x%02llx\n");

static void dp550_debugfs(struct malidp_hw_device *hwdev)
{
	struct dentry *dbg_coproc;

	dbg_coproc = debugfs_create_file("coprocessor", S_IRUSR | S_IWUSR,
			hwdev->dbg_folder, hwdev, &f_ops_coproc);
	if (dbg_coproc == NULL)
		dev_err(hwdev->device,
			"debugfs coprocessor is created error!\n");
}

static bool dp550_axi_burstlen_valid(u32 val)
{
	int i;

	/* for 8, 16, 32, 64 */
	for (i = 3; i < 7; i++)
		if (val == (1 << i))
			break;
	return (i < 7) ? true : false;
}

static irqreturn_t dp550_de_irq_handler(int irq, void *data)
{
	struct malidp_de_device *dev = data;
	u32 status, irq_mask, irq_vector, dc_status;
	unsigned long flags;
	struct malidp_hw_event event;

	spin_lock_irqsave(dev->hw_lock, flags);

	/* Drop any events if we already went to powersave */
	if (dev->op_mode == MALIDP_OP_MODE_POWERSAVE) {
		status = dev->pending_status;
		dev->pending_status = 0;
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return status & MALI_DP550_DC_IRQDE ? IRQ_HANDLED : IRQ_NONE;
	}

	dc_status = malidp_hw_read(dev->hwdev, MALI_DP550_REG_DC_STATUS);
	/* The IRQ does not belong to this device */
	if (!(dc_status & MALI_DP550_DC_IRQDE)) {
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return IRQ_NONE;
	}

	status = malidp_de_read(dev, MALI_DP550_REG_DE_STATUS);
	irq_mask = malidp_de_read(dev, MALI_DP550_REG_DE_IRQ_MSK);
	irq_vector = status & irq_mask;

	/* Get the timestamp as soon as possible for more accuracy */
	event.timestamp = ktime_get();
	event.type = MALIDP_HW_EVENT_NONE;

	if (irq_vector & MALI_DP550_DE_IRQ_UNR) {
		dev_err(dev->device, "%s: underrun error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_URUN;

		/* Check if this underrun was caused by AXI error */
		if (status & MALI_DP550_DE_AXIE) {
			dev_err(dev->device, "%s: axi error\n", __func__);
			event.type |= MALIDP_HW_ERROR_AXI;
		}
	}

	/* If there was a pointer update this is flip event. Otherwise
	 * this is only a regular vsync.
	 */
	if (irq_vector &  MALI_DP550_DE_IRQ_PL1) {
		event.type |= MALIDP_HW_EVENT_VSYNC;
		if (dc_status & MALI_DP550_DC_IRQ_CVAL) {
			event.type |= MALIDP_HW_EVENT_FLIP;
			dev->scene_changing = false;
			malidp_hw_write(dev->hwdev, MALI_DP550_DC_IRQ_CVAL,
					MALI_DP550_REG_DC_IRQ_CLR);
		}
	}

	/* IRQs not enabled should not be triggered */
	BUG_ON((irq_vector & MALI_DP550_DE_IRQ_PL2) ||
		(irq_vector & MALI_DP550_DE_IRQ_SAT));

	/* There must always be a valid event if the IRQ is triggered */
	BUG_ON(event.type == MALIDP_HW_EVENT_NONE);

	/* Disable the offending error IRQ if there was an error */
	if (event.type & MALIDP_HW_EVENT_ERROR)
		malidp_de_clearbits(dev, MALI_DP550_DE_IRQ_UNR,
			MALI_DP550_REG_DE_IRQ_MSK);

	/* Clear IRQ */
	malidp_de_write(dev, status, MALI_DP550_REG_DE_IRQ_CLR);

	/* If there's a new scene, re-enable the error interrupts */
	if (event.type & MALIDP_HW_EVENT_FLIP) {
		uint32_t mask_bits = MALI_DP550_DE_IRQ_UNR;
		if ((mask_bits & irq_mask) != mask_bits)
			malidp_de_setbits(dev, mask_bits, MALI_DP550_REG_DE_IRQ_MSK);
	}

	malidp_hw_event_queue_enqueue(dev->ev_queue, &event);

	spin_unlock_irqrestore(dev->hw_lock, flags);

	return IRQ_WAKE_THREAD;
}

static irqreturn_t dp550_se_irq_handler(int irq, void *data)
{
	struct malidp_se_device *dev = data;
	u32 status, irq_mask, irq_vector, dc_status;
	unsigned long flags;
	struct malidp_hw_event event;

	spin_lock_irqsave(dev->hw_lock, flags);

	/* Drop any events if we already went to powersave */
	if (dev->op_mode == MALIDP_OP_MODE_POWERSAVE) {
		status = dev->pending_status;
		dev->pending_status = 0;
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return status & MALI_DP550_DC_IRQSE ? IRQ_HANDLED : IRQ_NONE;
	}

	dc_status = malidp_hw_read(dev->hwdev, MALI_DP550_REG_DC_STATUS);
	/* The IRQ does not belong to this device */
	if (!(dc_status & MALI_DP550_DC_IRQSE)) {
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return IRQ_NONE;
	}

	status = malidp_se_read(dev, SE_REG_STATUS);
	irq_mask = malidp_se_read(dev, SE_REG_MASKIRQ);
	irq_vector = status & irq_mask;

	dev_dbg(dev->device, "%s: start\n", __func__);

	/* IRQs not enabled should not be triggered */
	BUG_ON(irq_vector & MALI_DP550_SE_IRQ_PI);

	/* Get the timestamp as soon as possible for more accuracy */
	event.timestamp = ktime_get();
	event.type = MALIDP_HW_EVENT_NONE;

	if (status & MALI_DP550_SE_OVR) {
		dev_err(dev->device, "%s: overrun error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_ORUN;
	}

	if (status & MALI_DP550_SE_AXIE) {
		dev_err(dev->device, "%s: axi error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_AXI;
	}

	if (status & MALI_DP550_SE_IBUSY) {
		dev_err(dev->device, "%s: init busy error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_IBUSY;
	}

	/*
	 * Prog line 1 means mw interface has begun to write out. Disable the
	 * interface so that it does not write when the next frame period
	 * starts.
	 */
	if (irq_vector &  MALI_DP550_SE_IRQ_PL) {
		event.type |= MALIDP_HW_EVENT_START;

		if (dev->scene_changing) {
			event.type |= MALIDP_HW_EVENT_FLIP;
			dev->scene_changing = false;
		}
	}

	if (irq_vector & MALI_DP550_SE_IRQ_EOW) {
		event.type |= MALIDP_HW_EVENT_STOP;
	}

	malidp_hw_event_queue_enqueue(dev->ev_queue, &event);

	/* Clear interrupts */
	malidp_se_write(dev, irq_vector, MALI_DP550_REG_SE_IRQ_CLR);

	dev_dbg(dev->device, "%s: end\n", __func__);

	spin_unlock_irqrestore(dev->hw_lock, flags);

	return IRQ_WAKE_THREAD;
}

/* Implementation of DP550 API */
static bool dp550_de_axi_valid(u32 attr, u32 val)
{
	switch (attr) {
	case MALIDP_ATTR_DE_BURSTLEN:
		return dp550_axi_burstlen_valid(val);
	case MALIDP_ATTR_DE_OUTSTRAN:
	case MALIDP_ATTR_DE_POUTSTDCAB:
		if ((val < 1) || (val > 32))
			return false;
		break;
	default:
		BUG();
	}

	return true;
}

static bool dp550_se_axi_valid(u32 attr, u32 val)
{
	switch (attr) {
	case MALIDP_ATTR_SE_BURSTLEN:
		return dp550_axi_burstlen_valid(val);
	case MALIDP_ATTR_SE_OUTSTRAN:
		if ((val < 1) || (val > 32))
			return false;
		break;
	case MALIDP_ATTR_SE_WCACHE:
		if (val > 15)
			return false;
		break;
	default:
		BUG();
	}

	return true;
}

static u32 dp550_rotmem_required(const struct malidp_hw_buffer *hw_buf)
{
	u32 bytes_per_col;

	switch (hw_buf->fmt) {
	/* 8 lines at 4 bytes per pixel */
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
	case MALIDP_FORMAT_VYU30: /* 4:4:4 10-bit per component */
	case MALIDP_FORMAT_XYUV: /* 4:4:4 8-bit per component */
	/* 16 lines at 2 bytes per pixel */
	case DRM_FORMAT_RGBA5551:
	case DRM_FORMAT_ABGR1555:
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_UYVY:
	case DRM_FORMAT_YUYV:
	case MALIDP_FORMAT_NV16AFBC: /* 4:2:2 8-bit per component */
	case MALIDP_FORMAT_YUV10_420AFBC: /* 4:2:0 10-bit per component */
	case MALIDP_FORMAT_Y0L2:
	case MALIDP_FORMAT_P010:
		bytes_per_col = 32;
		break;
	/* 16 lines at 1.5 bytes per pixel */
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_YUV420:
	case MALIDP_FORMAT_NV12AFBC:
		bytes_per_col = 24;
		break;
	default:
		BUG();
	}
	return hw_buf->cmp_rect.src_w * bytes_per_col;
}

static void dp550_disable_irq(struct malidp_hw_device *hwdev)
{
	malidp_hw_write(hwdev, 0,
		hwdev->hw_regmap->se_base + SE_REG_MASKIRQ);
	malidp_hw_write(hwdev, 0, MALI_DP550_REG_DC_IRQ_MSK);
	malidp_hw_write(hwdev, 0,
		hwdev->hw_regmap->de_base + MALI_DP550_REG_DE_IRQ_MSK);
}

/* Must be called with the power_mutex held */
static enum malidp_op_mode dp550_change_op_mode(
		struct malidp_hw_device *hwdev,
		enum malidp_op_mode mode)
{
	unsigned long flags;
	enum malidp_op_mode old_mode_de;
	u32 status_bits = 0;
	u32 status;

	BUG_ON(mode == MALIDP_OP_MODE_UNKNOWN);


	/* We're protected by the power mutex, so just read the mode */
	old_mode_de = malidp_de_get_op_mode(hwdev->de_dev);
	dev_dbg(hwdev->device, "performing op mode change: %s->%s\n",
		op_mode_name[old_mode_de], op_mode_name[mode]);
	if (old_mode_de == MALIDP_OP_MODE_UNKNOWN) {
		/* reset */
		dev_dbg(hwdev->device, "Requesting configuration reset\n");
		malidp_hw_setbits(hwdev, MALI_DP550_DC_CRST,
			MALI_DP550_REG_DC_CTRL);
	} else if (old_mode_de == mode) {
		return old_mode_de;
	} else if (old_mode_de == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * Exiting powersave mode, so clear the status registers
		 * and re-enable interrupts
		 */
		status = malidp_de_read(hwdev->de_dev,
					MALI_DP550_REG_DE_STATUS);
		malidp_de_write(hwdev->de_dev, status,
				MALI_DP550_REG_DE_IRQ_CLR);
		malidp_de_write(hwdev->de_dev, MALI_DP550_DE_IRQ_UNR |
				MALI_DP550_DE_IRQ_PL1,
				MALI_DP550_REG_DE_IRQ_MSK);
		status = malidp_se_read(hwdev->se_dev,
					MALI_DP550_REG_DE_STATUS);
		malidp_se_write(hwdev->se_dev, status,
				MALI_DP550_REG_SE_IRQ_CLR);
		malidp_se_write(hwdev->se_dev, MALI_DP550_SE_IRQ_PL |
				MALI_DP550_SE_IRQ_EOW, SE_REG_MASKIRQ);
	}

	spin_lock_irqsave(&hwdev->hw_lock, flags);

	switch (mode) {
	case MALIDP_OP_MODE_POWERSAVE:
	case MALIDP_OP_MODE_CONFIG:
		malidp_hw_setbits(hwdev, MALI_DP550_DC_CM,
			MALI_DP550_REG_DC_CTRL);
		status_bits = MALI_DP550_DC_CM;
		break;
	case MALIDP_OP_MODE_NORMAL:
		malidp_hw_clearbits(hwdev, MALI_DP550_DC_CM,
			MALI_DP550_REG_DC_CTRL);
		break;
	case MALIDP_OP_MODE_UNKNOWN:
	default:
		BUG();
	}

	/* Wait for the mode change to be applied */
	do {
		status = malidp_hw_read(hwdev,
			MALI_DP550_REG_DC_STATUS);
	} while ((status & MALI_DP550_DC_CM) != status_bits);

	/* If we did a configuration reset, we have to clear the CRST bit */
	if (old_mode_de == MALIDP_OP_MODE_UNKNOWN) {
		malidp_hw_clearbits(hwdev, MALI_DP550_DC_CRST,
				    MALI_DP550_REG_DC_CTRL);
	}

	/*
	 * On entering powersave, disable IRQs and store the pending status
	 * incase there is still one left to be handled
	 */
	if (mode == MALIDP_OP_MODE_POWERSAVE) {
		status = malidp_hw_read(hwdev, MALI_DP550_REG_DC_STATUS);
		hwdev->de_dev->pending_status = status;
		hwdev->se_dev->pending_status = status;

		dp550_disable_irq(hwdev);

		status = malidp_de_read(hwdev->de_dev,
					MALI_DP550_REG_DE_STATUS);
		malidp_de_write(hwdev->de_dev, status,
				MALI_DP550_REG_DE_IRQ_CLR);

		status = malidp_se_read(hwdev->se_dev, SE_REG_STATUS);
		malidp_se_write(hwdev->se_dev, status,
				MALI_DP550_REG_SE_IRQ_CLR);
	}

	hwdev->de_dev->op_mode = mode;
	hwdev->se_dev->op_mode = mode;

	dev_dbg(hwdev->device, "mode change ok: %s\n", op_mode_name[mode]);

	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	return old_mode_de;
}

static int dp550_de_hw_cfg(struct malidp_de_device *de_dev)
{
	/* Set AXI configuration */
	if (!malidp_de_attr_valid(de_dev, MALIDP_ATTR_DE_OUTSTRAN,
					de_dev->outstran)) {
		dev_warn(de_dev->device, "%s : invalid value '%d' for de_axi_outstran\n",
				__func__, de_dev->outstran);
		de_dev->outstran = DE_DEFAULT_AXI_OUTSTRAN;
	}

	if (!malidp_de_attr_valid(de_dev, MALIDP_ATTR_DE_POUTSTDCAB,
					de_dev->poutstdcab)) {
		dev_warn(de_dev->device, "%s : invalid value '%d' for de_axi_poutstdcab\n",
				__func__, de_dev->poutstdcab);
		de_dev->poutstdcab = DE_DEFAULT_AXI_POUTSTDCAB;
	}

	if (!malidp_de_attr_valid(de_dev, MALIDP_ATTR_DE_BURSTLEN,
					de_dev->burstlen)) {
		dev_warn(de_dev->device, "%s : invalid value '%d' for de_axi_burstlen\n",
				__func__, de_dev->burstlen);
		de_dev->burstlen = DE_DEFAULT_AXI_BURSTLEN;
	}

	malidp_de_set_axi_cfg(de_dev, de_dev->outstran,
			de_dev->poutstdcab,
			de_dev->burstlen);

	/* Initialize the ARQOS settings */
	malidp_de_init_axi_qos(de_dev,
		de_dev->arqos_threshold_low,
		de_dev->arqos_threshold_high,
		de_dev->arqos_red,
		de_dev->arqos_green);

	/* Set default background (black (0, 0, 0))*/
	malidp_de_write(de_dev, 0, MALI_DP550_REG_BG_COLOR);

	/* Clear display function, as CRST might not have worked */
	malidp_de_write(de_dev, 0, MALI_DP550_REG_DE_DISP_FUNC);

	/* Set prefetch_line to default value */
	malidp_de_clearbits(de_dev, DP550_PREFETCH_LINE_MASK,
		MALI_DP550_REG_DE_CTRL);
	malidp_de_setbits(de_dev, DP550_PREFETCH_LINE_SET(DE_DEFAULT_PREFETCH_LINE),
		MALI_DP550_REG_DE_CTRL);

	/*
	 * We are always interested on getting an IRQ as soon as a frame begins
	 * to be scanned out.
	 */
	malidp_de_write(de_dev, DE_LINE_INT_1(DE_DEFAULT_PREFETCH_LINE),
		MALI_DP550_REG_LINE_INT_CTRL);
	malidp_de_write(de_dev, MALI_DP550_DE_IRQ_UNR |
			MALI_DP550_DE_IRQ_PL1, MALI_DP550_REG_DE_IRQ_MSK);

	/* Write alpha lookup tables */
	malidp_de_write_alpha_lookup(de_dev);

	/* Disable all the layers so we don't scan out any stale config */
	malidp_de_disable_all_layers(de_dev);

	malidp_de_cleanup_yuv2rgb_coeffs(de_dev);

	dev_dbg(de_dev->device, "%s : success!\n", __func__);
	return 0;
}

static void dp550_de_modeset(struct malidp_de_device *de_dev,
		struct malidp_de_hwmode *hwmode)
{
	u32 sync_ctl = DE_H_SYNCWIDTH(hwmode->hsw) | DE_V_SYNCWIDTH(hwmode->vsw);

	if (hwmode->vsync_pol_pos)
		sync_ctl |= MALI_DP550_VSP;
	if (hwmode->hsync_pol_pos)
		sync_ctl |= MALI_DP550_HSP;

	malidp_de_write(de_dev, DE_H_FRONTPORCH(hwmode->hfp) | DE_H_BACKPORCH(hwmode->hbp),
			MALI_DP550_REG_H_INTERVALS);
	malidp_de_write(de_dev, DE_V_FRONTPORCH(hwmode->vfp) | DE_V_BACKPORCH(hwmode->vbp),
			MALI_DP550_REG_V_INTERVALS);
	malidp_de_write(de_dev, sync_ctl, MALI_DP550_REG_SYNC_CTRL);
	malidp_de_write(de_dev, DE_H_ACTIVE(hwmode->h_active) | DE_V_ACTIVE(hwmode->v_active),
		MALI_DP550_REG_HV_ACT_SIZE);

}

static void dp550_de_set_gamma_coeff(struct malidp_de_device *de_dev, u32 *coeffs)
{
	malidp_de_set_coeftab(de_dev, MALI_DP550_DE_COEFTAB_GAMMA,
		coeffs);
}

static u32 dp550_de_fmt_fixup(u32 drm_format, u32 flags)
{
	if (flags & MALIDP_FLAG_AFBC) {
		switch (drm_format) {
		case DRM_FORMAT_ARGB2101010:
		case DRM_FORMAT_RGBA1010102:
		case DRM_FORMAT_BGRA1010102:
			return DRM_FORMAT_ABGR2101010;
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_BGRA8888:
			return DRM_FORMAT_ABGR8888;
		case DRM_FORMAT_RGB888:
			return DRM_FORMAT_BGR888;
		case DRM_FORMAT_RGBA5551:
			return DRM_FORMAT_ABGR1555;
		case DRM_FORMAT_RGB565:
			return DRM_FORMAT_BGR565;
		case DRM_FORMAT_YUYV:
		case DRM_FORMAT_UYVY:
			return MALIDP_FORMAT_NV16AFBC;
		}
	}
	return drm_format;
}

static int dp550_se_hw_cfg(struct malidp_se_device *se_dev)
{
	malidp_se_set_axi_cfg(se_dev, se_dev->outstran,
			se_dev->burstlen,
			se_dev->wcache,
			se_dev->wqos);

	/* Set initial image enhancer state */
	se_dev->enh_cfg = MALIDP_SE_ENHANCER_OFF;
	malidp_se_clearbits(se_dev, SE_ENH_H_EN | SE_ENH_V_EN,
			MALI_DP550_REG_SE_CTL);
	malidp_se_write(se_dev, SE_SET_ENH_LIMIT_LOW(SE_ENH_LOW_LEVEL) |
			SE_SET_ENH_LIMIT_HIGH(SE_ENH_HIGH_LEVEL),
			malidp550_regmap.se_regmap.enhancer_control);

	/*
	 * Enable OFM in SE, disable everything else as CRST might not
	 * have worked
	 */
	malidp_se_write(se_dev, MALI_DP550_SE_CTL_OFM,
			MALI_DP550_REG_SE_CTL);
	/*
	 * We get an IRQ as soon as a frame starts to be written to memory and
	 * when config valid is latched by the init signal.
	 */
	malidp_se_write(se_dev, MALI_DP550_SE_PL_INTERVAL(32) | MALI_DP550_SE_PL_LINE(0),
		MALI_DP550_REG_SE_PL);
	malidp_se_write(se_dev, MALI_DP550_SE_IRQ_PL | MALI_DP550_SE_IRQ_EOW,
		SE_REG_MASKIRQ);

	/* Force coefficients to be updated next time they are used */
	se_dev->rgb2yuv_coeffs = NULL;
	se_dev->v_coeffstab = 0xffff;
	se_dev->h_coeffstab = 0xffff;

	dev_dbg(se_dev->device, "%s: success!\n", __func__);
	return 0;
}

static void dp550_se_set_scaler_coeff(struct malidp_se_device *se_dev,
		enum malidp_scaling_coeff_set hcoeff,
		enum malidp_scaling_coeff_set vcoeff)
{
	u32 mask = MALI_DP550_SE_CTL_VCSEL(MALI_DP550_SE_CTL_xSEL_MASK) |
		   MALI_DP550_SE_CTL_HCSEL(MALI_DP550_SE_CTL_xSEL_MASK);
	u32 new_value = MALI_DP550_SE_CTL_VCSEL(vcoeff + 1) |
			MALI_DP550_SE_CTL_HCSEL(hcoeff + 1);

	malidp_se_clearbits(se_dev, mask, MALI_DP550_REG_SE_CTL);

	malidp_se_setbits(se_dev, new_value, MALI_DP550_REG_SE_CTL);
}

static bool dp550_se_limitation_check(struct malidp_se_device *se_dev,
		struct malidp_se_scaler_conf *s0)
{
	bool ret = true;
	struct malidp_hw_device *hwdev = se_dev->hwdev;
	struct drm_mode_modeinfo mode;
	unsigned long flags;
	unsigned long mclk =
		clk_get_rate(hwdev->mclk);
	unsigned long pxclk =
		clk_get_rate(hwdev->pxclk);
	unsigned long numerator, denominator;

	spin_lock_irqsave(&hwdev->hw_lock, flags);
	malidp_de_modeget(hwdev->de_dev, &mode);
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	/*
	 * Equation:
	 * a = (max(h_input_size, h_output_size) * v_input_size) /
	 *         ((h_total_size - 2) * v_output_size)
	 *
	 * mclk = a * pxclk
	 *
	 * i.e. (h_total_size - 2) * v_output_size * mclk >=
	 *         max(h_input_size, h_output_size) * v_input_size * pxclk
	 */

	numerator = max(s0->input_w, s0->output_w) * s0->input_h;
	denominator = (mode.htotal - 2) * s0->output_h;

	if ((u64)mclk * denominator < (u64)pxclk * numerator) {
		dev_err(hwdev->device,
			"%s: Clock ratio (mclk/pxclk) is not high enough for downscale factor.",
			__func__);
		ret = false;
	}

	return ret;
}

static u32 dp550_se_calc_downscaling_threshold(u32 mclk, u32 pxlclk,
		struct drm_mode_modeinfo *mode)
{
	u64 mclk64 = mclk;

	mclk64 *= mode->htotal - 2;

	/* factor is (16.16) fix point */
	mclk64 <<= 16;
	do_div(mclk64, pxlclk);

	return mclk64;
}
