.section .data

.section .bss

.section .text
#.type multiple, @function

.global _start
_start:
	movl $13, %eax
	movl $9, %ebx

	call mymultiple
	call multiple

label:
	movl %eax, %ebx
	movl $1, %eax
	int $0x80

multiple:
	pushl %ebp
	movl %esp, %ebp

	mull %ebx

	movl %ebp, %esp
	popl %ebp
	ret
