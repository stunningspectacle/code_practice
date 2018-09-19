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



#ifndef _MALIDP_HW_H_
#define _MALIDP_HW_H_

#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <uapi/drm/drm_mode.h>
#include "malidp_hw_types.h"
#include "malidp_sysfs.h"

#define MALIDP_MAX_EVENT_STRING 100
#define MALIDP_MAX_LAYERS 4
#define MALIDP_MAX_SMART_LAYERS 4

enum malidp_hw_product {
	MALI_DP500 = 0,
	MALI_DP550
};

enum malidp_op_mode {
	MALIDP_OP_MODE_NORMAL = 0,
	MALIDP_OP_MODE_CONFIG,
	MALIDP_OP_MODE_POWERSAVE,
	MALIDP_OP_MODE_TEST,
	/* Must only be used at driver init */
	MALIDP_OP_MODE_UNKNOWN,
};

enum malidp_hw_intf_type {
	MALIDP_HW_INTF_PRIMARY,
	MALIDP_HW_INTF_MEMORY,
};

struct malidp_intf_hw_info {
	char name[MALIDP_NAME_LEN];
	enum malidp_hw_intf_type type;
	u32 idx;
};

enum malidp_hw_layer_type {
	MALIDP_HW_LAYER_VIDEO,
	MALIDP_HW_LAYER_GRAPHICS,
	MALIDP_HW_LAYER_SMART,
};

struct malidp_layer_hw_info {
	int index;
	char name[MALIDP_NAME_LEN];
	enum malidp_hw_layer_type type;
	/* Bitwise-OR of MALIDP_LAYER_FEATURE_xxx supported by this layer */
	u32 features;
	unsigned int n_supported_formats;
	int n_max_planes;
	const u32 *supported_formats;
	/* The HW id of the format at the same index in the supported_formats list
	 * Can be null if the index itself should be used (e.g. in DP550)
	 */
	const u32 *format_ids;
	const u32 n_supported_layers;
	const u32 regs_base;
	const u32 ad_ctrl_reg;
	const u32 ad_crop_h_reg;
	const u32 ad_crop_v_reg;
	union {
		struct {
			const u32 stride_offset;
			const u32 ptr0_low_offset;
			const u32 ptr0_high_offset;
			const u32 p3_stride_offset;
		};

		struct {
			const u32 ls_r1_in_size;
			const u32 ls_r1_offset;
			const u32 ls_r1_stride;
			const u32 ls_r1_ptr_low;
			const u32 ls_r1_ptr_high;
		};
	};
	const u32 yuv2rgb_reg_offset;
};

struct malidp_hw_regmap;
struct malidp_product_api;

struct malidp_hw_topology {
	u32 product_id;
	const struct malidp_intf_hw_info *interfaces;
	int n_interfaces;
	const struct malidp_layer_hw_info *layers;
	int n_layers;
	u32 n_scalers;
	u32 n_supported_afbc_formats;
	const u32 *supported_afbc_formats;
	u64 supported_afbc_splitblk;
	u32 n_mw_formats;
	const u32 *mw_formats;
	const u32 *mw_format_ids;
	u32 n_xform_invalid_formats;
	const u32 *xform_invalid_formats;
	const struct malidp_product_api *dp_api;
	const struct malidp_hw_regmap *regmap;
};

struct malidp_line_size_hw_info {
	u32 max_line_size;
	u32 min_line_size;
	u32 input_fifo_size;
	u32 default_rotmem_size;
};

struct malidp_hw_device {
	void __iomem *regs;
	const struct malidp_hw_topology *topology;
	const struct malidp_line_size_hw_info *ls_info;
	/*
	 * Protects access to the HW features, needs to be taken by the DE and
	 * SE IRQ handlers to avoid races.
	 */
	spinlock_t hw_lock;
	/*
	 * Must be held when calling malidp_hw_change_op_mode() to
	 * serialise operating mode changes and ensure the reported operating
	 * mode is consistent.
	 */
	struct mutex power_mutex;
	struct device *device;
	struct malidp_de_device *de_dev;
	struct malidp_se_device *se_dev;
	struct clk *pxclk, *mclk, *aclk, *pclk;
	u32 rotmem_size;
	/* clock_ratio=mclk/pclk, it is fix point data (16.16)*/
	u32 clock_ratio;
	/* Threshold for downscaling. Fix point data (16.16) */
	u32 downscaling_threshold;

	char hw_event_mask[MALIDP_MAX_EVENT_STRING];
	enum malidp_hw_partition_type partition_type;

	const struct malidp_product_api *dp_api;
	const struct malidp_hw_regmap *hw_regmap;

	struct dentry *dbg_folder;
	u32 core_id;
	/* Only used in debugfs and not available for dp500 */
	bool cproc_en;
};

struct malidp_hw_configuration {
	const struct malidp_line_size_hw_info *ls_configs;
	int n_configs;
	enum malidp_hw_partition_type partition_type;
};

/* Description of the hardware, in order of increasing specificity */
struct malidp_hw_description {
	/* Things defined by the HW version */
	const struct malidp_hw_topology *topology;
	/* Things defined by this configuration of the HW version */
	struct malidp_hw_configuration *config;
	/* Things defined by this platform */
	struct malidp_hw_pdata *pdata;
};

struct malidp_hw_smart_layer_state {
	const struct malidp_layer_hw_info *ls_hw_layer;
	/* The size of smart layer bounding box */
	u16 ls_bbox_top;
	u16 ls_bbox_left;
	u16 ls_bbox_bottom;
	u16 ls_bbox_right;
	/* The smart layer bounding box background color in ARGB8888 format */
	u32 ls_bbox_argb;
	/* The number of active smart layers */
	u8 n_smart_layers;
	/* The array of indexes of the smart layer HW buffers */
	u8 ls_hw_buf_idx[MALIDP_MAX_SMART_LAYERS];
	/* Indicate the bbox is from user space */
	bool ls_bbox_from_user;
};

struct malidp_hw_state_priv;

struct malidp_hw_state {
	u32 n_bufs;
	struct malidp_hw_buffer *bufs;
	struct malidp_hw_state_priv *hw_priv;
	struct malidp_hw_smart_layer_state ls_state;
};

/*
 * This function should find out what hardware is available and populate
 * hw_desc appropriately.
 *
 * @hw_desc[out] The hardware description
 * @product[in] The product code for this device.
 * @pdata[in] The platform-specific information for this device.
 */
void malidp_hw_enumerate(struct malidp_hw_description *hw_desc,
		enum malidp_hw_product product, struct malidp_hw_pdata *pdata);

int malidp_hw_validate(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *hw_state);

void malidp_hw_state_free(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *hw_state);

int malidp_hw_commit(struct malidp_hw_device *hwdev,
		struct malidp_hw_state *hw_state);

int malidp_hw_set_callback(struct malidp_hw_device *dev,
		const struct malidp_intf_hw_info *hw_intf,
		void (*callback)(struct device *, void *, struct malidp_hw_event_queue *),
		void *opaque);

int malidp_hw_modeset(struct malidp_hw_device *dev, struct drm_mode_modeinfo *mode);

char *malidp_hw_get_event_string(char *string, int max, struct malidp_hw_event *event);

struct malidp_hw_device *malidp_hw_init(struct platform_device *pdev,
			       struct malidp_hw_description *hw_desc);

void malidp_hw_exit(struct malidp_hw_device *hwdev);

u32 malidp_hw_get_core_id(struct malidp_hw_device *hwdev);

int malidp_hw_get_resources(struct platform_device *pdev,
			       struct device_node *nproot,
			       struct malidp_hw_pdata *pdata);

void malidp_hw_update_gamma_settings(struct malidp_hw_device *hwdev,
				     bool enable,
				     u32 *coeffs);

bool malidp_hw_format_is_yuv(u32 format);
bool malidp_hw_format_has_alpha(u32 format);
u32 malidp_hw_format_bpp(u32 format);

uint32_t malidp_hw_rotmem_size_get(struct malidp_hw_device *hwdev);
void malidp_hw_supported_dimensions_get(struct malidp_hw_device *hwdev,
					u32 *min_width, u32 *min_height,
					u32 *max_width, u32 *max_height);
enum malidp_hw_partition_type
		malidp_hw_rotmem_type_get(struct malidp_hw_device *hwdev);
const struct malidp_hw_topology
		*malidp_hw_get_topology(struct malidp_hw_device *hwdev);
u32 malidp_hw_clock_ratio_get(struct malidp_hw_device *hwdev);
int malidp_hw_clock_ratio_set(struct malidp_hw_device *hwdev,
		u32 new_clock_ratio);
u32 malidp_hw_downscaling_threshold(struct malidp_hw_device *hwdev);
u32 malidp_hw_get_fifo_size(struct malidp_hw_device *hwdev);
int malidp_hw_set_mclk(struct malidp_hw_device *hwdev, u32 pxclk);

int malidp_hw_get_attr(struct malidp_hw_device *hwdev, u32 attr, u32 *val);
int malidp_hw_set_attr(struct malidp_hw_device *hwdev, u32 attr, u32 val);

void malidp_hw_set_de_output_depth(struct malidp_hw_device *hwdev, u8 red,
		u8 green, u8 blue);

int malidp_hw_update_color_adjustment(struct malidp_hw_device *hwdev,
	u16 red_x, u16 red_y, u16 green_x, u16 green_y, u16 blue_x, u16 blue_y,
	u16 white_x, u16 white_y);

int malidp_hw_debugfs_init(struct malidp_hw_device *hwdev,
	struct dentry *parent);

struct malidp_hw_event_queue *malidp_hw_event_queue_create(size_t n_events);
void malidp_hw_event_queue_destroy(struct malidp_hw_event_queue *queue);
void malidp_hw_event_queue_enqueue(struct malidp_hw_event_queue *queue,
		struct malidp_hw_event *event);
void malidp_hw_event_queue_dequeue(struct malidp_hw_event_queue *queue,
		struct malidp_hw_event *event);

void malidp_hw_disable_all_layers_and_mw(struct malidp_hw_device *hwdev);
void malidp_hw_clear_mw(struct malidp_hw_state *hw_state);

u32 malidp_hw_read(struct malidp_hw_device *hwdev, u32 reg);
void malidp_hw_write(struct malidp_hw_device *hwdev, u32 value, u32 reg);
void malidp_hw_setbits(struct malidp_hw_device *dev, u32 mask,
			u32 reg);
void malidp_hw_clearbits(struct malidp_hw_device *dev, u32 mask,
			u32 reg);
bool malidp_hw_buf_support_srgb(struct malidp_hw_buffer *hw_buf);
bool malidp_hw_pxclk_ok(struct malidp_hw_device *dev, long rate);
attr_visible_t malidp_hw_get_attr_visible_func(struct malidp_hw_device *hwdev);

int malidp_hw_runtime_resume(struct malidp_hw_device *hwdev);
int malidp_hw_runtime_suspend(struct malidp_hw_device *hwdev);
int malidp_hw_display_switch(struct malidp_hw_device *hwdev, bool power_on);
/*
 * To be used only by the DE/SE HW code. As indicated by the "atomic" suffix
 * these calls are not thread safe, hence they need to be called with the
 * HW spinlock taken.
 *
 */
void malidp_hw_commit_scene_atomic(struct malidp_hw_device *hwdev, bool set);
void malidp_hw_cfg_de_disable_mw_flows_atomic(struct malidp_hw_device *hwdev);

#endif /* _MALIDP_HW_H_ */
