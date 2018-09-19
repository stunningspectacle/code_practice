	.file	"test.c"
	.text
	.globl	myadd
	.type	myadd, @function
myadd:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$0, -4(%rbp)
	movq	$0, -16(%rbp)
	movl	$0, -8(%rbp)
	jmp	.L2
.L3:
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %edx
	addl	%edx, %eax
	cltq
	movq	-16(%rbp), %rdx
	imulq	%rdx, %rax
	movq	%rax, -16(%rbp)
	addl	$1, -8(%rbp)
	addl	$1, -4(%rbp)
.L2:
	cmpl	$9999, -8(%rbp)
	jle	.L3
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	myadd, .-myadd
	.globl	func
	.type	func, @function
func:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$40, %rsp
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	%edx, -28(%rbp)
	movl	%ecx, -32(%rbp)
	movl	%r8d, -36(%rbp)
	movl	$0, -8(%rbp)
	movl	$1, -4(%rbp)
	movl	$0, -8(%rbp)
	jmp	.L5
.L6:
	movl	-8(%rbp), %eax
	imull	-4(%rbp), %eax
	addl	%eax, -20(%rbp)
	movl	$0, %eax
	call	myadd
	addl	$1, -8(%rbp)
.L5:
	cmpl	$1999, -8(%rbp)
	jle	.L6
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	addl	%edx, %eax
	addl	-28(%rbp), %eax
	addl	-32(%rbp), %eax
	addl	-36(%rbp), %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	func, .-func
	.globl	test_as
	.type	test_as, @function
test_as:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	$100, -4(%rbp)
	leaq	-20(%rbp), %rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	movl	(%rax), %eax
	addl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	test_as, .-test_as
	.globl	args
	.type	args, @function
args:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	%edx, -28(%rbp)
	movl	%ecx, -32(%rbp)
	movl	%r8d, -36(%rbp)
	movl	%r9d, -40(%rbp)
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	addl	%edx, %eax
	addl	-28(%rbp), %eax
	addl	-32(%rbp), %eax
	addl	-36(%rbp), %eax
	addl	-40(%rbp), %eax
	movl	%eax, -8(%rbp)
	movl	24(%rbp), %eax
	movl	16(%rbp), %edx
	addl	%edx, %eax
	addl	32(%rbp), %eax
	addl	40(%rbp), %eax
	addl	48(%rbp), %eax
	addl	56(%rbp), %eax
	addl	64(%rbp), %eax
	movl	%eax, -4(%rbp)
	movl	-4(%rbp), %eax
	imull	-8(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	args, .-args
	.section	.rodata
.LC0:
	.string	"value=0x%x\n"
.LC1:
	.string	"value%n=0x%x%n\n"
.LC2:
	.string	"%d %d chars\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$0, -16(%rbp)
	movl	-16(%rbp), %edx
	movl	$.LC0, %eax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movzbl	-15(%rbp), %eax
	andl	$-61, %eax
	orl	$52, %eax
	movb	%al, -15(%rbp)
	movzbl	-14(%rbp), %eax
	andl	$-16, %eax
	orl	$1, %eax
	movb	%al, -14(%rbp)
	movzbl	-13(%rbp), %eax
	orl	$-128, %eax
	movb	%al, -13(%rbp)
	movl	-16(%rbp), %edx
	movl	$.LC1, %eax
	leaq	-4(%rbp), %rcx
	leaq	-8(%rbp), %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	-4(%rbp), %edx
	movl	-8(%rbp), %ecx
	movl	$.LC2, %eax
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$5, %r8d
	movl	$4, %ecx
	movl	$3, %edx
	movl	$2, %esi
	movl	$1, %edi
	call	func
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
