	.globl	_start
_start:
	popl %ecx
	popl %ecx

	movl $10,15(%ecx)
	movl $4, %eax
	movl $15, %edx
	int $0x80

	movl $1, %eax
	int $0x80
