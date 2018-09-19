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

#include "bastypes.h"
#include "malidp_disp.h"
#include "bl-malidp-display.h"

#include "bl-dcc.h"
#include "malidp_disp.h"

struct malidp_display *lcd = NULL;

static struct malidp_display disp_dev = {
	.preinit = 0,
};

extern unsigned int g_cgu_dcc_clk_rate;
struct malidp_hw_description hw_desc;
struct malidp_hw_pdata hw_pdata;
struct malidp_hw_device hw_dev;

static int malidp_setup(struct malidp_display *display)
{
	struct drm_mode_modeinfo mode;
	struct fb_videomode *timging = &display->panel.timing;

	/* power on lcd and enable clock*/
	if (display->ctrl_ops) {
		*((volatile int *)0xe4700a3c) = 0x1c;
		TRACE_MALIDP_PRINTF("Enable lcd clk: %d", display->disp_clk_rate);
		display->ctrl_ops->bl_lcd_power_on();
		g_cgu_dcc_clk_rate = display->disp_clk_rate;
		display->ctrl_ops->bl_dcc_power_on();
		*((volatile int *)0xe4700a3c) = 0x10;
	}

	memset(&mode, 0, sizeof(mode));

	mode.clock = g_cgu_dcc_clk_rate;
	mode.vrefresh = timing->refresh;

	mode.hdisplay = timing->xres;
	mode.hsync_start = mode.hdisplay + timing->right_margin;
	mode.hsync_end = mode.hsync_start + timing->hsync_len;
	mode.htotal = mode.hsync_end + timing->left_margin;

	mode.vdisplay = timing->yres;
	mode.vsync_start = mode.vdisplay + timing->lower_margin;
	mode.vsync_end = mode.vsync_start + timing->vsync_len;
	mode.vtotal = mode.vsync_end + timing->upper_margin;

	malidp_hw_update_gamma_settings(&hw_dev, false, NULL);
	malidp_hw_modeset(&hw_dev, &mode);

	//display->vop_ops->open(display, 0, 1);
	//display->vop_ops->load_screen(display, display->panel);

	return DISP_NO_ERR;
}

static int malidp_display_dev_init(struct malidp_display *display,
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
	//rk_vop_probe(display);
	malidp_hw_init(&hw_dev, &hw_desc);

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

int malidp_display_probe(bl_display_cbs_t *pbl_disp_ops, void *disp_par)
{
	struct malidp_display *display = &disp_dev;

	if (display->preinit)
		return DISP_NO_ERR;

	memset(display, 0, sizeof(*display));

	display->panel = rk_get_panel_info();

	malidp_hw_enumerate(&hw_desc, MALI_DP550, &hw_pdata);

	malidp_display_dev_init(display, pbl_disp_ops, disp_par);

	malidp_display_setup(display);

	display->preinit = 1;
	return DISP_NO_ERR;
}

int malidp_display_update_windows(unsigned int xpos, unsigned int ypos,
			      unsigned short xact, unsigned short yact,
			      unsigned short bpp, unsigned char *data)
{
	//struct fb_dsp_info fb_info;
	struct malidp_hw_state hw_state;
	struct malidp_hw_buffer hw_buffer;
	struct malidp_layer_hw_info layer;
	struct malidp_hw_state_priv hw_priv;

	struct malidp_display *display = &disp_dev;
	struct rk_panel_info *panel = display->panel;
	int n_hw_layers = hw_dev.topology->n_layers;

	if (xact == 0 || yact == 0 ||
	    xact > panel->timing.xres || yact > panel->timing.yres ||
	    bpp == 0)
		return DISP_ERR_INVALID_SIZE;

	if (!data)
		return DISP_ERR_INVALID_DATA;

	//memset(&fb_info, 0, sizeof(fb_info));
	memset(&hw_state, 0, sizeof(hw_state));
	memset(&hw_buffer, 0, sizeof(hw_buffer));

	layer = hw_dev.topology->layers[0];

	hw_state.n_bufs = 1;
	hw_state.bufs = &hw_buffer;

#if 0
	fb_info.layer_id = WIN0;
	fb_info.xpos = xpos;
	fb_info.ypos = ypos;
	fb_info.xact = xact;
	fb_info.yact = yact;
	fb_info.xsize = xact; /* panel->timing.xres; */
	fb_info.ysize = yact; /* panel->timing.yres; */
	fb_info.xvir = fb_info.xact;
	fb_info.yvir = fb_info.yact;
#endif
	hw_buffer.natural_w = xact;
	hw_buffer.natural_h = yact;
	hw_buffer.addr[0] = (unsigned long)data;
	hw_buffer.n_planes = 1;
	hw_buffer.flags = MALIDP_FLAG_BUFFER_INPUT;
	hw_buffer.hw_layer = &layer;

	switch (bpp) {	/* byte per pixel */
	case 4:
		//fb_info.format = ABGR888;
		hw_buffer.format = layer->supported_formats[DRM_FORMAT_ABGR8888];
		hw_buffer.alpha_mode = MALIDP_ALPHA_MODE_PIXEL;
		break;
	case 3:
		//fb_info.format = RGB888;
		hw_buffer.format = layer->supported_formats[DRM_FORMAT_RGB888];
		break;
	case 2:
		//fb_info.format = RGB565;
		hw_buffer.format = layer->supported_formats[DRM_FORMAT_RGB565];
		break;
	default:
		break;
	}

	/*
	 * here the virt addr is the same as the phys addr,
	 * so no need call __virt_to_phys(data);
	 */
	//fb_info.y_addr = (unsigned long)data;

	//display->vop_ops->panel_display(display, &fb_info);

	malidp_hw_commit(&hw_dev, &hw_state);

	return DISP_NO_ERR;
}

bl_dcc_result_type bl_dcc_show_image(unsigned int width,
				     unsigned int height,
				     unsigned int bpp,
				     unsigned char *data,
				     bl_display_cbs_t *pbl_disp_cb,
				     bl_lcd_config_t *pbl_lcd_conf,
				     bl_dcc_platform hw_id)
{
	struct rk_panel_info *panel;
	int xpos = 0, ypos = 0;

	if (hw_id >= BL_DCC_PLATFORM_INVALID)
		return BL_DCC_RESULT_INCORRECT_HWID;

	if (NULL == lcd) {
		lcd = vop_initialize_display_config((unsigned int *)pbl_lcd_conf);
		if (malidp_display_probe(pbl_disp_cb, (void *)lcd))
			return BL_DCC_RESULT_INIT_FAILED;
	}

	panel = rk_get_panel_info();
	if (width > panel->timing.xres || height > panel->timing.yres)
		return BL_DCC_RESULT_INCORRECT_SIZE;

	xpos = (panel->timing.xres - width) >> 1;
	ypos = (panel->timing.yres - height) >> 1;

	return malidp_display_update_windows(xpos, ypos, width, height, bpp, data);
}
