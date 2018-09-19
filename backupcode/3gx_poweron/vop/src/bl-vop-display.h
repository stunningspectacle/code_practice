/* ----------------------------------------------------------------------------
   Copyright (C) 2013-2014 Intel Mobile Communications GmbH

   Sec Class: Intel Confidential (IC)

   All rights reserved.
   ----------------------------------------------------------------------------
   This document contains proprietary information belonging to IMC.
   Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.
  ---------------------------------------------------------------------------*/

#ifndef __BL_VOP_DISPLAY_H__
#define __BL_VOP_DISPLAY_H__

#include "bl-dcc.h"
#include "vop_if_config.h"

struct vop_display {
	int type;
	int fps;	/* framerate */
	int xres;	/* pixel width */
	int yres;	/* pixel heigth */
	int xdpi;	/* pixel density per inch in x direction */
	int ydpi;	/* pixel density per inch in y direction */
	int bpp;
	unsigned char cs;
	struct display_reset reset;
	/*struct display_msg *msgs_power_on;
	struct display_msg *msgs_power_off;
	struct display_msg *msgs_sleep_in;
	struct display_msg *msgs_sleep_out;
	struct display_msg *msgs_init;*/
	struct display_msg *msgs_update;
	int (*dif_init) (struct vop_display *lcd);
	int (*dif_config) (struct vop_display *lcd, int type);
	int (*dif_stop) (struct vop_display *lcd);
	void (*panel_reset) (int gpio, struct vop_display *lcd);
	int (*panel_init) (struct vop_display *lcd, bl_display_cbs_t *pbl_disp_cb);
	int (*power_on) (struct vop_display *lcd);
	int (*sleep_in) (struct vop_display *lcd);
	int (*sleep_out) (struct vop_display *lcd);
	int (*power_off) (struct vop_display *lcd);
	void (*send_cmd) (struct vop_display *lcd,
					struct display_msg *msg,
					bl_display_cbs_t *pbl_disp_cb);
	void (*frame_prepare) (struct vop_display *lcd,
					int stride, int nlines);
	int (*frame_wfe) (struct vop_display *lcd);
	int (*set_rate) (struct vop_display *lcd, int val);
	int (*get_rate) (struct vop_display *lcd);
	struct vop_display_if dif;
	unsigned int clk_rate;
	struct display_msg **msgs_init;
};

typedef struct vop_display VOP_DISPLAY;

/*****
*
* Function Declaration
*
********************************************/
VOP_DISPLAY* vop_initialize_display_config(unsigned int *blob, unsigned char index);

int vop_dsi_set_phy_lock(struct vop_display *lcd);
void vop_dsi_start_video(struct vop_display *lcd);

#define TRACE_VOP_PRINTF(format, s...) usif_dbg_printf(format, ## s)

#endif

