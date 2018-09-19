/*
* (C) Copyright 2014-2015 Rockchip Electronics
*
* All rights reserved.
*
* Contents: This header file contains the register definitions and types
* for the vop hardware driver.
*/

#include "rk_disp.h"
#include "rk_vop_reg.h"

#define VOP_ERR(x...)	/*printf("[vop_err]" x)*/

VOP_REG vop_reg __attribute__((section("DCC_REG")));
VOP_REG regbak;

int rk_vop_win_enable(struct rk_display *disp, int win_id, bool enable)
{
	struct rk_win_info *win = &disp->win[win_id];

	if (win_id >= NUM_WIN) {
		VOP_ERR("%s: win id bigger than num_win\n", __func__);
		return DISP_ERR_INVALID_WINID;
	}

	win->state = enable;

	if (!enable) {
		switch (win_id) {
		case WIN0:
			VOP_MSK_REG(VOP_SYS_CTRL,
				    M_WIN0_EN, V_WIN0_EN(0));
			break;
		case WIN1:
			VOP_MSK_REG(VOP_SYS_CTRL,
				    M_WIN1_EN, V_WIN1_EN(0));
			break;
		default:
			break;
		}

		VOP_REG_CFG_DONE();
	}

	return DISP_NO_ERR;
}

void rk_vop_update_win_regs(struct rk_display *disp,
				   struct rk_win_info *win)
{
/*
	if (win->state == 0) {
		rk_vop_win_enable(disp, win->win_id, 0);
		return;
	}
*/
	switch (win->win_id) {
	case WIN0:
		VOP_MSK_REG(VOP_SYS_CTRL,
			    M_WIN0_EN |
			    M_WIN0_FORMAT |
			    M_WIN0_RB_SWAP,
			    V_WIN0_EN(win->state) |
			    V_WIN0_FORMAT(win->fmt_cfg) |
			    V_WIN0_RB_SWAP(win->swap_rb));
		VOP_WR_REG(VOP_WIN0_SCL_FACTOR_YRGB,
			   V_X_SCL_FACTOR(win->scale_yrgb_x) |
			   V_Y_SCL_FACTOR(win->scale_yrgb_y));
		VOP_WR_REG(VOP_WIN0_SCL_FACTOR_CBR,
			   V_X_SCL_FACTOR(win->scale_cbcr_x) |
			   V_Y_SCL_FACTOR(win->scale_cbcr_y));

		VOP_MSK_REG(VOP_WIN0_VIR, M_YRGB_VIR | M_CBBR_VIR,
			    V_YRGB_VIR(win->y_vir_stride) |
			    V_CBCR_VIR(win->uv_vir_stride));
		VOP_WR_REG(VOP_WIN0_ACT_INFO,
			   V_ACT_WIDTH(win->xact) |
			   V_ACT_HEIGHT(win->yact));
		VOP_WR_REG(VOP_WIN0_DSP_ST,
			   V_DSP_STX(win->dsp_stx) |
			   V_DSP_STY(win->dsp_sty));
		VOP_WR_REG(VOP_WIN0_DSP_INFO,
			   V_DSP_WIDTH(win->xsize) |
			   V_DSP_HEIGHT(win->ysize));

		VOP_WR_REG(VOP_WIN0_YRGB_MST, win->y_addr);
		VOP_WR_REG(VOP_WIN0_CBR_MST, win->uv_addr);
		break;
	case WIN1:
		VOP_MSK_REG(VOP_SYS_CTRL,
			    M_WIN1_EN |
			    M_WIN1_FORMAT |
			    M_WIN1_RB_SWAP,
			    V_WIN1_EN(win->state) |
			    V_WIN1_FORMAT(win->fmt_cfg) |
			    V_WIN1_RB_SWAP(win->swap_rb));

		/* this vop unsupport win1 scale */
		VOP_WR_REG(VOP_WIN1_DSP_INFO,
			   V_DSP_WIDTH(win->xsize) |
			   V_DSP_HEIGHT(win->ysize));
		VOP_WR_REG(VOP_WIN1_DSP_ST,
			   V_DSP_STX(win->dsp_stx) |
			   V_DSP_STY(win->dsp_sty));

		/* this vop win1 only support RGB */
		VOP_WR_REG(VOP_WIN1_YRGB_MST, win->y_addr);
		VOP_MSK_REG(VOP_WIN1_VIR, M_YRGB_VIR | M_CBBR_VIR,
			    V_YRGB_VIR(win->y_vir_stride) |
			    V_CBCR_VIR(0));
		break;
	default:
		VOP_ERR("%s: win id bigger than num_win\n", __func__);
		break;
	}
}

int rk_vop_set_par(struct rk_display *disp, struct fb_dsp_info *fb_info)
{
	struct rk_panel_info *panel = disp->panel;
	struct rk_win_info *win;
	unsigned int stride = 0;
	unsigned int uv_stride = 0;

	if (fb_info->layer_id >= NUM_WIN) {
		VOP_ERR("%s: layer id bigger than num_win\n", __func__);
		return DISP_NO_ERR;
	}

	win = &disp->win[fb_info->layer_id];

	win->win_id = fb_info->layer_id;
	win->xact = fb_info->xact;
	win->yact = fb_info->yact;
	win->xsize = fb_info->xsize;
	win->ysize = fb_info->ysize;
	win->dsp_stx = fb_info->xpos + panel->timing.left_margin +
			panel->timing.hsync_len;

	if (panel->timing.vmode == FB_VMODE_INTERLACED) {
		win->ysize /= 2;
		win->dsp_sty = fb_info->ypos / 2 +
		    panel->timing.upper_margin + panel->timing.vsync_len;
	} else {
		win->dsp_sty = fb_info->ypos +
		    panel->timing.upper_margin + panel->timing.vsync_len;
	}
	win->scale_yrgb_x = CALSCALE(win->xact, win->xsize);
	win->scale_yrgb_y = CALSCALE(win->yact, win->ysize);
	win->fmt_cfg = fb_info->format;

	switch (fb_info->format) {
	case ARGB888:
		stride = 4 * fb_info->xvir;
		win->swap_rb = 0;
		break;
	case XBGR888:
		win->fmt_cfg = ARGB888;
		stride = 4 * fb_info->xvir;
		win->swap_rb = 1;
		break;
	case ABGR888:
		win->fmt_cfg = ARGB888;
		stride = 4 * fb_info->xvir;
		win->swap_rb = 1;
		break;
	case RGB888:
		stride = 3 * fb_info->xvir;
		win->swap_rb = 0;
		break;
	case RGB565:
		stride = 2 * fb_info->xvir;
		win->swap_rb = 0;
		break;
	case YUV444:
		if (win->win_id == 0) {
			stride = fb_info->xvir;
			uv_stride = stride << 1;
			win->scale_cbcr_x =
				CALSCALE(win->xact, win->xsize);
			win->scale_cbcr_y =
				CALSCALE(win->yact, win->ysize);
			win->swap_rb = 0;
		} else {
			VOP_ERR("%s:un supported format!\n", __func__);
		}
		break;
	case YUV422:
		if (win->win_id == 0) {
			stride = fb_info->xvir;
			uv_stride = stride >> 1;
			win->scale_cbcr_x = CALSCALE((win->xact / 2),
						     win->xsize);
			win->scale_cbcr_y = CALSCALE(win->yact,
						     win->ysize);
			win->swap_rb = 0;
		} else {
			VOP_ERR("%s:un supported format!\n", __func__);
		}
		break;
	case YUV420:
		if (win->win_id == 0) {
			stride = fb_info->xvir;
			uv_stride = stride;
			win->scale_cbcr_x = CALSCALE(win->xact / 2,
						     win->xsize);
			win->scale_cbcr_y = CALSCALE(win->yact / 2,
						     win->ysize);
			win->swap_rb = 0;
		} else {
			VOP_ERR("%s:un supported format!\n", __func__);
		}
		break;
	default:
		VOP_ERR("%s:un supported format!\n", __func__);
		break;
	}

	win->y_vir_stride = stride >> 2;
	win->uv_vir_stride = uv_stride >> 2;

	/* here do not calc yoffset addr,yoffset is 0 as default, TODO modify */
	win->y_addr = fb_info->y_addr;
	win->uv_addr = fb_info->cbr_addr;

	return DISP_NO_ERR;
}

void rk_vop_panel_display(struct rk_display *disp,
				 struct fb_dsp_info *fb_info)
{
	int win_id = 0;
	struct rk_win_info *win;

	if (fb_info) {
		win_id = fb_info->layer_id;
		win = &disp->win[win_id];
		rk_vop_set_par(disp, fb_info);
		rk_vop_update_win_regs(disp, win);
		VOP_REG_CFG_DONE();
	}
}

int rk_vop_load_screen(struct rk_display *display,
			      struct rk_panel_info *panel)
{
	unsigned short hfp = panel->timing.right_margin;
	unsigned short hbp = panel->timing.left_margin;
	unsigned short vbp = panel->timing.upper_margin;
	unsigned short vfp = panel->timing.lower_margin;
	unsigned short xres = panel->timing.xres;
	unsigned short yres = panel->timing.yres;
	unsigned short hsync = panel->timing.hsync_len;
	unsigned short vsync = panel->timing.vsync_len;
	unsigned short vtotal = 0;
	unsigned short vsync_st_f1 = 0, vsync_end_f1 = 0;
	unsigned short vact_st_f1 = 0, vact_end_f1 = 0;
	unsigned short line_num = 0;

	/* let above to take effect */
	VOP_MSK_REG(VOP_SYS_CTRL, M_LCDC_STANDBY,
		    V_LCDC_STANDBY(1));

	switch (panel->screen_type) {
	case SCREEN_MIPI:
		VOP_MSK_REG(VOP_BUS_INTF_CTRL,
			    M_MIPI_DCLK_EN |
			    M_MIPI_DCLK_INVERT,
			    V_MIPI_DCLK_EN(1) |
			    V_MIPI_DCLK_INVERT(0));
		VOP_MSK_REG(VOP_MIPI_EDPI_CTRL,
			    M_EDPI_HALT_EN,
			    V_EDPI_HALT_EN(1));
		break;
	case SCREEN_LVDS:
		VOP_MSK_REG(VOP_BUS_INTF_CTRL,
			    M_LVDS_DCLK_EN |
			    M_LVDS_DCLK_INVERT,
			    V_LVDS_DCLK_EN(1) |
			    V_LVDS_DCLK_INVERT(0));
		break;
	case SCREEN_HDMI:
	case SCREEN_RGB:
		VOP_MSK_REG(VOP_BUS_INTF_CTRL,
			    M_RGB_DCLK_EN |
			    M_RGB_DCLK_INVERT,
			    V_RGB_DCLK_EN(1) |
			    V_RGB_DCLK_INVERT(0));
		break;
	default:
		VOP_ERR("un supported screen type!\n");
	}

	switch (panel->lcd_face) {
	case OUT_P565:
		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_DSP_OUT_FORMAT |
			    M_DITHER_DOWN_EN |
			    M_DITHER_DOWN_MODE |
			    M_DITHER_DOWN_SEL,
			    V_DSP_OUT_FORMAT(OUT_P565) |
			    V_DITHER_DOWN_EN(1) |
			    V_DITHER_DOWN_MODE(0) |
			    V_DITHER_DOWN_SEL(1));
		break;
	case OUT_P666:
		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_DSP_OUT_FORMAT |
			    M_DITHER_DOWN_EN |
			    M_DITHER_DOWN_MODE |
			    M_DITHER_DOWN_SEL,
			    V_DSP_OUT_FORMAT(OUT_P666) |
			    V_DITHER_DOWN_EN(1) |
			    V_DITHER_DOWN_MODE(1) |
			    V_DITHER_DOWN_SEL(1));
		break;
	case OUT_D888_P565:
		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_DSP_OUT_FORMAT |
			    M_DITHER_DOWN_EN |
			    M_DITHER_DOWN_MODE |
			    M_DITHER_DOWN_SEL,
			    V_DSP_OUT_FORMAT(OUT_P888) |
			    V_DITHER_DOWN_EN(1) |
			    V_DITHER_DOWN_MODE(0) |
			    V_DITHER_DOWN_SEL(1));
		break;
	case OUT_D888_P666:
		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_DSP_OUT_FORMAT |
			    M_DITHER_DOWN_EN |
			    M_DITHER_DOWN_MODE |
			    M_DITHER_DOWN_SEL,
			    V_DSP_OUT_FORMAT(OUT_P888) |
			    V_DITHER_DOWN_EN(1) |
			    V_DITHER_DOWN_MODE(1) |
			    V_DITHER_DOWN_SEL(1));
		break;
	case OUT_P888:
		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_DSP_OUT_FORMAT |
			    M_DITHER_DOWN_EN |
			    M_DITHER_UP_EN,
			    V_DSP_OUT_FORMAT(OUT_P888) |
			    V_DITHER_DOWN_EN(0) |
			    V_DITHER_UP_EN(0));
		break;
	default:
		VOP_ERR("un supported interface!\n");
		break;
	}

	VOP_MSK_REG(VOP_DSP_CTRL0,
		    M_HSYNC_POL |
		    M_VSYNC_POL |
		    M_DEN_POL |
		    M_DCLK_POL,
		    V_HSYNC_POL(panel->pin_hsync) |
		    V_VSYNC_POL(panel->pin_vsync) |
		    V_DEN_POL(panel->pin_den) |
		    V_DCLK_POL(panel->pin_dclk));

	VOP_MSK_REG(VOP_DSP_CTRL1,
		    M_BG_COLOR |
		    M_DSP_BG_SWAP |
		    M_DSP_RB_SWAP |
		    M_DSP_RG_SWAP |
		    M_BLANK_EN | M_BLACK_EN,
		    V_BG_COLOR(0x000000) |
		    V_DSP_BG_SWAP(panel->swap_gb) |
		    V_DSP_RB_SWAP(panel->swap_rb) |
		    V_DSP_RG_SWAP(panel->swap_rg) |
		    V_BLANK_EN(0) | V_BLACK_EN(0));

	/* config timing */
	VOP_WR_REG(VOP_DSP_HTOTAL_HS_END,
		   V_HSYNC(hsync) |
		   V_HORPRD(hsync + hbp + xres + hfp));
	VOP_WR_REG(VOP_DSP_HACT_ST_END,
		   V_HAEP(hsync + hbp + xres) |
		   V_HASP(hsync + hbp));

	if (panel->timing.vmode == FB_VMODE_INTERLACED) {
		vtotal = 2 * (vsync + vbp + vfp) + yres + 1;

		/* First Field Timing */
		VOP_WR_REG(VOP_DSP_VTOTAL_VS_END,
			   V_VSYNC(vsync) |
			   V_VERPRD(vtotal));
		VOP_WR_REG(VOP_DSP_VACT_ST_END,
			   V_VAEP(vsync + vbp + yres / 2) |
			   V_VASP(vsync + vbp));

		/* Second Field Timing */
		vsync_st_f1 = vsync + vbp + yres / 2 + vfp;
		vsync_end_f1 = 2 * vsync + vbp + yres / 2 + vfp;
		vact_st_f1 = 2 * (vsync + vbp) + yres / 2 + vfp + 1;
		vact_end_f1 = 2 * (vsync + vbp) + yres + vfp + 1;
		line_num = vsync + vbp + yres / 2;

		VOP_WR_REG(VOP_DSP_VS_ST_END_F1,
			   V_VSYNC_ST_F1(vsync_st_f1) |
			   V_VSYNC_END_F1(vsync_end_f1));
		VOP_WR_REG(VOP_DSP_VACT_ST_END_F1,
			   V_VAEP(vact_end_f1) |
			   V_VASP(vact_st_f1));

		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_INTERLACE_DSP_EN |
			    M_WIN0_YRGB_DEFLICK_EN |
			    M_WIN0_CBR_DEFLICK_EN |
			    M_INTERLACE_FIELD_POL |
			    M_WIN0_INTERLACE_EN |
			    M_WIN1_INTERLACE_EN,
			    V_INTERLACE_DSP_EN(1) |
			    V_WIN0_YRGB_DEFLICK_EN(1) |
			    V_WIN0_CBR_DEFLICK_EN(1) |
			    V_INTERLACE_FIELD_POL(0) |
			    V_WIN0_INTERLACE_EN(1) |
			    V_WIN1_INTERLACE_EN(1));
	} else {
		vtotal = vsync + vbp + yres + vfp;
		line_num = vsync + vbp + yres;

		VOP_WR_REG(VOP_DSP_VTOTAL_VS_END,
			   V_VSYNC(vsync) |
			   V_VERPRD(vtotal));
		VOP_WR_REG(VOP_DSP_VACT_ST_END,
			   V_VAEP(vsync + vbp + yres) |
			   V_VASP(vsync + vbp));

		VOP_MSK_REG(VOP_DSP_CTRL0,
			    M_INTERLACE_DSP_EN |
			    M_WIN0_YRGB_DEFLICK_EN |
			    M_WIN0_CBR_DEFLICK_EN |
			    M_INTERLACE_FIELD_POL |
			    M_WIN0_INTERLACE_EN |
			    M_WIN1_INTERLACE_EN,
			    V_INTERLACE_DSP_EN(0) |
			    V_WIN0_YRGB_DEFLICK_EN(0) |
			    V_WIN0_CBR_DEFLICK_EN(0) |
			    V_INTERLACE_FIELD_POL(0) |
			    V_WIN0_INTERLACE_EN(0) |
			    V_WIN1_INTERLACE_EN(0));
	}

	VOP_MSK_REG(VOP_INT_STATUS,
		    M_LF_INT_NUM,
		    V_LF_INT_NUM(line_num));

	/* let above to take effect */
	VOP_REG_CFG_DONE();

	if (display->trsm_ops && display->trsm_ops->enable)
		display->trsm_ops->enable(display);

	VOP_MSK_REG(VOP_SYS_CTRL, M_LCDC_STANDBY,
		    V_LCDC_STANDBY(0));

	return DISP_NO_ERR;
}

int rk_vop_open(struct rk_display *disp, int win_id, bool enable)
{
	if (win_id < NUM_WIN)
		rk_vop_win_enable(disp, win_id, enable);
	else {
		VOP_ERR("%s: invalid win id:%d\n", __func__, win_id);
	}

	return DISP_NO_ERR;
}

static struct rk_disp_vop_drv_ops vop_drv_ops = {
	.open = rk_vop_open,
	.load_screen = rk_vop_load_screen,
	.panel_display = rk_vop_panel_display,
};

void rk_vop_probe(struct rk_display *display)
{
	display->vop_ops = &vop_drv_ops;
}

