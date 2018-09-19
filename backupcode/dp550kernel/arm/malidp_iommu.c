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



#include <linux/device.h>
#include <linux/iommu.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>

#include "malidp_iommu.h"

#define MALIDP_DEV_ADDR_START   0x20000000 /* Start address of the VA space */
#define MALIDP_DEV_ADDR_SIZE    0x40000000 /* Size of the addres space */
#define MALIDP_DEV_ADDR_ORDER   0x0        /* Order of the VA allocations */

struct malidp_iommu_domain {
	struct iommu_domain *domain;
	struct device *dev;
	void *bitmap;
	size_t bits;
	unsigned int order;
	dma_addr_t base;
	spinlock_t lock;
};

struct malidp_iommu_mapping {
	struct sg_table *sgt;
	enum dma_data_direction dir;
};

#ifdef CONFIG_IOMMU_API

static void dump_sg_table(struct device *dev, struct sg_table *sgt)
{
	struct scatterlist *aux_sgl;
	int i;

	dev_dbg(dev, "sgt = %p:\n", sgt);
	dev_dbg(dev, "\t sgl = %p\n", sgt->sgl);
	dev_dbg(dev, "\t nents = %d\n", sgt->nents);
	dev_dbg(dev, "\t orig_nents = %d\n", sgt->orig_nents);
	for_each_sg(sgt->sgl, aux_sgl, sgt->nents, i) {
		dev_dbg(dev, "\t sgl(%d) = %p\n", i, aux_sgl);
		dev_dbg(dev, "\t\t dma_addr = 0x%llx\n",
			(unsigned long long)sg_dma_address(aux_sgl));
		dev_dbg(dev, "\t\t virt_addr = %p\n", sg_virt(aux_sgl));
		dev_dbg(dev, "\t\t phys_addr = 0x%llx\n",
			(unsigned long long)sg_phys(aux_sgl));
		dev_dbg(dev, "\t\t len = %u\n", aux_sgl->length);
	}
}

/*
 * Code taken from arch/arm/mm/dma-mapping.c to handle iova allocations.
 *
 * It has been slightly simplified:
 * - Do not support an extra MMU alignment (CONFIG_ARM_DMA_IOMMU_ALIGNMENT)
 * - Do not perform any cache maintenance (should already be done by the
 * exporter of the memory).
 * - Do not impose a maximum size to each sgl entry in an sgt.
 */
static inline dma_addr_t __alloc_iova(struct malidp_iommu_domain *dp_domain,
				      size_t size)
{
	unsigned int align = 0;
	unsigned int count, start;
	unsigned long flags;

	count = ((PAGE_ALIGN(size) >> PAGE_SHIFT) +
		 (1 << dp_domain->order) - 1) >> dp_domain->order;

	spin_lock_irqsave(&dp_domain->lock, flags);
	start = bitmap_find_next_zero_area(dp_domain->bitmap, dp_domain->bits, 0,
					   count, align);
	if (start > dp_domain->bits) {
		spin_unlock_irqrestore(&dp_domain->lock, flags);
		return DMA_ERROR_CODE;
	}

	bitmap_set(dp_domain->bitmap, start, count);
	spin_unlock_irqrestore(&dp_domain->lock, flags);

	return dp_domain->base + (start << (dp_domain->order + PAGE_SHIFT));
}

static inline void __free_iova(struct malidp_iommu_domain *dp_domain,
			       dma_addr_t addr, size_t size)
{
	unsigned int start = (addr - dp_domain->base) >>
			     (dp_domain->order + PAGE_SHIFT);
	unsigned int count = ((size >> PAGE_SHIFT) +
			      (1 << dp_domain->order) - 1) >> dp_domain->order;
	unsigned long flags;

	spin_lock_irqsave(&dp_domain->lock, flags);
	bitmap_clear(dp_domain->bitmap, start, count);
	spin_unlock_irqrestore(&dp_domain->lock, flags);
}

static int __map_sg_chunk(struct malidp_iommu_domain *dp_domain, struct scatterlist *sg,
			  size_t size, dma_addr_t *handle,
			  enum dma_data_direction dir, struct dma_attrs *attrs)
{
	dma_addr_t iova, iova_base;
	int ret = 0;
	unsigned int count;
	struct scatterlist *s;

	size = PAGE_ALIGN(size);
	*handle = DMA_ERROR_CODE;

	iova_base = iova = __alloc_iova(dp_domain, size);
	if (iova == DMA_ERROR_CODE)
		return -ENOMEM;

	for (count = 0, s = sg; count < (size >> PAGE_SHIFT); s = sg_next(s)) {
		phys_addr_t phys = page_to_phys(sg_page(s));
		unsigned int len = PAGE_ALIGN(s->offset + s->length);
		int prot = 0;

		dev_dbg(dp_domain->dev,
			"%s: s->offset = %u, s->length = %u, len = %u\n",
		       __func__, s->offset, s->length, len);

		if (dir == DMA_BIDIRECTIONAL)
			prot = IOMMU_READ | IOMMU_WRITE;
		else if (dir == DMA_TO_DEVICE)
			prot = IOMMU_READ;
		else if (dir == DMA_FROM_DEVICE)
			prot = IOMMU_WRITE;

		ret = iommu_map(dp_domain->domain, iova, phys, len, prot);
		if (ret < 0)
			goto fail;
		count += len >> PAGE_SHIFT;
		iova += len;
	}
	*handle = iova_base;

	return 0;
fail:
	__free_iova(dp_domain, iova_base, size);
	return ret;
}

static int __iommu_remove_mapping(struct malidp_iommu_domain *dp_domain,
				  dma_addr_t iova, size_t size)
{
	/*
	 * add optional in-page offset from iova to size and align
	 * result to page size
	 */
	size = PAGE_ALIGN((iova & ~PAGE_MASK) + size);
	iova &= PAGE_MASK;

	iommu_unmap(dp_domain->domain, iova, size);
	__free_iova(dp_domain, iova, size);
	return 0;
}

static int iommu_map_sg(struct malidp_iommu_domain *dp_domain,
		     struct scatterlist *sg, int nents,
		     enum dma_data_direction dir, struct dma_attrs *attrs)
{
	struct scatterlist *s = sg, *dma = sg, *start = sg;
	int i, count = 0;
	unsigned int offset = s->offset;
	unsigned int size = s->offset + s->length;

	dev_dbg(dp_domain->dev,
		"%s: s->offset = %u, s->length = %u, size = %u\n",
	       __func__, s->offset, s->length, size);

	for (i = 1; i < nents; i++) {
		s = sg_next(s);

		s->dma_address = DMA_ERROR_CODE;
		s->dma_length = 0;

		if (s->offset || (size & ~PAGE_MASK)) {
			if (__map_sg_chunk(dp_domain, start, size, &dma->dma_address,
			    dir, attrs) < 0)
				goto bad_mapping;

			dma->dma_address += offset;
			dma->dma_length = size - offset;

			size = offset = s->offset;
			start = s;
			dma = sg_next(dma);
			count += 1;
		}
		size += s->length;
	}
	if (__map_sg_chunk(dp_domain, start, size, &dma->dma_address, dir, attrs) < 0)
		goto bad_mapping;

	dma->dma_address += offset;
	dma->dma_length = size - offset;

	return count+1;

bad_mapping:
	for_each_sg(sg, s, count, i)
		__iommu_remove_mapping(dp_domain, sg_dma_address(s), sg_dma_len(s));
	return 0;
}

static void iommu_unmap_sg(struct malidp_iommu_domain *dp_domain,
			   struct scatterlist *sg, int nents,
			   enum dma_data_direction dir,
			   struct dma_attrs *attrs)
{
	struct scatterlist *s;
	int i;

	dev_dbg(dp_domain->dev,
		"%s: sg->offset = %u, sg->length = %u\n",
	       __func__, sg->offset, sg->length);

	for_each_sg(sg, s, nents, i) {
		if (sg_dma_len(s))
			__iommu_remove_mapping(dp_domain, sg_dma_address(s),
					       sg_dma_len(s));
	}
}
/* End of code from arch/arm/mm/dma-mapping.c*/

static struct sg_table *clone_sgt(struct sg_table *sgt_src)
{
	struct sg_table *sgt = NULL;
	struct scatterlist *s, *s_out;
	unsigned int i;
	int ret;

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt)
		return NULL;

	ret = sg_alloc_table(sgt, sgt_src->orig_nents, GFP_KERNEL);
	if (ret < 0 || !sgt) {
		kfree(sgt);
		return NULL;
	}

	s_out = sgt->sgl;
	for_each_sg(sgt_src->sgl, s, sgt_src->orig_nents, i) {
		sg_set_page(s_out, sg_page(s), s->length, s->offset);
		s_out = sg_next(s_out);
	}

	return sgt;
}

static void destroy_sgt(struct sg_table *sgt)
{
	sg_free_table(sgt);
	kfree(sgt);
}

struct malidp_iommu_mapping *malidp_iommu_map_sgt(struct malidp_iommu_domain *dp_domain,
			   struct sg_table *sgt,
			   enum dma_data_direction dir)
{
	struct malidp_iommu_mapping *map;
	int ret;

	map = kzalloc(sizeof(struct malidp_iommu_mapping), GFP_KERNEL);
	if (!map)
		return NULL;

	map->sgt = clone_sgt(sgt);
	if (!map->sgt)
		goto err_clone;

	map->dir = dir;

	dump_sg_table(dp_domain->dev, map->sgt);

	ret = iommu_map_sg(dp_domain, map->sgt->sgl, map->sgt->nents,
			       dir, NULL);
	dump_sg_table(dp_domain->dev, map->sgt);
	if (ret != 1) {
		/*
		 * If ret > 1 it means that we couldn't find contiguous VA
		 * space to fit the whole buffer. Since Mali DP don't have
		 * scatter gather DMA this cannot be handled by the HW.
		 */
		dev_err(dp_domain->dev, "could only map SG table as %d entries\n",
			ret);
		goto err_map_sg;
	}

	return map;

err_map_sg:
	destroy_sgt(map->sgt);
err_clone:
	kfree(map);
	return NULL;
}

void malidp_iommu_unmap_sgt(struct malidp_iommu_domain *dp_domain,
			    struct malidp_iommu_mapping *map)
{
	if (!map)
		return;

	dev_dbg(dp_domain->dev, "%s: sgt = %p\n", __func__, map->sgt);

	iommu_unmap_sg(dp_domain, map->sgt->sgl, map->sgt->nents,
				       map->dir, NULL);
	destroy_sgt(map->sgt);
	kfree(map);
}

dma_addr_t malidp_iommu_dma_addr(struct malidp_iommu_domain *dp_domain,
				 struct malidp_iommu_mapping *map)
{
	return sg_dma_address(map->sgt->sgl);
}

static int malidp_iommu_fault_handler(struct iommu_domain *domain,
		struct device *dev, unsigned long iova, int flags, void *data)
{
	struct device *dpdev = data;

	dev_err_ratelimited(dpdev, "iommu fault in %s access (iova = 0x%lx)\n",
			    (flags & IOMMU_FAULT_WRITE) ? "write" : "read",
			    iova);

	/*
	 * We cannot really do anything if we get a page fault. The memory
	 * should have been already mapped.
	 */
	return -EFAULT;
}

struct malidp_iommu_domain *malidp_iommu_init(struct device *dev,
				       struct bus_type *bus)
{
	unsigned int count = MALIDP_DEV_ADDR_SIZE >> (PAGE_SHIFT + MALIDP_DEV_ADDR_ORDER);
	unsigned int bitmap_size = BITS_TO_LONGS(count) * sizeof(long);
	struct malidp_iommu_domain *dp_domain;
	int ret;

	dp_domain = kzalloc(sizeof(struct malidp_iommu_domain), GFP_KERNEL);
	if (!dp_domain) {
		dev_err(dev, "could not alloc malidp iommu domain\n");
		return NULL;
	}

	dp_domain->domain = iommu_domain_alloc(bus);
	if (!dp_domain->domain) {
		dev_dbg(dev, "could not alloc iommu domain\n");
		goto err_alloc;
	}

	ret = iommu_attach_device(dp_domain->domain, dev);
	if (ret < 0) {
		dev_dbg(dev, "could not attach device to domain %p\n",
			dp_domain->domain);
		goto err_attach;
	}

	iommu_set_fault_handler(dp_domain->domain, malidp_iommu_fault_handler,
				dev);

	dp_domain->dev = dev;
	dp_domain->order = MALIDP_DEV_ADDR_ORDER;
	dp_domain->base = MALIDP_DEV_ADDR_START;
	spin_lock_init(&dp_domain->lock);

	dp_domain->bits = BITS_PER_BYTE * bitmap_size;

	dp_domain->bitmap = kzalloc(bitmap_size, GFP_KERNEL);
	if (!dp_domain->bitmap) {
		dev_err(dev, "could not alloc iommu bitmap\n");
		goto err_bitmap;
	}

	dev_info(dev, "attached to iommu domain %p", dp_domain->domain);

	return dp_domain;

err_bitmap:
	iommu_detach_device(dp_domain->domain, dp_domain->dev);
err_attach:
	iommu_domain_free(dp_domain->domain);
err_alloc:
	kfree(dp_domain);

	return NULL;
}

void malidp_iommu_exit(struct malidp_iommu_domain *dp_domain)
{
	iommu_detach_device(dp_domain->domain, dp_domain->dev);
	iommu_domain_free(dp_domain->domain);
	kfree(dp_domain);
}

#else /* CONFIG_IOMMU_API=n */

struct malidp_iommu_mapping *malidp_iommu_map_sgt(struct malidp_iommu_domain *dp_domain,
			   struct sg_table *sgt,
			   enum dma_data_direction dir)
{
	return NULL;
}

void malidp_iommu_unmap_sgt(struct malidp_iommu_domain *dp_domain,
			    struct malidp_iommu_mapping *map)
{
	return;
}

dma_addr_t malidp_iommu_dma_addr(struct malidp_iommu_domain *dp_domain,
				 struct malidp_iommu_mapping *map)
{
	return 0;
}

struct malidp_iommu_domain *malidp_iommu_init(struct device *dev,
				       struct bus_type *bus)
{
	return NULL;
}

void malidp_iommu_exit(struct malidp_iommu_domain *dp_domain)
{
	return;
}

#endif /* CONFIG_IOMMU_API */

