	.file	"fread.c"
	.section	.rodata
.LC0:
	.string	"r"
.LC1:
	.string	"abc.txt"
.LC2:
	.string	"a = %d, c = %c\n"
	.text
	.globl	print
	.type	print, @function
print:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	leaq	-16(%rbp), %rax
	movq	$0, (%rax)
	movl	$.LC0, %edx
	movl	$.LC1, %eax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	fopen
	movq	%rax, -8(%rbp)
	jmp	.L2
.L3:
	movzbl	-12(%rbp), %eax
	movsbl	%al, %edx
	movl	-16(%rbp), %ecx
	movl	$.LC2, %eax
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
.L2:
	leaq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$8, %esi
	movq	%rax, %rdi
	call	fread
	testq	%rax, %rax
	jne	.L3
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	fclose
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	print, .-print
	.section	.rodata
.LC3:
	.string	"r+"
	.text
	.globl	create
	.type	create, @function
create:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	$.LC3, %edx
	movl	$.LC1, %eax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	fopen
	movq	%rax, -16(%rbp)
	movl	$0, -4(%rbp)
	jmp	.L5
.L6:
	movl	-4(%rbp), %eax
	movl	%eax, -32(%rbp)
	movl	-4(%rbp), %eax
	addl	$97, %eax
	movb	%al, -28(%rbp)
	leaq	-32(%rbp), %rax
	movq	-16(%rbp), %rdx
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$8, %esi
	movq	%rax, %rdi
	call	fwrite
	addl	$1, -4(%rbp)
.L5:
	cmpl	$9, -4(%rbp)
	jle	.L6
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	fclose
	movl	$0, %eax
	call	print
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	create, .-create
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$.LC3, %edx
	movl	$.LC1, %eax
	movq	%rdx, %rsi
	movq	%rax, %rdi
	call	fopen
	movq	%rax, -8(%rbp)
	call	create
	jmp	.L8
.L9:
	movl	$100, -16(%rbp)
	movb	$107, -12(%rbp)
	leaq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$8, %esi
	movq	%rax, %rdi
	call	fwrite
.L8:
	leaq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	movq	%rdx, %rcx
	movl	$1, %edx
	movl	$8, %esi
	movq	%rax, %rdi
	call	fread
	testq	%rax, %rax
	jne	.L9
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	call	fclose
	movl	$0, %eax
	call	print
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
