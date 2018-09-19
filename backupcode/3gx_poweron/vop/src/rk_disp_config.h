/*
* (C) Copyright 2014-2015 Rockchip Electronics
*
* All rights reserved.
*
* Contents: This header file contains the definitions and types
* for the rk display platform driver.
*/

#ifndef _RK_DISP_CONFIG_H_
#define _RK_DISP_CONFIG_H_

#define OUT_P888	0	/* 24bit screen,connect to lcdc D0~D23 */
#define OUT_P666	1	/* 18bit screen,connect to lcdc D0~D17 */
#define OUT_P565	2
#define OUT_S888x	4
#define OUT_CCIR656	6
#define OUT_S888	8
#define OUT_S888DUMY	12
#define OUT_RGB_AAA	15
#define OUT_P16BPP4	24
#define OUT_D888_P666	0x21	/* connect to lcdc D2~D7, D10~D15, D18~D23 */
#define OUT_D888_P565	0x22

#define SCREEN_NULL		0
#define SCREEN_RGB		1
#define SCREEN_LVDS		2
#define SCREEN_DUAL_LVDS	3
#define SCREEN_MCU		4
#define SCREEN_TVOUT		5
#define SCREEN_HDMI		6
#define SCREEN_MIPI		7
#define SCREEN_DUAL_MIPI	8
#define SCREEN_EDP		9

#define LVDS_8BIT_1	0
#define LVDS_8BIT_2	1
#define LVDS_8BIT_3	2
#define LVDS_6BIT	3

#define PRMRY		1	/* primary display device */
#define EXTEND		2	/* extend display device */

#define FB_VMODE_INTERLACED	1	/* interlaced	*/

struct fb_videomode {
	/* const char *name; */
	unsigned int refresh;
	unsigned int xres;
	unsigned int yres;
	unsigned int pixclock;
	unsigned int left_margin;
	unsigned int right_margin;
	unsigned int upper_margin;
	unsigned int lower_margin;
	unsigned int hsync_len;		/* Horz sync pulse width */
	unsigned int vsync_len;		/* Vertical sync pulse width */
	unsigned int sync;
	unsigned int vmode;
	unsigned int flag;
};

/**
 * A structure for lcd panel information.
 */
struct rk_panel_info {
	unsigned int lcd_face;	/* lcd rgb tye (i.e. RGB888) */
	unsigned int screen_type;	/* LVDS/MIPI/RGB/HDMI */
	unsigned int lvds_format;

	struct fb_videomode timing;	/* screen display timing*/
	unsigned int width;	/* physical size of lcd width */
	unsigned int height;	/* physical size of lcd height */
	unsigned char pin_dclk;	/* Clock polarity */
	unsigned char pin_den;	/* Output Enable polarity */
	unsigned char pin_hsync;	/* Horizontal Sync polarity */
	unsigned char pin_vsync;	/* Vertical Sync polarity */
	unsigned char swap_rb;
	unsigned char swap_rg;
	unsigned char swap_gb;
};

#endif	/* _RK_DISP_CONFIG_H_ */
