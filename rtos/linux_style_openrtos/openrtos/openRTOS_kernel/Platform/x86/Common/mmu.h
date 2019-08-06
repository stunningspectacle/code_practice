/*++
 INTEL CONFIDENTIAL
 Copyright (c) 2012 - 2015 Intel Corporation. All Rights Reserved.

 The source code contained or described herein and all documents related
 to the source code ("Material") are owned by Intel Corporation or its
 suppliers or licensors. Title to the Material remains with Intel Corporation
 or its suppliers and licensors. The Material contains trade secrets and
 proprietary and confidential information of Intel or its suppliers and
 licensors. The Material is protected by worldwide copyright and trade secret
 laws and treaty provisions. No part of the Material may be used, copied,
 reproduced, modified, published, uploaded, posted, transmitted, distributed,
 or disclosed in any way without Intel's prior express written permission.

 No license under any patent, copyright, trade secret or other intellectual
 property right is granted to or conferred upon you by disclosure or delivery
 of the Materials, either expressly, by implication, inducement, estoppel or
 otherwise. Any license under such intellectual property rights must be
 express and approved by Intel in writing.
 --*/

#ifndef _OPENRTOS_MMU_H
#define _OPENRTOS_MMU_H

#include <ish_config.h>
#include <mmio.h>

#define MMU_PAGE_SIZE 4096

#define VTOP_ADDR (ISH_CONFIG_SRAM_BASE- BOOT_START_ADDR)

#define GDT_PHYS_SRAM_BASE    (VTOP_ADDR)
#define GDT_PHYS_SRAM_BASE_LO ((VTOP_ADDR & 0x0000ffff) >> 0)
#define GDT_PHYS_SRAM_BASE_MI ((VTOP_ADDR & 0x00ff0000) >> 16)
#define GDT_PHYS_SRAM_BASE_HI ((VTOP_ADDR & 0xff000000) >> 24)

#define GDT_PHYS_SRAM_LIMT    0xc01000
#define GDT_PHYS_SRAM_LIMT_LO 0x1000
#define GDT_PHYS_SRAM_LIMT_HI 0xc0

#define GDT_MMIO_SEG_LIMT_LO ((MMIO_LIMIT >> 12) & 0xFFFF)
#define GDT_MMIO_SEG_LIMT_HI (((MMIO_LIMIT >> 28) & 0xFF) | 0xC0)

#if ISH_CONFIG_SUPPORT_PAGING

// Defines for page directory entry (PDE) which points to a page table
#define MMU_PDE_PAGE_TABLE_MASK		0xfffff000

// Defines for page directory entry (PDE) which points to a 4MB memory
#define MMU_4MB_PDE_PAGE_MASK       0xffc00000
#define MMU_4MB_PDE_SET_PS          0x00000080

// Defines for page table entry (PTE) structure
#define MMU_PTE_PAGE_MASK           0xfffff000
#define MMU_ENTRY_PRESENT           0x00000001
#define MMU_ENTRY_WRITE             0x00000002
#define MMU_ENTRY_DIRTY             0x00000040
#define MMU_ENTRY_SUPERVISOR        0x00000000
#define MMU_ENTRY_WRITE_BACK        0x00000000
#define MMU_ENTRY_CACHING_ENABLE    0x00000000
#define MMU_ENTRY_CACHING_DISABLE   0x00000010
#define MMU_ENTRY_NOT_GLOBAL        0x00000000
#define MMU_ENTRY_ALLOC_MASK        0x00000200

#define MMIO_PTE_FLAGS  (MMU_ENTRY_PRESENT |    \
                         MMU_ENTRY_WRITE   |    \
                         MMU_ENTRY_CACHING_DISABLE)

/* code below only needed when not including this file for assembly */
#ifndef __ASSEMBLY__
  #include <stdint.h>

// Format of a page directory entry (PDE) which points to a page table
// Intel Software Developer Manual, Vol 3A,  Table 4-5
typedef union {
	uint32_t  value;
	struct {
		uint32_t p:1;       // present
		uint32_t rw:1;      // read/write */
		uint32_t us:1;      // user/supervisor
		uint32_t pwt:1;     // page-level write-through
		uint32_t pcd:1;     // page-level cache disable
		uint32_t a:1;       // accessed
		uint32_t ignored1:1;
		uint32_t ps:1;      // page size
		uint32_t ignored2:4;
		uint32_t pageTbl:20;// page table physical address
	};
} pageDirectoryEntryPT_t;


// Format of a page directory entry (PDE) which points to a 4MB memory
// Intel Software Developer Manual, Vol 3A,  Table 4-4
typedef union {
    uint32_t  value;
    struct {
        uint32_t p:1;       // present
        uint32_t rw:1;      // read/write
        uint32_t us:1;      // user/supervisor
        uint32_t pwt:1;     // page-level write-through
        uint32_t pcd:1;     // page-level cache disable
        uint32_t a:1;       // accessed
        uint32_t d:1;       // dirty
        uint32_t ps:1;      // page size
        uint32_t g:1;       // global
        uint32_t ignored1:3;
        uint32_t pat:1;     // If PAT is supported, indirectly determines the
                            // memory type used to access the 4 Mb page,
                            // otherwise must be 0
        uint32_t pageTbl:6; // Bits (M–1):32 of physical address of the 4-MByte page referenced by this entry
        uint32_t ignored2:3;
        uint32_t page:10;   // Bits 31:22 of physical address of the 4-MByte page referenced by this entry
    };
} pageDirectoryEntry4MB_t;


// Format of a 32-bit page table entry
// Intel Software Developer Manual, Vol 3A,  Table 4-6
typedef union {
    uint32_t  value;
    struct {
        uint32_t p:1;       // present
        uint32_t rw:1;      // read/write
        uint32_t us:1;      // user/supervisor
        uint32_t pwt:1;     // page-level write-through
        uint32_t pcd:1;     // page-level cache disable
        uint32_t a:1;       // accessed
        uint32_t d:1;       // dirty
        uint32_t pat:1;     // If PAT is supported, indirectly determines the
                            // memory type used to access the 4 Kb page,
                            // otherwise must be 0
        uint32_t g:1;       // global
        uint32_t alloc:1;   /* allocated: if 1 -> this PTE has been allocated/
                               reserved; this is only used by software,
                               i.e. this bit is ignored by the MMU */
        uint32_t custom:2;  // Ignored by h/w, available for use by s/w
        uint32_t page:20;   // physical address of the 4 Kb page
        };
} pageTableEntry_t;


typedef union {
	pageDirectoryEntryPT_t    pt;
	pageDirectoryEntry4MB_t   fourMb;
} pageDirectoryEntry_t;


/* Page Directory structure for 32-bit paging mode */

typedef struct {
	pageDirectoryEntry_t entry[1024];
} pageDirectory_t;
     

/* Page Table structure for 32-bit paging mode */

typedef struct {
	pageTableEntry_t entry[1024];
} pageTable_t;


void mmuMap(unsigned int virt, unsigned int phys, unsigned int size, unsigned int flags);
void mmuUnmap(unsigned int virt, unsigned int size);


#define MMU_PAGE_NUM(v) (((v) >> 12) & 0x3ff)

#endif /* _ASMLANGUAGE */

#endif

#endif /* _OPENRTOS_MMU_H */
