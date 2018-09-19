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

#include "rk_disp.h"

#define LVDS_BASE_ADDR	0xE1000140

/***********************************************************************
 * LVDS Register definition
***********************************************************************/

/* VOP_LVDS_CTRL(0x000): LVDS control register */
#define M_LVDS_SELECT		BITS(3, 0)
#define M_LVDS_MSBSEL		BITS(1, 2)
#define M_LVDS_CLK_EDGE		BITS(1, 3)
#define M_LVDS_DATA_BITS	BITS(3, 4)
#define M_LVDS_OFFSET_VOLT	BITS(7, 6)
#define M_LVDS_SWING		BITS(7, 9)
#define M_LVDS_PRE_EMPHASIS	BITS(3, 12)
#define M_LVDS_CLK_DS1		BITS(7, 14)
#define M_LVDS_POWER_MODE	BITS(1, 17)
#define M_LVDS_TTL_MODE_EN	BITS(1, 18)

#define V_LVDS_SELECT(x)	BITS_MASK(x, 3, 0)
#define V_LVDS_MSBSEL(x)	BITS_MASK(x, 1, 2)
#define V_LVDS_CLK_EDGE(x)	BITS_MASK(x, 1, 3)
#define V_LVDS_DATA_BITS(x)	BITS_MASK(x, 3, 4)
#define V_LVDS_OFFSET_VOLT(x)	BITS_MASK(x, 7, 6)
#define V_LVDS_SWING(x)		BITS_MASK(x, 7, 9)
#define V_LVDS_PRE_EMPHASIS(x)	BITS_MASK(x, 3, 12)
#define V_LVDS_CLK_DS1(x)	BITS_MASK(x, 7, 14)
#define V_LVDS_POWER_MODE(x)	BITS_MASK(x, 1, 17)
#define V_LVDS_TTL_MODE_EN(x)	BITS_MASK(x, 1, 18)

/* VOP_LVDS_PLL_STA(0x004): LVDS PLL lock status */
#define M_LVDS_PLL_LOCK		BITS(1, 0)

#define LVDS_WR_REG(addr, val)	(plvds->addr = (val))
#define LVDS_RD_REG(addr)	(plvds->addr)

#define LVDS_MSK_REG(addr, msk, val)			\
	do {						\
		unsigned int temp = LVDS_RD_REG(addr) & (0xffff - (msk));	\
		LVDS_WR_REG(addr, temp | ((val) & (msk)));			\
	} while (0)

typedef unsigned int regu32;

typedef volatile struct tagLVDS_REG {
	regu32 LVDS_CTRL;	/* 0x00 lvds control Register */
	regu32 LVDS_PLL_STA;	/* 0x04 lvds pll lock status Register */
} LVDS_REG, *pLVDS_REG;

LVDS_REG *plvds;

static int nano_lvds_disable(struct rk_display *display)
{
	LVDS_MSK_REG(LVDS_CTRL, M_LVDS_POWER_MODE,
		     V_LVDS_POWER_MODE(0));
	return DISP_NO_ERR;
}

static int nano_lvds_enable(struct rk_display *display)
{
	struct rk_panel_info *panel = display->panel;
	unsigned char data_bits = 0;
	unsigned char ttl_en = 0;

	if (!panel)
		panel = rk_get_panel_info();

	/* power down */
	LVDS_MSK_REG(LVDS_CTRL, M_LVDS_POWER_MODE,
		     V_LVDS_POWER_MODE(0));

	/* config parameter */
	ttl_en = (panel->screen_type == SCREEN_RGB) ? 1 : 0;
	if (ttl_en) {
		LVDS_MSK_REG(LVDS_CTRL,
			     M_LVDS_TTL_MODE_EN,
			     V_LVDS_TTL_MODE_EN(1));
	} else {
		/* 1: LVDS_8_BIT; 0: LVDS_6_BIT */
		data_bits = (panel->lcd_face == OUT_P888) ? 1 : 0;
		LVDS_MSK_REG(LVDS_CTRL,
			     M_LVDS_TTL_MODE_EN |
			     M_LVDS_SELECT |
			     M_LVDS_MSBSEL |
			     M_LVDS_DATA_BITS,
			     V_LVDS_TTL_MODE_EN(0) |
			     V_LVDS_SELECT(panel->lvds_format) |
			     V_LVDS_MSBSEL(1) |	/* msb d7 */
			     V_LVDS_DATA_BITS(data_bits));
	}

	/* power up */
	LVDS_MSK_REG(LVDS_CTRL, M_LVDS_POWER_MODE,
		     V_LVDS_POWER_MODE(1));
	return DISP_NO_ERR;
}

static struct rk_disp_trsm_drv_ops lvds_drv_ops = {
	.enable = nano_lvds_enable,
	.disable = nano_lvds_disable,
};

void nano_lvds_probe(struct rk_display *display)
{
	plvds = (LVDS_REG *)LVDS_BASE_ADDR;
	display->trsm_ops = &lvds_drv_ops;
}

