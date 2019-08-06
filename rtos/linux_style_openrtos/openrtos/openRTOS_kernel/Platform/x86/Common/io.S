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
	/*-------------------------------------------------------------------------
	                  GLOBAL ASSEMBLY LANGUAGE ROUTINES
	--------------------------------------------------------------------------*/

	/* inb */
	.globl	inb
	inb:	movl	4(%esp), %edx
	xorl	%eax, %eax	# clr eax
	inb	%dx, %al
	ret

	/* inw */
	.globl	inw
	inw:	movl	4(%esp), %edx
	xorl	%eax, %eax	# clr eax
	inw		%dx, %ax
	ret

	/* inl */
	.globl  inl
	inl:	movl	4(%esp), %edx
	xorl	%eax, %eax
	inl		%dx, %eax
	ret

	/* outb */
	.globl	outb
	outb:	movl	4(%esp), %edx
	movl	8(%esp), %eax
	outb	%al, %dx
	ret

	/* outw */
	.globl	outw
	outw:	movl	4(%esp), %edx
	movl	8(%esp), %eax
	outw	%ax, %dx
	ret

	/* outl */
	.globl	outl
	outl:	movl	4(%esp), %edx
	movl	8(%esp), %eax
	outl	%eax, %dx
	ret

.end
