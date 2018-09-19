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



#ifndef _MALIDP_DE_DEVICE_H_
#define _MALIDP_DE_DEVICE_H_

#include <linux/interrupt.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <uapi/drm/drm_mode.h>
#include "malidp_hw_types.h"
#include "malidp_sysfs.h"

#define DE_H_FRONTPORCH(x)	(((x) & 0xfff) << 0)
#define DE_H_BACKPORCH(x)	(((x) & 0x3ff) << 16)
#define DE_V_FRONTPORCH(x)	(((x) & 0xff) << 0)
#define DE_V_BACKPORCH(x)	(((x) & 0xff) << 16)
#define DE_H_SYNCWIDTH(x)	(((x) & 0x3ff) << 0)
#define DE_V_SYNCWIDTH(x)	(((x) & 0xff) << 16)
#define DE_H_ACTIVE(x)	(((x) & 0x1fff) << 0)
#define DE_V_ACTIVE(x)	(((x) & 0x1fff) << 16)

#define DE_LINE_INT_1(x)	((x) & 0x3fff)
#define DE_LINE_INT_2(x)	(((x) & 0x3fff) << 16)


#define	DE_AXI_OUTSTDCAPB_MASK	0xff
#define	DE_AXI_OUTSTDCAPB(x)	(((x) & DE_AXI_OUTSTDCAPB_MASK) << 0)
#define	DE_AXI_POUTSTDCAB_MASK	0x3f
#define	DE_AXI_POUTSTDCAB(x)	(((x) & DE_AXI_POUTSTDCAB_MASK) << 8)
#define	DE_AXI_BURSTLEN_MASK	0xff
#define	DE_AXI_BURSTLEN(x)	(((x) & DE_AXI_BURSTLEN_MASK) << 16)

#define DE_SET_FLOWCFG_MASK	0x3
#define DE_SET_FLOWCFG(x)	(((x) & DE_SET_FLOWCFG_MASK) << 16)
#define DE_GAMMA_EN		0x1
#define DE_CADJ_EN		(1 << 4)
#define DE_DITH_EN		(1 << 12)

#define DE_OUT_DEPTH_MASK		0xf
#define DE_OUT_DEPTH_R_SHIFT		16
#define DE_OUT_DEPTH_R(x)		(((x) & DE_OUT_DEPTH_MASK) << DE_OUT_DEPTH_R_SHIFT)
#define DE_OUT_DEPTH_G_SHIFT		8
#define DE_OUT_DEPTH_G(x)		(((x) & DE_OUT_DEPTH_MASK) << DE_OUT_DEPTH_G_SHIFT)
#define DE_OUT_DEPTH_B_SHIFT		0
#define DE_OUT_DEPTH_B(x)		(((x) & DE_OUT_DEPTH_MASK) << DE_OUT_DEPTH_B_SHIFT)

#define DE_REG_COEFTAB_ADDR		0x30
#define		DE_COEFTAB_ADDR_MASK		0x3f
#define		DE_COEFTAB_TABLE_MASK		0x7
#define		DE_COEFTAB_ADDR_SHIFT		0
#define		DE_COEFTAB_GAMMA_SHIFT		16
#define		DE_COEFTAB_LV_DEGAMMA_SHIFT	19
#define		DE_COEFTAB_GAMMA		(DE_COEFTAB_TABLE_MASK << DE_COEFTAB_GAMMA_SHIFT)
#define		DE_COEFTAB_LV_DEGAMMA		(DE_COEFTAB_TABLE_MASK << DE_COEFTAB_LV_DEGAMMA_SHIFT)
#define		DE_COEFTAB_INTAB_ADDR(x)	(((x) & DE_COEFTAB_ADDR_MASK) << DE_COEFTAB_ADDR_SHIFT)
#define DE_REG_COEFTAB_DATA		0x34
#define		DE_COEFTAB_DATA(a, b)		((((a) & 0xfff) << 16) | (((b) & 0xfff) << 0))
#define DE_N_COEFTAB_COEFS		64
#define DE_N_COLORADJ_COEFS		12

/* Layer Common Registers */
#define DE_REG_L_FORMAT			0x0
#define		DE_L_FMT_MASK			0x3f
#define		DE_L_SET_FMT(x)			(((x) & DE_L_FMT_MASK) << 0)
#define DE_REG_L_CONTROL		0x4
#define		DE_L_EN				(1 << 0)
#define		DE_L_FCFG_MASK		0x7
#define		DE_L_FCFG(x)			(((x) & DE_L_FCFG_MASK) << 1)
#define		DE_GET_L_FCFG(x)		((x >> 1) & DE_L_FCFG_MASK)
#define		DE_L_IGEN			(1 << 4)
#define		DE_L_VALID			(1 << 5)
#define		DE_L_IGSEL_MASK			(0x3 << 6)
#define		DE_L_IGSEL_IG			(0 << 6)
#define		DE_L_IGSEL_SRGB			(1 << 6)
#define		DE_L_IGSEL_LINEAR		(2 << 6)
#define		DE_L_ROT_90			(1 << 8)
#define		DE_L_ROT_180			(2 << 8)
#define		DE_L_HFLIP			(1 << 10)
#define		DE_L_VFLIP			(1 << 11)
#define		DE_L_TRANS_MASK			(DE_L_ROT_90 | DE_L_ROT_180 | DE_L_HFLIP | DE_L_VFLIP)
#define		DE_L_SET_TRANS(x)		(((x) << 8) & DE_L_TRANS_MASK)
#define		DE_L_COMPOSE_PIXEL		(1 << 12)
#define		DE_L_COMPOSE_BG			(2 << 12)
#define		DE_L_PREMULT			(1 << 14)
#define		DE_L_ALPHA(x)			(((x) & 0xff) << 16)
#define DE_REG_L_COMPOSE		0x8
#define		DE_L_ALPHA0(x)			((x) & 0xff)
#define		DE_L_ALPHA1(x)			(((x) & 0xff) << 8)
#define		DE_L_ALPHA2(x)			(((x) & 0xff) << 16)
#define		DE_L_ALPHA3(x)			(((x) & 0xff) << 24)
#define DE_REG_L_SIZE			0xC
#define DE_REG_L_SIZE_CMP		0x10
#define		DE_L_SIZE_H(x)			(((x) & 0x1fff) << 0)
#define		DE_L_SIZE_V(x)			(((x) & 0x1fff) << 16)
#define DE_REG_L_OFFSET			0x14
#define		DE_L_OFFSET_H(x)		(((x) & 0x1fff) << 0)
#define		DE_L_OFFSET_V(x)		(((x) & 0x1fff) << 16)

/* Video Layer Specific Registers */
#define DE_REG_LV1_STRIDE		0x18
#define DE_REG_LV2_STRIDE		0x1C
#define DE_REG_LV3_STRIDE		0x20
#define DE_REG_LV1_PTR0_LOW		0x24
#define DE_REG_LV1_PTR0_HIGH		0x28
#define DE_REG_LV1_PTR1_LOW		0x2C
#define DE_REG_LV1_PTR1_HIGH		0x30
#define DE_REG_LV2_PTR0_LOW		0x34
#define DE_REG_LV2_PTR0_HIGH		0x38
#define DE_REG_LV2_PTR1_LOW		0x3C
#define DE_REG_LV2_PTR1_HIGH		0x40
#define DE_REG_LV3_PTR0_LOW		0x44
#define DE_REG_LV3_PTR0_HIGH		0x48
#define DE_REG_LV3_PTR1_LOW		0x4C
#define DE_REG_LV3_PTR1_HIGH		0x50
#define DE_REG_LV1I_PTR0_LOW		0x54
#define DE_REG_LV1I_PTR0_HIGH		0x58
#define DE_REG_LV1I_PTR1_LOW		0x5C
#define DE_REG_LV1I_PTR1_HIGH		0x60
#define DE_REG_LV2I_PTR0_LOW		0x64
#define DE_REG_LV2I_PTR0_HIGH		0x68
#define DE_REG_LV2I_PTR1_LOW		0x6C
#define DE_REG_LV2I_PTR1_HIGH		0x70
#define DE_REG_LV3I_PTR0_LOW		0x74
#define DE_REG_LV3I_PTR0_HIGH		0x78
#define DE_REG_LV3I_PTR1_LOW		0x7C
#define DE_REG_LV3I_PTR1_HIGH		0x80
#define DE_REG_LV1_PTR0_R_LOW		0x84
#define DE_REG_LV1_PTR0_R_HIGH		0x88
#define DE_REG_LV1_PTR1_R_LOW		0x8C
#define DE_REG_LV1_PTR1_R_HIGH		0x90
#define DE_REG_LV2_PTR0_R_LOW		0x94
#define DE_REG_LV2_PTR0_R_HIGH		0x98
#define DE_REG_LV2_PTR1_R_LOW		0x9C
#define DE_REG_LV2_PTR1_R_HIGH		0xA0
#define DE_REG_LV3_PTR0_R_LOW		0xA4
#define DE_REG_LV3_PTR0_R_HIGH		0xA8
#define DE_REG_LV3_PTR1_R_LOW		0xAC
#define DE_REG_LV3_PTR1_R_HIGH		0xB0

/* Graphics Layer Specific Registers */
#define DE_REG_LG_STRIDE		0x18
#define DE_REG_LG_PTR0_LOW		0x1C
#define DE_REG_LG_PTR0_HIGH		0x20
#define DE_REG_LG_PTR1_LOW		0x24
#define DE_REG_LG_PTR1_HIGH		0x28

/* Smart Layer Specific Registers */
#define DE_REG_LS_BBOX_ARGB		0x18
#define DE_REG_LS_ENABLE		0x1C
#define		DE_LS_EN_NUM(x)		(((x) & 0x7) << 0)
#define DE_REG_LS_R1_IN_SIZE		0x20
#define DE_REG_LS_R1_OFFSET		0x24
#define DE_REG_LS_R1_STRIDE		0x28
#define DE_REG_LS_R1_PTR_LOW		0x2C
#define DE_REG_LS_R1_PTR_HIGH		0x30

#define DE_REG_LS_R2_IN_SIZE		0x34

#define DE_REG_LS_Rn_ADDR_DELTA	(DE_REG_LS_R2_IN_SIZE - DE_REG_LS_R1_IN_SIZE)

/* AFBC Decoder Registers */
#define DE_AD_EN			(1 << 0)
#define DE_AD_YTR			(1 << 4)
#define DE_AD_BS			(1 << 8)
#define DE_AD_CROP_LEFT(x)		(((x) & 0x1FFF) << 0)
#define DE_AD_CROP_RIGHT(x)		(((x) & 0x1FFF) << 16)
#define DE_AD_CROP_TOP(x)		(((x) & 0x1FFF) << 0)
#define DE_AD_CROP_BOTTOM(x)		(((x) & 0x1FFF) << 16)

/* RQOS Register */
#define		DE_RQOS_LOW(x)		(((x) & 0xFFF) << 0)
#define		DE_RQOS_RED(x)		(((x) & 0xF) << 12)
#define		DE_RQOS_HIGH(x)		(((x) & 0xFFF) << 16)
#define		DE_RQOS_GREEN(x)	(((x) & 0xF) << 28)

#define DE_DEFAULT_AXI_OUTSTRAN		16
#define DE_DEFAULT_AXI_POUTSTDCAB	16
#define DE_DEFAULT_AXI_BURSTLEN		16
#define DE_DEFAULT_AXI_ARQOS_LOW	112
#define DE_DEFAULT_AXI_ARQOS_HIGH	128
#define DE_DEFAULT_AXI_ARQOS_RED	1
#define DE_DEFAULT_AXI_ARQOS_GREEN	0

/* Background is always set to black color */
#define DE_FIXED_BG_R   0
#define DE_FIXED_BG_G   0
#define DE_FIXED_BG_B   0

#define DE_DEFAULT_PREFETCH_LINE	5

enum malidp_de_flow_cmp_cfg {
	MALIDP_DE_CMP_FLOW_INTERNAL,
	MALIDP_DE_CMP_FLOW_SE0,
	/* Other configurations are currently reserved */
};

enum malidp_de_flow_layer_cfg {
	MALIDP_DE_LAYER_FLOW_LOCAL = 0,
	MALIDP_DE_LAYER_FLOW_SIMULT_SE0 = 1,
	MALIDP_DE_LAYER_FLOW_SCALE_SE0 = 3,
	/* Other values are currently reserved */
};

/*
 * This structure contains all the relevant HW parameters we need to change
 * the mode in the DE:
 * - clock: frequency of the pixclk in Hz
 * Horizontal sync parameters:
 * - h_active: size of the horizontal active area in pixels
 * - hbp: horizontal backporch in pixels
 * - hfp: horizontal front porch in pixels
 * - hsw: horizontal sync pulse width
 * - hsync_pol_pos: horizontal sync polarity (true=positive, false=negative)
 * Vertical sync parameters:
 * - v_active: size of the vertical active area in pixels
 * - vbp: vertical backporch in pixels
 * - vfp: vertical front porch in pixels
 * - vsw: vertical sync pulse width
 * - vsync_pol_pos: vertical sync polarity (true=positive, false=negative)
 */
struct malidp_de_hwmode {
	u32 clock;
	u32 h_active, hbp, hfp, hsw;
	bool hsync_pol_pos;
	u32 v_active, vbp, vfp, vsw;
	bool vsync_pol_pos;
};

struct malidp_de_device {
	void __iomem *regs;
	enum malidp_op_mode op_mode;
	struct drm_mode_modeinfo current_mode;
	struct device *device;
	struct malidp_hw_device *hwdev;
	void (*flip_callback)(struct device *, void *, struct malidp_hw_event_queue *);
	void *callback_opaque;
	const s32 *yuv2rgb_coeffs[MALIDP_MAX_LAYERS];
	/*
	 * Used to indicate that the next PTR_UPDATE IRQ is due to a
	 * content change as opposed to a mode change or disabling the memory
	 * interface. Access must be protected by the hwdev hw_lock.
	 */
	bool scene_changing;
	struct malidp_hw_event_queue *ev_queue;
	/*
	 * This spinlock protects accesses to registers and clocks.
	 * Also protects the shared variables in this structure:
	 * "event", "op_mode" and "flip_callback".
	 */
	spinlock_t *hw_lock;

	/*
	 * Gamma-related settings
	 */
	bool gamma_enabled;
	u32 gamma_coeffs[DE_N_COEFTAB_COEFS];

	/* Color adjustment settings */
	u16 color_adjustment_coeffs[DE_N_COLORADJ_COEFS];

	/* Output depth for dithering */
	u8 red_bits, green_bits, blue_bits;

	/* Attributes accessible through sysfs */
	u16 burstlen;
	u8 outstran;
	u8 poutstdcab;

	u16 arqos_threshold_low;
	u16 arqos_threshold_high;
	u8 arqos_red;
	u8 arqos_green;

	const struct malidp_de_regmap *de_regmap;
	/* Stored after entering PSM to handle any residual IRQs */
	u32 pending_status;
};

int malidp_dt_parse_de(struct platform_device *pdev,
			       struct device_node *nproot,
			       struct malidp_hw_pdata *pdata);

int malidp_de_fmt_drm2hw(struct malidp_de_device *dev,
	struct malidp_hw_buffer *buf);

struct malidp_de_device *malidp_de_hw_init(struct malidp_hw_device *hwdev,
			     struct platform_device *pdev,
			     struct malidp_hw_pdata *pdata,
			     spinlock_t *hw_lock);

void malidp_de_set_axi_cfg(struct malidp_de_device *dev, u32 outstran,
				u32 poutstdcab, u32 burstlen);
bool malidp_de_attr_valid(struct malidp_de_device *dev,
		u32 attr, u32 val);
void malidp_de_init_axi_qos(struct malidp_de_device *dev,
		u32 low, u32 high, u32 red_code, u32 green_code);
void malidp_de_write_alpha_lookup(struct malidp_de_device *dev);
void malidp_de_hw_exit(struct malidp_de_device *dev);

void malidp_de_write(struct malidp_de_device *dev,
				u32 value, u32 reg);
u32 malidp_de_read(struct malidp_de_device *dev, u32 reg);
void malidp_de_setbits(struct malidp_de_device *dev, u32 mask,
		u32 reg);
void malidp_de_clearbits(struct malidp_de_device *dev,
		u32 mask, u32 reg);
irqreturn_t malidp_de_irq_thread_handler(int irq, void *data);

int malidp_de_modeset(struct malidp_de_device *dev,
		struct drm_mode_modeinfo *mode);

void malidp_de_set_coeftab(struct malidp_de_device *dev,
	u32 table, const u32 *coeffs);
/*
 * The following functions need to be called while holding the HW spinlock
 * unless they are used at initialization or exit time.
 *
 */
void malidp_de_cfg_cmp_flow(struct malidp_de_device *dev,
			enum malidp_de_flow_cmp_cfg);

void malidp_de_cfg_layer_flow(struct malidp_de_device *dev,
			const struct malidp_layer_hw_info *hw_layer,
			enum malidp_de_flow_layer_cfg cfg);

enum malidp_de_flow_layer_cfg malidp_de_get_layer_flow(struct malidp_de_device *dev,
			const struct malidp_layer_hw_info *hw_layer);

void malidp_de_cfg_smart_state(struct malidp_de_device *dev,
			const struct malidp_hw_smart_layer_state *ls_state);

int malidp_de_cfg_layer(struct malidp_de_device *dev,
			struct malidp_hw_buffer *buf);

void malidp_de_disable_all_layers(struct malidp_de_device *dev);

void malidp_de_set_flip_callback(struct malidp_de_device *dev,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque);

void malidp_de_modeget(struct malidp_de_device *dev,
		       struct drm_mode_modeinfo *mode);

enum malidp_op_mode malidp_de_get_op_mode(struct malidp_de_device *dev);

int malidp_de_get_attr(struct malidp_de_device *dev, u32 attr, u32 *val);
int malidp_de_set_attr(struct malidp_de_device *dev, u32 attr, u32 val);
int malidp_de_save_attr(struct malidp_de_device *dev, u32 attr, u32 val);

void malidp_de_update_gamma_settings(struct malidp_de_device *dev,
			bool enable, u32 *coeffs);
void malidp_de_store_output_depth(struct malidp_de_device *dev,
		u8 red, u8 green, u8 blue);

int malidp_de_update_cadj_coeffs(struct malidp_de_device *dev,
	u16 red_x, u16 red_y, u16 green_x, u16 green_y,
	u16 blue_x, u16 blue_y, u16 white_x, u16 white_y);

void malidp_de_debugfs_init(struct malidp_de_device *dev,
		struct dentry *folder);

void malidp_de_cleanup_yuv2rgb_coeffs(struct malidp_de_device *dev);
#endif /* _MALIDP_DE_DEVICE_H_ */
