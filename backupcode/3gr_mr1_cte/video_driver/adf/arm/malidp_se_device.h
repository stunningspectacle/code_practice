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



#ifndef _MALIDP_SE_DEVICE_H_
#define _MALIDP_SE_DEVICE_H_

#include "malidp_hw_types.h"
#include "malidp_hw.h"
#include <linux/interrupt.h>

#define SE_REG_STATUS			0x000
#define		SE_ST_CONFIG		(1 << 0)
#define		SE_ST_PTR_UPDATE	(1 << 4)
#define		SE_ST_INIT_BUSY		(1 << 5)
#define		SE_ST_AXIERR		(1 << 8)
#define		SE_ST_OVERRUN		(1 << 9)
#define		SE_ST_PROGLINE1		(1 << 12)
#define		SE_ST_PROGLINE2		(1 << 13)
#define		SE_ST_CONFIG_ACTIVE	(1 << 17)
#define		SE_ST_PSM_ACTIVE	(1 << 18)
#define		SE_ST_AXI_BUSY		(1 << 28)
#define		SE_ST_IRQ		(1 << 31)
#define SE_REG_SETIRQ			0x004
#define SE_REG_MASKIRQ			0x008
#define		SE_IRQ_CONFIG			(1 << 0)
#define		SE_IRQ_PTR_UPDATE		(1 << 4)
#define		SE_IRQ_INIT_BUSY		(1 << 5)
#define		SE_IRQ_AXI_ERR			(1 << 8)
#define		SE_IRQ_OVERRUN			(1 << 9)
#define		SE_IRQ_PROGLINE1		(1 << 12)
#define		SE_IRQ_PROGLINE2		(1 << 13)
#define		SE_IRQ_ENABLE			(1 << 31)

/* Control register bits and mask */
#define		SE_SCALING_EN			(1 << 0)
#define		SE_ALPHA_EN			(1 << 1)
#define		SE_ENH_H_EN			(1 << 2)
#define		SE_ENH_V_EN			(1 << 3)
#define		SE_RGBO_IF_EN			(1 << 4)
#define		SE_MW_IF_MASK			0x3
#define		SE_MW_IF_SET(x)			(((x) & SE_MW_IF_MASK) << 5)
#define		SE_RGB_MTH_SEL			(1 << 8)
#define		SE_ALPHA_MTH_SEL		(1 << 9)
#define		SE_ARGB_MTH_SET(x)		((x << 8) & (SE_RGB_MTH_SEL | SE_ALPHA_MTH_SEL))
#define		SE_PTR_VALID			(1 << 12)
#define		SE_SOFTRESET_REQ		(1 << 16)
#define		SE_CONFIG_REQ			(1 << 17)
#define		SE_MMU_PROT			(1 << 25)
#define		SE_ENDIAN			(1 << 28)

#define SE_REG_LINE_INT_CTRL		0x010
#define		SE_LINE_INT_MASK		0x3fff
#define		SE_LINE_INT_1(x)		(((x) & SE_LINE_INT_MASK) << 0)
#define		SE_LINE_INT_2(x)		(((x) & SE_LINE_INT_MASK) << 16)

/* AXI register bits and mask */
#define		SE_AXI_OUTSTDCAPB_MASK		0xff
#define		SE_AXI_OUTSTDCAPB(x)		(((x) & SE_AXI_OUTSTDCAPB_MASK) << 0)
#define		SE_AXI_WCACHE_MASK		0xf
#define		SE_AXI_WCACHE(x)		(((x) & SE_AXI_WCACHE_MASK) << 8)
#define		SE_AXI_BURSTLEN_MASK		0xff
#define		SE_AXI_BURSTLEN(x)		(((x) & SE_AXI_BURSTLEN_MASK) << 16)
#define		SE_AXI_WQOS_MASK		0xf
#define		SE_AXI_WQOS(x)			(((x) & SE_AXI_WQOS_MASK) << 28)

#define SE_REG_SECURE_CTRL		0x01c

/* Layer control register offset */
#define SE_REG_L0_IN_SIZE		0x00
#define SE_REG_L0_OUT_SIZE		0x04
#define SE_REG_L0_3DSTRUCT		0x08
#define SE_REG_L1_SIZE			0x0C
#define		SE_SET_V_SIZE(x)		(((x) & 0x1fff) << 16)
#define		SE_SET_H_SIZE(x)		(((x) & 0x1fff) << 0)

/* Scaling control registers offset */
#define SE_REG_H_INIT_PH		0x00
#define SE_REG_H_DELTA_PH		0x04
#define SE_REG_V_INIT_PH		0x08
#define SE_REG_V_DELTA_PH		0x0c
#define SE_REG_COEFFTAB_ADDR	0x10
#define		SE_COEFFTAB_ADDR_MASK	0x7f
#define		SE_V_COEFFTAB		(1 << 8)
#define		SE_H_COEFFTAB		(1 << 9)
#define		SE_SET_V_COEFFTAB_ADDR(x) \
				(SE_V_COEFFTAB | ((x) & SE_COEFFTAB_ADDR_MASK))
#define		SE_SET_H_COEFFTAB_ADDR(x) \
				(SE_H_COEFFTAB | ((x) & SE_COEFFTAB_ADDR_MASK))
#define SE_REG_COEFFTAB_DATA	0x14
#define		SE_COEFFTAB_DATA_MASK	0x3fff
#define		SE_SET_COEFFTAB_DATA(x)	((x) & SE_COEFFTAB_DATA_MASK)

/* Enhance coeffents reigster offset */
#define	SE_REG_ENH_COEFF1	0x04
#define		SE_ENH_LIMIT_MASK		0xfff
#define		SE_ENH_LIMIT_LOW_SHIFT		0
#define		SE_ENH_LIMIT_HIGH_SHIFT		16
#define		SE_SET_ENH_LIMIT_LOW(x)		(((x) & SE_ENH_LIMIT_MASK) << SE_ENH_LIMIT_LOW_SHIFT)
#define		SE_SET_ENH_LIMIT_HIGH(x)	(((x) & SE_ENH_LIMIT_MASK) << SE_ENH_LIMIT_HIGH_SHIFT)
#define		SE_SET_ENH_COEFF(x)		(((x) & 0x7ff) << 0)

#define SE_ENH_LOW_LEVEL 24
#define SE_ENH_HIGH_LEVEL 63

/* Color space conversion register offset */
#define SE_REG_CONV_COEFF1	0x04

#define SE_REG_CONV_GAMMA_ADDR		0x0A4
#define SE_REG_CONV_GAMMA_DATA		0x0A8

/* Memory write out */
#define SE_REG_FORMAT		0x00
#define		SE_WFORMAT_MASK		0x3F
#define		SE_SET_WFORMAT(x)	(((x) & SE_WFORMAT_MASK) << 0)
#define SE_REG_WP1_STRIDE	0x04
#define SE_REG_WP2_STRIDE	0x08
#define SE_REG_WP1_PTR0_LOW	0x0C
#define SE_REG_WP1_PTR0_HIGH	0x10
#define SE_REG_WP1_PTR1_LOW		0x14
#define SE_REG_WP1_PTR1_HIGH	0x18
#define SE_REG_WP1_PTR0_R_LOW	0x1C
#define SE_REG_WP1_PTR0_R_HIGH	0x20
#define SE_REG_WP1_PTR1_R_LOW	0x24
#define SE_REG_WP1_PTR1_R_HIGH	0x28
#define SE_REG_WP2_PTR0_LOW		0x2C
#define SE_REG_WP2_PTR0_HIGH	0x30
#define SE_REG_WP2_PTR1_LOW		0x34
#define SE_REG_WP2_PTR1_HIGH	0x38
#define SE_REG_WP2_PTR0_R_LOW	0x3C
#define SE_REG_WP2_PTR0_R_HIGH	0x40
#define SE_REG_WP2_PTR1_R_LOW	0x44
#define SE_REG_WP2_PTR1_R_HIGH	0x48

#define SE_DEFAULT_AXI_BURSTLEN	16
#define SE_DEFAULT_AXI_OUTSTRAN	16
#define SE_DEFAULT_AXI_AWCACHE	0x0 /* Device non-bufferable */
#define SE_DEFAULT_AXI_AWQOS	0x0 /* Not performing any QoS scheme */


enum malidp_se_mw_mode {
	MALIDP_SE_MW_DISABLE = 0,
	MALIDP_SE_MW_L0,
	MALIDP_SE_MW_L1,
};

enum malidp_se_scaling_algorithms {
	MALIDP_SE_ARGB_PP = 0,		/* PolyPhase algorithm for both Alpha and RGB */
	MALIDP_SE_RGB_NN = 1,		/* Nearest Neighbor algorithm for RGB, and PolyPhase for Alpha */
	MALIDP_SE_A_PP = 1,			/* Same as MALIDP_SE_RGB_NN */
	MALIDP_SE_RGB_PP = 2,		/* PolyPhase algorithm for RGB, and Nearest Neighbor algorithm for Alpha */
	MALIDP_SE_A_NN = 2,			/* Same as MALIDP_SE_RGB_PP */
	MALIDP_SE_ARGB_NN = 3,		/* Nearest Neighbor algorithm for both Alpha and RGB */
};

enum malidp_se_enhancer_cfg {
	MALIDP_SE_ENHANCER_HORZ = 0,
	MALIDP_SE_ENHANCER_VERT = 1,
	MALIDP_SE_ENHANCER_BOTH = 2,
	MALIDP_SE_ENHANCER_OFF  = 3,
};

enum malidp_scaling_coeff_set {
	/* For upscaling */
	MALIDP_UPSCALING_COEFFS = 0,
	/* For noscaling or 1.5x downscaling */
	MALIDP_DOWNSCALING_1_5_COEFFS = 1,
	/* For 1.5x to 2x downscaling */
	MALIDP_DOWNSCALING_2_COEFFS = 2,
	/* For 2x to 2.75x downscaling */
	MALIDP_DOWNSCALING_2_75_COEFFS = 3,
	/* For 2.75x to 4x downscaling */
	MALIDP_DOWNSCALING_4_COEFFS = 4,
};

struct malidp_se_mw_conf {
	enum malidp_se_mw_mode mode;
	struct malidp_hw_buffer *buf;
};

struct malidp_se_scaler_conf {
	bool rgbo_enable;
	bool scaling_enable;
	u16 input_w, input_h;
	u16 output_w, output_h;
	enum malidp_scaling_coeff_set v_coeffs_set;
	enum malidp_scaling_coeff_set h_coeffs_set;
	enum malidp_se_scaling_algorithms al;
	u32 v_init_phase, v_delta_phase;
	u32 h_init_phase, h_delta_phase;
	bool scale_alpha;
	enum malidp_se_enhancer_cfg enh_cfg;
};

struct malidp_se_device {
	void __iomem *regs;
	enum malidp_op_mode op_mode;
	struct device *device;
	struct malidp_hw_device *hwdev;
	void (*flip_callback)(struct device *, void *, struct malidp_hw_event_queue *);
	void *callback_opaque;
	const s32 *rgb2yuv_coeffs;
	u16 v_coeffstab, h_coeffstab;
	enum malidp_se_enhancer_cfg enh_cfg;
	/*
	 * Used to indicate that the next LINE1 IRQ is due to a
	 * content change as opposed to re-writing the previous
	 * scene to memory again.
	 */
	bool scene_changing;
	struct malidp_hw_event_queue *ev_queue;
	/*
	 * This spinlock protects accesses to registers and clocks.
	 * Also protects the shared variables in this structure:
	 * "event".
	 */
	spinlock_t *hw_lock;

	/* Attributes accessible through sysfs */
	u16 burstlen;
	u8 outstran;
	u8 wqos;
	u8 wcache;

	const struct malidp_se_regmap *se_regmap;
	/* Stored after entering PSM to handle any residual IRQs */
	u32 pending_status;

	u32 n_mw_formats;
	const u32 *mw_formats;
	/*
	 * The HW id of the format at the same index in the supported_format
	 * list
	 */
	const u32 *mw_format_ids;
};

struct malidp_se_device *malidp_se_hw_init(struct malidp_hw_device *hwdev,
			     struct platform_device *pdev,
			     struct malidp_hw_pdata *pdata,
			     spinlock_t *hw_lock);

int malidp_se_hw_cfg(struct malidp_se_device *dev,
		struct malidp_hw_pdata *pdata);

void malidp_se_hw_exit(struct malidp_se_device *dev);

int malidp_dt_parse_se(struct platform_device *pdev,
			       struct device_node *nproot,
			       struct malidp_hw_pdata *pdata);

int malidp_se_fmt_drm2mw(struct malidp_se_device *dev, u32 drm_fmt);

void malidp_se_write(struct malidp_se_device *dev,
				u32 value, u32 reg);
u32 malidp_se_read(struct malidp_se_device *dev, u32 reg);
void malidp_se_setbits(struct malidp_se_device *dev, u32 mask,
				u32 reg);
void malidp_se_clearbits(struct malidp_se_device *dev,
			u32 mask, u32 reg);
irqreturn_t malidp_se_irq_thread_handler(int irq, void *data);
bool malidp_se_attr_valid(struct malidp_se_device *dev, u32 attr, u32 val);
void malidp_se_set_axi_cfg(struct malidp_se_device *dev, u32 outstran,
		u32 burstlen, u32 wcache, u32 wqos);
/*
 * The following functions need to be called while holding the HW spinlock
 * unless they are used at initialization or exit time.
 */
void malidp_se_set_flip_callback(struct malidp_se_device *dev,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque);

void malidp_se_cfg_processing(struct malidp_se_device *dev,
				struct malidp_se_mw_conf *mw_cfg,
				struct malidp_se_scaler_conf *s0);

void malidp_se_set_scaling_dependent_state(struct malidp_se_device *dev,
				struct malidp_se_scaler_conf *s0);

int malidp_se_get_attr(struct malidp_se_device *dev, u32 attr, u32 *val);
int malidp_se_set_attr(struct malidp_se_device *dev, u32 attr, u32 val);
int malidp_se_save_attr(struct malidp_se_device *dev, u32 attr, u32 val);
#endif /* _MALIDP_SE_DEVICE_H_ */
