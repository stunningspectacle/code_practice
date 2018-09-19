/*
 *
 * (C) COPYRIGHT 2014 ARM Limited. All rights reserved.
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



#ifndef __MALIDP_IOMMU_H__
#define __MALIDP_IOMMU_H__

#include <linux/iommu.h>

struct malidp_iommu_domain;
struct malidp_iommu_mapping;

struct malidp_iommu_mapping *malidp_iommu_map_sgt(struct malidp_iommu_domain *dp_domain,
			   struct sg_table *sgt,
			   enum dma_data_direction dir);

void malidp_iommu_unmap_sgt(struct malidp_iommu_domain *dp_domain,
			    struct malidp_iommu_mapping *map);

dma_addr_t malidp_iommu_dma_addr(struct malidp_iommu_domain *dp_domain,
				 struct malidp_iommu_mapping *map);

struct malidp_iommu_domain *malidp_iommu_init(struct device *dev,
				       struct bus_type *bus);

void malidp_iommu_exit(struct malidp_iommu_domain *dp_domain);

#endif /* __MALIDP_IOMMU_H__ */
