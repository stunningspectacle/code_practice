.section .data
c:
	.ascii "kill"
c1:
	.ascii "kill"
c2:
	.ascii "kila"

.section .bss
	.lcomm buf, 256

.section .text
.global _start

_start:
	leal c, %esi
	leal buf, %edi
	lodsl
	movl $256, %ecx
	shrl $2, %ecx
	rep stosl

	xorl %ebx, %ebx
	movl $10, %edx
	leal c1, %esi
	leal c2, %edi
	cmpsl
	cmove %edx, %ebx

	movl $1, %eax
	int $0x80

