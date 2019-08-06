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
 * x86 mmu ASM implementation.
 * Implemented according to Intel Developer's Manual
 *
 * Author - Alexander Brill.
 *****************************************************************************/
#include <mmu.h>
#include <mmio.h>


.global mmuInitStart
.extern mmuInitDone

.extern _Gdt;

#if ISH_CONFIG_SUPPORT_PAGING

.section .text.ish_defined_section.init_flow
mmuInitStart:


	call mmuInit

	//Enable Paging
	movl	$mmuPageDirectoryPhys, %eax
	movl	%eax, %cr3

	movl	%cr4, %eax
	orl	    $0x000010, %eax			// CR4.PSE = 1 (enables 4MB page size)
	andl	$0xffffffdf, %eax		// CR4.PAE = 0 (pysical address extention)
	movl	%eax, %cr4

	movl	%cr0, %eax
	orl		$0x80010000, %eax		// CR0.PG = 1, CR0.WP = 1
	movl	%eax, %cr0

	movw    $0x10,%ax
	movw    %ax,%ds
	movw    %ax,%es

	movw    $0x28,%ax
	movw    %ax,%ss

	movw    $0x30,%ax
	movw	%ax, %gs

	movw    $0x0,%ax
	movw	%ax,%fs

	lgdt	_Gdt

    movw    $0x40, %ax           // tss selector	
    ltr     %ax 
	ljmpl	$8,$pagingEnabled

pagingEnabled:
    #call	mmuRemoveOneToOneMapping    // need one to one mapping for aon task
	jmp		mmuInitDone

#endif
