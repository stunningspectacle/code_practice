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

/******************************************************************************
 * mmu.c
 *
 * x86 mmu implementation.
 * Implemented according to Intel Developer's Manual
 *
 * Author - Alexander Brill.
 *****************************************************************************/
#include <defs.h>

#if ISH_CONFIG_SUPPORT_PAGING

#include <FreeRTOSConfig.h>
#include <mmu.h>
#include <mmio.h>
#include "page_mgr.h"
#include "ish_sections.h"
#include "utils.h"
#include "os_api.h"
#include "aon_shared.h"


__stationary_kernel_data__ volatile pageDirectory_t	mmuPageDirectory	__attribute__((aligned(MMU_PAGE_SIZE)));
__stationary_kernel_data__ volatile pageTable_t		mmuPageTable  		__attribute__((aligned(MMU_PAGE_SIZE)));

#if ISH_CONFIG_SUPPORT_ISH_HW

// linker symbols
extern unsigned int ISH_BOOT_START;
extern unsigned int __image_size;

#define ish_boot_start ((unsigned)&ISH_BOOT_START)
#define image_size ((unsigned)&__image_size)

// main-sections barriers: boot code, code, boot data and data sections.
extern unsigned int __bootcode_start;
extern unsigned int __bootcode_end;
extern unsigned int __code_start;
extern unsigned int __code_end;
extern unsigned int __bootdata_start;
extern unsigned int __bootdata_end;
extern unsigned int __stacks_start;
extern unsigned int __stacks_end;
extern unsigned int __data_start;



#define bootcode_start  ((unsigned)&__bootcode_start)
#define bootcode_end  ((unsigned)&__bootcode_end)
#define code_start  ((unsigned)&__code_start)
#define code_end  ((unsigned)&__code_end)
#define bootdata_start  ((unsigned)&__bootdata_start)
#define bootdata_end  ((unsigned)&__bootdata_end)
#define stacks_start  ((unsigned)&__stacks_start)
#define stacks_end  ((unsigned)&__stacks_end)
#define data_start  ((unsigned)&__data_start)


// inner-sections barriers
extern unsigned int __stat_code_start;
extern unsigned int __stat_code_end;
extern unsigned int __pinned_code_start;
extern unsigned int __pinned_code_end;
extern unsigned int __init_code_start;
extern unsigned int __init_code_end;
extern unsigned int __stat_data_start;
extern unsigned int __stat_data_end;
extern unsigned int __pinned_data_start;
extern unsigned int __pinned_data_end;
extern unsigned int __pinned_bss_start;
extern unsigned int __pinned_bss_end;
extern unsigned int __init_data_start;
extern unsigned int __init_data_end;
extern unsigned int __bss_end;

#define stat_code_start  ((unsigned)&__stat_code_start)
#define stat_code_end  ((unsigned)&__stat_code_end)
#define pinned_code_start  ((unsigned)&__pinned_code_start)
#define pinned_code_end  ((unsigned)&__pinned_code_end)
#define init_code_start  ((unsigned)&__init_code_start)
#define init_code_end  ((unsigned)&__init_code_end)
#define stat_data_start  ((unsigned)&__stat_data_start)
#define stat_data_end  ((unsigned)&__stat_data_end)
#define pinned_data_start  ((unsigned)&__pinned_data_start)
#define pinned_data_end  ((unsigned)&__pinned_data_end)
#define pinned_bss_start  ((unsigned)&__pinned_bss_start)
#define pinned_bss_end  ((unsigned)&__pinned_bss_end)
#define init_data_start  ((unsigned)&__init_data_start)
#define init_data_end  ((unsigned)&__init_data_end)
#define bss_end  ((unsigned)&__bss_end)

__pinned_kernel_data__ uint32_t dram_base = 0;

#define MMU_RANGE (SRAM_SIZE/MMU_PAGE_SIZE)
#define MMU_PAGES_IN_RANGE(start, end) (((end) - (start))) / MMU_PAGE_SIZE

#define LINEAR_TO_DRAM_PHYS(addr) ((addr & 0xFFFFF000) + dram_base)
#define LINEAR_TO_SRAM_PHYS(addr) (((addr - ish_boot_start) & 0xFFFFF000) + SRAM_BASE)

#define SRAM_PHYS_TO_LINEAR(addr) (addr - (SRAM_BASE - ish_boot_start))

#define MMU_BCODE_PAGE_NUM(addr) MMU_PAGES_IN_RANGE(bootcode_start, addr)

#define MMU_BDATA_PAGE_NUM(addr) MMU_BCODE_PAGE_NUM(addr)

typedef uint16_t MMU_PG_INDEX;	/* used to identify a page table entry */
typedef uint16_t MMU_PG_COUNT;	/* used to count # of page table entries */

#define MMU_PG_DIR_IX(vaddr) ((vaddr) >> 22) 
#define MMU_PG_TBL_IX(vaddr) ((MMU_PG_INDEX)((VIRT_ADDR)(vaddr) >> 12 & 0x3ff))

#define PHYS_ADDR_NONE		(PHYS_ADDR)(-1)

#define PAGE_DIR_4MB_INDEX  (SRAM_BASE >> 22)

__pinned_kernel_code__ static inline void invalidateTLB()
{
    __asm__ volatile (
            "movl  %%cr3, %%eax;\n\t"
            "movl  %%eax, %%cr3;\n\t"
            :
            :
            : "eax"
             );
}



__init_flow_code__ void mmuPageTableInit()
{

	// Map boot code & data segments in the page table
	mmuMap(bootcode_start, LINEAR_TO_SRAM_PHYS(bootcode_start), bootcode_end - bootcode_start, MMU_ENTRY_PRESENT);
	mmuMap(bootdata_start, LINEAR_TO_SRAM_PHYS(bootcode_end), bootdata_end - bootdata_start, MMU_ENTRY_PRESENT | MMU_ENTRY_DIRTY | MMU_ENTRY_WRITE);

    // Map system stack
	mmuMap(stacks_start, LINEAR_TO_DRAM_PHYS(stacks_start), stacks_end - stacks_start, MMU_ENTRY_WRITE);

    // Map code sections to DRAM addresses
    mmuMap(code_start, LINEAR_TO_DRAM_PHYS(code_start), code_end - code_start, 0);

    // Map data & bss
    mmuMap(data_start, LINEAR_TO_DRAM_PHYS(data_start), bss_end - data_start, MMU_ENTRY_WRITE);


    dram_mark_busy(ALIGN_UP(image_size, MMU_PAGE_SIZE)/MMU_PAGE_SIZE);
}

__init_flow_code__ void mmuInitSramRange(unsigned int sec_start, unsigned int sec_end, unsigned int page, uint8_t flags)
{
	unsigned int size, dram_addr;
	unsigned int aligned_sec_start = ALIGN_DOWN(sec_start, MMU_PAGE_SIZE);
	unsigned int aligned_sec_end = ALIGN_UP(sec_end, MMU_PAGE_SIZE);


	size = MMU_PAGES_IN_RANGE(aligned_sec_start, aligned_sec_end);
	dram_addr = LINEAR_TO_DRAM_PHYS(aligned_sec_start);

	alloc_sram(page, size, 0, aligned_sec_start, dram_addr, flags);
}

__init_flow_code__ void mmuInitSram()
{
	// ----- map boot code
	mmuInitSramRange(init_code_start, init_code_end, MMU_BCODE_PAGE_NUM(init_code_start), MEM_ATTR_NONE);
	mmuInitSramRange(stat_code_start, stat_code_end, MMU_BCODE_PAGE_NUM(stat_code_start), MEM_ATTR_PIN | MEM_ATTR_STAT);
	mmuInitSramRange(pinned_code_start, pinned_code_end, MMU_BCODE_PAGE_NUM(pinned_code_start), MEM_ATTR_PIN);

	// ----- map boot data
	mmuInitSramRange(init_data_start, init_data_end, MMU_BDATA_PAGE_NUM(init_data_start), MEM_ATTR_NONE);
	mmuInitSramRange(stat_data_start, stat_data_end, MMU_BDATA_PAGE_NUM(stat_data_start), MEM_ATTR_PIN | MEM_ATTR_STAT);
	mmuInitSramRange(pinned_data_start, pinned_data_end, MMU_BDATA_PAGE_NUM(pinned_data_start), MEM_ATTR_PIN);
	mmuInitSramRange(pinned_bss_start, pinned_bss_end, MMU_BDATA_PAGE_NUM(pinned_bss_start), MEM_ATTR_PIN);

    // ----- map aon task memory
#if !ISH_CONFIG_SUPPORT_AON_INDEPENDENT_BANK
    ASSERT_CONST(((AON_TASK_BASE - SRAM_BASE)>>MMU_PAGE_SHIFT) < SRAM_PAGES);
    unsigned aon_page_index = (AON_TASK_BASE - SRAM_BASE)>>MMU_PAGE_SHIFT;
	mmuInitSramRange(SRAM_PHYS_TO_LINEAR(AON_TASK_BASE), SRAM_PHYS_TO_LINEAR(AON_TASK_BASE) + AON_MEMORY_SIZE, aon_page_index, MEM_ATTR_PIN | MEM_ATTR_STAT);
#endif

}

__init_flow_code__ void mmuInit (void)
{
	uint32_t  value;
	uint32_t page_dir_4MB_index = SRAM_BASE >> 22;
    extern uint32_t mmuPageTablePhys;

	/*
	 * A single page directory is not big enough to map both data and text.
	 * Create a page directory entry that maps the text section directly
	 * onto a 4 MB page (i.e. no need for a page table).
	 */
	value = (MMU_ENTRY_PRESENT |
			 MMU_ENTRY_WRITE |
			 MMU_ENTRY_SUPERVISOR |		
			 MMU_ENTRY_WRITE_BACK |
			 MMU_ENTRY_CACHING_ENABLE |
			 MMU_ENTRY_NOT_GLOBAL |		
			 MMU_4MB_PDE_SET_PS |		
			 (SRAM_BASE & MMU_4MB_PDE_PAGE_MASK));

	mmuPageDirectory.entry[page_dir_4MB_index].fourMb.value = value;

	value = (MMU_ENTRY_PRESENT |
				 MMU_ENTRY_WRITE |
				 MMU_ENTRY_SUPERVISOR |
				 MMU_ENTRY_WRITE_BACK |
				 MMU_ENTRY_CACHING_ENABLE |
				 MMU_ENTRY_NOT_GLOBAL |
				 MMU_4MB_PDE_SET_PS |
				 (ISH_CONFIG_AON_BASE & MMU_4MB_PDE_PAGE_MASK));

	    page_dir_4MB_index = ISH_CONFIG_AON_BASE >> 22;
		mmuPageDirectory.entry[page_dir_4MB_index].fourMb.value = value;

	value =	(MMU_ENTRY_PRESENT |
			 MMU_ENTRY_WRITE |
			 MMU_ENTRY_SUPERVISOR |     
			 MMU_ENTRY_WRITE_BACK |     
			 MMU_ENTRY_CACHING_ENABLE | 
			 ((PHYS_ADDR)((PHYS_ADDR) &mmuPageTablePhys ) & MMU_PDE_PAGE_TABLE_MASK));

	mmuPageDirectory.entry[0].pt.value = value;

#if !ISH_CONFIG_SUPPORT_DEMAND_PAGING
	// should be removed once we have demand paging support on GLV
	value = (MMU_ENTRY_PRESENT			|
			 MMU_ENTRY_WRITE			|
			 MMU_ENTRY_SUPERVISOR		|
			 MMU_ENTRY_WRITE_BACK		|
			 MMU_ENTRY_CACHING_ENABLE	|
             MMU_ENTRY_ALLOC_MASK       |  
			 (0 & MMU_PTE_PAGE_MASK));

	uint32_t page;
	for (page = 1; page < MMU_RANGE; ++page)
	{
	    if ((page*MMU_PAGE_SIZE) >= (uint32_t)&__stacks_start) //DCCM
        {
            mmuPageTable.entry[page].value = value | ((SRAM_BASE + ISH_CONFIG_DATA_CCM_OFFSET + ((page - 1) * MMU_PAGE_SIZE)) & MMU_PTE_PAGE_MASK);

        }
        else //ICCM
        {
   		    mmuPageTable.entry[page].value = value | ((SRAM_BASE + ((page - 1) * MMU_PAGE_SIZE)) & MMU_PTE_PAGE_MASK);
        }
	}
#else
	pageManagerInit(0x38, &dram_base); // pass which selector to use as temporary mmio segment
	mmuPageTableInit();
	mmuInitSram();
#endif
}

__pinned_kernel_code__ void mmuRemoveOneToOneMapping(void)
{
	uint32_t page_dir_4MB_index = SRAM_BASE >> 22;
	mmuPageDirectory.entry[page_dir_4MB_index].fourMb.value = 0;
}

__pinned_kernel_code__ void mmuMap(unsigned int virt, unsigned int phys, unsigned int size, unsigned int flags)
{
	uint32_t value = (flags          			|
					 MMU_ENTRY_SUPERVISOR 		|  /* Allow PL3 access to 4MB region. */
					 MMU_ENTRY_WRITE_BACK 		|  /* Also enable copy-back caching */
					 MMU_ENTRY_CACHING_ENABLE 	|  /* mode for the page table.      */
                     MMU_ENTRY_ALLOC_MASK       |  /* allocated flag */ 
					 (0 & MMU_PTE_PAGE_MASK));

	for (int v = virt; v < virt + size; v += MMU_PAGE_SIZE, phys += MMU_PAGE_SIZE)
	{
		mmuPageTable.entry[MMU_PAGE_NUM(v)].value = value | (phys & MMU_PTE_PAGE_MASK);
	}

	invalidateTLB();
}


__pinned_kernel_code__ void mmuUnmap(unsigned int virt, unsigned int size)
{
    for (int v = virt; v < virt + size; v += MMU_PAGE_SIZE)
    {
        mmuPageTable.entry[MMU_PAGE_NUM(v)].value &= ~MMU_ENTRY_PRESENT;
    }

    invalidateTLB();
}

__pinned_kernel_code__ uint32_t nanoCpuMmuVirtToPhys(uint32_t id, uint32_t vAddr)
{
	(void) id;

    uint16_t pdeIdx;
    uint16_t pteIdx;
    PHYS_ADDR pAddr = PHYS_ADDR_NONE;
    pageTableEntry_t pte; 

    pdeIdx = MMU_PG_DIR_IX(vAddr);
    if(pdeIdx >=0 && pdeIdx < 1024) {

        if(pdeIdx == PAGE_DIR_4MB_INDEX) {
            // vAddr lies in the 4 MB regions
            pAddr = (PHYS_ADDR)mmuPageDirectory.entry[PAGE_DIR_4MB_INDEX].fourMb.value;
            pAddr = (PHYS_ADDR)(pAddr & MMU_4MB_PDE_PAGE_MASK);
            pAddr |= (PHYS_ADDR)(vAddr & ~MMU_4MB_PDE_PAGE_MASK);
        } else if (pdeIdx == 0){ 
            pteIdx = MMU_PG_TBL_IX(vAddr);
            pte.value = mmuPageTable.entry[pteIdx].value;
            pAddr = pte.page << MMU_PAGE_SHIFT;
        } else {
            syslog(LOG_COMP_DMA|LOG_ERR, "invalid page table directory indx:%d for virtual addr:0x%08x", pdeIdx, vAddr); 
        }
    } else {
        syslog(LOG_COMP_DMA|LOG_ERR, "page table directory indx:%d for virtual addr:0x%08x is out of range:[0,1024)", pdeIdx, vAddr); 
    }

    return pAddr | (vAddr & MMU_PAGE_MASK);
}

#endif
#endif
