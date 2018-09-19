/*
 *
 * (C) COPYRIGHT 2013-2014 ARM Limited. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 *
 * A copy of the licence is included with the program, and can also be obtained
 * from Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 *
 */



#ifndef _MALIDP_ADF_OVERLAY_H_
#define _MALIDP_ADF_OVERLAY_H_

const struct malidp_layer_hw_info *malidp_adf_ovr_get_hw_layer(struct adf_overlay_engine *eng);

int malidp_adf_ovr_add_layers(struct malidp_device *dp_dev,
		const struct malidp_layer_hw_info *layers, int n_layers);

int malidp_adf_ovr_destroy_layers(struct malidp_device *dp_dev);

#endif /* _MALIDP_ADF_OVERLAY_H_ */
