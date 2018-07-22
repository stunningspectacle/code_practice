	.globl	_start
_start:
	popl %ecx
	popl %ecx

	movb $10,15(%ecx)
	movb $4, %al
	movb $15, %dl
	int $0x80

	movb $1, %al
	int $0x80
