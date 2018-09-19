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

#include "bl-vop-display.h"
#include "rk_disp.h"

VOP_DISPLAY vop_config_blob;
struct rk_panel_info lcd_panel_info;
__attribute__((section("DISPLAY_BUFFER"))) struct display_msg *blob_msgs_init[VOP_MAX_NUMBER_OF_MSGS];
__attribute__((section("DISPLAY_BUFFER"))) struct display_msg blob_msg[VOP_MAX_NUMBER_OF_MSGS];
__attribute__((section("DISPLAY_BUFFER"))) unsigned char blob_datas[VOP_MAX_NUMBER_OF_MSGS][VOP_MAX_LENGTH_OF_DATA];

void parse_blob(unsigned int *blob, struct display_msg *t_msgs_init[],
		       struct display_reset *t_reset,
		       PANEL_RES * t_panel_res,
		       struct vop_display_if *t_dif,
		       struct rk_panel_info *t_panel_info,
		       struct display_msg t_msg[],
		       unsigned char t_datas[][VOP_MAX_LENGTH_OF_DATA], unsigned char index)
{
	int i = -1, cmd = -1,  length = 0, j = 0x0;
	VBT_HEADER vbt_hdr;
	int init_seq = 0;
	unsigned int start_addr = 0, count = 0, size = 0;

	vbt_hdr = *((VBT_HEADER *)&blob[0]);
#ifdef HOST_DEBUG
	printf("vbt_hdr.magic_word = 0x%x\n", vbt_hdr.magic_word);
	printf("vbt_hdr.version = 0x%x\n", vbt_hdr.version);
	printf("vbt_hdr.vbt_size = 0x%x\n", vbt_hdr.vbt_size);
	printf("vbt_hdr.vop_if_config_version = 0x%x\n",
	       vbt_hdr.vop_if_config_version);
#endif

	start_addr += sizeof(VBT_HEADER);

	while(count < index) {
		size = blob[(start_addr/4)+1];
		start_addr += size;
		count++;
	}
	blob = &blob[(start_addr/4)];

	while (blob[++i] != PANEL_INDEX_STOP) {
		if (blob[i] == DCS_CMD_INIT_START)
			init_seq = 0x01;

		if (blob[i] == DCS_CMD_INIT_END)
			init_seq = 0x0;

		if (blob[i] == DCS_CMD_GENERIC && init_seq) {
			cmd++;
			t_msg[cmd].type = blob[i + 1];
			t_msg[cmd].header = blob[i + 2];
			length = 0;
			i += 3;
			while (blob[i] < DCS_CMD_START)
				t_datas[cmd][length++] = blob[i++];

			t_msg[cmd].length = length;
			t_msg[cmd].datas = t_datas[cmd];
			t_msgs_init[cmd] = &t_msg[cmd];

			/* check for delay or LP */
			t_msg[cmd].flags = 0x0;
			t_msg[cmd].delay = 0x0;

			for (j = 0; j < 2; j++) {
				if (blob[i] == DCS_CMD_DELAY) {
					t_msg[cmd].delay = blob[i + 1];
					i += 2;
				}

				if (blob[i] == DCS_CMD_LP) {
					t_msg[cmd].flags = blob[i + 1];
					i += 2;
				}
			}

			/* rewind 1 */
			i--;
		}

		if (blob[i] == DCS_CMD_RESET) {
			t_reset->value1 = blob[i + 1];
			t_reset->mdelay1 = blob[i + 2];

			t_reset->value2 = blob[i + 3];
			t_reset->mdelay2 = blob[i + 4];

			t_reset->value3 = blob[i + 5];
			t_reset->mdelay3 = blob[i + 6];

			i += 7;
		}

		if (blob[i] == DCS_CMD_RESOLUTION){
			i++;
			*t_panel_res = *((PANEL_RES *)&blob[i]);
		}

		if (blob[i] == DCS_CMD_DISPLAY_IF) {
			i++;
			*t_dif = *((struct vop_display_if *)&blob[i]);
		}

		if (blob[i] == DCS_CMD_PANEL_INFO) {
			i++;
			*t_panel_info = *((struct rk_panel_info *)&blob[i]);
		}
	}

	t_msgs_init[cmd + 1] = 0x0;
}

VOP_DISPLAY *vop_initialize_display_config(unsigned int *blob, unsigned char index)
{
	struct display_msg **dcs_seq;
	struct display_reset *reset_ptr;
	PANEL_RES  *panel_res;
	struct vop_display_if *dif;
	struct rk_panel_info *panel_info;

	vop_config_blob.msgs_init = blob_msgs_init;

	dcs_seq = vop_config_blob.msgs_init;
	reset_ptr = &vop_config_blob.reset;
	panel_res = (PANEL_RES *)&vop_config_blob.type;
	dif = &vop_config_blob.dif;
	panel_info = &lcd_panel_info;

	parse_blob(blob, dcs_seq, reset_ptr, panel_res,
		   dif, panel_info, blob_msg, blob_datas, index);

	/* the clk_rate should be moved to the blob. TBD */
	/* vop_config_blob.clk_rate = 416000000; */

	return &vop_config_blob;
}

struct rk_panel_info *rk_get_panel_info(void)
{
	return &lcd_panel_info;
}

