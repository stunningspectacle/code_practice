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
#include "bl-vop-display.h"

#include "bl-dcc.h"
#include "rk_disp.h"

struct vop_display *lcd = NULL;

bl_dcc_result_type bl_dcc_show_image(unsigned int width,
				     unsigned int height,
				     unsigned int bpp,
				     unsigned char *data,
				     bl_display_cbs_t *pbl_disp_cb,
				     bl_lcd_config_t *pbl_lcd_conf,
				     bl_dcc_platform hw_id, unsigned char index)
{
	struct rk_panel_info *panel;
	int xpos = 0, ypos = 0;

	if (hw_id >= BL_DCC_PLATFORM_INVALID)
		return BL_DCC_RESULT_INCORRECT_HWID;

	if (NULL == lcd) {
		lcd = vop_initialize_display_config((unsigned int *)pbl_lcd_conf, index);
		if (rk_display_probe(pbl_disp_cb, (void *)lcd))
			return BL_DCC_RESULT_INIT_FAILED;
	}

	panel = rk_get_panel_info();
	if (width > panel->timing.xres || height > panel->timing.yres)
		return BL_DCC_RESULT_INCORRECT_SIZE;

	xpos = (panel->timing.xres - width) >> 1;
	ypos = (panel->timing.yres - height) >> 1;

	return rk_display_update_windows(xpos, ypos, width, height, bpp, data);
}

__attribute__((weak))int usif_dbg_printf(const char * format, ...)
{
  return 0;
}

