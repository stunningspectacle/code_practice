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


#include <mmio.h>
	.extern ulTopOfSystemStack
	.extern ulInterruptNesting

/*-----------------------------------------------------------*/

.macro portFREERTOS_INTERRUPT_ENTRY

	/* Save general purpose registers. */
	pusha

	/* If ulInterruptNesting is zero the rest of the task context will need
	saving and a stack switch might be required. */
	movl	ulInterruptNesting, %eax
	test	%eax, %eax
	jne		2f

	/* Find the TCB. */
	movl 	pxCurrentTCB, %eax

	/* Stack location is first item in the TCB. */
	movl	%esp, (%eax)

	/* Switch stacks. */
	movl 	ulTopOfSystemStack, %esp
	movl	%esp, %ebp

	2:
	/* Increment nesting count. */
	add 	$1, ulInterruptNesting

.endm
/*-----------------------------------------------------------*/

.macro portINTERRUPT_EPILOGUE

	cli
	sub		$1, ulInterruptNesting

	/* If the nesting has unwound to zero. */
	movl	ulInterruptNesting, %eax
	test	%eax, %eax
	jne		2f

	/* If a yield was requested then select a new TCB now. */
	movl	ulPortYieldPending, %eax
	test	%eax, %eax
	je		1f
	movl	$0, ulPortYieldPending
	call	vTaskSwitchContext

	1:
	/* Stack location is first item in the TCB. */
	movl 	pxCurrentTCB, %eax
	movl	(%eax), %esp


	2:
	popa

.endm
/*-----------------------------------------------------------*/

.macro portFREERTOS_INTERRUPT_EXIT

	portINTERRUPT_EPILOGUE
	/* EOI. */
	movl	$0x00, %gs:(LAPIC_EOI)
	iret

.endm
