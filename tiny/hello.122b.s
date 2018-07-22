.LC0:
	.string	"Hello World!"
	.text
	.globl	_start
	.type	_start, @function
_start:
.LFB0:
	xorl %eax, %eax
	movb $4, %al
	xorl %ebx, %ebx
	incl %ebx
	movl $.LC0, %ecx
	xorl %edx, %edx
	movb $13, %dl
	int $0x80

	xorl %eax, %eax
	incl %eax
	xorl %ebx, %ebx
	int $0x80
