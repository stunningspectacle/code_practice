/*
 *
 * (C) COPYRIGHT 2013-2015 ARM Limited. All rights reserved.
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



#ifndef _MALIDP_ADF_INTERFACE_H_
#define _MALIDP_ADF_INTERFACE_H_

#include <uapi/video/malidp_adf.h>
#include <uapi/video/adf.h>
#include <video/adf.h>
#include <video/video_tx.h>

#include "malidp_drv.h"
#include "malidp_hw.h"
#include "malidp_iommu.h"

struct malidp_intf_memory_cfg {
	struct sync_fence *mw_fence;
	struct adf_buffer *mw_buf;
	struct adf_buffer_mapping *mw_map;
	struct malidp_iommu_mapping **iommu_map;
	int mw_buf_index;
	bool signaled;
};

struct malidp_intf_memory_cfg *malidp_intf_create_mw_cfg(struct adf_interface *adf_intf,
				     struct malidp_device *dp_dev,
				     struct adf_buffer *buf, int buf_index,
				     struct adf_buffer_mapping *map,
				     struct malidp_iommu_mapping **iommu_map);

void malidp_adf_intf_destroy_mw_cfg(struct adf_interface *adf_intf,
				struct malidp_intf_memory_cfg *mw_cfg);

void malidp_intf_wait(struct adf_interface *adf_intf,
			      struct malidp_device *dp_dev);

void malidp_intf_prepare(struct adf_interface *adf_intf,
			 struct malidp_intf_memory_cfg *mw_cfg);

int malidp_adf_intf_get_dp_type(struct adf_interface *adf_intf);

int malidp_adf_intf_add_interfaces(struct malidp_device *dp_dev,
		const struct malidp_intf_hw_info *interfaces, u32 n_interfaces,
		struct video_tx_device *tx);
int malidp_adf_intf_destroy_interfaces(struct malidp_device *dp_dev);

void malidp_adf_intf_restore_drmmode(struct malidp_device *dev);
#endif /* _MALIDP_ADF_INTERFACE_H_ */
