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



#ifndef _MALIDP_DRV_H_
#define _MALIDP_DRV_H_

#include <linux/clk.h>
#include <video/adf.h>
#include <linux/debugfs.h>

struct malidp_hw_device;
struct malidp_iommu_domain;

struct malidp_device {
	struct malidp_iommu_domain *iommu_domain;
	struct malidp_hw_device *hw_dev;
	struct device *device;
	struct adf_device adf_dev;
	char name[ADF_NAME_LEN];
	u32 id;
	u32 core_id;
	u8 current_dpms;

#ifdef CONFIG_PM_SLEEP
	atomic_t suspending;
	struct notifier_block dp_pm_nb;
#endif
	struct dentry *dbg_folder;
};

#endif /* _MALIDP_DRV_H_ */
