	.file	"test.c"
	.text
	.globl	myffs
	.type	myffs, @function
myffs:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -24(%rbp)
	movl	$0, -4(%rbp)
	movq	-24(%rbp), %rax
	andl	$65535, %eax
	testq	%rax, %rax
	jne	.L2
	addl	$16, -4(%rbp)
	shrq	$16, -24(%rbp)
.L2:
	movq	-24(%rbp), %rax
	andl	$255, %eax
	testq	%rax, %rax
	jne	.L3
	addl	$8, -4(%rbp)
	shrq	$8, -24(%rbp)
.L3:
	movq	-24(%rbp), %rax
	andl	$15, %eax
	testq	%rax, %rax
	jne	.L4
	addl	$4, -4(%rbp)
	shrq	$4, -24(%rbp)
.L4:
	movq	-24(%rbp), %rax
	andl	$3, %eax
	testq	%rax, %rax
	jne	.L5
	addl	$2, -4(%rbp)
	shrq	$2, -24(%rbp)
.L5:
	movq	-24(%rbp), %rax
	andl	$1, %eax
	testq	%rax, %rax
	jne	.L6
	addl	$1, -4(%rbp)
.L6:
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	myffs, .-myffs
	.section	.rodata
.LC0:
	.string	"try %d, %d, %d\n"
	.text
	.globl	try
	.type	try, @function
try:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	%edx, -12(%rbp)
	movl	$.LC0, %eax
	movl	-12(%rbp), %ecx
	movl	-8(%rbp), %edx
	movl	-4(%rbp), %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	try, .-try
	.globl	myfree
	.type	myfree, @function
myfree:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	free
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	myfree, .-myfree
	.section	.rodata
.LC1:
	.string	"0x%x "
.LC2:
	.string	"abc=%p\n"
.LC3:
	.string	"abc=%s\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$10, %edi
	call	malloc
	movq	%rax, -16(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L10
.L11:
	movl	-4(%rbp), %eax
	cltq
	addq	-16(%rbp), %rax
	movl	-4(%rbp), %edx
	addl	$97, %edx
	movb	%dl, (%rax)
	addl	$1, -4(%rbp)
.L10:
	cmpl	$9, -4(%rbp)
	jle	.L11
	movl	$0, -4(%rbp)
	jmp	.L12
.L13:
	movl	-4(%rbp), %eax
	cltq
	addq	-16(%rbp), %rax
	movzbl	(%rax), %eax
	movsbl	%al, %edx
	movl	$.LC1, %eax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addl	$1, -4(%rbp)
.L12:
	cmpl	$19, -4(%rbp)
	jle	.L13
	movl	$10, %edi
	call	putchar
	movl	$.LC2, %eax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$.LC3, %eax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	myfree
	movl	$.LC2, %eax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$.LC3, %eax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
