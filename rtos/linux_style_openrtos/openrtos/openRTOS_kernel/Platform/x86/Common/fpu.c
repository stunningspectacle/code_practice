/*++
 INTEL CONFIDENTIAL
 Copyright (c) 2012 - 2016 Intel Corporation. All Rights Reserved.

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
 * fpu.c
 *
 *
 *****************************************************************************/
#include <ish_sections.h>
#include <fpu.h>


extern TCB_t *pxCurrentTCB;
extern TCB_t *pxFpTcbOwner;
extern void sys_fatal_device_not_available(EXC_FRAME *exc);
/**
 * Handler for floating point exception handling.
 * To prevent unnecessary save and load floating point registers when they are
 * not in use - FP is set to be disabled - when a new task uses FP operations it's
 * being set to enabled, and the registers of the previous fp owner are saved.
 * In addition in each context switch  - in a case the new thread is known as uses
 * FP and it is different than the current fp owner - all fp register are saved
 * and the registers for the new thread are restored.
 */
__pinned_kernel_code__ void fp_device_not_available_handler(EXC_FRAME *exc)
{

	(void)exc;

	//Enable FP (clear CR0[TS]).
	__asm__ volatile ("clts\n\t");

	//Save FP context when there is a previous fpOwner.
	if(pxFpTcbOwner)
	{
		//Save the FPU context of the previous thread
		__asm__ volatile (
				"fnsave (%0);\n\t"
				:
				: "r" (pxFpTcbOwner->fpContextPtr) : "memory"
				);
		__asm volatile("fwait\n\t");
	}

	//Allocate fp context for the new thread if needed
	if(pxCurrentTCB->fpContextPtr == NULL)
	{
		if(currentNumberOfFpTasks >= ISH_CONFIG_MAX_FPU_TASKS)
		{
			sys_fatal_device_not_available(exc);
			return;
		}
		currentNumberOfFpTasks++;
		pxCurrentTCB->fpContextPtr = FPU_POOL + ((currentNumberOfFpTasks - 1) * portFPU_CONTEXT_SIZE_BYTES);
		// Initialise the floating point registers.
		__asm volatile(	"fninit\n\t" );

	}

	pxFpTcbOwner = pxCurrentTCB;
}

