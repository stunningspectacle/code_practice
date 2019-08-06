/*
	OpenRTOS V8.2.1  Copyright (C) 2015 Real Time Engineers Ltd.

	This file is part of the OpenRTOS product.

	OpenRTOS is distributed exclusively by WITTENSTEIN high integrity systems,
	and is subject to the terms of the License granted to your organization,
	including its warranties and limitations on use and distribution. It cannot be
	copied or reproduced in any way except as permitted by the License.

	Licenses authorize use by processor, compiler, business unit, and product.
	
	WITTENSTEIN high integrity systems is a trading name of WITTENSTEIN
	aerospace & simulation ltd, Registered Office: Brown's Court, Long Ashton
	Business Park, Yanley Lane, Long Ashton, Bristol, BS41 9LB, UK.
	Tel: +44 (0) 1275 395 600, fax: +44 (0) 1275 393 630.
	E-mail: info@HighIntegritySystems.com

	http://www.HighIntegritySystems.com
*/

#include <FreeRTOSConfig.h>

	.extern mmuInitStart
	.extern main
	.extern mmapHW
	.global _start_freertos
	.global mmuInitDone
#ifdef BSP_clanton
	.global _mboot_hdr
	.global __text_start
	.global __bss_start
	.global __bss_end
 	/* Local constants for multiboot section */
  	.set ALIGN,    	1<<0             	/* align loaded modules on page boundaries */
	.set MEMINFO,  	1<<1             	/* provide memory map */
	.set MAGIC,    	0x1BADB002       	/* 'magic number' lets bootloader find the header */
	.set FLAGS, 	ALIGN|MEMINFO  		/* this is the multiboot 'flag' field */
	.set CHECKSUM, 	-(MAGIC + FLAGS) 	/* checksum of above */
#endif
 
 /* Entry point see linker script */
	.section .entrypoint
	.align 4
	
#ifdef BSP_clanton
 	/* Skip mb header */
	jmp _start_freertos
	/* Multiboot header */
	_mboot_hdr:
	.long MAGIC							/* offset = 0 */
	.long FLAGS							/* offset = 4 */
	.long CHECKSUM						/* offset = 8 */
 	.long _mboot_hdr					/* should be header address - offset = 12 */
	.long __text_start					/* load address (start of text) - offset = 16 */
	.long __bss_start					/* load end address (end of data) - offset = 20*/
	.long __bss_end 					/* bss end address - offset = 24*/
	.long _start_freertos				/* entry_addr - offset = 28*/
#endif

.func _start_freertos
_start_freertos:

	cli
	lgdt  _GdtPrePagingPhy 
	
#if ISH_CONFIG_SUPPORT_PAGING
/* use DS with address base pointing to the loaded location of the boot data section */
	movw	$0x20,%ax //load temporary segment base
	movw	%ax,%ds
	movw	%ax,%es
	movw	%ax,%ss
	movw	%ax,%gs

#else
	movw    $0x10,%ax
	movw    %ax,%ds
	movw    %ax,%es

	movw    $0x28,%ax
	movw    %ax,%ss

	movw    $0x30,%ax
	movw	%ax, %gs
#endif
    
	/* fs is reserved, so point it to an invalid entry */
	movw	$0,%ax
	movw	%ax,%fs

#if ISH_CONFIG_SUPPORT_PAGING
/* use CS with address base = (SRAM DATA BASE - 0x1000). this way, at the moment, we can use logical addressing, eventhough paging is not enabled yet */
	ljmp	    $0x18, $useCodeSegment
#else
	ljmp	    $0x8, $useCodeSegment
#endif

useCodeSegment:


	/* Clear pinned .bss section.  using logical address, due to current segment base */
	xorl	%eax, %eax
	movl	$__pinned_bss_start,  %edi
	movl	$pinned_bss_size_words, %ecx
	rep stosl

	/* Initialize stack space and stack pointer. using logical address, due to current segment base */
	movl	$0xecececec, %eax
	movl	$ulSystemStack, %edi
	movl	$configISR_STACK_SIZE, %ecx
	rep stosl
	movl	%edi, %esp

 	/* No frame yet. */
	movl %esp, %ebp

#ifndef BSP_glv
	/* Enable write-back caching by clearing the NW and CD bits */
	movl	%cr0, %eax
	andl	$0x9fffffff, %eax		// CR0.NW = 0, CR0.CD = 0
	movl	%eax, %cr0
#endif


#if ISH_CONFIG_SUPPORT_PAGING
#ifndef BSP_clanton
	jmp mmuInitStart
mmuInitDone:
	call mmapHW
    call updateSramDataBase
#endif
#endif

    call setupInitialIDT
    call zeroBss

	/* Initialize IOAPIC */
	call initIOApic

	jmp c_entry_point

.endfunc

.end
