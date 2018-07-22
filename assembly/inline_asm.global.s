	.file	"inline_asm.c"
	.comm	glb_a,4,4
	.comm	glb_b,4,4
	.text
	.globl	my_exit
	.type	my_exit, @function
my_exit:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
#APP
# 8 "inline_asm.c" 1
	movl $1, %eax	
	movl $123, %ebx 
	int $0x80		
	
# 0 "" 2
#NO_APP
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
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
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	$10, glb_a
	movl	$20, glb_b
#APP
# 19 "inline_asm.c" 1
	pusha
	movl glb_a, %eax	
	addl glb_b, %eax	
	movl %eax, glb_b	
	popa
	
# 0 "" 2
#NO_APP
	movl	glb_b, %edx
	movl	$.LC0, %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	printf
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	use_glb, .-use_glb
	.section	.rodata
.LC1:
	.string	"this is the first line"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	andl	$-16, %esp
	subl	$16, %esp
	movl	$.LC1, (%esp)
	call	puts
	call	use_glb
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	main, .-main
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
