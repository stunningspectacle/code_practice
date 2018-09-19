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

#include "scu_reg.h"
#include "dsi_hwregs.h"
#include "bl-vop-display.h"
#include "../rk_disp.h"

/* definition */
//#define DSI_DBG

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

#define BYTES_TO_PIXELS(bytes, bpp) (DIV_ROUND_CLOSEST(bytes * 8, bpp))
#define PIXELS_TO_BYTES(pixels, bpp) (DIV_ROUND_CLOSEST(pixels * bpp, 8))

#define NSEC_PER_SEC       1000000000L

#ifdef DSI_DBG
#define DSI_DBG1(x...) TRACE_VOP_PRINTF(x)
#define DSI_DBG2(x...) TRACE_VOP_PRINTF(x)
#define DSI_DBG3(x...) TRACE_VOP_PRINTF(x)
#else
#define DSI_DBG1(x...)
#define DSI_DBG2(x...)
#define DSI_DBG3(x...)
#endif

#define DSI_ERR(x...) TRACE_VOP_PRINTF(x)

#define DSI_CFG_DATA_PIX	0
#define DSI_CFG_DATA_DAT	1
#define DSI_CFG_SOURCE_DPI 0
#define DSI_CFG_SOURCE_TXD 1

static int g_dsi_cfg_eot = 1; /* default: eot */
static int g_dsi_cfg_gate = 0; /* default: no gate */

#define DSI_CFG_COMMON(_en_) (\
	BITFLDS(EXR_DSI_CFG_GATE,	1) |\
	BITFLDS(EXR_DSI_CFG_EOT,	1) |\
	BITFLDS(EXR_DSI_CFG_TURN,	0) |\
	BITFLDS(EXR_DSI_CFG_VALID,	0) |\
	BITFLDS(EXR_DSI_CFG_STP,	0) |\
	BITFLDS(EXR_DSI_CFG_ULPS,	0) |\
	BITFLDS(EXR_DSI_CFG_ID,	0) |\
	BITFLDS(EXR_DSI_CFG_VSYNC,	1) |\
	BITFLDS(EXR_DSI_CFG_PSYNC,	1) |\
	BITFLDS(EXR_DSI_CFG_EN,	_en_) \
	)

#define DSI_CFG_OFF(_mode_) (\
		DSI_CFG_COMMON(0) |\
		BITFLDS(EXR_DSI_CFG_LANES, 0) |\
		BITFLDS(EXR_DSI_CFG_LP, 0) |\
		BITFLDS(EXR_DSI_CFG_MODE, _mode_)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))

#define DSI_CFG_INIT(_nlanes_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (_nlanes_-1)) |\
		BITFLDS(EXR_DSI_CFG_LP, 0) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))
/* need_to_modify
#define DSI_CFG_RX_LP_DATA(_nlanes_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (_nlanes_-1)) |\
		BITFLDS(EXR_DSI_CFG_TX, 1) |\
		BITFLDS(EXR_DSI_CFG_LP, 1) |\
		BITFLDS(EXR_DSI_CFG_TURN, 1) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))
*/
#define DSI_CFG_RX_LP_STP(_stp_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, 0) |\
		BITFLDS(EXR_DSI_CFG_TX, 0) |\
		BITFLDS(EXR_DSI_CFG_LP, 1) |\
		BITFLDS(EXR_DSI_CFG_STP, _stp_) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))

#define DSI_CFG_TX_LP_DATA(_nlanes_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (0)) |\
		BITFLDS(EXR_DSI_CFG_LP, 1) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))

/* need_to_modify
#define DSI_CFG_TX_HS_DATA_ACK(_nlanes_) (\
		DSICFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (_nlanes_-1)) |\
		BITFLDS(EXR_DSI_CFG_TX, 1) |\
		BITFLDS(EXR_DSI_CFG_LP, 0) |\
		BITFLDS(EXR_DSI_CFG_TURN, 1) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT))|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))
*/

#define DSI_CFG_TX_HS_DATA(_nlanes_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (_nlanes_-1)) |\
		BITFLDS(EXR_DSI_CFG_LP, 0) |\
		BITFLDS(EXR_DSI_CFG_MODE, DSI_CMD)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_DAT)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_TXD))

#define DSI_CFG_TX_HS_PIXEL(_nlanes_, _mode_) (\
		DSI_CFG_COMMON(1) |\
		BITFLDS(EXR_DSI_CFG_LANES, (_nlanes_-1)) |\
		BITFLDS(EXR_DSI_CFG_TX, 1) |\
		BITFLDS(EXR_DSI_CFG_LP, 0) |\
		BITFLDS(EXR_DSI_CFG_MODE, _mode_)|\
		BITFLDS(EXR_DSI_CFG_DATA, DSI_CFG_DATA_PIX)|\
		BITFLDS(EXR_DSI_CFG_SOURCE, DSI_CFG_SOURCE_DPI))

#define DSI_RATE_N_MAX	(0xFF)
#define DSI_RATE_M_MAX	(0xF)
#define DSI_RATE_MAX	(1000000000)
#define DSI_REF_CLK_KHZ (26000)
#define DSI_RATE_MIN	(160000000)

#define DSI_RATE(n, m) (DSI_REF_CLK_KHZ * (n + 1) / (m + 1) * 1000)
#define DSI_RATE_OVERHEAD(r)    (r / 10 * 11)

#define TLPX_NS 50

static bl_display_cbs_t *disp_ctrl_ops;

void dsi_wr32tofifo(struct vop_display *display, unsigned int data)
{
	/* Write data to the DIF FIFO */
	dsi_write_field(EXR_DSI_TXD, data);
}

int dsi_waitfor_dsidir(struct vop_display *display, unsigned int value)
{
	unsigned int reg = 0xFF;
	int ret = 0;

	while (reg != value)
		dsi_read_field(EXR_DSI_STAT_DSI_DIR, &reg);

	return ret;
}

/**
 * Common TX functions
 */

static int dsi_completion_fin_timeout_ms(int ms)
{
	unsigned int dsi_irq_status = 0;
	unsigned int dsi_irq_clear = 0;

	while (ms-- > 0) {
		dsi_read_field(EXR_DSI_RIS, &dsi_irq_status);
		if (dsi_irq_status & DSI_IRQ_ERR_DSIFIN) {
			/* clear treated handled interrupts */
			dsi_irq_clear |= DSI_IRQ_ERR_DSIFIN;
			dsi_write_field(EXR_DSI_ICR, dsi_irq_clear);
			break;
		}
		disp_ctrl_ops->bl_sleep(1);
	}

	if (ms > 0)
		return 1;
	else
		return 0;
}

static void dsi_wait_status(unsigned int reg, unsigned int value,
			    unsigned int mask, unsigned int delay,
			    unsigned int count)
{
	unsigned int read_value = 0;

	do {
		dsi_read_field(reg, &read_value);
		if ((read_value & mask) == value)
			break;

		if (delay)
			disp_ctrl_ops->bl_sleep(delay);
	} while (--count);
	if (0 == count)
		DSI_ERR("dsi_wait_status reg 0x%x fail", reg);
}

void dsi_mipidsi_send_short_packet(struct vop_display *display,
					  struct display_msg *msg,
					  unsigned int dsicfg)
{
	unsigned int dsihead =
		BITFLDS(EXR_DSI_HEAD_WCNT, msg->header) |
		BITFLDS(EXR_DSI_HEAD_HEADER, msg->type);

	if (msg->length) {
		unsigned char *data_msg = msg->datas;

		dsihead |= data_msg[0]<<16;
	}
	DSI_DBG2("dsi short pkt: (head:0x%08x cfg:0x%08x",
		 dsihead, dsicfg);

	dsi_write_field(EXR_DSI_VID3,
			BITFLDS(EXR_DSI_VID3_PIXEL_PACKETS, 1));

	dsi_write_field(EXR_DSI_HEAD, dsihead);
	dsi_write_field(EXR_DSI_CFG,
			dsicfg | BITFLDS(EXR_DSI_CFG_HEAD_LAT, 1));
	dsi_write_field(EXR_DSI_CFG, dsicfg);
	dsi_write_field(EXR_DSI_CFG,
			dsicfg | BITFLDS(EXR_DSI_CFG_TX, 1) |
			BITFLDS(EXR_DSI_CFG_CFG_LAT, 1));
	dsi_write_field(EXR_DSI_CFG,
			dsicfg | BITFLDS(EXR_DSI_CFG_TX, 1));
}

void dsi_mipidsi_send_long_packet_dma(struct vop_display *display,
					     struct display_msg *msg,
					     unsigned int dsicfg)
{
	unsigned char *data_msg = msg->datas;
	unsigned int length = msg->length;
	unsigned int dsihead =
		BITFLDS(EXR_DSI_HEAD_CMD, msg->header) |
		BITFLDS(EXR_DSI_HEAD_WCNT, msg->length+1) |
		BITFLDS(EXR_DSI_HEAD_HEADER, msg->type);

	DSI_DBG2("dsi long dma pkt: wcnt:0x%04x (head:0x%08x cfg:0x%08x)",
		 msg->length+1, dsihead, dsicfg);

	dsi_write_field(EXR_DSI_VID3,
			BITFLDS(EXR_DSI_VID3_PIXEL_PACKETS, 1));
	dsi_write_field(EXR_DSI_VID6,
			BITFLDS(EXR_DSI_VID6_LAST_PIXEL, msg->length));
	dsi_write_field(EXR_DSI_TPS_CTRL,
			BITFLDS(EXR_DSI_TPS_CTRL_TPS, msg->length));

	while (length > 0) {
		int j = 0;
		unsigned int reg = 0;

		for (j = 0; j < 4 && length; j++) {
			length--;
			reg |= (*data_msg++)<<(j*8);
		}

		dsi_wr32tofifo(display, reg);
		DSI_DBG2("payload 0x%08x\n", reg);
	}

	dsi_write_field(EXR_DSI_HEAD, dsihead);
	dsi_write_field(EXR_DSI_CFG,
			dsicfg | BITFLDS(EXR_DSI_CFG_HEAD_LAT, 1));
	dsi_write_field(EXR_DSI_CFG, dsicfg);
	dsi_write_field(EXR_DSI_CFG,
			dsicfg | BITFLDS(EXR_DSI_CFG_TX, 1) |
			BITFLDS(EXR_DSI_CFG_CFG_LAT, 1));
	dsi_write_field(EXR_DSI_CFG, dsicfg | BITFLDS(EXR_DSI_CFG_TX, 1));
}

int dsi_mipidsi_force_ownership(struct vop_display *display)
{
	dsi_write_field(EXR_DSI_CFG, DSI_CFG_RX_LP_STP(1));
	dsi_write_field(EXR_DSI_CFG, DSI_CFG_RX_LP_STP(0));
	return 0;
}

int dsi_mipidsi_ack_wait(struct vop_display *display)
{
	unsigned int nwords, nbytes;
	unsigned int data = 0;

	dsi_waitfor_dsidir(display, 0);

	dsi_read_field(EXR_DSI_FIFO_STAT_RXFFS, &nwords);

	if (nwords) {
		dsi_read_field(EXR_DSI_RPS_STAT, &nbytes);
		while (nwords) {
			dsi_read_field(EXR_DSI_RXD, &data);
			nwords--;
		}
		DSI_ERR("error returned (0x%x)\n", data);
	}

	return data;
}

void dsi_send_cmd(struct vop_display *display,
			 struct display_msg *msg,
			 bl_display_cbs_t *bl_disp_cb)
{
	int ret = 0;
	unsigned int dsicfg;

	if (msg->flags & LCD_MSG_LP)
		dsicfg = DSI_CFG_TX_LP_DATA(1);
	else
		dsicfg = DSI_CFG_TX_HS_DATA(display->dif.u.dsi.nblanes);

	if (msg->length <= 1)
		dsi_mipidsi_send_short_packet(display, msg, dsicfg);
	else
		dsi_mipidsi_send_long_packet_dma(display, msg, dsicfg);

	DSI_DBG2("wait for eoc\n");
	ret = dsi_completion_fin_timeout_ms(100);

	if (!ret) {
		DSI_ERR("dsifin interrupt timedout");
	} else {
		DSI_DBG2("eoc received\n");
#ifdef USE_DSI_ACKNOWLEDGE
		ret = dsi_mipidsi_ack_wait(display);
		dsi_mipidsi_force_ownership(display);
#endif
	}

	if (msg->delay)
		bl_disp_cb->bl_sleep(msg->delay);
}

int dsi_get_rate(struct vop_display *display)
{
	return DSI_RATE(display->dif.u.dsi.n, display->dif.u.dsi.m);
}

int dsi_get_bllp(struct vop_display *display,
			int nlines, int bytes, int clk,
			int fps, int bitrate, int nlanes,
			int *bllp_time, int *line_time)
{
	/* bits per frames */
	unsigned int bitpframe = bytes * nlines * 8;
	/* maximum framerate */
	unsigned int maxfrate = bitrate * nlanes / bitpframe;
	/* shortest line time */
	unsigned int slt = NSEC_PER_SEC / maxfrate / nlines;
	/* target line time */
	unsigned int tlt = NSEC_PER_SEC / (fps + 1) / nlines;
	/* clock cycle duration in ps */
	unsigned int clk_time = 1000000 / (clk/1000000);

	if (display->dif.u.dsi.video_mode == DSI_BURST)
		*bllp_time = ((tlt - slt) * 1000) / clk_time;
	else
		*bllp_time = 0;

	*line_time = tlt * 1000 / clk_time;

	DSI_DBG2("%d bytes / %d lines\n", bytes, nlines);
	DSI_DBG2("bits / frame = %d bits\n", bitpframe);
	DSI_DBG2("fps target %d fps (max=%d)\n", fps, maxfrate);
	DSI_DBG2("active time  = %d ns\n", slt);
	DSI_DBG2("target time  = %d ns\n", tlt);
	DSI_DBG2("clock cycle  = %d\n", clk_time);
	DSI_DBG2("bllp_time 0x%08x(%d)\n", *bllp_time, *bllp_time);
	DSI_DBG2("line_time 0x%08x(%d)\n", *line_time, *line_time);

	if (fps >= maxfrate) {
		DSI_ERR("target framerate(%d) cannot be reached, maxfrate %d",
				fps, maxfrate);
		*bllp_time = 0;
		return 0;
	}

	return 0;
}

int dsi_configure_video_mode(struct vop_display *display,
				    int stride, int nlines)
{
	unsigned int vid0, vid1, vid2, vid3, vid4, vid5, vid6;
	struct vop_display_if_mipi_dsi *dif = &display->dif.u.dsi;

	if (dif->mode != DSI_VIDEO) {
		DSI_ERR("%s: not video mode\n", __func__);
		return -1;
	}

	dsi_get_bllp(display,
		     nlines + dif->vfp + dif->vbp + dif->vsa,
		     stride + dif->hfp + dif->hbp,
		     dif->dc_clk_rate,
		     display->fps,
		     dsi_get_rate(display),
		     dif->nblanes, &dif->bllp_time, &dif->line_time);

	vid0 =	BITFLDS(EXR_DSI_VID0_HFP, (!!dif->hfp))|
		BITFLDS(EXR_DSI_VID0_HBP, (!!dif->hbp))|
		BITFLDS(EXR_DSI_VID0_HSA, (!!dif->hsa))|
		BITFLDS(EXR_DSI_VID0_HFP_LP, dif->hfp_lp)|
		BITFLDS(EXR_DSI_VID0_HBP_LP, dif->hbp_lp)|
		BITFLDS(EXR_DSI_VID0_HSA_LP, dif->hsa_lp)|
		BITFLDS(EXR_DSI_VID0_HFP_BYTES, dif->hfp)|
		BITFLDS(EXR_DSI_VID0_HBP_BYTES, dif->hbp)|
		BITFLDS(EXR_DSI_VID0_HSA_BYTES, dif->hsa);

	vid1 =	BITFLDS(EXR_DSI_VID1_VACT_LINES, nlines)|
		BITFLDS(EXR_DSI_VID1_MODE, dif->video_mode)|
		BITFLDS(EXR_DSI_VID1_ID, dif->id)|
		BITFLDS(EXR_DSI_VID1_PIXEL, dif->video_pixel)|
		BITFLDS(EXR_DSI_VID1_FILL_BUFFER_TO, 0xFF);

	vid2 =	BITFLDS(EXR_DSI_VID2_VFP, dif->vfp)|
		BITFLDS(EXR_DSI_VID2_VBP, dif->vbp)|
		BITFLDS(EXR_DSI_VID2_VSA, dif->vsa);

	vid3 =	BITFLDS(EXR_DSI_VID3_PIXEL_BYTES, stride)|
		BITFLDS(EXR_DSI_VID3_PIXEL_PACKETS, 1);

	vid4 =	BITFLDS(EXR_DSI_VID4_BLANK_BYTES, stride)|
		BITFLDS(EXR_DSI_VID4_BLANK_PACKETS, 0);

	vid5 =	BITFLDS(EXR_DSI_VID5_LINE_TIME, dif->line_time)|
		BITFLDS(EXR_DSI_VID5_BLLP_TIME, dif->bllp_time);

	vid6 =	BITFLDS(EXR_DSI_VID6_LAST_BLANK, stride)|
		BITFLDS(EXR_DSI_VID6_LAST_PIXEL, stride);

	TRACE_VOP_PRINTF(
		"MIPI-DSI video:0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x)",
		vid0, vid1, vid2, vid3, vid4, vid5, vid6);

	dsi_write_field(EXR_DSI_VID0, vid0);
	dsi_write_field(EXR_DSI_VID1, vid1);
	dsi_write_field(EXR_DSI_VID2, vid2);
	dsi_write_field(EXR_DSI_VID3, vid3);
	dsi_write_field(EXR_DSI_VID4, vid4);
	dsi_write_field(EXR_DSI_VID5, vid5);
	dsi_write_field(EXR_DSI_VID6, vid6);

	return 0;
}

/* need_to_modify
static int SMS05120496_once;
void dsi_start_video(struct vop_display *display)
{
	struct dsi_display_if_mipi_dsi *dif =
					&display->dif.dsi;
	if (display->dif.dsi.mode != DSI_VIDEO) {
		DSI_DBG2("[dsi]not video mode\n");
		return;
	}
	if (SMS05120496_once) {
		unsigned int dsicfg =
			DSI_CFG_TX_HS_PIXEL(dif->nblanes, dif->mode);
		gra_write_field(pdata, INR_DIF_DSICFG, dsicfg);
		SMS05120496_once--;
	}
}
*/

void dsi_frame_prepare(struct vop_display *display,
			      int stride, int nlines)
{
	struct display_msg *msg;
	unsigned int dsicfg;

	msg = display->msgs_update;

	if (!msg) {
		DSI_DBG2("no msg for update command\n");
		return;
	}
	dsicfg = DSI_CFG_TX_HS_PIXEL(display->dif.u.dsi.nblanes,
				     display->dif.u.dsi.mode);

	if (display->dif.u.dsi.mode == DSI_VIDEO)
		return;
}

/**
 * Callbacks
 */
void dsi_set_phy(struct vop_display *display, int on)
{
	unsigned int phy0 = 0, phy1 = 0, phy2 = 0, phy3 = 0, stat = 0, pllstat;
	struct vop_display_if_mipi_dsi *dif = &display->dif.u.dsi;

	if (!on) {
		phy0 = BITFLDS(EXR_DSI_PHY0_SHARE, 0x0) |
			BITFLDS(EXR_DSI_PHY0_M, 0) |
			BITFLDS(EXR_DSI_PHY0_N, 0xFF) |
			BITFLDS(EXR_DSI_PHY0_POWERUP, dif->pwup) |
			BITFLDS(EXR_DSI_PHY0_CALIB, dif->calib) |
			BITFLDS(EXR_DSI_PHY0_TO_LP_HS_REQ,
				dif->to_lp_hs_req);
	} else {
		phy0 = BITFLDS(EXR_DSI_PHY0_SHARE, 0x0) |
			BITFLDS(EXR_DSI_PHY0_M, dif->m) |
			BITFLDS(EXR_DSI_PHY0_N, dif->n) |
			BITFLDS(EXR_DSI_PHY0_POWERUP, dif->pwup) |
			BITFLDS(EXR_DSI_PHY0_CALIB, dif->calib) |
			BITFLDS(EXR_DSI_PHY0_TO_LP_HS_REQ,
				dif->to_lp_hs_req);
	}

	phy1 =	BITFLDS(EXR_DSI_PHY1_TO_LP_HS_DIS,
			dif->to_lp_hs_dis) |
		BITFLDS(EXR_DSI_PHY1_TO_LP_EOT,
			dif->to_lp_hs_eot) |
		BITFLDS(EXR_DSI_PHY1_TO_HS_ZERO,
			dif->to_hs_zero) |
		BITFLDS(EXR_DSI_PHY1_TO_HS_FLIP,
			dif->to_hs_flip) |
		BITFLDS(EXR_DSI_PHY1_LP_CLK_DIV,
			dif->lp_clk_div);

	phy2 =	BITFLDS(EXR_DSI_PHY2_HS_CLK_PRE,
			dif->to_hs_clk_pre) |
		BITFLDS(EXR_DSI_PHY2_HS_CLK_POST,
			dif->to_hs_clk_post) |
		BITFLDS(EXR_DSI_PHY2_DAT_DELAY,
			dif->data_delay) |
		BITFLDS(EXR_DSI_PHY2_CLK_DELAY,
			dif->clock_delay) |
		BITFLDS(EXR_DSI_PHY2_LPTX_TFALL,
			dif->lp_tx_tfall);

	phy3 =	BITFLDS(EXR_DSI_PHY3_EN, 0x1) |
		BITFLDS(EXR_DSI_PHY3_LPTX_TRISE,
			dif->lp_tx_trise) |
		BITFLDS(EXR_DSI_PHY3_LPTX_VREF,
			dif->lp_tx_vref);

	TRACE_VOP_PRINTF("MIPI-DSI @%d bps (%d,%d): 0x%08x 0x%08x 0x%08x 0x%08x)",
		 dsi_get_rate(display),
		 dif->n, dif->m,
		 phy0, phy1, phy2, phy3);

	dsi_write_field(EXR_DSI_PHY0, phy0);
	dsi_write_field(EXR_DSI_PHY1, phy1);
	dsi_write_field(EXR_DSI_PHY2, phy2);
	dsi_write_field(EXR_DSI_PHY3, phy3);

	if (on) {
		/* wait for PLL lock */
		dsi_wait_status(EXR_DSI_STAT_DSI_LOCK, 1, 1, 0, 1000);
	}
}

int dsi_set_phy_lock(struct vop_display *display)
{
	/* need_to_modify
	struct vop_drvdata *pdata = m_to_vopdata(display, display);
	if (down_interruptible(&pdata->sem))
		return -ERESTARTSYS;
	*/
	dsi_set_phy(display, 1);

	/* need_to_modify
	up(&pdata->sem);
	*/
	return 0;
}

void dsi_send_msglist(struct vop_display *display,
			     struct display_msg **msgs,
			     bl_display_cbs_t *bl_display_cb)
{
	int i = 0;

	while (msgs[i]) {
		bl_display_cb->bl_sleep(1);
		dsi_send_cmd(display, msgs[i], bl_display_cb);
		/* Next msg from tab */
		i++;
	}
}

int dsi_panel_init(struct vop_display *display, bl_display_cbs_t *bl_display_cb)
{
	struct display_msg **msgs = display->msgs_init;

	if (msgs)
		dsi_send_msglist(display, msgs, bl_display_cb);

	return 0;
}

int dsi_stop(struct vop_display *display)
{
	/* swicth off PLL */
	dsi_set_phy(display, 0);
	/* switch off phy */
	dsi_write_field(EXR_DSI_PHY3,
			BITFLDS(EXR_DSI_PHY3_EN, 0x0));
	/* switch off DSI block */
	dsi_write_field(EXR_DSI_CFG, DSI_CFG_OFF(DSI_CMD));

	dsi_write_field(EXR_DSI_CLC,
			BITFLDS(EXR_DSI_CLC_RUN, DSI_MODE_CONF));

	return 0;
}

int dsi_init(struct vop_display *display)
{
	unsigned int stat, clcstat;

	dsi_write_field(EXR_DSI_CLC,
			BITFLDS(EXR_DSI_CLC_RUN, DSI_MODE_RUN));

	clcstat = BITFLDS(EXR_DSI_CLC_STAT_RUN, 1) |
		BITFLDS(EXR_DSI_CLC_STAT_MODEN, 1) |
		BITFLDS(EXR_DSI_CLC_STAT_KID, 1);
	dsi_read_field(EXR_DSI_CLC_STAT, &stat);

	while ((stat & clcstat) != clcstat) {
		dsi_read_field(EXR_DSI_CLC_STAT, &stat);
		DSI_DBG2("wait dsi state run 0x%08x (0x%08x)\n"
				stat, clcstat);
	}

	dsi_wait_status(EXR_DSI_CLC_STAT, clcstat, clcstat, 0, 1000);
	dsi_write_field(EXR_DSI_CLK, 0x000F000F);
	dsi_write_field(EXR_DSI_TO0, 0);
	dsi_write_field(EXR_DSI_TO1, 0);
	dsi_write_field(EXR_DSI_CFG, DSI_CFG_RX_LP_STP(1));

	return 0;
}

void dsi_config(struct vop_display *display, int type)
{
	unsigned int dsicfg;

	if (type == DIF_TX_DATA) {
		dsi_write_field(EXR_DSI_CFG, DSI_CFG_OFF(DSI_CMD));
		dsi_set_phy(display, 1);
		dsi_write_field(EXR_DSI_CFG,
				DSI_CFG_INIT(display->dif.u.dsi.nblanes));
		return;
	}

	dsi_write_field(EXR_DSI_CFG, DSI_CFG_OFF(DSI_VIDEO));
	dsi_configure_video_mode(display, PIXELS_TO_BYTES(display->xres, display->bpp),
				 display->yres);
	dsi_set_phy(display, 1);
	dsicfg = DSI_CFG_TX_HS_PIXEL(display->dif.u.dsi.nblanes, display->dif.u.dsi.mode);
	dsi_write_field(EXR_DSI_CFG, dsicfg);
	TRACE_VOP_PRINTF("MIPI-DSI video: dsicfg 0x%08x", dsicfg);
}

int dsi_dphy_calculation(struct vop_display *display)
{
	int ui_ps = 0, ths_prepare_ns, ths_trail_ns, ths_prepare_zero_ns,
		tclk_post_ns, tclk_prepare_ns;
	struct vop_display_if_mipi_dsi *dif = &display->dif.u.dsi;

	if (dif->display_if_dts) {
		TRACE_VOP_PRINTF("using dts default parameters");
		return 0;
	}

	if (dif->brdef)
		ui_ps = DIV_ROUND_CLOSEST(1000000000,
					  dif->brdef / 1000);

	/*
	 * THS-PREPARE is between 40ns + 4UI and 85ns + 6UI, we set the THS-PREPARE
	 * to average of THS-PREPARE.
	 * THS-PREPARE = (40ns + 4UI + 85ns + 6UI) / 2 = 62.5ns + 5UI
	 */
	ths_prepare_ns = DIV_ROUND_CLOSEST(62500 + 5 * ui_ps, 1000);

	/*
	 * THS-TRAIL is 60ns + 4UI, we set THS-TRAIL to 63ns + 4UI for safety
	 * margin.
	 */
	ths_trail_ns = DIV_ROUND_UP(63000 + 4 * ui_ps, 1000);

	/*
	 * THS-PREPARE + THS-ZERO is 145ns + 10UI, we set THS-PREPARE + THS-ZERO
	 * to 152.25ns + 11UI for safety margin.
	 */
	ths_prepare_zero_ns = DIV_ROUND_UP(152250 + 11 * ui_ps, 1000);

	/*
	 * TCLK-POST is 60ns + 52UI, we set TCLK-POST to 63ns + 55UI for safety
	 * margin.
	 */
	tclk_post_ns = DIV_ROUND_UP(63000 + 55 * ui_ps, 1000);

	/*
	 * TCLK-PREPARE is between 38ns and 95ns, we set TCLK-PREPARE to average
	 * of TCLK-PREPARE 67.
	 * margin.
	 */
	tclk_prepare_ns = 67;

	dif->pwup = 6;
	dif->calib = 3;
	dif->data_delay = 7;
	dif->clock_delay = 7;
	dif->lp_tx_tfall = 2;
	dif->lp_tx_trise = 2;
	dif->lp_tx_vref = 31;
	dif->lp_clk_div =
		DIV_ROUND_UP(dif->dc_clk_rate / 1000 * TLPX_NS,
			     1000000) - 1;
	dif->to_lp_hs_req = dif->lp_clk_div;
	dif->to_hs_flip =
		DIV_ROUND_UP(dif->brdef /
			     1000 * ths_trail_ns, 1000000 * 8);
	dif->to_hs_zero =
		DIV_ROUND_UP(dif->brdef /
			     1000 * ths_prepare_zero_ns, 1000000 * 8) - 5;
	dif->to_lp_hs_eot =
		DIV_ROUND_UP(dif->dc_clk_rate / 1000 *
		(ths_trail_ns + 18), 1000000) + DIV_ROUND_UP(3000000,
		dif->brdef / 1000);
	dif->to_lp_hs_dis =
		DIV_ROUND_UP(dif->dc_clk_rate / 1000 *
			     ths_prepare_ns, 1000000) - 1;
	dif->to_hs_clk_post =
		DIV_ROUND_UP(dif->dc_clk_rate / 1000 * tclk_post_ns,
			     1000000);
	dif->to_hs_clk_pre =
		DIV_ROUND_UP(dif->dc_clk_rate / 1000 *
			     tclk_prepare_ns, 1000000) + 5;
}

void dsi_rate_calculation(struct vop_display *display)
{
	int diff, diff_min = DSI_RATE_MAX, n = 0, m = 0;
	struct vop_display_if_mipi_dsi *dif = &display->dif.u.dsi;

	if (!dif->display_if_dts) {
		dif->brdef = DSI_RATE_OVERHEAD((display->xres +
			BYTES_TO_PIXELS(dif->hfp, display->bpp) +
			BYTES_TO_PIXELS(dif->hbp, display->bpp) +
			BYTES_TO_PIXELS(dif->hsa, display->bpp)) * (display->yres +
			dif->vfp + dif->vbp + dif->vsa) *
			display->fps / dif->nblanes * display->bpp);
	}

	if (dif->brdef > DSI_RATE_MAX)
		dif->brdef = DSI_RATE_MAX;
	else if (dif->brdef < DSI_RATE_MIN)
		dif->brdef = DSI_RATE_MIN;

	for (m = 1; m <= DSI_RATE_M_MAX; m++) {
		for (n = 1; n <= DSI_RATE_N_MAX; n++) {
			diff = DSI_RATE(n, m) - dif->brdef;

			if (diff < 0)
				continue;

			if (diff < diff_min) {
				diff_min = diff;
				dif->n = n;
				dif->m = m;
			}
		}
	}
}

static void dma_reset(struct vop_display *display, bl_display_cbs_t *bl_disp_cb)
{
	*((volatile int *)0xe4801000) = 0x20;
	bl_disp_cb->bl_sleep(1);
	*((volatile int *)0xe4801004) = 0x20;
}

static void dsi_reset(struct vop_display * display, bl_display_cbs_t *bl_disp_cb)
{
	setScu_SET_RSTMODS1_DSIRST(&scu, 1);
	bl_disp_cb->bl_sleep(1);
	setScu_CLR_RSTMODS1_DSIRST(&scu, 1);
	bl_disp_cb->bl_sleep(8);
}

int dsi_enable(struct rk_display *rk_disp)
{
	struct vop_display *display = (struct vop_display *)rk_disp->disp_par;

	dma_reset(display, rk_disp->ctrl_ops);
	dsi_reset(display, rk_disp->ctrl_ops);
	dsi_init(display);
	dsi_config(display, DIF_TX_DATA);

#if defined (SF3GR_MRD6_COHOS3GR)
	// cohos3gr platform use GPIO_68 <-> LCD_DCDC_EN
	// GPIO_69 <-> LCM_VCC_EN, Set the two pin high output
	*((volatile int *)0xE4600310) = 0x1700;
	*((volatile int *)0xE4600314) = 0x1700;
	rk_disp->ctrl_ops->bl_sleep(10);
#endif
	*((volatile int *)0xE4600290) = 0x1700;
	rk_disp->ctrl_ops->bl_sleep(10);
	*((volatile int *)0xE4600290) = 0x1500;
	rk_disp->ctrl_ops->bl_sleep(10);
	*((volatile int *)0xE4600290) = 0x1700;
	rk_disp->ctrl_ops->bl_sleep(10);

	dsi_panel_init(display, rk_disp->ctrl_ops);

	dsi_config(display, DIF_TX_PIXELS);

	return 0;
}

static struct rk_disp_trsm_drv_ops dsi_drv_ops = {
	.enable = dsi_enable,
};

int dsi_probe(struct rk_display *rk_disp)
{
	struct vop_display *display = (struct vop_display *)rk_disp->disp_par;

	disp_ctrl_ops = rk_disp->ctrl_ops;
	rk_disp->trsm_ops = &dsi_drv_ops;

	display->fps = display->dif.u.dsi.fps;
	if (display->fps < 20 || display->fps > 100) {
		TRACE_VOP_PRINTF("DSI: invalid fps %d, set to default", display->fps);
		display->fps = 60;
	}
	g_dsi_cfg_eot = (display->dif.u.dsi.eot)?1:0;
	g_dsi_cfg_gate = (display->dif.u.dsi.gate)?1:0;
	display->xres = rk_disp->panel->timing.xres;
	display->yres = rk_disp->panel->timing.yres;
	display->dif.u.dsi.brmin = 104000000;
	display->dif.u.dsi.brmax = DSI_RATE_MAX;

	if (rk_disp->panel->lcd_face == OUT_P565) {
		display->bpp = 16;
		display->dif.u.dsi.video_pixel = DSI_PIX_BIT16P;
	} else if (rk_disp->panel->lcd_face == OUT_P666) {
		display->bpp = 18;
		display->dif.u.dsi.video_pixel = DSI_PIX_BIT18P;
	} else {
		display->bpp = 24;
		display->dif.u.dsi.video_pixel = DSI_PIX_BIT24P;
	}

	dsi_rate_calculation(display);
	dsi_dphy_calculation(display);

	return 0;
}

