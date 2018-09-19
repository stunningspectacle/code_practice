/*
 *
 * (C) COPYRIGHT 2014-2015 ARM Limited. All rights reserved.
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



#ifndef _MALIDP_SYSFS_H_
#define _MALIDP_SYSFS_H_

#include "malidp_drv.h"

typedef umode_t (*attr_visible_t)(struct kobject*,
			struct attribute *, int);

int malidp_sysfs_init(struct malidp_device *dev);
void malidp_sysfs_destroy(struct malidp_device *dev);

#endif /* _MALIDP_SYSFS_H_ */
