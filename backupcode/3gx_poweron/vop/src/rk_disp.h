/*
* (C) Copyright 2014-2015 Rockchip Electronics
*
* All rights reserved.
*
* Contents: This header file contains the definitions and types
* for the rk display platform driver.
*/

#ifndef _RK_DISP_H_
#define _RK_DISP_H_

#include "bl-dcc.h"
#include "rk_disp_config.h"

#ifndef BITS
#define BITS(x, bit)            ((x) << (bit))
#endif

#ifndef BITS_MASK
#define BITS_MASK(x, mask, bit) BITS((x) & (mask), bit)
#endif

#ifndef bool
#define bool	unsigned char
#endif

#define FB_VMODE_INTERLACED	1	/* interlaced	*/

enum rk_fb_data_format {
	ARGB888 = 0,
	RGB888 = 1,
	RGB565 = 2,
	YUV420 = 4,
	YUV422 = 5,
	YUV444 = 6,
	XRGB888,
	XBGR888,
	ABGR888,
};

enum lay_id {
	WIN0 = 0,
	WIN1,
	NUM_WIN,
};

enum disp_result_type {
	DISP_NO_ERR = 0,
	DISP_ERR_NODEV,
	DISP_ERR_INVALID_SIZE,
	DISP_ERR_INVALID_DATA,
	DISP_ERR_INVALID_WINID,
	DISP_ERR_INIT_FAILED,
	DISP_ERR_POWER_FAILED,
	DISP_ERR_UPDATE_FAILED,
	DISP_ERR_UNKNOWN,
};

struct fb_dsp_info {
	enum lay_id layer_id;
	enum rk_fb_data_format format;
	unsigned long y_addr;
	unsigned long cbr_addr;	/* Cbr memory start address */
	unsigned int xpos;	/* start point in panel-->VOP_WINx_DSP_ST */
	unsigned int ypos;
	unsigned short xsize;	/* display window width/height-->VOP_WINx_DSP_INFO */
	unsigned short ysize;
	unsigned short xact;	/* origin display window size-->VOP_WINx_ACT_INFO */
	unsigned short yact;
	unsigned short xvir;	/* virtual width/height-->VOP_WINx_VIR */
	unsigned short yvir;
};

struct rk_win_info {
	unsigned char win_id;
	unsigned char fmt_cfg;
	unsigned char swap_rb;
	bool state;	/* on or off */

	unsigned short xsize;	/* display window width/height */
	unsigned short ysize;
	unsigned short xact;	/* origin display window size */
	unsigned short yact;

	unsigned int dsp_stx;
	unsigned int dsp_sty;

	unsigned int scale_yrgb_x;
	unsigned int scale_yrgb_y;
	unsigned int scale_cbcr_x;
	unsigned int scale_cbcr_y;

	unsigned int y_vir_stride;
	unsigned int uv_vir_stride;
	unsigned long y_addr;
	unsigned long uv_addr;
};

struct rk_display;

struct rk_disp_vop_drv_ops {
	int (*open)(struct rk_display *disp, int win_id, bool enable);
	int (*load_screen)(struct rk_display *disp,
			   struct rk_panel_info *panel);
	int (*panel_display)(struct rk_display *disp,
			     struct fb_dsp_info *fb_info);
	void (*config_done)(struct rk_display *disp);
};

struct rk_disp_trsm_drv_ops {
	int (*enable)(struct rk_display *display);
	int (*disable)(struct rk_display *display);
};

struct rk_display {
	bool preinit;
	unsigned char vop_id;
	unsigned int disp_clk_rate;
	struct rk_panel_info *panel;
	struct rk_win_info win[NUM_WIN];
	struct rk_disp_vop_drv_ops *vop_ops;
	struct rk_disp_trsm_drv_ops *trsm_ops;	/* Transmitter ops */
	bl_display_cbs_t *ctrl_ops;
	void *disp_par;
};

struct rk_panel_info *rk_get_panel_info(void);
int rk_display_probe(bl_display_cbs_t *pbl_disp_ops, void *disp_par);
int rk_display_update_windows(unsigned int xpos, unsigned int ypos,
			      unsigned short xact, unsigned short yact,
			      unsigned short bpp, unsigned char *data);

#endif	/* _RK_DISP_H_ */
