/*
* (C) Copyright 2014-2015 Rockchip Electronics
*
* All rights reserved.
*
* Contents: This header file contains the definitions and types
* for the rk display platform driver.
*/

#include "rk_disp.h"
#include "bl-vop-display.h"

static struct rk_display disp_dev = {
	.preinit = 0,
};

extern unsigned int g_cgu_dcc_clk_rate;

int rk_display_setup(struct rk_display *display)
{
	/* power on lcd and enable clock*/
	if (display->ctrl_ops) {
		*((volatile int *)0xe4700a3c) = 0x1c;
		TRACE_VOP_PRINTF("Enable lcd clk: %d", display->disp_clk_rate);
		display->ctrl_ops->bl_lcd_power_on();
		g_cgu_dcc_clk_rate = display->disp_clk_rate;
		display->ctrl_ops->bl_dcc_power_on();
		*((volatile int *)0xe4700a3c) = 0x10;
	}

	display->vop_ops->open(display, 0, 1);
	display->vop_ops->load_screen(display, display->panel);

	return DISP_NO_ERR;
}

int rk_display_dev_init(struct rk_display *display,
			       bl_display_cbs_t *pbl_disp_ops,
			       void *disp_par)
{
	struct vop_display *vop = (struct vop_display *)disp_par;
	if (!display)
		return DISP_ERR_NODEV;

	if (SCREEN_MIPI == display->panel->screen_type) {
		if (vop->dif.u.dsi.dc_clk_rate == 0)
			vop->dif.u.dsi.dc_clk_rate = 297000000;
		display->disp_clk_rate = vop->dif.u.dsi.dc_clk_rate;
	} else
		display->disp_clk_rate = 2 * display->panel->timing.pixclock;
	TRACE_VOP_PRINTF("screen_type %d, disp_clk_rate %d",
			display->panel->screen_type, display->disp_clk_rate);
	display->ctrl_ops = pbl_disp_ops;
	display->disp_par = disp_par;
	rk_vop_probe(display);

	switch (display->panel->screen_type) {
	case SCREEN_RGB:
	case SCREEN_LVDS:
		nano_lvds_probe(display);
		break;
	case SCREEN_MIPI:
		dsi_probe(display);
		break;
	default:
		break;
	}

	return DISP_NO_ERR;
}

int rk_display_probe(bl_display_cbs_t *pbl_disp_ops, void *disp_par)
{
	struct rk_display *display = &disp_dev;

	if (display->preinit)
		return DISP_NO_ERR;

	memset(display, 0, sizeof(*display));

	display->panel = rk_get_panel_info();

	rk_display_dev_init(display, pbl_disp_ops, disp_par);

	rk_display_setup(display);

	display->preinit = 1;
	return DISP_NO_ERR;
}

int rk_display_update_windows(unsigned int xpos, unsigned int ypos,
			      unsigned short xact, unsigned short yact,
			      unsigned short bpp, unsigned char *data)
{
	struct fb_dsp_info fb_info;
	struct rk_display *display = &disp_dev;
	struct rk_panel_info *panel = display->panel;

	if (xact == 0 || yact == 0 ||
	    xact > panel->timing.xres || yact > panel->timing.yres ||
	    bpp == 0)
		return DISP_ERR_INVALID_SIZE;

	if (!data)
		return DISP_ERR_INVALID_DATA;

	memset(&fb_info, 0, sizeof(fb_info));
	fb_info.layer_id = WIN0;
	fb_info.xpos = xpos;
	fb_info.ypos = ypos;
	fb_info.xact = xact;
	fb_info.yact = yact;
	fb_info.xsize = xact; /* panel->timing.xres; */
	fb_info.ysize = yact; /* panel->timing.yres; */
	fb_info.xvir = fb_info.xact;
	fb_info.yvir = fb_info.yact;

	switch (bpp) {	/* byte per pixel */
	case 4:
		fb_info.format = ABGR888;
		break;
	case 3:
		fb_info.format = RGB888;
		break;
	case 2:
		fb_info.format = RGB565;
		break;
	default:
		break;
	}

	/*
	 * here the virt addr is the same as the phys addr,
	 * so no need call __virt_to_phys(data);
	 */
	fb_info.y_addr = (unsigned long)data;

	display->vop_ops->panel_display(display, &fb_info);

	return DISP_NO_ERR;
}
