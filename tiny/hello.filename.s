.LC0:
	.text
	.globl	_start
	.type	_start, @function
_start:
.LFB0:
	popl %ecx
	popl %ecx

	xorl %eax, %eax
	movb $4, %al
	xorl %ebx, %ebx
	incl %ebx
	xorl %edx, %edx
	movb $15, %dl
	int $0x80

	xorl %eax, %eax
	incl %eax
	xorl %ebx, %ebx
	int $0x80
