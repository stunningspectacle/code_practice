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



#ifndef _MALIDP_ADF_H_
#define _MALIDP_ADF_H_

#include <linux/types.h>
#include <video/adf.h>
#include <video/video_tx.h>

#include "malidp_drv.h"
#include "malidp_hw.h"
#include "malidp_iommu.h"
#include "malidp_adf_interface.h"
#include "malidp_adf_overlay.h"

#define to_malidp_device(dev) container_of(dev, \
			struct malidp_device, adf_dev)

#define MALIDP_MAX_INTERFACES 2
#define MALIDP_MAX_BUFFERS 10

struct malidp_driver_state {
	int n_intfs;
	struct adf_interface *intf_list[MALIDP_MAX_INTERFACES];
	struct malidp_hw_state hw_state;
	struct malidp_intf_memory_cfg *mw_cfg;
	struct malidp_iommu_mapping *iommu_maps[MALIDP_MAX_BUFFERS][ADF_MAX_PLANES];
};

int malidp_adf_init(struct malidp_device *dp_dev,
		struct malidp_hw_description *hw_desc,
		struct video_tx_device *tx);
int malidp_adf_destroy(struct malidp_device *dp_dev);

void malidp_adf_post_cleanup(struct adf_device *dev,
		struct adf_pending_post *post);

int malidp_adf_runtime_resume(struct malidp_device *dp_dev);
void malidp_adf_cleanup_signaled_mw(struct malidp_driver_state *state);
void malidp_adf_waiting_for_mw(struct malidp_driver_state *state);
#endif /* _MALIDP_ADF_H_ */
