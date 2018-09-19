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



#ifndef _MALIDP_HW_TYPES_H_
#define _MALIDP_HW_TYPES_H_

#include <linux/ktime.h>
#include <linux/ioport.h>

#define MALIDP_NAME_LEN 30


#define MALIDP_HW_EVENT_NONE	0
#define MALIDP_HW_EVENT_FLIP	(1 << 0)
#define MALIDP_HW_EVENT_ERROR	(1 << 1)
#define		MALIDP_HW_ERROR_URUN	(1 << 6)
#define		MALIDP_HW_ERROR_ORUN	(1 << 7)
#define		MALIDP_HW_ERROR_AXI	(1 << 8)
#define		MALIDP_HW_ERROR_QFULL	(1 << 9)
#define		MALIDP_HW_ERROR_IBUSY	(1 << 10)
#define MALIDP_HW_EVENT_VSYNC	(1 << 2)
#define MALIDP_HW_EVENT_START	(1 << 3)
#define MALIDP_HW_EVENT_NEWCFG	(1 << 4)
#define MALIDP_HW_EVENT_STOP	(1 << 5)

/* Attributes which are exposed via sysfs */
#define MALIDP_ATTR_FLAG_CM	(1 << 0)
#define MALIDP_ATTR_FLAG_DE	(1 << 1)
#define MALIDP_ATTR_FLAG_SE	(1 << 2)
#define MALIDP_ATTR_BURSTLEN	((1 << 3) | MALIDP_ATTR_FLAG_CM)
#define MALIDP_ATTR_OUTSTRAN	((2 << 3) | MALIDP_ATTR_FLAG_CM)
#define MALIDP_ATTR_CACHE	((3 << 3) | MALIDP_ATTR_FLAG_CM)
#define MALIDP_ATTR_QOS		((4 << 3) | MALIDP_ATTR_FLAG_CM)
#define MALIDP_ATTR_RQOS_LOW	(5 << 3)
#define MALIDP_ATTR_RQOS_HIGH	(6 << 3)
#define MALIDP_ATTR_RQOS_RED	(7 << 3)
#define MALIDP_ATTR_RQOS_GREEN	(8 << 3)
#define MALIDP_ATTR_FIFO_SIZE	(9 << 3)
#define MALIDP_ATTR_POUTSTDCAB  ((10 << 3) | MALIDP_ATTR_FLAG_CM)
#define MALIDP_ATTR_DE_BURSTLEN	(MALIDP_ATTR_BURSTLEN | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_POUTSTDCAB (MALIDP_ATTR_POUTSTDCAB | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_OUTSTRAN	(MALIDP_ATTR_OUTSTRAN | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_RQOS_LOW	(MALIDP_ATTR_RQOS_LOW | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_RQOS_HIGH (MALIDP_ATTR_RQOS_HIGH | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_RQOS_RED	(MALIDP_ATTR_RQOS_RED | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_RQOS_GREEN (MALIDP_ATTR_RQOS_GREEN | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_DE_FIFO_SIZE (MALIDP_ATTR_FIFO_SIZE | MALIDP_ATTR_FLAG_DE)
#define MALIDP_ATTR_SE_BURSTLEN	(MALIDP_ATTR_BURSTLEN | MALIDP_ATTR_FLAG_SE)
#define MALIDP_ATTR_SE_OUTSTRAN	(MALIDP_ATTR_OUTSTRAN | MALIDP_ATTR_FLAG_SE)
#define MALIDP_ATTR_SE_WQOS	(MALIDP_ATTR_QOS | MALIDP_ATTR_FLAG_SE)
#define MALIDP_ATTR_SE_WCACHE	(MALIDP_ATTR_CACHE | MALIDP_ATTR_FLAG_SE)

enum malidp_hw_partition_type {
	MALIDP_HW_PARTITION_FIXED,
};

struct malidp_hw_event {
	u32 type;
	ktime_t timestamp;
};

struct malidp_hw_event_queue;

struct malidp_hw_scale_rect {
	u16 src_w, src_h;
	u16 dest_w, dest_h;
};

/** struct malidp_hw_buffer - HW specific buffer structure
  *
  * The following parameters allow to specify source cropping
  * when the target is the Display Engine. For the Scaling Engine
  * only "w" and "h" are used as the output size for the memory
  * write-out interface:
  * @natural_w:	active width of the buffer in pixels, as it is in memory
  * @natural_h:	active height of the buffer in pixels, as it is in memory
  * @h_offset:	horizontal offset of the buffer in pixels
  * @v_offset:	vertical offset of the buffer in pixels
  *
  * @fmt:	fourcc pixel format of the buffer
  * @hw_fmt:	hardware-specific pixel format of the buffer
  * @addr[3]:	DMA enabled address for each plane in the buffer
  * @pitch[3]:	stride of the DMA transfer in bytes for every plane
  * @n_planes:	number of planes used for this buffer
  * @flags:	malidp specific flags
  * @mw_scaling_enable: this buffer will be scaled, and the scaling result
  * will be used in memory write-out.
  * @mw_rect:	source and destination sizes for memory-write buffer
  * @cmp_scaling_enable: this buffer will be scaled, and the scaling result
  * will be used in the composition.
  * @cmp_rect:	source and destination sizes for composition, after any
  * rotation has been applied. For an output buffer, src and dest must be the
  * same, which will also match the "natural" dimensions.
  * @write_out_enable:	this buffer will be written-out to memory,
  * if all the DE (input) buffers have this set and the output buffer too
  * this means we want to write-out the result of the composition.
  *
  * The following parameters are specific to the DE only:
  * @alpha_mode: MALIDP_ALPHA_MODE_xxx flags to set the kind of blending
  * @layer_alpha: this is the alpha value that will be used if layer level
  * blending is selected.
  * @afbc_crop_l: left AFBC cropping.
  * @afbc_crop_r: right AFBC cropping.
  * @afbc_crop_t: top AFBC cropping.
  * @afbc_crop_b: bottom AFBC cropping.
  * @ls_rect_idx: smart layer rectangle register index.
  * @hw_layer: HW specific layer information that provides information on
  * what DE layer this buffer is aiming for. This will be NULL for buffers
  * aiming the SE write-out interface.
  * @requirements: the hardware features required for this buffer
  * Bitwise OR of MALIDP_LAYER_FEATURE_xxx
  */
struct malidp_hw_buffer {
	u16 natural_w, natural_h;
	u16 h_offset, v_offset;
	u32 fmt;
	u32 hw_fmt;
	dma_addr_t addr[3];
	u32 pitch[3];
	u32 n_planes;
	u32 flags;
	u32 transform;
	bool mw_scaling_enable;
	struct malidp_hw_scale_rect mw_rect;
	bool cmp_scaling_enable;
	struct malidp_hw_scale_rect cmp_rect;
	/* DE specific */
	bool write_out_enable;
	u32 alpha_mode;
	u8 layer_alpha;
	u16 afbc_crop_l, afbc_crop_r;
	u16 afbc_crop_t, afbc_crop_b;
	u8 ls_rect_idx;
	const struct malidp_layer_hw_info *hw_layer;
	u32 requirements;
};

struct malidp_hw_pdata {
	void __iomem *regs;
	int se_irq, de_irq;
	struct clk *pxclk, *mclk, *aclk, *pclk;
	u32 de_axi_burstlen;
	u32 de_axi_poutstdcab;
	u32 de_axi_outstran;
	u32 de_axi_arqos_low;
	u32 de_axi_arqos_high;
	u32 de_axi_arqos_red;
	u32 de_axi_arqos_green;
	u32 se_axi_burstlen;
	u32 se_axi_outstran;
	u32 se_axi_awcache;
	u32 se_axi_awqos;
	u32 rotmem_size;
	u32 dp_id;
};

#endif /* _MALIDP_HW_TYPES_H_ */
