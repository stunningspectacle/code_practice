/*
* (C) Copyright 2014-2015 Rockchip Electronics
*
* All rights reserved.
*
* Contents: This header file contains the register definitions and types
* for the vop hardware driver.
*/

#ifndef __RK_VOP_REG_H__
#define __RK_VOP_REG_H__

#ifndef BITS
#define BITS(x, bit)            ((x) << (bit))
#endif

#ifndef BITS_MASK
#define BITS_MASK(x, mask, bit) BITS((x) & (mask), bit)
#endif

#define CALSCALE(x, y)		((((unsigned int)(x - 1)) * 0x1000) / (y - 1))

/* VOP Register Control function */
#define VOP_READ_BIT(addr, msk)	((regbak.addr = vop_reg.addr) & (msk))
#define VOP_WR_REG(addr, val)	vop_reg.addr = (val);	\
				regbak.addr = vop_reg.addr
#define VOP_RD_REG(addr)	(vop_reg.addr)
#define VOP_SET_BIT(addr, msk)	vop_reg.addr = ((regbak.addr) |= (msk))
#define VOP_CLR_BIT(addr, msk)	vop_reg.addr = ((regbak.addr) &= ~(msk))
#define VOP_SET_REGBIT(addr, msk)	vop_reg.addr = ((vop_reg.addr) |= (msk))
#define VOP_MSK_REG(addr, msk, val)	(regbak.addr) &= ~(msk);	\
					vop_reg.addr = (regbak.addr |= (val))

#define VOP_REG_CFG_DONE()	VOP_WR_REG(VOP_REG_CFG_DONE, 0x01)

/********************************************************************
**                          Definition of Structure                                *
********************************************************************/
typedef unsigned int regu32;

typedef volatile struct tagVOP_REG
{
	/* offset 0x00~0x94 */
	regu32 VOP_SYS_CTRL;	/* 0x00 system control register */
	regu32 VOP_DSP_CTRL0;	/* 0x04 display control register 0 */
	regu32 VOP_DSP_CTRL1;	/* 0x08 display control register 1 */
	regu32 RESERVED;	/* 0x0c */
	regu32 VOP_INT_STATUS;	/* 0x10 Interrupt ctrl and status register */
	regu32 VOP_ALPHA_CTRL;	/* 0x14 alpha ctrl register */
	regu32 VOP_WIN0_COLOR_KEY;	/* 0x18 Win0 colorkey register */
	regu32 VOP_WIN1_COLOR_KEY;	/* 0x1c Win1 colorkey register */
	regu32 VOP_WIN0_YRGB_MST;	/* 0x20 Win0 YRGB memory start address */
	regu32 VOP_WIN0_CBR_MST;	/* 0x24 Win0 Cbr memory start address */
	regu32 VOP_WIN1_VIR;		/* 0x28 Win1 virtual stride */
	regu32 VOP_BUS_INTF_CTRL;	/* 0x2c bus interface ctrl */
	regu32 VOP_WIN0_VIR;		/* 0x30 Win0 virtual stride */
	regu32 VOP_WIN0_ACT_INFO;	/* 0x34 Win0 active window width/height */
	regu32 VOP_WIN0_DSP_INFO;	/* 0x38 Win0 display width/height on panel */
	regu32 VOP_WIN0_DSP_ST;	/* 0x3c Win0 display start point on panel */
	regu32 VOP_WIN0_SCL_FACTOR_YRGB;	/* 0x40 Win0 YRGB scaling factor */
	regu32 VOP_WIN0_SCL_FACTOR_CBR;	/* 0x44 Win0 Cbr scaling factor */
	regu32 VOP_WIN0_SCL_OFFSET;	/* 0x48 Win0 scaling start point offset */
	regu32 VOP_WIN1_YRGB_MST;	/* 0x4c Win1 YRGB memory start address */
	regu32 VOP_WIN1_DSP_INFO;	/* 0x50 Win1 display width/height on panel */
	regu32 VOP_WIN1_DSP_ST;	/* 0x54 Win1 display start point on panel */
	regu32 VOP_HWC_YRGB_MST;	/* 0x58 Win1 YRGB memory start address */
	regu32 VOP_HWC_DSP_ST;	/* 0x5c Win1 display start point on panel */
	regu32 RESERVED1[3];	/* 0x60~0x68 */
	regu32 VOP_DSP_HTOTAL_HS_END;	/* 0x6c Panel scanning horizontal width
					 * and hsync pulse end point */
	regu32 VOP_DSP_HACT_ST_END;	/* 0x70 Panel active horizontal
					 * scanning start/end point */
	regu32 VOP_DSP_VTOTAL_VS_END;	/* 0x74 Panel scanning vertical
					 * height and vsync pulse end point */
	regu32 VOP_DSP_VACT_ST_END;	/* 0x78 Panel active vertical
					 * scanning start/end point */
	regu32 VOP_DSP_VS_ST_END_F1;	/* 0x7c Vertical scanning start point
					 * and vsync pulse end point of
					 * even filed in interlace mode */
	regu32 VOP_DSP_VACT_ST_END_F1;	/* 0x80 Vertical scanning active start/end
					 * point of even filed in interlace mode */
	regu32 VOP_DMA_GATHER;	/* 0x84 AXI read transfer gather ctrl */
	regu32 RESERVED2[2];	/* 0x88~0x8c */
	regu32 VOP_REG_CFG_DONE;	/* 0x90 REGISTER CONFIG FINISH */
	regu32 VOP_VERSION_INFO;	/* 0x94 */
	regu32 RESERVED3[24];	/* 0x98~0xf4 */
	regu32 VOP_MIPI_EDPI_CTRL;	/* 0xf8 eDPI interface control */
} VOP_REG, *pVOP_REG;

/*******************register definition**********************/
/* VOP_SYS_CTRL(0x00): System control */
#define M_WIN0_EN		BITS(1, 0)
#define M_WIN1_EN		BITS(1, 1)
#define M_HWC_EN		BITS(1, 2)
#define M_WIN0_FORMAT		BITS(7, 3)
#define M_WIN1_FORMAT		BITS(7, 6)
#define M_HWC_LUT_EN		BITS(1, 9)
#define M_HWC_SIZE		BITS(1, 10)
#define M_WIN0_RB_SWAP		BITS(1, 15)
#define M_WIN0_ALPHA_SWAP	BITS(1, 16)
#define M_WIN0_Y8_SWAP		BITS(1, 17)
#define M_WIN0_UV_SWAP		BITS(1, 18)
#define M_WIN1_RB_SWAP		BITS(1, 19)
#define M_WIN1_ALPHA_SWAP	BITS(1, 20)
#define M_WIN0_OTSD_DISABLE	BITS(1, 22)
#define M_WIN1_OTSD_DISABLE	BITS(1, 23)
#define M_DMA_BURST_LENGTH	BITS(3, 24)
#define M_HWC_LODAD_EN		BITS(1, 26)
#define M_DSP_LUT_EN		BITS(1, 28)
#define M_DMA_STOP		BITS(1, 29)
#define M_LCDC_STANDBY		BITS(1, 30)
#define M_AUTO_GATING_EN	BITS(1, 31)

#define V_WIN0_EN(x)		BITS_MASK(x, 1, 0)
#define V_WIN1_EN(x)		BITS_MASK(x, 1, 1)
#define V_HWC_EN(x)		BITS_MASK(x, 1, 2)
#define V_WIN0_FORMAT(x)	BITS_MASK(x, 7, 3)
#define V_WIN1_FORMAT(x)	BITS_MASK(x, 7, 6)
#define V_HWC_LUT_EN(x)		BITS_MASK(x, 1, 9)
#define V_HWC_SIZE(x)		BITS_MASK(x, 1, 10)
#define V_WIN0_RB_SWAP(x)	BITS_MASK(x, 1, 15)
#define V_WIN0_ALPHA_SWAP(x)	BITS_MASK(x, 1, 16)
#define V_WIN0_Y8_SWAP(x)	BITS_MASK(x, 1, 17)
#define V_WIN0_UV_SWAP(x)	BITS_MASK(x, 1, 18)
#define V_WIN1_RB_SWAP(x)	BITS_MASK(x, 1, 19)
#define V_WIN1_ALPHA_SWAP(x)	BITS_MASK(x, 1, 20)
#define V_WIN0_OTSD_DISABLE(x)	BITS_MASK(x, 1, 22)
#define V_WIN1_OTSD_DISABLE(x)	BITS_MASK(x, 1, 23)
#define V_DMA_BURST_LENGTH(x)	BITS_MASK(x, 3, 24)
#define V_HWC_LODAD_EN(x)	BITS_MASK(x, 1, 26)
#define V_DSP_LUT_EN(x)		BITS_MASK(x, 1, 28)
#define V_DMA_STOP(x)		BITS_MASK(x, 1, 29)
#define V_LCDC_STANDBY(x)	BITS_MASK(x, 1, 30)
#define V_AUTO_GATING_EN(x)	BITS_MASK(x, 1, 31)

/* VOP_DSP_CTRL0(0x04): Display control */
#define M_DSP_OUT_FORMAT	BITS(0x0f, 0)
#define M_HSYNC_POL		BITS(1, 4)
#define M_VSYNC_POL		BITS(1, 5)
#define M_DEN_POL		BITS(1, 6)
#define M_DCLK_POL		BITS(1, 7)
#define M_WIN0_TOP		BITS(1, 8)
#define M_DITHER_UP_EN		BITS(1, 9)
#define M_DITHER_DOWN_MODE	BITS(1, 10)
#define M_DITHER_DOWN_EN	BITS(1, 11)
#define M_INTERLACE_DSP_EN	BITS(1, 12)
#define M_INTERLACE_FIELD_POL	BITS(1, 13)
#define M_WIN0_INTERLACE_EN	BITS(1, 14)
#define M_WIN1_INTERLACE_EN	BITS(1, 15)
#define M_WIN0_YRGB_DEFLICK_EN	BITS(1, 16)
#define M_WIN0_CBR_DEFLICK_EN	BITS(1, 17)
#define M_WIN0_ALPHA_MODE	BITS(1, 18)
#define M_WIN1_ALPHA_MODE	BITS(1, 19)
#define M_WIN0_CSC_MODE		BITS(3, 20)
#define M_WIN1_CSC_MODE		BITS(1, 22)
#define M_WIN0_YUV_CLIP		BITS(1, 23)
#define M_DSP_CCIR656_AVG	BITS(1, 24)
#define M_DITHER_DOWN_SEL	BITS(1, 27)
#define M_HWC_ALPHA_MODE	BITS(1, 28)
#define M_ALPHA_MODE_SEL0	BITS(1, 29)
#define M_ALPHA_MODE_SEL1	BITS(1, 30)
#define M_SW_OVERLAY_MODE	BITS(1, 31)

#define V_DSP_OUT_FORMAT(x)	BITS_MASK(x, 0x0f, 0)
#define V_HSYNC_POL(x)		BITS_MASK(x, 1, 4)
#define V_VSYNC_POL(x)		BITS_MASK(x, 1, 5)
#define V_DEN_POL(x)		BITS_MASK(x, 1, 6)
#define V_DCLK_POL(x)		BITS_MASK(x, 1, 7)
#define V_WIN0_TOP(x)		BITS_MASK(x, 1, 8)
#define V_DITHER_UP_EN(x)	BITS_MASK(x, 1, 9)
#define V_DITHER_DOWN_MODE(x)	BITS_MASK(x, 1, 10)
#define V_DITHER_DOWN_EN(x)	BITS_MASK(x, 1, 11)
#define V_INTERLACE_DSP_EN(x)	BITS_MASK(x, 1, 12)
#define V_INTERLACE_FIELD_POL(x)	BITS_MASK(x, 1, 13)
#define V_WIN0_INTERLACE_EN(x)		BITS_MASK(x, 1, 14)
#define V_WIN1_INTERLACE_EN(x)		BITS_MASK(x, 1, 15)
#define V_WIN0_YRGB_DEFLICK_EN(x)	BITS_MASK(x, 1, 16)
#define V_WIN0_CBR_DEFLICK_EN(x)	BITS_MASK(x, 1, 17)
#define V_WIN0_ALPHA_MODE(x)	BITS_MASK(x, 1, 18)
#define V_WIN1_ALPHA_MODE(x)	BITS_MASK(x, 1, 19)
#define V_WIN0_CSC_MODE(x)	BITS_MASK(x, 3, 20)
#define V_WIN1_CSC_MODE(x)	BITS_MASK(x, 1, 22)
#define V_WIN0_YUV_CLIP(x)	BITS_MASK(x, 1, 23)
#define V_DSP_CCIR656_AVG(x)	BITS_MASK(x, 1, 24)
#define V_DITHER_DOWN_SEL(x)	BITS_MASK(x, 1, 27)
#define V_HWC_ALPHA_MODE(x)	BITS_MASK(x, 1, 28)
#define V_ALPHA_MODE_SEL0(x)	BITS_MASK(x, 1, 29)
#define V_ALPHA_MODE_SEL1(x)	BITS_MASK(x, 1, 30)
#define V_SW_OVERLAY_MODE(x)	BITS_MASK(x, 1, 31)

/* VOP_DSP_CTRL1(0x08): Display control and backgrounp setting */
#define M_BG_COLOR		BITS(0xffffff, 0)
#define M_BG_B			BITS(0xff, 0)
#define M_BG_G			BITS(0xff, 8)
#define M_BG_R			BITS(0xff, 16)
#define M_BLANK_EN		BITS(1, 24)
#define M_BLACK_EN		BITS(1, 25)
#define M_DSP_BG_SWAP		BITS(1, 26)
#define M_DSP_RB_SWAP		BITS(1, 27)
#define M_DSP_RG_SWAP		BITS(1, 28)
#define M_DSP_OUT_ZERO		BITS(1, 31)

#define V_BG_COLOR(x)		BITS_MASK(x, 0xffffff, 0)
#define V_BG_B(x)		BITS_MASK(x, 0xff, 0)
#define V_BG_G(x)		BITS_MASK(x, 0xff, 8)
#define V_BG_R(x)		BITS_MASK(x, 0xff, 16)
#define V_BLANK_EN(x)		BITS_MASK(x, 1, 24)
#define V_BLACK_EN(x)		BITS_MASK(x, 1, 25)
#define V_DSP_BG_SWAP(x)	BITS_MASK(x, 1, 26)
#define V_DSP_RB_SWAP(x)	BITS_MASK(x, 1, 27)
#define V_DSP_RG_SWAP(x)	BITS_MASK(x, 1, 28)
#define V_DSP_OUT_ZERO(x)	BITS_MASK(x, 1, 31)

/* VOP_INT_MASK(0x0c): frame start interrupt mask */
#define M_FS_MASK_EN		BITS(1, 3)
#define V_FS_MASK_EN(x)		BITS_MASK(x, 1, 3)

/* VOP_INT_STATUS(0x10): Interrupt control and status */
#define M_HS_INT_STA		BITS(1, 0)
#define M_FS_INT_STA		BITS(1, 1)
#define M_LF_INT_STA		BITS(1, 2)
#define M_BUS_ERR_INT_STA	BITS(1, 3)
#define M_HS_INT_EN		BITS(1, 4)
#define M_FS_INT_EN		BITS(1, 5)
#define M_LF_INT_EN		BITS(1, 6)
#define M_BUS_ERR_INT_EN	BITS(1, 7)
#define M_HS_INT_CLEAR		BITS(1, 8)
#define M_FS_INT_CLEAR		BITS(1, 9)
#define M_LF_INT_CLEAR		BITS(1, 10)
#define M_BUS_ERR_INT_CLEAR	BITS(1, 11)
#define M_LF_INT_NUM		BITS(0xfff, 12)
#define M_WIN0_EMPTY_INT_EN	BITS(1, 24)
#define M_WIN1_EMPTY_INT_EN	BITS(1, 25)
#define M_WIN0_EMPTY_INT_CLEAR	BITS(1, 26)
#define M_WIN1_EMPTY_INT_CLEAR	BITS(1, 27)
#define M_WIN0_EMPTY_INT_STA	BITS(1, 28)
#define M_WIN1_EMPTY_INT_STA	BITS(1, 29)
#define M_FS_RAW_STA		BITS(1, 30)
#define M_LF_RAW_STA		BITS(1, 31)

#define V_HS_INT_EN(x)		BITS_MASK(x, 1, 4)
#define V_FS_INT_EN(x)		BITS_MASK(x, 1, 5)
#define V_LF_INT_EN(x)		BITS_MASK(x, 1, 6)
#define V_BUS_ERR_INT_EN(x)	BITS_MASK(x, 1, 7)
#define V_HS_INT_CLEAR(x)	BITS_MASK(x, 1, 8)
#define V_FS_INT_CLEAR(x)	BITS_MASK(x, 1, 9)
#define V_LF_INT_CLEAR(x)	BITS_MASK(x, 1, 10)
#define V_BUS_ERR_INT_CLEAR(x)	BITS_MASK(x, 1, 11)
#define V_LF_INT_NUM(x)		BITS_MASK(x, 0xfff, 12)
#define V_WIN0_EMPTY_INT_EN(x)	BITS_MASK(x, 1, 24)
#define V_WIN1_EMPTY_INT_EN(x)	BITS_MASK(x, 1, 25)
#define V_WIN0_EMPTY_INT_CLEAR(x)	BITS_MASK(x, 1, 26)
#define V_WIN1_EMPTY_INT_CLEAR(x)	BITS_MASK(x, 1, 27)

/* VOP_ALPHA_CTRL(0x14): Alpha Blending control */
#define M_WIN0_ALPHA_EN		BITS(1, 0)
#define M_WIN1_ALPHA_EN		BITS(1, 1)
#define M_HWC_ALPAH_EN		BITS(1, 2)
#define M_WIN0_ALPHA_VAL	BITS(0xff, 4)
#define M_WIN1_ALPHA_VAL	BITS(0xff, 12)
#define M_HWC_ALPAH_VAL		BITS(0xff, 20)

#define V_WIN0_ALPHA_EN(x)	BITS_MASK(x, 1, 0)
#define V_WIN1_ALPHA_EN(x)	BITS_MASK(x, 1, 1)
#define V_HWC_ALPAH_EN(x)	BITS_MASK(x, 1, 2)
#define V_WIN0_ALPHA_VAL(x)	BITS_MASK(x, 0xff, 4)
#define V_WIN1_ALPHA_VAL(x)	BITS_MASK(x, 0xff, 12)
#define V_HWC_ALPAH_VAL(x)	BITS_MASK(x, 0xff, 20)

/*
 * VOP_WIN0_COLOR_KEY(0x18)
 * VOP_WIN1_COLOR_KEY(0x1c)
 * Color key setting
 */
#define M_COLOR_KEY_VAL		BITS(0xffffff, 0)
#define M_COLOR_KEY_EN		BITS(1, 24)

#define V_COLOR_KEY_VAL(x)	BITS_MASK(x, 0xffffff, 0)
#define V_COLOR_KEY_EN(x)	BITS_MASK(x, 1, 24)

/*
 * Layer Control Registers
 */

/*
 * VOP_WIN1_VIR(0x28)
 * VOP_WIN0_VIR(0x30)
 * virtual stride
 */
#define M_YRGB_VIR		BITS(0x1fff, 0)
#define M_CBBR_VIR		BITS(0x1fff, 16)
#define V_YRGB_VIR(x)		BITS_MASK(x, 0x1fff, 0)
#define V_CBBR_VIR(x)		BITS_MASK(x, 0x1fff, 16)

#define V_ARGB888_VIRWIDTH(x)	BITS_MASK(x, 0x1fff, 0)
#define V_RGB888_VIRWIDTH(x)	BITS_MASK(((x*3)>>2)+((x)%3), 0x1fff, 0)
#define V_RGB565_VIRWIDTH(x)	BITS_MASK(DIV_ROUND_UP(x, 2), 0x1fff, 0)
#define V_YUV_VIRWIDTH(x)	BITS_MASK(DIV_ROUND_UP(x, 4), 0x1fff, 0)
#define V_CBCR_VIR(x)		BITS_MASK(x, 0x1fff, 16)

/*
 * VOP_WIN0_ACT_INFO(0x34)
 * VOP_WIN1_ACT_INFO(0xb4)
 * active window width/height
 */
#define M_ACT_WIDTH		BITS(0x1fff, 0)
#define M_ACT_HEIGHT		BITS(0x1fff, 16)
#define V_ACT_WIDTH(x)		BITS_MASK(x - 1, 0x1fff, 0)
#define V_ACT_HEIGHT(x)		BITS_MASK(x - 1, 0x1fff, 16)

/*
 * VOP_WIN0_DSP_INFO(0x38)
 * VOP_WIN1_DSP_INFO(0x50)
 * display width/height on panel
 */
#define M_DSP_WIDTH		BITS(0x7ff, 0)
#define M_DSP_HEIGHT		BITS(0x7ff, 16)
#define V_DSP_WIDTH(x)		BITS_MASK(x - 1, 0x7ff, 0)
#define V_DSP_HEIGHT(x)		BITS_MASK(x - 1, 0x7ff, 16)

/* VOP_WIN0_DSP_ST(0x3c)
* VOP_WIN1_DSP_ST(0x54)
* VOP_HWC_DSP_ST(0x5c)
* display start point on panel
*/
#define M_DSP_STX		BITS(0xfff, 0)
#define M_DSP_STY		BITS(0xfff, 16)
#define V_DSP_STX(x)		BITS_MASK(x, 0xfff, 0)
#define V_DSP_STY(x)		BITS_MASK(x, 0xfff, 16)

/*
 * VOP_WIN0_SCL_FACTOR_YRGB(0x40)
 * VOP_WIN0_SCL_FACTOR_CBR(0x44)
 * scaling factor
 */
#define M_X_SCL_FACTOR		BITS(0xffff, 0)
#define M_Y_SCL_FACTOR		BITS(0xffff, 16)
#define V_X_SCL_FACTOR(x)	BITS_MASK(x, 0xffff, 0)
#define V_Y_SCL_FACTOR(x)	BITS_MASK(x, 0xffff, 16)

/* VOP_WIN0_SCL_OFFSET(0x48): scaling start point offset */
#define M_X_SCL_OFFSET_YRGB(x)	BITS_MASK(x, 0xff, 0)
#define M_X_SCL_OFFSET_CBR(x)	BITS_MASK(x, 0xff, 8)
#define M_Y_SCL_OFFSET_YRGB(x)	BITS_MASK(x, 0xff, 0)
#define M_Y_SCL_OFFSET_CBR(x)	BITS_MASK(x, 0xff, 8)
#define V_X_SCL_OFFSET_YRGB(x)	BITS_MASK(x, 0xff, 0)
#define V_X_SCL_OFFSET_CBR(x)	BITS_MASK(x, 0xff, 8)
#define V_Y_SCL_OFFSET_YRGB(x)	BITS_MASK(x, 0xff, 0)
#define V_Y_SCL_OFFSET_CBR(x)	BITS_MASK(x, 0xff, 8)

/*
 * Display timing Registers
 */
/*
 * VOP_DSP_HTOTAL_HS_END(0x6c)
 * Panel scanning horizontal width and hsync pulse end point
 */
#define V_HSYNC(x)		BITS_MASK(x, 0xfff, 0)	/* hsync pulse width */
#define V_HORPRD(x)		BITS_MASK(x, 0xfff, 16)	/* horizontal period */

/*
 * VOP_DSP_HACT_ST_END(0x70)
 * Panel active horizontal scanning start point and end point
 */
#define V_HAEP(x)		BITS_MASK(x, 0xfff, 0)
#define V_HASP(x)		BITS_MASK(x, 0xfff, 16)

/*
 * VOP_DSP_VTOTAL_VS_END(0x74)
 * Panel scanning vertical height and vsync pulse end point
 */
#define V_VSYNC(x)		BITS_MASK(x, 0xfff, 0)
#define V_VERPRD(x)		BITS_MASK(x, 0xfff, 16)

/*
 * VOP_DSP_VACT_ST_END(0x78)
 * Panel active vertical scanning start point and end point
 */
#define V_VAEP(x)		BITS_MASK(x, 0xfff, 0)
#define V_VASP(x)		BITS_MASK(x, 0xfff, 16)

/*
 * VOP_DSP_VS_ST_END_F1(0x7c)
 * Vertical scanning start point and vsync pulse
 * end point of even filed in interlace mode
 */
#define V_VSYNC_END_F1(x)	BITS_MASK(x, 0xfff, 0)
#define V_VSYNC_ST_F1(x)	BITS_MASK(x, 0xfff, 16)

/*
 * VOP_DSP_VACT_ST_END_F1(0x80)
 * Vertical scanning active start point and end
 * point of even filed in interlace mode
 */
#define V_VAEP_F1(x)		BITS_MASK(x, 0xfff, 0)
#define V_VASP_F1(x)		BITS_MASK(x, 0xfff, 16)

/*
 * Bus Control Registers
 */
/* VOP_BUS_INTF_CTRL(0x2c): Bus interface control */
#define M_IO_PAD_CLK			BITS(1, 31)
#define M_MIPI_DCLK_INVERT              BITS(1, 29)
#define M_MIPI_DCLK_EN                  BITS(1, 28)
#define M_LVDS_DCLK_INVERT              BITS(1, 27)
#define M_LVDS_DCLK_EN                  BITS(1, 26)
#define M_RGB_DCLK_INVERT               BITS(1, 25)
#define M_RGB_DCLK_EN                   BITS(1, 24)
#define M_AXI_OUTSTANDING_MAX_NUM	BITS(0x1f, 12)
#define M_AXI_MAX_OUTSTANDING_EN	BITS(1, 11)
#define M_MMU_EN			BITS(1, 10)
#define M_NOC_HURRY_THRESHOLD		BITS(0xf, 6)
#define M_NOC_HURRY_VALUE		BITS(3, 4)
#define M_NOC_HURRY_EN			BITS(1, 3)
#define M_NOC_QOS_VALUE			BITS(3, 1)
#define M_NOC_QOS_EN			BITS(1, 0)

#define V_IO_PAD_CLK(x)			BITS_MASK(x, 1, 31)
#define V_CORE_CLK_DIV_EN(x)		BITS_MASK(x, 1, 30)
#define V_MIPI_DCLK_INVERT(x)           BITS_MASK(x, 1, 29)
#define V_MIPI_DCLK_EN(x)               BITS_MASK(x, 1, 28)
#define V_LVDS_DCLK_INVERT(x)           BITS_MASK(x, 1, 27)
#define V_LVDS_DCLK_EN(x)               BITS_MASK(x, 1, 26)
#define V_RGB_DCLK_INVERT(x)            BITS_MASK(x, 1, 25)
#define V_RGB_DCLK_EN(x)                BITS_MASK(x, 1, 24)
#define V_AXI_OUTSTANDING_MAX_NUM(x)	BITS_MASK(x, 0x1f, 12)
#define V_AXI_MAX_OUTSTANDING_EN(x)	BITS_MASK(x, 1, 11)
#define V_MMU_EN(x)			BITS_MASK(x, 1, 10)
#define V_NOC_HURRY_THRESHOLD(x)	BITS_MASK(x, 0xf, 6)
#define V_NOC_HURRY_VALUE(x)		BITS_MASK(x, 3, 4)
#define V_NOC_HURRY_EN(x)		BITS_MASK(x, 1, 3)
#define V_NOC_QOS_VALUE(x)		BITS_MASK(x, 3, 1)
#define V_NOC_QOS_EN(x)			BITS_MASK(x, 1, 0)

/* VOP_DMA_GATHER(0x84): AXI read transfer gather setting */
#define M_WIN1_AXI_GATHER_NUM		BITS(0xf, 12)
#define M_WIN0_CBCR_AXI_GATHER_NUM	BITS(0x7, 8)
#define M_WIN0_YRGB_AXI_GATHER_NUM	BITS(0xf, 4)
#define M_WIN1_AXI_GAHTER_EN		BITS(1, 2)
#define M_WIN0_CBCR_AXI_GATHER_EN	BITS(1, 1)
#define M_WIN0_YRGB_AXI_GATHER_EN	BITS(1, 0)

#define V_WIN1_AXI_GATHER_NUM(x)	BITS_MASK(x, 0xf, 12)
#define V_WIN0_CBCR_AXI_GATHER_NUM(x)	BITS_MASK(x, 0x7, 8)
#define V_WIN0_YRGB_AXI_GATHER_NUM(x)	BITS_MASK(x, 0xf, 4)
#define V_WIN1_AXI_GAHTER_EN(x)		BITS_MASK(x, 1, 2)
#define V_WIN0_CBCR_AXI_GATHER_EN(x)	BITS_MASK(x, 1, 1)
#define V_WIN0_YRGB_AXI_GATHER_EN(x)	BITS_MASK(x, 1, 0)

/*
 * Transmitter interface Control Registers
 */
/* VOP_MIPI_EDPI_CTRL(0xf8): eDPI interface control for MIPI_DSI */
#define M_EDPI_HALT_EN		BITS(1, 0)
#define M_EDPI_TEAR_EN		BITS(1, 1)
#define M_EDPI_CTRL_MODE	BITS(1, 2)
#define M_EDPI_FRM_ST		BITS(1, 3)
#define M_DSP_HOLD_STATUS	BITS(1, 4)

#define V_EDPI_HALT_EN(x)	BITS_MASK(x, 1, 0)
#define V_EDPI_TEAR_EN(x)	BITS_MASK(x, 1, 1)
#define V_EDPI_CTRL_MODE(x)	BITS_MASK(x, 1, 2)
#define V_EDPI_FRM_ST(x)	BITS_MASK(x, 1, 3)
#define V_DSP_HOLD_STATUS(x)	BITS_MASK(x, 1, 4)

#endif
