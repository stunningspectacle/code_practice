/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef _LINUX_ROCKCHIP_ION_H
#define _LINUX_ROCKCHIP_ION_H
#include <linux/ion.h>
#define ROCKCHIP_ION_VERSION "v1.1"
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
enum ion_heap_ids {
 ION_VMALLOC_HEAP_ID = 0,
 ION_CARVEOUT_HEAP_ID = 2,
 ION_CMA_HEAP_ID = 4,
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 ION_SECURE_HEAP_ID = 5,
};
#define ION_HEAP(bit) (1 << (bit))
struct ion_phys_data {
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
 ion_user_handle_t handle;
 unsigned long phys;
 unsigned long size;
};
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define ION_IOC_ROCKCHIP_MAGIC 'R'
#define ION_IOC_GET_PHYS _IOWR(ION_IOC_ROCKCHIP_MAGIC, 0,   struct ion_phys_data)
#define ION_IOC_ALLOC_SECURE _IOWR(ION_IOC_ROCKCHIP_MAGIC, 1,   struct ion_phys_data)
#define ION_IOC_FREE_SECURE _IOWR(ION_IOC_ROCKCHIP_MAGIC, 2,   struct ion_phys_data)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#endif
