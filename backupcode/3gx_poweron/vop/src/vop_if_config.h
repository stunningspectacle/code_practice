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

#ifndef VOP_DISPLAY_IF_CONFIG_H_
#define VOP_DISPLAY_IF_CONFIG_H_


/*
 * The blob tool, identifies the interface definition through ,
 * the definition of  VOP_DISPLAY_IF_CONFIG_VER.
 * If any of the following structure needs to be changed,
 * the version number shall be increased by 1
 */
#define VOP_DISPLAY_IF_CONFIG_VER   0x02

struct xgold_lcd_periph_parameters {
    unsigned char cs_polarity;
    unsigned char cd_polarity;
    unsigned char wr_polarity;
    unsigned char rd_polarity;
    unsigned char hd_polarity;
    unsigned char vd_polarity;
};

struct xgold_lcd_timing_parameters {
    unsigned addr_delay_ns;
    unsigned cs_act_ns;
    unsigned data_delay_ns;
    unsigned wr_rd_act_ns;
    unsigned wr_rd_deact_ns;
    unsigned cs_deact_ns;
    unsigned access_cycle_ns;
};

enum {
    DIF_TX_DATA = 1,
    DIF_TX_PIXELS = 2,
};

#define LCD_MSG_CMD     0
#define LCD_MSG_DAT     1


typedef struct {
            int type;
            int fps;    /* framerate */
            int xres;   /* pixel width */
            int yres;   /* pixel heigth */
            int xdpi;   /* pixel density per inch in x direction */
            int ydpi;   /* pixel density per inch in y direction */
} PANEL_RES;
struct display_msg {
    /*const char *name;*/
    unsigned char header;
    unsigned char type;
    int length;
    int delay;		/*in ms */
    unsigned int flags;
    unsigned char *datas;
};


#define LCD_MSG_LP  1


enum {
    VOP_IF_MIPI_DBI = 1,
    VOP_IF_MIPI_DSI = 2,
};

#define DISPLAY_IS_MIPI_DBI_IF(_t_) \
    (_t_ == VOP_IF_MIPI_DBI)
#define DISPLAY_IS_MIPI_DSI_IF(_t_) \
        (_t_ == VOP_IF_MIPI_DSI)

enum dsi_mode_t {
    DSI_VIDEO = 0,
    DSI_CMD = 1,
};

enum dsi_pix_stream_t {
    DSI_PIX_BIT16P = 0,
    DSI_PIX_BIT18P = 1,
    DSI_PIX_BIT18L = 2,
    DSI_PIX_BIT24P = 3,
    DSI_PIX_UNKNOWN = 0xFF
};

enum dsi_video_mode_t {
    DSI_ACTIVE = 0,
    DSI_PULSES = 1,
    DSI_EVENTS = 2,
    DSI_BURST = 3,
    DSI_UNKNOWN = 0xFF
};

struct vop_display_if_mipi_dsi {
    int dc_clk_rate;
    int mode;
    int brmin;
    int brdef;
    int brmax;
    int nblanes;
    int id;
/* phy0 */
    int share;
    int m;
    int n;
    int pwup;
    int calib;
    int to_lp_hs_req;
/* phy1 */
    int to_lp_hs_dis;
    int to_lp_hs_eot;
    int to_hs_zero;
    int to_hs_flip;
    int lp_clk_div;
/* phy2 */
    int to_hs_clk_pre;
    int to_hs_clk_post;
    int data_delay;
    int clock_delay;
    int lp_tx_tfall;
/* phy3 */
    int en;
    int lp_tx_trise;
    int lp_tx_vref;

/* video mode settings*/
    /* Timing: All values in pixclocks, except pixclock (of course) */
    int hfp; /* horizontal front porch (cycles/bytes) */
    int hfp_lp; /* LP / HS */
    int hbp; /* horizontal back porch (bytes) */
    int hbp_lp;
    int hsa; /* horizontal sync active (bytes) */
    int hsa_lp;
    int vfp; /* vertical front porch (line) */
    int vbp; /* vertical back porch (line) */
    int vsa; /* vertical sync active (line) */
    int video_mode;
    int video_pixel;
    int bllp_time;
    int line_time;
    int display_if_dts;

    int fps;
    int eot;
    int gate;
};

struct vop_display_if_mipi_dbi {
    /* data on bus */
    int segments_per_pix;
    int bits_per_segment;
    /* timing */
    unsigned addr_delay_ns;
    unsigned cs_act_ns;
    unsigned data_delay_ns;
    unsigned wr_rd_act_ns;
    unsigned wr_rd_deact_ns;
    unsigned cs_deact_ns;
    unsigned access_cycle_ns;
    unsigned int mux_params[32];
    struct xgold_lcd_periph_parameters periph_params;
};

struct vop_display_if {
    int type;
    int ncfg;
    union {
        struct vop_display_if_mipi_dbi dbi;
        struct vop_display_if_mipi_dsi dsi;
    } u;
};

struct display_reset {
    int value1;
    int mdelay1;
    int value2;
    int mdelay2;
    int value3;
    int mdelay3;
};

typedef enum e_dcs_cmd_type
{

    DCS_CMD_START       = 0x100,
    DCS_CMD_RESET       = 0x101,
    DCS_CMD_INIT_START  = 0x102,
    DCS_CMD_GENERIC     = 0x103,
    DCS_CMD_INIT_END    = 0x104,
    DCS_CMD_DELAY       = 0x105,
    DCS_CMD_LP          = 0x106,
    DCS_CMD_RESOLUTION   = 0x107,
    DCS_CMD_DISPLAY_IF   = 0x108,
    DCS_CMD_PANEL_INFO	= 0x109,
    DCS_CMD_END         = 0x1FF

} dcs_cmd_type;

typedef enum e_multi_display_type
{

    PANEL_INDEX_START       	= 0x12340100,
    PANEL_INDEX_STOP       		= 0x12340101,
    PANEL_INDEX_DEFAULT_SIZE  = 0x12340102,

    PANEL_INDEX_END         = 0x123401FF

} multi_display_type;

/*
 * If any of the following structure needs to be changed,
 * the version number definiton, VBT_VERSION, shall be increased by 1
 */

#define VBT_VERSION    0x2
#define VBT_MAGIC_WORD 0xC001C0DE

typedef struct {
    unsigned int magic_word;
    unsigned int version;
    unsigned int vbt_size;
    unsigned int vop_if_config_version;
    unsigned char reserved[52]; //total: 64 bytes- Reserved for future use
} VBT_HEADER;

#define DISPLAY_CONFIG_DATA_SIZE_IN_BYTES	(100 * 1024)
#define VOP_MAX_NUMBER_OF_MSGS  1024
#define VOP_MAX_LENGTH_OF_DATA  512


#endif

