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
#include <linux/interrupt.h>
#include "malidp_product_api.h"
#include "malidp_de_device.h"
#include "malidp_se_device.h"
#include <uapi/video/malidp_adf.h>

#define MALI_DP500_REG_DE_STATUS	0x0000
#define		MALI_DP500_DE_LV_PTR_UPDATE	(1 << 8)
#define		MALI_DP500_DE_LG1_PTR_UPDATE	(1 << 9)
#define		MALI_DP500_DE_LG2_PTR_UPDATE	(1 << 10)
#define		MALI_DP500_DE_STATUS_CONFIG	(1 << 17)
#define		MALI_DP500_DE_STATUS_PSM	(1 << 18)
#define		MALI_DP500_DE_STATUS_TEST	(1 << 19)
#define		MALI_DP500_DE_STATUS_IRQ	(1 << 31)

#define MALI_DP500_REG_DE_MASKIRQ	0x0008
#define         MALI_DP500_DE_IRQ_UNDERRUN	(1 << 0)
#define         MALI_DP500_DE_IRQ_AXI_ERR	(1 << 4)
#define		MALI_DP500_DE_IRQ_PROG_LINE1	(1 << 5)
#define		MALI_DP500_DE_IRQ_PROG_LINE2	(1 << 6)
#define		MALI_DP500_DE_IRQ_SAT_ERR	(1 << 7)
#define		MALI_DP500_DE_IRQ_LV_PTR	(1 << 8)
#define		MALI_DP500_DE_IRQ_LG1_PTR	(1 << 9)
#define		MALI_DP500_DE_IRQ_LG2_PTR	(1 << 10)
#define		MALI_DP500_DE_IRQ_CONF_MODE	(1 << 11)
#define		MALI_DP500_DE_IRQ_ENABLE	(1 << 31)

#define MALI_DP500_REG_DE_CONTROL	0x000C
#define		MALI_DP500_DE_PREFETCH_LINE_MASK	0x1fff
#define		MALI_DP500_DE_PREFETCH_LINE_SHIFT	0
#define		MALI_DP500_DE_PREFETCH_LINE_SET(x)	\
	(((x) & MALI_DP500_DE_PREFETCH_LINE_MASK) << MALI_DP500_DE_PREFETCH_LINE_SHIFT)
#define		MALI_DP500_DE_SOFTRESET_REQ_EN	(1 << 16)
#define		MALI_DP500_DE_CONF_MODE_REQ_EN	(1 << 17)
#define		MALI_DP500_DE_PSM_REQ_EN	(1 << 18)
#define		MALI_DP500_DE_TESTMODE_REQ_EN	(1 << 19)
#define		MALI_DP500_DE_HSYNC_POL_POS	(1 << 20)
#define		MALI_DP500_DE_VSYNC_POL_POS	(1 << 21)

#define MALI_DP500_REG_DE_LINE_INT_CTRL		0x0010
#define		MALI_DP500_DE_AXI_OUTSTDCAPB_MASK	0xff
#define		MALI_DP500_DE_AXI_OUTSTDCAPB(x)	(((x) & MALI_DP500_DE_AXI_OUTSTDCAPB_MASK) << 0)
#define		MALI_DP500_DE_AXI_BURSTLEN_MASK	0xff
#define		MALI_DP500_DE_AXI_BURSTLEN(x)	(((x) & MALI_DP500_DE_AXI_BURSTLEN_MASK) << 16)

#define MALI_DP500_REG_DE_SECURE_CTRL		0x001C
#define MALI_DP500_REG_DE_H_INTERVALS		0x0028
#define MALI_DP500_REG_DE_V_INTERVALS		0x002C
#define MALI_DP500_REG_DE_SYNCWIDTH		0x0030
#define MALI_DP500_REG_DE_HV_ACTIVESIZE		0x0034

#define MALI_DP500_REG_DE_BG_COLOR_RG		0x003C
#define		MALI_DP500_DE_BG_COLOR_R(x)	(((x) & 0xfff) << 0)
#define		MALI_DP500_DE_BG_COLOR_G(x)	(((x) & 0xfff) << 16)
#define MALI_DP500_REG_DE_BG_COLOR_B		0x0040
#define		MALI_DP500_DE_BG_COLOR_B(x)	(((x) & 0xfff) << 0)

#define MALI_DP500_DE_COEFTAB_GAMMA	(DE_COEFTAB_TABLE_MASK << DE_COEFTAB_GAMMA_SHIFT)
#define MALI_DP500_DE_COEFTAB_LV_DEGAMMA	(DE_COEFTAB_TABLE_MASK << 19)

#define MALI_DP500_REG_ID		0x0FD0
#define MALI_DP500_REG_CFG_VALID	0x0F00
#define MALI_DP500_REG_DE_BASE		0x0000
#define MALI_DP500_REG_SE_BASE		(3 * SZ_1K)

#define MALI_DP500_REG_DE_AXI_CTL	0x0014
#define MALI_DP500_REG_DE_DISP_FUNC	0x0020
#define MALI_DP500_REG_DE_OD		0x0044
#define MALI_DP500_REG_DE_COLORCOEFFS 0x0078
#define MALI_DP500_REG_DE_QOS		0x0500
#define MALI_DP500_REG_YUV2RGB		0x0048

#define MALI_DP500_REG_SE_CTL		0x000C
#define MALI_DP500_REG_SE_AXI_CRL	0x0014
#define MALI_DP500_REG_SE_L_CTL		0x0020
#define MALI_DP500_REG_SE_SCL_CTL	0x0030
#define MALI_DP500_REG_SE_ENH_CTL	0x0048
#define MALI_DP500_REG_SE_COV_CTL	0x0070
#define MALI_DP500_REG_SE_MW_CTL	0x0200

#define MALI_DP500_LV_BASE	0X0100
#define MALI_DP500_LG1_BASE	0x0200
#define MALI_DP500_LG2_BASE	0x0300

#define MALI_DP500_AD_LV_CTRL	0x400
#define MALI_DP500_AD_LV_CROP_H	0x404
#define MALI_DP500_AD_LV_CROP_V	0x408
#define MALI_DP500_AD_LG1_CTRL	0x40C
#define MALI_DP500_AD_LG1_CROP_H	0x410
#define MALI_DP500_AD_LG1_CROP_V	0x414
#define MALI_DP500_AD_LG2_CTRL	0x418
#define MALI_DP500_AD_LG2_CROP_H	0x41C
#define MALI_DP500_AD_LG2_CROP_V	0x420

extern const struct malidp_intf_hw_info dp_interfaces[];
extern const char *const op_mode_name[];

#define SE_N_SCALING_COEFFS_SET 5
#define SE_N_SCALING_COEFFS_IN_SET	96
static const u16 dp500_se_scaling_coeffs[SE_N_SCALING_COEFFS_SET][SE_N_SCALING_COEFFS_IN_SET] = {
	/* SET 0: upscaling, without scaling */
	{	0x0000, 0x0001, 0x0007, 0x0011, 0x001e, 0x002e, 0x003f, 0x0052,
		0x0064, 0x0073, 0x007d, 0x0080, 0x007a, 0x006c, 0x0053, 0x002f,
		0x0000, 0x3fc6, 0x3f83, 0x3f39, 0x3eea, 0x3e9b, 0x3e4f, 0x3e0a,
		0x3dd4, 0x3db0, 0x3da2, 0x3db1, 0x3dde, 0x3e2f, 0x3ea5, 0x3f40,
		0x0000, 0x00e5, 0x01ee, 0x0315, 0x0456, 0x05aa, 0x0709, 0x086c,
		0x09c9, 0x0b15, 0x0c4a, 0x0d5d, 0x0e4a, 0x0f06, 0x0f91, 0x0fe5,
		0x1000, 0x0fe5, 0x0f91, 0x0f06, 0x0e4a, 0x0d5d, 0x0c4a, 0x0b15,
		0x09c9, 0x086c, 0x0709, 0x05aa, 0x0456, 0x0315, 0x01ee, 0x00e5,
		0x0000, 0x3f40, 0x3ea5, 0x3e2f, 0x3dde, 0x3db1, 0x3da2, 0x3db0,
		0x3dd4, 0x3e0a, 0x3e4f, 0x3e9b, 0x3eea, 0x3f39, 0x3f83, 0x3fc6,
		0x0000, 0x002f, 0x0053, 0x006c, 0x007a, 0x0080, 0x007d, 0x0073,
		0x0064, 0x0052, 0x003f, 0x002e, 0x001e, 0x0011, 0x0007, 0x0001
	},
	/* SET 1: without scaling, 1.5x down scaling*/
	{	0x0059, 0x004f, 0x0041, 0x002e, 0x0016, 0x3ffb, 0x3fd9, 0x3fb4,
		0x3f8c, 0x3f62, 0x3f36, 0x3f09, 0x3edd, 0x3eb3, 0x3e8d, 0x3e6c,
		0x3e52, 0x3e3f, 0x3e35, 0x3e37, 0x3e46, 0x3e61, 0x3e8c, 0x3ec5,
		0x3f0f, 0x3f68, 0x3fd1, 0x004a, 0x00d3, 0x0169, 0x020b, 0x02b8,
		0x036e, 0x042d, 0x04f2, 0x05b9, 0x0681, 0x0745, 0x0803, 0x08ba,
		0x0965, 0x0a03, 0x0a91, 0x0b0d, 0x0b75, 0x0bc6, 0x0c00, 0x0c20,
		0x0c28, 0x0c20, 0x0c00, 0x0bc6, 0x0b75, 0x0b0d, 0x0a91, 0x0a03,
		0x0965, 0x08ba, 0x0803, 0x0745, 0x0681, 0x05b9, 0x04f2, 0x042d,
		0x036e, 0x02b8, 0x020b, 0x0169, 0x00d3, 0x004a, 0x3fd1, 0x3f68,
		0x3f0f, 0x3ec5, 0x3e8c, 0x3e61, 0x3e46, 0x3e37, 0x3e35, 0x3e3f,
		0x3e52, 0x3e6c, 0x3e8d, 0x3eb3, 0x3edd, 0x3f09, 0x3f36, 0x3f62,
		0x3f8c, 0x3fb4, 0x3fd9, 0x3ffb, 0x0016, 0x002e, 0x0041, 0x004f
	},
	/* SET 2: 1.5x down scaling, 2x down scaling */
	{	0x3f19, 0x3f03, 0x3ef0, 0x3edf, 0x3ed0, 0x3ec5, 0x3ebd, 0x3eb9,
		0x3eb9, 0x3ebf, 0x3eca, 0x3ed9, 0x3eef, 0x3f0a, 0x3f2c, 0x3f52,
		0x3f7f, 0x3fb0, 0x3fe8, 0x0026, 0x006a, 0x00b4, 0x0103, 0x0158,
		0x01b1, 0x020d, 0x026c, 0x02cd, 0x032f, 0x0392, 0x03f4, 0x0455,
		0x04b4, 0x051e, 0x0585, 0x05eb, 0x064c, 0x06a8, 0x06fe, 0x074e,
		0x0796, 0x07d5, 0x080c, 0x0839, 0x085c, 0x0875, 0x0882, 0x0887,
		0x0881, 0x0887, 0x0882, 0x0875, 0x085c, 0x0839, 0x080c, 0x07d5,
		0x0796, 0x074e, 0x06fe, 0x06a8, 0x064c, 0x05eb, 0x0585, 0x051e,
		0x04b4, 0x0455, 0x03f4, 0x0392, 0x032f, 0x02cd, 0x026c, 0x020d,
		0x01b1, 0x0158, 0x0103, 0x00b4, 0x006a, 0x0026, 0x3fe8, 0x3fb0,
		0x3f7f, 0x3f52, 0x3f2c, 0x3f0a, 0x3eef, 0x3ed9, 0x3eca, 0x3ebf,
		0x3eb9, 0x3eb9, 0x3ebd, 0x3ec5, 0x3ed0, 0x3edf, 0x3ef0, 0x3f03
	},
	/* SET 3: 2x down scaling, 2.75x down scaling */
	{	0x3f51, 0x3f60, 0x3f71, 0x3f84, 0x3f98, 0x3faf, 0x3fc8, 0x3fe3,
		0x0000, 0x001f, 0x0040, 0x0064, 0x008a, 0x00b1, 0x00da, 0x0106,
		0x0133, 0x0160, 0x018e, 0x01bd, 0x01ec, 0x021d, 0x024e, 0x0280,
		0x02b2, 0x02e4, 0x0317, 0x0349, 0x037c, 0x03ad, 0x03df, 0x0410,
		0x0440, 0x0468, 0x048f, 0x04b3, 0x04d6, 0x04f8, 0x0516, 0x0533,
		0x054e, 0x0566, 0x057c, 0x0590, 0x05a0, 0x05ae, 0x05ba, 0x05c3,
		0x05c9, 0x05c3, 0x05ba, 0x05ae, 0x05a0, 0x0590, 0x057c, 0x0566,
		0x054e, 0x0533, 0x0516, 0x04f8, 0x04d6, 0x04b3, 0x048f, 0x0468,
		0x0440, 0x0410, 0x03df, 0x03ad, 0x037c, 0x0349, 0x0317, 0x02e4,
		0x02b2, 0x0280, 0x024e, 0x021d, 0x01ec, 0x01bd, 0x018e, 0x0160,
		0x0133, 0x0106, 0x00da, 0x00b1, 0x008a, 0x0064, 0x0040, 0x001f,
		0x0000, 0x3fe3, 0x3fc8, 0x3faf, 0x3f98, 0x3f84, 0x3f71, 0x3f60
	},
	/* SET 4: 2.75x down scaling, 4x down scaling */
	{	0x0094, 0x00a9, 0x00be, 0x00d4, 0x00ea, 0x0101, 0x0118, 0x012f,
		0x0148, 0x0160, 0x017a, 0x0193, 0x01ae, 0x01c8, 0x01e4, 0x01ff,
		0x021c, 0x0233, 0x024a, 0x0261, 0x0278, 0x028f, 0x02a6, 0x02bd,
		0x02d4, 0x02eb, 0x0302, 0x0319, 0x032f, 0x0346, 0x035d, 0x0374,
		0x038a, 0x0397, 0x03a3, 0x03af, 0x03bb, 0x03c6, 0x03d1, 0x03db,
		0x03e4, 0x03ed, 0x03f6, 0x03fe, 0x0406, 0x040d, 0x0414, 0x041a,
		0x0420, 0x041a, 0x0414, 0x040d, 0x0406, 0x03fe, 0x03f6, 0x03ed,
		0x03e4, 0x03db, 0x03d1, 0x03c6, 0x03bb, 0x03af, 0x03a3, 0x0397,
		0x038a, 0x0374, 0x035d, 0x0346, 0x032f, 0x0319, 0x0302, 0x02eb,
		0x02d4, 0x02bd, 0x02a6, 0x028f, 0x0278, 0x0261, 0x024a, 0x0233,
		0x021c, 0x01ff, 0x01e4, 0x01c8, 0x01ae, 0x0193, 0x017a, 0x0160,
		0x0148, 0x012f, 0x0118, 0x0101, 0x00ea, 0x00d4, 0x00be, 0x00a9
	}
};

static const u32 malidp_de_bt709_degamma_coeffs[DE_N_COEFTAB_COEFS] = {
	DE_COEFTAB_DATA(2, 0),
	DE_COEFTAB_DATA(6, 0),
	DE_COEFTAB_DATA(11, 2),
	DE_COEFTAB_DATA(17, 5),
	DE_COEFTAB_DATA(23, 9),
	DE_COEFTAB_DATA(30, 15),
	DE_COEFTAB_DATA(36, 22),
	DE_COEFTAB_DATA(43, 31),
	DE_COEFTAB_DATA(50, 42),
	DE_COEFTAB_DATA(57, 55),
	DE_COEFTAB_DATA(64, 69),
	DE_COEFTAB_DATA(72, 85),
	DE_COEFTAB_DATA(79, 103),
	DE_COEFTAB_DATA(87, 123),
	DE_COEFTAB_DATA(95, 145),
	DE_COEFTAB_DATA(103, 168),
	DE_COEFTAB_DATA(111, 194),
	DE_COEFTAB_DATA(119, 222),
	DE_COEFTAB_DATA(127, 251),
	DE_COEFTAB_DATA(135, 283),
	DE_COEFTAB_DATA(144, 317),
	DE_COEFTAB_DATA(152, 353),
	DE_COEFTAB_DATA(161, 391),
	DE_COEFTAB_DATA(169, 431),
	DE_COEFTAB_DATA(178, 474),
	DE_COEFTAB_DATA(187, 518),
	DE_COEFTAB_DATA(195, 565),
	DE_COEFTAB_DATA(204, 614),
	DE_COEFTAB_DATA(213, 665),
	DE_COEFTAB_DATA(222, 718),
	DE_COEFTAB_DATA(231, 774),
	DE_COEFTAB_DATA(241, 832),
	DE_COEFTAB_DATA(250, 892),
	DE_COEFTAB_DATA(259, 954),
	DE_COEFTAB_DATA(268, 1019),
	DE_COEFTAB_DATA(278, 1086),
	DE_COEFTAB_DATA(287, 1155),
	DE_COEFTAB_DATA(297, 1227),
	DE_COEFTAB_DATA(306, 1301),
	DE_COEFTAB_DATA(316, 1378),
	DE_COEFTAB_DATA(325, 1457),
	DE_COEFTAB_DATA(335, 1538),
	DE_COEFTAB_DATA(345, 1622),
	DE_COEFTAB_DATA(354, 1708),
	DE_COEFTAB_DATA(364, 1797),
	DE_COEFTAB_DATA(374, 1888),
	DE_COEFTAB_DATA(384, 1981),
	DE_COEFTAB_DATA(394, 2077),
	DE_COEFTAB_DATA(404, 2176),
	DE_COEFTAB_DATA(414, 2277),
	DE_COEFTAB_DATA(424, 2380),
	DE_COEFTAB_DATA(434, 2486),
	DE_COEFTAB_DATA(444, 2595),
	DE_COEFTAB_DATA(454, 2706),
	DE_COEFTAB_DATA(464, 2819),
	DE_COEFTAB_DATA(475, 2936),
	DE_COEFTAB_DATA(485, 3054),
	DE_COEFTAB_DATA(495, 3176),
	DE_COEFTAB_DATA(506, 3299),
	DE_COEFTAB_DATA(516, 3426),
	DE_COEFTAB_DATA(527, 3555),
	DE_COEFTAB_DATA(537, 3687),
	DE_COEFTAB_DATA(547, 3821),
	DE_COEFTAB_DATA(558, 3958),
};

/*
 * Supported formats are defined so that their index in the array matches
 * the value for the lx_format field in the LX_FORMAT registers.
 */
const u32 malidp500_input_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	/* Formats below here supported by Video layer only */
	DRM_FORMAT_UYVY,  /* 1-plane UYVY */
	DRM_FORMAT_YUYV,  /* 1-plane YUYV */
	DRM_FORMAT_NV12,  /* YUV 4:2:0 2-plane */
	DRM_FORMAT_YUV420,/* YUV 4:2:0 3-plane */
	MALIDP_FORMAT_XYUV, /* [31:0] X:Y:Cb:Cr 8:8:8:8 little endian */
	MALIDP_FORMAT_VYU30,/* [31:0] X:Cr:Y:Cb 2:10:10:10 little endian */
	MALIDP_FORMAT_YUV10_420AFBC, /* AFBC compressed YUV 4:2:0, 10 bits per component */
	MALIDP_FORMAT_NV12AFBC, /* AFBC compressed YUV 4:2:0. Maps to NV12 internally */
};

const u32 malidp500_output_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_XRGB8888,
	DRM_FORMAT_XBGR8888,
	DRM_FORMAT_NV12,
};

const u32 malidp500_output_format_ids[] = {
	0,
	1,
	2,
	3,
	4,
	5,
	14,
};

const u32 malidp500_afbc_formats[] = {
	DRM_FORMAT_ARGB2101010,
	DRM_FORMAT_ABGR2101010,
	DRM_FORMAT_ARGB8888,
	DRM_FORMAT_ABGR8888,
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
	DRM_FORMAT_RGBA5551,
	DRM_FORMAT_ABGR1555,
	DRM_FORMAT_RGB565,
	DRM_FORMAT_BGR565,
	DRM_FORMAT_UYVY,
	DRM_FORMAT_YUYV,
	MALIDP_FORMAT_XYUV,
	MALIDP_FORMAT_VYU30,
	MALIDP_FORMAT_YUV10_420AFBC,
	MALIDP_FORMAT_NV12AFBC
};

const u32 malidp500_xform_invalid_formats[] = {
	DRM_FORMAT_RGB888,
	DRM_FORMAT_BGR888,
};

/* Define layer information */
static struct malidp_layer_hw_info dp500_hw_layers[] = {
	{
		.index = 0,
		.name = "Video-1",
		.type = MALIDP_HW_LAYER_VIDEO,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC |
			    MALIDP_LAYER_FEATURE_SCALING,
		.n_supported_formats = ARRAY_SIZE(malidp500_input_formats),
		.supported_formats = malidp500_input_formats,
		.n_max_planes = 3,
		.n_supported_layers = 1,
		.regs_base = MALI_DP500_LV_BASE,
		.ad_ctrl_reg = MALI_DP500_AD_LV_CTRL,
		.ad_crop_h_reg = MALI_DP500_AD_LV_CROP_H,
		.ad_crop_v_reg = MALI_DP500_AD_LV_CROP_V,
		.stride_offset = DE_REG_LV1_STRIDE,
		.ptr0_low_offset = DE_REG_LV1_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LV1_PTR0_HIGH,
		.p3_stride_offset = DE_REG_LV3_STRIDE,
		.yuv2rgb_reg_offset = MALI_DP500_REG_YUV2RGB,
	},
	{
		.index = 1,
		.name = "Graphics-1",
		.type = MALIDP_HW_LAYER_GRAPHICS,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC |
			    MALIDP_LAYER_FEATURE_SCALING,
		.n_supported_formats = 12,
		.supported_formats = malidp500_input_formats,
		.n_max_planes = 1,
		.n_supported_layers = 1,
		.regs_base = MALI_DP500_LG1_BASE,
		.ad_ctrl_reg = MALI_DP500_AD_LG1_CTRL,
		.ad_crop_h_reg = MALI_DP500_AD_LG1_CROP_H,
		.ad_crop_v_reg = MALI_DP500_AD_LG1_CROP_V,
		.stride_offset = DE_REG_LG_STRIDE,
		.ptr0_low_offset = DE_REG_LG_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LG_PTR0_HIGH,
	},
	{
		.index = 2,
		.name = "Graphics-2",
		.type = MALIDP_HW_LAYER_GRAPHICS,
		.features = MALIDP_LAYER_FEATURE_TRANSFORM |
			    MALIDP_LAYER_FEATURE_AFBC,
		.n_supported_formats = 12,
		.supported_formats = malidp500_input_formats,
		.n_max_planes = 1,
		.n_supported_layers = 1,
		.regs_base = MALI_DP500_LG2_BASE,
		.ad_ctrl_reg = MALI_DP500_AD_LG2_CTRL,
		.ad_crop_h_reg = MALI_DP500_AD_LG2_CROP_H,
		.ad_crop_v_reg = MALI_DP500_AD_LG2_CROP_V,
		.stride_offset = DE_REG_LG_STRIDE,
		.ptr0_low_offset = DE_REG_LG_PTR0_LOW,
		.ptr0_high_offset = DE_REG_LG_PTR0_HIGH,
	}
};

static const struct malidp_hw_regmap malidp500_regmap = {
	.id_registers = MALI_DP500_REG_ID,
	.config_valid = MALI_DP500_REG_CFG_VALID,
	.de_base = MALI_DP500_REG_DE_BASE,
	.se_base = MALI_DP500_REG_SE_BASE,
	.de_regmap = {
		.axi_control = MALI_DP500_REG_DE_AXI_CTL,
		.disp_func = MALI_DP500_REG_DE_DISP_FUNC,
		.output_depth = MALI_DP500_REG_DE_OD,
		.coloradj_coeff = MALI_DP500_REG_DE_COLORCOEFFS,
		.qos_control = MALI_DP500_REG_DE_QOS
	},
	.se_regmap = {
		.control = MALI_DP500_REG_SE_CTL,
		.axi_control = MALI_DP500_REG_SE_AXI_CRL,
		.layers_control = MALI_DP500_REG_SE_L_CTL,
		.scaling_control = MALI_DP500_REG_SE_SCL_CTL,
		.enhancer_control = MALI_DP500_REG_SE_ENH_CTL,
		.conv_control = MALI_DP500_REG_SE_COV_CTL,
		.mw_control = MALI_DP500_REG_SE_MW_CTL
	},
};

/* Declare of product APIs */
static void dp500_disable_irq(struct malidp_hw_device *);
static enum malidp_op_mode dp500_change_op_mode(
		struct malidp_hw_device *,
		enum malidp_op_mode);
static int dp500_de_hw_cfg(struct malidp_de_device *);
static void dp500_de_modeset(struct malidp_de_device *,
		struct malidp_de_hwmode *);
static void dp500_de_set_gamma_coeff(struct malidp_de_device *,
		u32 *coeffs);
static u32 dp500_de_fmt_fixup(u32 drm_format, u32 flags);
static irqreturn_t dp500_de_irq_handler(int irq, void *data);
static int dp500_se_hw_cfg(struct malidp_se_device *);
static void dp500_se_set_scaler_coeff(struct malidp_se_device *,
		enum malidp_scaling_coeff_set hcoeff,
		enum malidp_scaling_coeff_set vcoeff);
static bool dp500_se_limitation_check(struct malidp_se_device *,
		struct malidp_se_scaler_conf *);
static u32 dp500_se_calc_downscaling_threshold(u32, u32,
		struct drm_mode_modeinfo *);
static umode_t dp500_attr_visible(struct kobject *obj,
		struct attribute *attr, int n);
static bool dp500_de_axi_valid(u32 attr, u32 val);
static bool dp500_se_axi_valid(u32 attr, u32 val);
static irqreturn_t dp500_se_irq_handler(int irq, void *data);
static u32 dp500_rotmem_required(const struct malidp_hw_buffer *hw_buf);

static const struct malidp_product_api dp500_api = {
	.change_op_mode = dp500_change_op_mode,
	.disable_irq = dp500_disable_irq,
	.attr_visible = dp500_attr_visible,
	.rotmem_size_required = dp500_rotmem_required,
	.de_api = {
		.hw_cfg = dp500_de_hw_cfg,
		.modeset = dp500_de_modeset,
		.set_gamma_coeff = dp500_de_set_gamma_coeff,
		.fmt_fixup = dp500_de_fmt_fixup,
		.axi_valid = dp500_de_axi_valid,
		.irq_handler = dp500_de_irq_handler,
	},
	.se_api = {
		.hw_cfg = dp500_se_hw_cfg,
		.set_scaler_coeff = dp500_se_set_scaler_coeff,
		.limitation_check = dp500_se_limitation_check,
		.calc_downscaling_threshold = dp500_se_calc_downscaling_threshold,
		.axi_valid = dp500_se_axi_valid,
		.irq_handler = dp500_se_irq_handler,
	}
};

static struct malidp_hw_topology malidp_dp500_topology = {
	.product_id = MALIDP_DP500_PRODUCT_ID,
	.interfaces = dp_interfaces,
	.n_interfaces = 2,
	.layers = dp500_hw_layers,
	.n_layers = ARRAY_SIZE(dp500_hw_layers),
	.n_scalers = 1,
	.n_supported_afbc_formats = ARRAY_SIZE(malidp500_afbc_formats),
	.supported_afbc_formats = malidp500_afbc_formats,
	.supported_afbc_splitblk = 0,
	.dp_api = &dp500_api,
	.regmap = &malidp500_regmap,
	.n_mw_formats = ARRAY_SIZE(malidp500_output_formats),
	.mw_formats = malidp500_output_formats,
	.mw_format_ids = malidp500_output_format_ids,
	.n_xform_invalid_formats = ARRAY_SIZE(malidp500_xform_invalid_formats),
	.xform_invalid_formats = malidp500_xform_invalid_formats,
};

static const struct malidp_line_size_hw_info dp500_ls_configs[] = {
	{
		.max_line_size = 2048,
		.min_line_size = 2,
		.input_fifo_size = 2048,
		.default_rotmem_size = 64 * SZ_1K,
	},
	{
		.max_line_size = 4096,
		.min_line_size = 2,
		.input_fifo_size = 4096,
		.default_rotmem_size = 128 * SZ_1K,
	}
};

static struct malidp_hw_configuration malidp_hw_dp500_cf = {
	.ls_configs = dp500_ls_configs,
	.n_configs = ARRAY_SIZE(dp500_ls_configs),
	.partition_type = MALIDP_HW_PARTITION_FIXED,
};

umode_t dp500_attr_visible(struct kobject *obj, struct attribute *attr,
		int n)
{
	const char *hidden_attr_name[] = {
#ifdef DEBUG
		"de_poutstdcab",
#endif
		NULL /* Last item */
	};
	int i = 0;

	/* if attr name is found in hidden list, don't show it */
	while (hidden_attr_name[i] != NULL) {
		if (strcmp(attr->name, hidden_attr_name[i]) == 0)
			return 0;
		i++;
	}

	return attr->mode;
}

void malidp_dp500_get_hw_description(struct malidp_hw_description *hwdes)
{
	hwdes->topology = &malidp_dp500_topology;
	hwdes->config = &malidp_hw_dp500_cf;
}

static bool dp500_axi_burstlen_valid(u32 val)
{
	int i;
	/* for 1, 2, 4, ... 256 */
	for (i = 0; i < 9; i++)
		if (val == (1 << i))
			break;
	return (i < 9) ? true : false;
}

/*
 * Change the operation mode of the DE;
 * @dev: pointer to the private malidp device structure.
 * @new_mode: the mode we want the device to operate.
 */
static enum malidp_op_mode
dp500_de_op_change(struct malidp_de_device *dev,
	enum malidp_op_mode new_mode)
{
	u32 all_modes = MALI_DP500_DE_CONF_MODE_REQ_EN |
			MALI_DP500_DE_PSM_REQ_EN |
			MALI_DP500_DE_TESTMODE_REQ_EN;
	enum malidp_op_mode old_mode = dev->op_mode;
	u32 status = all_modes;
	u32 control;

	dev_dbg(dev->device, "performing DE mode change: %s->%s\n",
		op_mode_name[old_mode], op_mode_name[new_mode]);

	/* Verify that the HW is currently in the mode that we expected */
	if (likely(old_mode != MALIDP_OP_MODE_UNKNOWN)) {
		status = malidp_de_read(dev, MALI_DP500_REG_DE_STATUS);
		if (status & MALI_DP500_DE_STATUS_CONFIG)
			BUG_ON(old_mode != MALIDP_OP_MODE_CONFIG);
		else if (status & MALI_DP500_DE_STATUS_PSM)
			BUG_ON(old_mode != MALIDP_OP_MODE_POWERSAVE);
		else if (status & MALI_DP500_DE_STATUS_TEST)
			BUG_ON(old_mode != MALIDP_OP_MODE_TEST);
		else
			BUG_ON(old_mode != MALIDP_OP_MODE_NORMAL);
	}

	if (old_mode == new_mode) {
		return old_mode;
	} else if (old_mode == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * Exiting powersave mode, so clear the status register
		 * and re-enable interrupts
		 */
		status = malidp_de_read(dev, MALI_DP500_REG_DE_STATUS);
		malidp_de_write(dev, status, MALI_DP500_REG_DE_STATUS);
		malidp_de_setbits(dev, MALI_DP500_DE_IRQ_UNDERRUN |
					MALI_DP500_DE_IRQ_AXI_ERR |
					MALI_DP500_DE_IRQ_PROG_LINE1,
					MALI_DP500_REG_DE_MASKIRQ);
	}

	/* Unset any previous mode */
	control = malidp_de_read(dev, MALI_DP500_REG_DE_CONTROL);
	control &= (~all_modes);

	if (new_mode == MALIDP_OP_MODE_CONFIG) {
		control |= MALI_DP500_DE_CONF_MODE_REQ_EN;
	} else if (new_mode == MALIDP_OP_MODE_POWERSAVE) {
		control |= MALI_DP500_DE_PSM_REQ_EN;
	} else {
		dev->scene_changing = false;
		malidp_hw_commit_scene_atomic(dev->hwdev, true);
	}

	/* Commit the new mode */
	malidp_de_write(dev, control, MALI_DP500_REG_DE_CONTROL);

	/*
	 * Verify the new mode has been applied. We rely on status and control
	 * registers having the same flags for setting/checking the mode.
	 */
	while ((status & all_modes) != (control & all_modes))
		status = malidp_de_read(dev, MALI_DP500_REG_DE_STATUS);

	if (new_mode == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * Clear the IRQ masks so that we don't get asked to handle
		 * interrupts which happen in the future.
		 * We can't just clear the global mask because this prevents
		 * IRQ line from getting cleared.
		 */
		malidp_de_clearbits(dev, MALI_DP500_DE_IRQ_UNDERRUN |
				MALI_DP500_DE_IRQ_AXI_ERR |
				MALI_DP500_DE_IRQ_PROG_LINE1,
				MALI_DP500_REG_DE_MASKIRQ);
		/*
		 * If an IRQ has happened whilst changing mode, we must handle
		 * it (because the GIC has already seen it), without touching
		 * registers (because we're about to turn the clocks off), so
		 * we store the status register for use by our IRQ handler.
		 */
		dev->pending_status = status;
		/* Clear any interrupt flags */
		malidp_de_write(dev, status, MALI_DP500_REG_DE_STATUS);
	}

	dev->op_mode = new_mode;

	dev_dbg(dev->device, "DE mode change ok: %s->%s\n",
		op_mode_name[old_mode], op_mode_name[new_mode]);

	return old_mode;
}

/*
 * Change the operation mode of the SE and return the old value;
 * @dev: pointer to the private malidp device structure.
 * @new_mode: the mode we want the device to operate.
 */
static enum malidp_op_mode
dp500_se_op_change(struct malidp_se_device *dev,
	enum malidp_op_mode new_mode)
{
	u16 reg_control = malidp500_regmap.se_regmap.control;
	u32 status;
	enum malidp_op_mode old_mode = dev->op_mode;;

	dev_dbg(dev->device, "performing SE mode change: %s->%s\n",
		op_mode_name[old_mode], op_mode_name[new_mode]);

	/* Verify that the HW is currently in the mode that we expected */
	if (likely(old_mode != MALIDP_OP_MODE_UNKNOWN)) {
		status = malidp_se_read(dev, SE_REG_STATUS);
		/* This order is important. PSM has highest priority */
		if (status & SE_ST_PSM_ACTIVE)
			BUG_ON(old_mode != MALIDP_OP_MODE_POWERSAVE);
		else if (status & SE_ST_CONFIG_ACTIVE)
			BUG_ON(old_mode != MALIDP_OP_MODE_CONFIG);
		else
			BUG_ON(old_mode != MALIDP_OP_MODE_NORMAL);
	}

	if (old_mode == new_mode) {
		return old_mode;
	} else if (old_mode == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * Exiting powersave mode, so clear the status register
		 * and re-enable interrupts
		 */
		status = malidp_se_read(dev, SE_REG_STATUS);
		malidp_se_write(dev, status, SE_REG_STATUS);
		malidp_se_setbits(dev, SE_IRQ_OVERRUN | SE_IRQ_AXI_ERR |
			SE_IRQ_PROGLINE1 | SE_IRQ_PTR_UPDATE, SE_REG_MASKIRQ);
	}

	if (new_mode == MALIDP_OP_MODE_CONFIG) {
		malidp_se_setbits(dev, SE_CONFIG_REQ, reg_control);
	} else if (new_mode == MALIDP_OP_MODE_NORMAL) {
		malidp_se_clearbits(dev, SE_CONFIG_REQ, reg_control);
	} else if (new_mode == MALIDP_OP_MODE_POWERSAVE) {
		/*
		 * POWERSAVE mode is controlled by the DE, but we will enter
		 * config mode here, so that we can guarantee all processing
		 * is finished
		 */
		malidp_se_setbits(dev, SE_CONFIG_REQ, reg_control);
		do {
			status = malidp_se_read(dev, SE_REG_STATUS);
		} while (!(status & SE_ST_CONFIG_ACTIVE));
		/*
		 * Clear the IRQ masks so that we don't get asked to handle
		 * interrupts which happen in the future.
		 * We can't just clear the global mask because this prevents
		 * IRQ line from getting cleared.
		 */
		malidp_se_clearbits(dev, SE_IRQ_OVERRUN | SE_IRQ_AXI_ERR |
			SE_IRQ_PROGLINE1 | SE_IRQ_PTR_UPDATE, SE_REG_MASKIRQ);
		/*
		 * If an IRQ has happened whilst changing mode, we must handle
		 * it (because the GIC has already seen it) without touching
		 * registers (because we're about to turn the clocks off) so
		 * we store the status register for use by our IRQ handler.
		 */
		dev->pending_status = status;
		/* Clear any interrupt flags */
		malidp_se_write(dev, status, SE_REG_STATUS);
	}

	dev->op_mode = new_mode;

	dev_dbg(dev->device, "SE mode change ok: %s\n", op_mode_name[new_mode]);

	return old_mode;
}

static irqreturn_t dp500_de_irq_handler(int irq, void *data)
{
struct malidp_de_device *dev = data;
	u32 status, irq_mask, irq_vector;
	unsigned long flags;
	struct malidp_hw_event event;

	spin_lock_irqsave(dev->hw_lock, flags);

	/* Drop any events if we already went to powersave */
	if (unlikely(dev->op_mode == MALIDP_OP_MODE_POWERSAVE)) {
		status = dev->pending_status;
		dev->pending_status = 0;
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return status & MALI_DP500_DE_STATUS_IRQ ? IRQ_HANDLED :
			IRQ_NONE;
	}

	status = malidp_de_read(dev, MALI_DP500_REG_DE_STATUS);
	irq_mask = malidp_de_read(dev, MALI_DP500_REG_DE_MASKIRQ);
	irq_vector = status & irq_mask;

	/* The IRQ does not belong to this device */
	if (!(status & MALI_DP500_DE_STATUS_IRQ)) {
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return IRQ_NONE;
	}

	/* Get the timestamp as soon as possible for more accuracy */
	event.timestamp = ktime_get();
	event.type = MALIDP_HW_EVENT_NONE;

	if (irq_vector & MALI_DP500_DE_IRQ_UNDERRUN) {
		dev_err(dev->device, "%s: underrun error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_URUN;
	}

	if (irq_vector & MALI_DP500_DE_IRQ_AXI_ERR) {
		dev_err(dev->device, "%s: axi error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_AXI;
	}

	/* If there was a pointer update this is flip event. Otherwise
	 * this is only a regular vsync.
	 */
	if (irq_vector &  MALI_DP500_DE_IRQ_PROG_LINE1) {
		event.type |= MALIDP_HW_EVENT_VSYNC;
		if ((status & MALI_DP500_DE_LV_PTR_UPDATE) ||
		    (status & MALI_DP500_DE_LG1_PTR_UPDATE) ||
		    (status & MALI_DP500_DE_LG2_PTR_UPDATE)) {
			event.type |= MALIDP_HW_EVENT_NEWCFG;
			if (dev->scene_changing) {
				event.type |= MALIDP_HW_EVENT_FLIP;
				dev->scene_changing = false;
			}
		}
	}

	/* IRQs not enabled should not be triggered */
	BUG_ON((irq_vector & MALI_DP500_DE_IRQ_PROG_LINE2) ||
		(irq_vector & MALI_DP500_DE_IRQ_LV_PTR) ||
		(irq_vector & MALI_DP500_DE_IRQ_LG1_PTR) ||
		(irq_vector & MALI_DP500_DE_IRQ_LG2_PTR) ||
		(irq_vector & MALI_DP500_DE_IRQ_SAT_ERR) ||
		(irq_vector & MALI_DP500_DE_IRQ_CONF_MODE));

	/* There must always be a valid event if the IRQ is triggered */
	BUG_ON(event.type == MALIDP_HW_EVENT_NONE);

	/* Disable the offending error IRQ if there was an error */
	if (event.type & MALIDP_HW_EVENT_ERROR) {
		uint32_t mask_bits = irq_vector &
				(MALI_DP500_DE_IRQ_UNDERRUN |
				MALI_DP500_DE_IRQ_AXI_ERR);
		malidp_de_clearbits(dev, mask_bits, MALI_DP500_REG_DE_MASKIRQ);
	}

	/* Clear the status register */
	malidp_de_write(dev, status, MALI_DP500_REG_DE_STATUS);

	/* If there's a new scene, re-enable the error interrupts */
	if (event.type & MALIDP_HW_EVENT_FLIP) {
		uint32_t mask_bits = (MALI_DP500_DE_IRQ_UNDERRUN |
					MALI_DP500_DE_IRQ_AXI_ERR);
		if ((mask_bits & irq_mask) != mask_bits)
			malidp_de_setbits(dev, mask_bits, MALI_DP500_REG_DE_MASKIRQ);
	}

	malidp_hw_event_queue_enqueue(dev->ev_queue, &event);

	spin_unlock_irqrestore(dev->hw_lock, flags);

	return IRQ_WAKE_THREAD;
}

static irqreturn_t dp500_se_irq_handler(int irq, void *data)
{
	struct malidp_se_device *dev = data;
	u32 status, irq_mask, irq_vector;
	unsigned long flags;
	struct malidp_hw_event event;

	spin_lock_irqsave(dev->hw_lock, flags);

	/* Drop any events if we already went to powersave */
	if (unlikely(dev->op_mode == MALIDP_OP_MODE_POWERSAVE)) {
		status = dev->pending_status;
		dev->pending_status = 0;
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return status & SE_ST_IRQ ? IRQ_HANDLED : IRQ_NONE;
	}

	status = malidp_se_read(dev, SE_REG_STATUS);
	malidp_se_write(dev, status, SE_REG_STATUS);

	irq_mask = malidp_se_read(dev, SE_REG_MASKIRQ);
	irq_vector = status & irq_mask;

	/* The IRQ does not belong to this device */
	if (!(status & SE_ST_IRQ)) {
		spin_unlock_irqrestore(dev->hw_lock, flags);
		return IRQ_NONE;
	}

	dev_dbg(dev->device, "%s: start\n", __func__);

	/* IRQs not enabled should not be triggered */
	BUG_ON(irq_vector & SE_IRQ_CONFIG);

	/* Get the timestamp as soon as possible for more accuracy */
	event.timestamp = ktime_get();
	event.type = MALIDP_HW_EVENT_NONE;

	if (irq_vector & SE_IRQ_OVERRUN) {
		dev_err(dev->device, "%s: overrun error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_ORUN;
	}

	if (irq_vector & SE_IRQ_AXI_ERR) {
		dev_err(dev->device, "%s: axi error\n", __func__);
		event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_AXI;
	}

	/*
	 * The init busy signal occurs in HW some cycles before the global
	 * valid, which means we can use the IRQ handler for SE_IRQ_PTR_UPDATE
	 * to check the status for SE_ST_INIT_BUSY.
	 */
	if (irq_vector & SE_IRQ_PTR_UPDATE) {
		if (status & SE_ST_INIT_BUSY &&
		    (malidp_se_read(dev, SE_REG_STATUS) & SE_ST_AXI_BUSY)) {
			dev_err(dev->device, "%s: init busy error\n",
				__func__);
			event.type |= MALIDP_HW_EVENT_ERROR | MALIDP_HW_ERROR_IBUSY;
		}

		event.type |= MALIDP_HW_EVENT_NEWCFG;
		if (!dev->scene_changing)
			event.type |= MALIDP_HW_EVENT_STOP;
	}

	/*
	 * Prog line 1 means mw interface has begun to write out. Disable the
	 * interface so that it does not write when the next frame period
	 * starts.
	 */
	if (irq_vector &  SE_IRQ_PROGLINE1) {
		struct malidp_se_mw_conf cfg = {
			.mode = MALIDP_SE_MW_DISABLE,
		};
		dev_dbg(dev->device, "%s: disable memory interface\n", __func__);
		malidp_se_cfg_processing(dev, &cfg, NULL);
		malidp_hw_cfg_de_disable_mw_flows_atomic(dev->hwdev);

		malidp_hw_commit_scene_atomic(dev->hwdev, true);

		event.type |= MALIDP_HW_EVENT_START;

		if (dev->scene_changing) {
			event.type |= MALIDP_HW_EVENT_FLIP;
			dev->scene_changing = false;
		}
	}

	/* There must always be a valid event if the IRQ is triggered */
	BUG_ON(event.type == MALIDP_HW_EVENT_NONE);

	/* Disable the offending interrupts if there was an error */
	if (event.type & MALIDP_HW_EVENT_ERROR) {
		uint32_t mask_bits = irq_vector & (SE_IRQ_OVERRUN | SE_IRQ_AXI_ERR);
		malidp_se_clearbits(dev, mask_bits, SE_REG_MASKIRQ);
	}

	/* If we're starting a new frame then re-enable error interrupts */
	if (event.type & MALIDP_HW_EVENT_FLIP) {
		uint32_t mask_bits = (SE_IRQ_OVERRUN | SE_IRQ_AXI_ERR);
		if ((mask_bits & irq_mask) != mask_bits)
			malidp_se_setbits(dev, mask_bits, SE_REG_MASKIRQ);
	}

	malidp_hw_event_queue_enqueue(dev->ev_queue, &event);

	dev_dbg(dev->device, "%s: end\n", __func__);

	spin_unlock_irqrestore(dev->hw_lock, flags);

	return IRQ_WAKE_THREAD;
}

static u32 dp500_choose_scaling_coeffset(
	enum malidp_scaling_coeff_set coeffset)
{
	u32 i = 4;

	switch (coeffset) {
	case MALIDP_UPSCALING_COEFFS:
	case MALIDP_DOWNSCALING_1_5_COEFFS:
	case MALIDP_DOWNSCALING_2_COEFFS:
	case MALIDP_DOWNSCALING_2_75_COEFFS:
	case MALIDP_DOWNSCALING_4_COEFFS:
	  i = (u32)coeffset;
		break;
	default:
		BUG();
	}
	return i;
}

static void dp500_se_writing_pp_coeffstab(
		struct malidp_se_device *dev,
		u32 orientation, u16 addr, u16 coeffs_set)
{
	int i;
	u16 scl_reg = dev->se_regmap->scaling_control;

	malidp_se_write(dev, orientation |
		((addr) & SE_COEFFTAB_ADDR_MASK),
		scl_reg + SE_REG_COEFFTAB_ADDR);
	for (i = 0; i < SE_N_SCALING_COEFFS_IN_SET; i++)
		malidp_se_write(dev, SE_SET_COEFFTAB_DATA(
				dp500_se_scaling_coeffs[coeffs_set][i]),
			 scl_reg + SE_REG_COEFFTAB_DATA);
}

/* Implementation of product APIs */
static bool dp500_de_axi_valid(u32 attr, u32 val)
{
	switch (attr) {
	case MALIDP_ATTR_DE_BURSTLEN:
		return dp500_axi_burstlen_valid(val);
	case MALIDP_ATTR_DE_OUTSTRAN:
		if ((val < 1) || (val > 32))
			return false;
		break;
	case MALIDP_ATTR_DE_POUTSTDCAB:
		if (val != 0)
			return false;
		break;
	default:
		BUG();
	}

	return true;
}

static bool dp500_se_axi_valid(u32 attr, u32 val)
{
	switch (attr) {
	case MALIDP_ATTR_SE_BURSTLEN:
		return dp500_axi_burstlen_valid(val);
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

static u32 dp500_rotmem_required(const struct malidp_hw_buffer *hw_buf)
{
	u32 bpp = malidp_hw_format_bpp(hw_buf->fmt);
	/*
	* The available rotation memory for each layer must be
	* big enough to fit 8 lines worth of rotated,
	* uncompressed, pixel data
	*
	* This memory used by a layer can be calculated
	* (in bytes) for a paritucular pixel format as:
	* size = (rotated_width * bits_per_pixel * 8) / 8
	*
	* We multiply by 8 for number of lines but
	* we also divide by 8 for converting bits to bytes.
	* These 2 cancel out meaning the final calculation is:
	* size_in_bytes = rotated_width * bits_per_pixel
	*/
	return hw_buf->cmp_rect.src_w * bpp;
}


void dp500_disable_irq(struct malidp_hw_device *hwdev)
{
	malidp_hw_write(hwdev, 0,
		hwdev->hw_regmap->se_base + SE_REG_MASKIRQ);
	malidp_hw_write(hwdev, 0,
		hwdev->hw_regmap->de_base + MALI_DP500_REG_DE_MASKIRQ);
}

enum malidp_op_mode dp500_change_op_mode(
		struct malidp_hw_device *hwdev,
		enum malidp_op_mode mode)
{
	unsigned long flags;
	enum malidp_op_mode old_mode_de, old_mode_se;

	BUG_ON(mode == MALIDP_OP_MODE_UNKNOWN);

	/* We're protected by the power mutex, so just read the mode */
	old_mode_de = malidp_de_get_op_mode(hwdev->de_dev);
	if (old_mode_de == mode)
		return old_mode_de;

	spin_lock_irqsave(&hwdev->hw_lock, flags);

	/* if old mode is unknown, device should be soft-reset */
	if (old_mode_de == MALIDP_OP_MODE_UNKNOWN) {
		malidp_se_setbits(hwdev->se_dev, SE_SOFTRESET_REQ,
					malidp500_regmap.se_regmap.control);
		malidp_de_setbits(hwdev->de_dev, MALI_DP500_DE_SOFTRESET_REQ_EN,
					MALI_DP500_REG_DE_CONTROL);
	}


	/*
	 * Changing the SE first is always the safest option, but the mode
	 * change will not take effect until the DE exits powersave mode
	 */
	old_mode_se = dp500_se_op_change(hwdev->se_dev, mode);
	dp500_de_op_change(hwdev->de_dev, mode);

	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	BUG_ON(old_mode_de != old_mode_se);

	return old_mode_de;
}

static int dp500_de_hw_cfg(struct malidp_de_device *de_dev)
{
	extern const u32 malidp_de_bt709_degamma_coeffs[];

	/* Set AXI configuration */
	if (!malidp_de_attr_valid(de_dev, MALIDP_ATTR_DE_OUTSTRAN,
				de_dev->outstran)) {
		dev_warn(de_dev->device, "%s : invalid value '%d' for de_axi_outstran\n",
			__func__, de_dev->outstran);
		de_dev->outstran = DE_DEFAULT_AXI_OUTSTRAN;
	}
	if (!malidp_de_attr_valid(de_dev, MALIDP_ATTR_DE_BURSTLEN,
			de_dev->burstlen)) {
		dev_warn(de_dev->device, "%s : invalid value '%d' for de_axi_burstlen\n",
			__func__, de_dev->burstlen);
		de_dev->burstlen = DE_DEFAULT_AXI_BURSTLEN;
	}
	malidp_de_set_axi_cfg(de_dev, de_dev->outstran, 0, de_dev->burstlen);

	/* Initialize the ARQOS settings */
	malidp_de_init_axi_qos(de_dev,
			de_dev->arqos_threshold_low,
			de_dev->arqos_threshold_high,
			de_dev->arqos_red,
			de_dev->arqos_green);


	/* Set default background */
	malidp_de_write(de_dev, MALI_DP500_DE_BG_COLOR_R(DE_FIXED_BG_R) |
			MALI_DP500_DE_BG_COLOR_G(DE_FIXED_BG_G),
			MALI_DP500_REG_DE_BG_COLOR_RG);
	malidp_de_write(de_dev, MALI_DP500_DE_BG_COLOR_B(DE_FIXED_BG_B),
			MALI_DP500_REG_DE_BG_COLOR_B);

	/* Set prefetch_line to default value */
	malidp_de_clearbits(de_dev,
		MALI_DP500_DE_PREFETCH_LINE_MASK << MALI_DP500_DE_PREFETCH_LINE_SHIFT,
		MALI_DP500_REG_DE_CONTROL);
	malidp_de_setbits(de_dev,
		MALI_DP500_DE_PREFETCH_LINE_SET(DE_DEFAULT_PREFETCH_LINE),
		MALI_DP500_REG_DE_CONTROL);

	/*
	 * We are always interested on getting an IRQ as soon as a frame begins
	 * to be scanned out.
	 */
	malidp_de_write(de_dev, DE_LINE_INT_1(DE_DEFAULT_PREFETCH_LINE),
			MALI_DP500_REG_DE_LINE_INT_CTRL);
	malidp_de_write(de_dev, MALI_DP500_DE_IRQ_UNDERRUN |
			MALI_DP500_DE_IRQ_AXI_ERR |
			MALI_DP500_DE_IRQ_PROG_LINE1 |
			MALI_DP500_DE_IRQ_ENABLE, MALI_DP500_REG_DE_MASKIRQ);

	/* Write alpha lookup tables */
	malidp_de_write_alpha_lookup(de_dev);

	/* Set the Video layer inverse-gamma coefficients */
	malidp_de_set_coeftab(de_dev, MALI_DP500_DE_COEFTAB_LV_DEGAMMA,
			malidp_de_bt709_degamma_coeffs);

	/* Always enable dithering */
	malidp_de_setbits(de_dev, DE_DITH_EN, MALI_DP500_REG_DE_DISP_FUNC);

	/* Disable all the layers so we don't scan out any stale config */
	malidp_de_disable_all_layers(de_dev);

	malidp_de_cleanup_yuv2rgb_coeffs(de_dev);

	dev_dbg(de_dev->device, "%s : success!\n", __func__);
	return 0;
}

static void dp500_de_modeset(struct malidp_de_device *de_dev,
		struct malidp_de_hwmode *hwmode)
{
	malidp_de_write(de_dev, DE_H_FRONTPORCH(hwmode->hfp) |
			DE_H_BACKPORCH(hwmode->hbp),
			MALI_DP500_REG_DE_H_INTERVALS);
	malidp_de_write(de_dev, DE_V_FRONTPORCH(hwmode->vfp) |
			DE_V_BACKPORCH(hwmode->vbp),
			MALI_DP500_REG_DE_V_INTERVALS);
	malidp_de_write(de_dev, DE_H_SYNCWIDTH(hwmode->hsw) |
			DE_V_SYNCWIDTH(hwmode->vsw),
			MALI_DP500_REG_DE_SYNCWIDTH);
	malidp_de_write(de_dev, DE_H_ACTIVE(hwmode->h_active) |
			DE_V_ACTIVE(hwmode->v_active),
			MALI_DP500_REG_DE_HV_ACTIVESIZE);

	if (hwmode->hsync_pol_pos)
		malidp_de_setbits(de_dev, MALI_DP500_DE_HSYNC_POL_POS,
					MALI_DP500_REG_DE_CONTROL);
	else
		malidp_de_clearbits(de_dev, MALI_DP500_DE_HSYNC_POL_POS,
					MALI_DP500_REG_DE_CONTROL);

	if (hwmode->vsync_pol_pos)
		malidp_de_setbits(de_dev, MALI_DP500_DE_VSYNC_POL_POS,
					MALI_DP500_REG_DE_CONTROL);
	else
		malidp_de_clearbits(de_dev, MALI_DP500_DE_VSYNC_POL_POS,
					MALI_DP500_REG_DE_CONTROL);
}

static u32 dp500_de_fmt_fixup(u32 drm_format, u32 flags)
{
	if (flags & MALIDP_FLAG_AFBC) {
		switch (drm_format) {
		case MALIDP_FORMAT_NV12AFBC:
			/*
			 * This format is only used for buffer dimension
			 * validation, and needs to be transformed to the
			 * generic version to be written to the HW
			 */
			drm_format = DRM_FORMAT_NV12;
			break;
		}
	}
	return drm_format;
}

static int dp500_se_hw_cfg(struct malidp_se_device *se_dev)
{
	u32 status;

	/*
	 * Make sure we're in CFM first. The HW layer should make sure the
	 * DE is not in PSM for this to succeed
	 */
	do {
		status = malidp_se_read(se_dev, SE_REG_STATUS);
	} while (!(status & SE_ST_CONFIG_ACTIVE));

	/* Set AXI configuration */
	if (!malidp_se_attr_valid(se_dev, MALIDP_ATTR_SE_OUTSTRAN,
				  se_dev->outstran)) {
		dev_warn(se_dev->device, "%s : invalid value '%d' for se_axi_outstran\n",
			 __func__, se_dev->outstran);
		se_dev->outstran = SE_DEFAULT_AXI_OUTSTRAN;
	}
	if (!malidp_se_attr_valid(se_dev, MALIDP_ATTR_SE_BURSTLEN,
				  se_dev->burstlen)) {
		dev_warn(se_dev->device, "%s : invalid value '%d' for se_axi_burstlen\n",
			 __func__, se_dev->burstlen);
		se_dev->burstlen = SE_DEFAULT_AXI_BURSTLEN;
	}
	if (!malidp_se_attr_valid(se_dev, MALIDP_ATTR_SE_WCACHE,
				  se_dev->wcache)) {
		dev_warn(se_dev->device, "%s : invalid value '%d' for se_axi_awcache\n",
			 __func__, se_dev->wcache);
		se_dev->wcache = SE_DEFAULT_AXI_AWCACHE;
	}
	if (!malidp_se_attr_valid(se_dev, MALIDP_ATTR_SE_WQOS,
				  se_dev->wqos)) {
		dev_warn(se_dev->device, "%s : invalid value '%d' for se_axi_awqos\n",
			 __func__, se_dev->wqos);
		se_dev->wqos = SE_DEFAULT_AXI_AWQOS;
	}
	malidp_se_set_axi_cfg(se_dev, se_dev->outstran,
			se_dev->burstlen,
			se_dev->wcache,
			se_dev->wqos);

	/* Set initial image enhancer state */
	se_dev->enh_cfg = MALIDP_SE_ENHANCER_OFF;
	malidp_se_clearbits(se_dev, SE_ENH_H_EN | SE_ENH_V_EN,
			MALI_DP500_REG_SE_CTL);
	malidp_se_write(se_dev, SE_SET_ENH_LIMIT_LOW(SE_ENH_LOW_LEVEL) |
			SE_SET_ENH_LIMIT_HIGH(SE_ENH_HIGH_LEVEL),
			malidp500_regmap.se_regmap.enhancer_control);

	/*
	 * We get an IRQ as soon as a frame starts to be written to memory and
	 * when config valid is latched by the init signal.
	 */
	malidp_se_write(se_dev, SE_LINE_INT_1(0), SE_REG_LINE_INT_CTRL);
	malidp_se_write(se_dev, SE_IRQ_OVERRUN | SE_IRQ_AXI_ERR |
			SE_IRQ_PROGLINE1 | SE_IRQ_ENABLE | SE_IRQ_PTR_UPDATE,
			SE_REG_MASKIRQ);

	se_dev->rgb2yuv_coeffs = NULL;
	se_dev->v_coeffstab = 0xffff;
	se_dev->h_coeffstab = 0xffff;

	dev_dbg(se_dev->device, "%s: success!\n", __func__);
	return 0;
}

static void dp500_de_set_gamma_coeff(struct malidp_de_device *de_dev, u32 *coeffs)
{
	malidp_de_set_coeftab(de_dev, MALI_DP500_DE_COEFTAB_GAMMA,
				coeffs);
}

static void dp500_se_set_scaler_coeff(struct malidp_se_device *se_dev,
		enum malidp_scaling_coeff_set hcoeff,
		enum malidp_scaling_coeff_set vcoeff)
{
	u32 h = dp500_choose_scaling_coeffset(hcoeff);
	u32 v = dp500_choose_scaling_coeffset(vcoeff);

	if ((h == v) && (hcoeff != se_dev->h_coeffstab ||
		vcoeff != se_dev->v_coeffstab)) {
		se_dev->h_coeffstab = hcoeff;
		se_dev->v_coeffstab = vcoeff;

		dp500_se_writing_pp_coeffstab(se_dev,
			(SE_V_COEFFTAB | SE_H_COEFFTAB), 0, v);
	} else {
		if (se_dev->v_coeffstab != vcoeff) {
			se_dev->v_coeffstab = vcoeff;

			dp500_se_writing_pp_coeffstab(se_dev,
				SE_V_COEFFTAB, 0, v);
		}

		if (se_dev->h_coeffstab != hcoeff) {
			se_dev->h_coeffstab = hcoeff;

			dp500_se_writing_pp_coeffstab(se_dev,
				SE_H_COEFFTAB, 0, h);
		}
	}
}

static bool dp500_se_limitation_check(struct malidp_se_device *se_dev,
		struct malidp_se_scaler_conf *s0)
{
	bool ret = true;
	struct malidp_hw_device *hwdev = se_dev->hwdev;
	struct drm_mode_modeinfo mode;
	unsigned long flags;
	unsigned long mclk =
		clk_get_rate(hwdev->mclk) / 100; /* 10KHz*/
	unsigned long pxclk =
		clk_get_rate(hwdev->pxclk) / 1000; /* KHz*/
	unsigned long input_size = s0->input_w * s0->input_h;
	unsigned long a;

	spin_lock_irqsave(&hwdev->hw_lock, flags);
	malidp_de_modeget(hwdev->de_dev, &mode);
	spin_unlock_irqrestore(&hwdev->hw_lock, flags);

	/*
	 * Equation:
	 *	a = 1.5*(h_input_size*v_input_size) /
	 *		(h_total_size*v_output_size)
	 *
	 *	mclk = a*pxclk if a >= 1.5
	 *			or
	 *	mclk = 1.5*pxclk if a < 1.5
	 *
	 *	To avoid float calculaiton, using 15
	 *	instead of 1.5, and 10Khz is
	 *	measurement unit of mclk.
	*/
	a = 15 * input_size /
		(mode.htotal * s0->output_h);
	if (a < 15)
		a = 15;
	if (mclk < a * pxclk) {
		dev_err(hwdev->device,
			"%s: Clock ratio (mclk/pxclk) is not high enough for downscale factor.",
			__func__);
		ret = false;
	}

	return ret;
}

static u32 dp500_se_calc_downscaling_threshold(u32 mclk, u32 pxlclk,
		struct drm_mode_modeinfo *mode)
{
	u64 mclk64 = mclk;

	/* if mclk/pxlclk < 1.5, returns 0 */
	mclk64 = mclk64 * 2;
	pxlclk = pxlclk * 3;
	if (mclk64 < pxlclk) {
		return 0;
	}

	mclk64 *= mode->htotal;

	/* factor is (16.16) fix point */
	mclk64 <<= 16;
	do_div(mclk64, pxlclk);

	return mclk64;
}

