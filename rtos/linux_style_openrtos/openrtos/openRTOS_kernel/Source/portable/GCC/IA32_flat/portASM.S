/*
	OpenRTOS V8.2.3  Copyright (C) 2015 Real Time Engineers Ltd.

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

.file "portASM.S"
#include "FreeRTOSConfig.h"
#include "ISR_Support.h"
#include <ISHRegs.h>

	.extern pxCurrentTCB
	.extern vTaskSwitchContext
	.extern vPortCentralInterruptHandler
	.extern xTaskIncrementTick
	.extern vPortAPICErrorHandler
	.extern ulPortYieldPending
	.extern idle_ticks
	.extern wdt_reload
	.extern _wdtIntHandler

	.global vPortStartFirstTask
	.global vPortCentralInterruptWrapper
	.global vPortAPICErrorHandlerWrapper
	.global vPortTimerHandler
	.global vPortYieldCall
	.global vPortAPICSpuriousHandler
	.global vPortWdtHandler

	.text

/*-----------------------------------------------------------*/

.align 4
.func vPortYieldCall
vPortYieldCall:
	/* Save general purpose registers. */
	pusha

	call wdt_reload

	/* Find the TCB. */
	movl 	pxCurrentTCB, %eax

	/* Stack location is first item in the TCB. */
	movl	%esp, (%eax)

	call vTaskSwitchContext

	/* Find the location of pxCurrentTCB again - a callee saved register could
	be used in place of eax to prevent this second load, but that then relies
	on the compiler and other asm code. */
	movl 	pxCurrentTCB, %eax
	movl	(%eax), %esp


	popa
	iret

.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortWdtHandler
vPortWdtHandler:
	/* setup stack as vPortExcFrame layout */
	pushl $0                /* push error code */
	pusha
	pushl $0                /* pseudo CR2 */
	/* push vPortExcFrame pointer to stack, argument to _wdtIntHandler */
	pushl %esp
	call _wdtIntHandler
	addl $8, %esp           /* discard vPortExcFrame pointer and CR2 */
	popa
	addl $4, %esp           /* discard error code */

	/* EOI IOAPIC and LAPIC */
	movl $WDT_VEC, %gs:(IOAPIC_EOI)
	movl $0x00, %gs:(LAPIC_EOI)

	iret
.endfunc


.align 4
.func vPortStartFirstTask
vPortStartFirstTask:

	/* Find the TCB. */
	movl 	pxCurrentTCB, %eax

	/* Stack location is first item in the TCB. */
	movl	(%eax), %esp

	/* Restore general purpose registers. */
	popa

    /* INTEL CHANGE: init flow has been completed */
	movl $0x1, initDone

	iret
.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortAPICErrorHandlerWrapper
vPortAPICErrorHandlerWrapper:
	pusha
	call	vPortAPICErrorHandler
	popa
	/* EOI. */
	movl	$0x00, %gs:(LAPIC_EOI)
	iret
.endfunc
/*-----------------------------------------------------------*/

.align 4
.func vPortTimerHandler
vPortTimerHandler:

	/* Save general purpose registers. */
	pusha

	/* Find the TCB. */
	movl 	pxCurrentTCB, %eax

	/* Stack location is first item in the TCB. */
	movl	%esp, (%eax)

	/* Switch stacks. */
	movl 	ulTopOfSystemStack, %esp
	movl	%esp, %ebp

	/* Increment nesting count. */
	add 	$1, ulInterruptNesting

	call 	xTaskIncrementTick

	sti

	/* Is a switch to another task required? */
	test	%eax, %eax
	je		_skip_context_switch
	cli
	call	vTaskSwitchContext


_skip_context_switch:
	cli

	/* Decrement the variable used to determine if a switch to a system
	stack is necessary. */
	sub		$1, ulInterruptNesting


	/* Stack location is first item in the TCB. */
	movl 	pxCurrentTCB, %eax
	movl	(%eax), %esp

	popa

	/* EOI. */
	movl	$0x00, %gs:(LAPIC_EOI)
	iret

.endfunc
/*-----------------------------------------------------------*/

.if configUSE_COMMON_INTERRUPT_ENTRY_POINT == 1

	.align 4
	.func vPortCentralInterruptWrapper
	vPortCentralInterruptWrapper:

		portFREERTOS_INTERRUPT_ENTRY

		cmpl  $0x0, idle_ticks
		je    exit_from_idle_done
		pushl idle_ticks
		call  pm_return_from_idle
		pop   %eax                       /* remove parameter */
		movl  $0, idle_ticks

	exit_from_idle_done:
		movl $LAPIC_TOP_ISR_REGISTER, %eax	/* Highest In Service Register (ISR) long word. */
		movl $8, %ecx					/* Loop counter. */

	next_isr_long_word:
		test %ecx, %ecx					/* Loop counter reached 0? */
		je wrapper_epilogue				/* Looked at all ISR registers without finding a bit set. */
		sub $1, %ecx					/* Sub 1 from loop counter. */
		movl %gs:(%eax), %ebx				/* Load next ISR long word. */
		sub $0x10, %eax					/* Point to next ISR long word in case no bits are set in the current long word. */
		test %ebx, %ebx					/* Are there any bits set? */
		je next_isr_long_word			/* Look at next ISR long word if no bits were set. */
		sti
		bsr %ebx, %ebx					/* A bit was set, which one? */
		movl $32, %eax					/* Destination operand for following multiplication. */
		mul %ecx						/* Calculate base vector for current register, 32 vectors per register. */
		add %ebx, %eax					/* Add bit offset into register to get final vector number. */
		push %eax						/* Vector number is function parameter. */
		call vPortCentralInterruptHandler
		pop %eax						/* Remove parameter. */

	wrapper_epilogue:
		/* Yes, we send EOI to I/O APIC *and* then send EOI to LAPIC.
		 * This is contrary to what is written in the books - EOI to LAPIC causes
		 * automatic sending of corresponding EOI message to I/O APIC -
		 * but this is how ISH hardware works.
		 */
		movl %eax, %gs:(IOAPIC_EOI)
		portFREERTOS_INTERRUPT_EXIT

	.endfunc

.endif /* configUSE_COMMON_INTERRUPT_ENTRY_POINT */
/*-----------------------------------------------------------*/

.align 4
.func vPortAPISpuriousHandler
vPortAPICSpuriousHandler:
	iret

.endfunc

.end







