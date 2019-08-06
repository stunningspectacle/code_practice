/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2014 Intel Corporation. All Rights Reserved.

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

#include <ish_config.h>

#if ISH_CONFIG_SUPPORT_PAGING

#include "mmu.h"
#include "exception_support.h"
#include "os_api.h"
#include "page_mgr.h"
#include "exception.h"

#if (ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER == 1)

extern RTOS_TRACE_STRING tr_paging_channel ;
extern RTOS_TRACE_HANDLE tr_paging_handle  ;

#endif

__pinned_kernel_data__ int pf_counter = 0;

extern volatile  pageTable_t  mmuPageTable;
extern void fatal_error_handler(uint32_t reason, const EXC_FRAME *exc);

__pinned_kernel_code__ void demandPgMgrHook(vPortExcFrame* exc_frame)
{
    volatile MMU_PAGE_TABLE_ENTRY * pte;

#ifdef MINIMA
    RTOS_TRACE_ISR_BEGIN(tr_paging_handle);
#endif

    pte = mmuPageTable.entry + MMU_PAGE_NUM(exc_frame->cr2);
    
#if ISH_CONFIG_SUPPORT_DEMAND_PAGING
    if(pte->alloc == 0 ||(pf_counter++ != 0) || (demandPgPageFaultHandler(0, exc_frame, pte) != OS_OK))
#endif
    {
    	printk("******* Fatal page fault occurred while accessing 0x%x, pf_counter = %d *******\n", exc_frame->cr2, pf_counter);
    	fatal_error_handler(EXC_PAGE_FAULT, exc_frame);
    	// Unreachable code
    }
#if ISH_CONFIG_SUPPORT_DEMAND_PAGING
    pf_counter --;
#endif

#ifdef MINIMA
    RTOS_TRACE_ISR_END(0);
#endif
}


__pinned_kernel_code__ volatile MMU_PAGE_TABLE * get_mmu_page_table()
{
	return &mmuPageTable;
}

#endif
