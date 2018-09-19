/*
 *
 * (C) COPYRIGHT 2014-2015 ARM Limited. All rights reserved.
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



#ifndef _MALIDP_PRODUCT_API_H_
#define _MALIDP_PRODUCT_API_H_

#include "malidp_hw.h"
#include "malidp_de_device.h"
#include "malidp_se_device.h"
#include "malidp_sysfs.h"

/* HW description accessors for the different products.
 * Could live in separate files, but that would cause lots of
 * file-bloat
 */
void malidp_dp500_get_hw_description(struct malidp_hw_description *);
void malidp_dp550_get_hw_description(struct malidp_hw_description *);

enum malidp_scaling_coeff_set;

struct malidp_de_product_api {
	/* Must be called with the DE in Config Mode
	 * Analogous to the current malidp_de_hw_cfg implementation
	 *  - Register interrupt handler (use common "soft" handler)
	 *  - Set AXI config
	 *  - Set background color
	 *  - Set interrupts
	 *  - Set alpha lookup tables
	 *  - Write coefficients if necessary
	 */
	int (*hw_cfg)(struct malidp_de_device *);
	/* Must be called with the DE in Config Mode
	 * Write the display mode to the HW registers
	 * Can't be common due to HSP and VSP moving
	 */
	void (*modeset)(struct malidp_de_device *, struct malidp_de_hwmode *);
	/* Called from malidp_de_cfg_layer and at modeset
	 * Set the gamma/inverse gamma lookup table, writing coefficients if
	 * needed. For inverse gamma, coeffs should be NULL.
	 */
	void (*set_gamma_coeff)(struct malidp_de_device *, u32 *coeffs);
	/* Fixup a DRM format according to buffer flags, if necessary.
	 * Returns the fixed-up format, to be converted to a hardware ID.
	 */
	u32 (*fmt_fixup)(u32 drm_format, u32 flags);
	/* Call from malidp_de_attr_valid
	 * Validate AXI setting for DE:
	 * Three attributes: BURSTLEN, POUTSTDCAB and OUTSTRAN
	 */
	bool (*axi_valid)(u32 attr, u32 val);
	/* Shouldn't be called directly. It is used for requestig IRQ */
	irqreturn_t (*irq_handler)(int irq, void *data);
};

struct malidp_se_product_api {
	/* Must be called with the SE in Config Mode
	 * Analogous to the current malidp_de_hw_cfg implementation
	 *  - Register interrupt handler (use common "soft" handler)
	 *  - Set AXI config
	 *  - Set image enhancer state
	 *  - Set interrupts
	 */
	int (*hw_cfg)(struct malidp_se_device *);
	/* Called from malidp_se_cfg_processing
	 * Select the scaler coefficients, writing the table if necessary
	 */
	void (*set_scaler_coeff)(struct malidp_se_device *,
		enum malidp_scaling_coeff_set hcoeff,
		enum malidp_scaling_coeff_set vcoeff);
	/* Checking the scaling limitation
	 */
	bool (*limitation_check)(struct malidp_se_device *,
			struct malidp_se_scaler_conf *);
	/* Calculate the downscaling threshold
	 */
	u32 (*calc_downscaling_threshold)(u32, u32, struct drm_mode_modeinfo *);
	/* Call from malidp_se_attr_valid
	 * Validate AXI setting for SE:
	 * Tow attributes: BURSTLEN and OUTSTRAN
	 */
	bool (*axi_valid)(u32 attr, u32 val);
	/* Shouldn't be called directly. It is used for requestig IRQ */
	irqreturn_t (*irq_handler)(int irq, void *data);
};

struct malidp_product_api {
	/* Change the operating mode of the DP
	 * This must put both the DE and SE in the given mode (or its closest
	 * equivalent), returning the old mode
	 */
	enum malidp_op_mode (*change_op_mode)(struct malidp_hw_device *,
		enum malidp_op_mode);
	/* Called from hw_init and hw_exit
	 * Disable device interrupt.
	 */
	void (*disable_irq)(struct malidp_hw_device *);
	/* Called by sysfs to check the condition of every attributes
	 */
	attr_visible_t attr_visible;
	/* Called from hw_debugfs_init.
	 * This API is used for the elements specific to the product.
	 */
	void (*debugfs_func)(struct malidp_hw_device *);
	/* Calculate rotation memory size */
	u32 (*rotmem_size_required)(const struct malidp_hw_buffer *);
	/* The APIs for the DE and SE of this product */
	struct malidp_de_product_api de_api;
	struct malidp_se_product_api se_api;
};

/* Register offsets for use by the generic de/se/hw layer */
struct malidp_de_regmap {
	u16 axi_control;
	u16 disp_func;
	u16 output_depth;
	/* Offset of the first color adjustment coefficient register */
	u16 coloradj_coeff;
	u16 qos_control;
};

struct malidp_se_regmap {
	/* Offset of the SE_CONTROL register */
	u16 control;
	u16 axi_control;
	/* Offset of the Layers Control register block */
	u16 layers_control;
	/* Offset of the Scaling Control register block */
	u16 scaling_control;
	/* Offset of the Image Enhancement register block */
	u16 enhancer_control;
	/* Offset of the RGB_YUV_CONTROL register */
	u16 conv_control;
	/* Offset of the Memory Write register block */
	u16 mw_control;
};

struct malidp_hw_regmap {
	/* Offset of the PERIPHERAL_ID 4 register */
	u16 id_registers;
	/* Offset of the Configuration Valid register */
	u16 config_valid;
	/* Base address offset of the DE registers */
	u16 de_base;
	/* Base address offset of the SE registers */
	u16 se_base;
	/* Regmap descriptors for the DE and SE */
	struct malidp_de_regmap de_regmap;
	struct malidp_se_regmap se_regmap;
};

#endif /* _MALIDP_PRODUCT_API_H_ */
