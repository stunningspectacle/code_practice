	.file	"inline_asm.c"
	.globl	glb_a
	.data
	.align 4
	.type	glb_a, @object
	.size	glb_a, 4
glb_a:
	.long	1223
	.globl	glb_b
	.align 4
	.type	glb_b, @object
	.size	glb_b, 4
glb_b:
	.long	38
	.text
	.globl	my_exit
	.type	my_exit, @function
my_exit:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
#APP
# 8 "inline_asm.c" 1
	movl $1, %eax	
	movl $123, %ebx 
	int $0x80		
	
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	my_exit, .-my_exit
	.section	.rodata
.LC0:
	.string	"glb_b = %d\n"
	.text
	.globl	use_glb
	.type	use_glb, @function
use_glb:
.LFB1:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	$10, glb_a(%rip)
	movl	$20, glb_b(%rip)
#APP
# 19 "inline_asm.c" 1
	pusha
	movl glb_a, %eax	
	addl glb_b, %eax	
	movl %eax, glb_b	
	popa
	
# 0 "" 2
#NO_APP
	movl	glb_b(%rip), %edx
	movl	$.LC0, %eax
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE1:
	.size	use_glb, .-use_glb
	.section	.rodata
.LC1:
	.string	"before add: %d, %d\n"
.LC2:
	.string	"After add: %d, %d\n"
	.text
	.globl	my_add
	.type	my_add, @function
my_add:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	movl	$100, -24(%rbp)
	movl	$200, -20(%rbp)
	movl	$.LC1, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	.cfi_offset 3, -24
	call	printf
	movl	-24(%rbp), %eax
	movl	%eax, -40(%rbp)
	movl	-20(%rbp), %edx
	movl	-40(%rbp), %eax
	movl	%edx, %ebx
#APP
# 34 "inline_asm.c" 1
	imull %eax, %ebx 
	movl %ebx, %eax 
	
# 0 "" 2
#NO_APP
	movl	%eax, -36(%rbp)
	movl	-36(%rbp), %eax
	movl	%eax, -20(%rbp)
	movl	$.LC2, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	my_add, .-my_add
	.section	.rodata
.LC3:
	.string	"%s, %s\n"
	.text
	.globl	copy_str
	.type	copy_str, @function
copy_str:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$96, %rsp
	movq	%fs:40, %rax
	movq	%rax, -8(%rbp)
	xorl	%eax, %eax
	movl	$1936287828, -80(%rbp)
	movl	$544434464, -76(%rbp)
	movl	$1931508077, -72(%rbp)
	movl	$1852404340, -68(%rbp)
	movq	$103, -64(%rbp)
	movl	$0, -56(%rbp)
	movw	$0, -52(%rbp)
	movl	$544501614, -48(%rbp)
	movl	$1768976227, -44(%rbp)
	movl	$2032165989, -40(%rbp)
	movl	$29797, -36(%rbp)
	movq	$0, -32(%rbp)
	movl	$0, -24(%rbp)
	movw	$0, -20(%rbp)
	movl	$30, -84(%rbp)
	movl	$.LC3, %eax
	leaq	-48(%rbp), %rdx
	leaq	-80(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	leaq	-80(%rbp), %rax
	leaq	-48(%rbp), %rdx
	movl	-84(%rbp), %ecx
	movq	%rax, %rsi
	movq	%rdx, %rdi
#APP
# 47 "inline_asm.c" 1
	cld
	rep movsb
	
# 0 "" 2
#NO_APP
	movl	$.LC3, %eax
	leaq	-48(%rbp), %rdx
	leaq	-80(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movq	-8(%rbp), %rax
	xorq	%fs:40, %rax
	je	.L5
	call	__stack_chk_fail
.L5:
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	copy_str, .-copy_str
	.section	.rodata
.LC4:
	.string	"%d, %d, %d\n"
	.text
	.globl	place_holder
	.type	place_holder, @function
place_holder:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	movl	$123, -28(%rbp)
	movl	$456, -24(%rbp)
	movl	$0, -20(%rbp)
	movl	$.LC4, %eax
	movl	-20(%rbp), %ecx
	movl	-24(%rbp), %edx
	movl	-28(%rbp), %esi
	movq	%rax, %rdi
	movl	$0, %eax
	.cfi_offset 3, -24
	call	printf
	movl	-28(%rbp), %eax
	movl	-24(%rbp), %edx
#APP
# 62 "inline_asm.c" 1
	imull %eax, %edx
	movl %edx, %ebx
	
# 0 "" 2
#NO_APP
	movl	%ebx, -20(%rbp)
	movl	$.LC4, %eax
	movl	-20(%rbp), %ecx
	movl	-24(%rbp), %edx
	movl	-28(%rbp), %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	place_holder, .-place_holder
	.section	.rodata
.LC5:
	.string	"%d, %d\n"
	.text
	.globl	same_place_holder
	.type	same_place_holder, @function
same_place_holder:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	movl	$123, -24(%rbp)
	movl	$456, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	.cfi_offset 3, -24
	call	printf
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	movl	%edx, %ebx
#APP
# 76 "inline_asm.c" 1
	imull %eax, %ebx
	
# 0 "" 2
#NO_APP
	movl	%ebx, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	same_place_holder, .-same_place_holder
	.globl	named_place_holder
	.type	named_place_holder, @function
named_place_holder:
.LFB6:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	movl	$1111, -24(%rbp)
	movl	$2222, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	.cfi_offset 3, -24
	call	printf
	movl	-24(%rbp), %eax
	movl	-20(%rbp), %edx
	movl	%edx, %ebx
#APP
# 88 "inline_asm.c" 1
	imull %eax, %ebx
	
# 0 "" 2
#NO_APP
	movl	%ebx, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE6:
	.size	named_place_holder, .-named_place_holder
	.globl	changed_register
	.type	changed_register, @function
changed_register:
.LFB7:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$24, %rsp
	movl	$123, -24(%rbp)
	movl	$456, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	.cfi_offset 3, -24
	call	printf
	movl	-24(%rbp), %edx
	movl	-20(%rbp), %eax
	movl	%eax, %ebx
#APP
# 100 "inline_asm.c" 1
	movl %edx, %eax
	addl %eax, %ebx
	
# 0 "" 2
#NO_APP
	movl	%ebx, -20(%rbp)
	movl	$.LC5, %eax
	movl	-20(%rbp), %edx
	movl	-24(%rbp), %ecx
	movl	%ecx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$24, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE7:
	.size	changed_register, .-changed_register
	.globl	use_memory
	.type	use_memory, @function
use_memory:
.LFB8:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$16, %rsp
	movl	$1235, -4(%rbp)
	movl	$5, -12(%rbp)
	movl	$0, -8(%rbp)
	movl	-8(%rbp), %ecx
	movl	-12(%rbp), %edx
	movl	$.LC4, %eax
	movl	-4(%rbp), %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	movl	-4(%rbp), %eax
#APP
# 114 "inline_asm.c" 1
	divl -12(%rbp)
	movl %eax, -8(%rbp)
	
# 0 "" 2
#NO_APP
	movl	-8(%rbp), %ecx
	movl	-12(%rbp), %edx
	movl	$.LC4, %eax
	movl	-4(%rbp), %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE8:
	.size	use_memory, .-use_memory
	.globl	use_label
	.type	use_label, @function
use_label:
.LFB9:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	-20(%rbp), %eax
	movl	-24(%rbp), %edx
#APP
# 125 "inline_asm.c" 1
	cmpl %eax, %edx
	jg greater
	movl %eax, -4(%rbp)
	jmp end
	greater:
	movl %edx, -4(%rbp)
	end:
	
# 0 "" 2
#NO_APP
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE9:
	.size	use_label, .-use_label
	.globl	use_local_label
	.type	use_local_label, @function
use_local_label:
.LFB10:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)
	movl	%esi, -24(%rbp)
	movl	-20(%rbp), %eax
	movl	-24(%rbp), %edx
#APP
# 140 "inline_asm.c" 1
	cmpl %eax, %edx
	jg 0f
	movl %eax, -4(%rbp)
	jmp 1f
	0:
	movl %edx, -4(%rbp)
	1:
	
# 0 "" 2
#NO_APP
	movl	-4(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE10:
	.size	use_local_label, .-use_local_label
	.globl	use_imm
	.type	use_imm, @function
use_imm:
.LFB11:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %edx
	movl	%esi, %eax
	movb	%dl, -4(%rbp)
	movb	%al, -8(%rbp)
#APP
# 154 "inline_asm.c" 1
	mov $10, %eax
	
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE11:
	.size	use_imm, .-use_imm
	.globl	inb_outb
	.type	inb_outb, @function
inb_outb:
.LFB12:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %edx
	movl	%esi, %eax
	movb	%dl, -20(%rbp)
	movb	%al, -24(%rbp)
	movb	$0, -1(%rbp)
#APP
# 165 "inline_asm.c" 1
	outb -20(%rbp), -24(%rbp)
	
# 0 "" 2
# 170 "inline_asm.c" 1
	inb -24(%rbp), -1(%rbp)
	
# 0 "" 2
#NO_APP
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE12:
	.size	inb_outb, .-inb_outb
	.section	.rodata
.LC6:
	.string	"this is the first line"
.LC7:
	.string	"%d is greater\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB13:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$40, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movl	$.LC6, %edi
	.cfi_offset 3, -24
	call	puts
	movq	-48(%rbp), %rax
	addq	$16, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	atoi
	movl	%eax, %ebx
	movq	-48(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	atoi
	movl	%ebx, %esi
	movl	%eax, %edi
	call	use_local_label
	movl	%eax, -20(%rbp)
	movl	$.LC7, %eax
	movl	-20(%rbp), %edx
	movl	%edx, %esi
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf
	addq	$40, %rsp
	popq	%rbx
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE13:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
