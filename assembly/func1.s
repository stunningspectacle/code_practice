.section .text
.type mymultiple, @function
.global mymultiple
mymultiple:
	pushl %ebp
	movl %esp, %ebp

	mull %ebx

	movl %ebp, %esp
	popl %ebp
	ret
