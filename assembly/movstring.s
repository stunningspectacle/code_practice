.section .data
v1:
	.asciz "This is a string\n"

.equ len, .-v1

.section .bss
	.lcomm buf, 50

.section .text
.global _start

_start:
	movl $len, %ebx
	movl $len, %ebx

	movl $v1, %esi
	movl $0, %esi

	movl $len, %ecx
	leal v1, %esi
	leal buf, %edi
loop1:
	movsb
	loop loop1

	movl $1, %eax
	int $0x80


