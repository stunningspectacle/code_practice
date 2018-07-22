.section .data
s:
	.asciz "The value is %d\n"
values:
	.int 10, 20, 30, 40, 50, 60, 70, 80, 90
myint:
	.int 12345
.section .bss
.section .text
.global _start

_start:
	#movl $10, s
	#movl %eax, s
	#movl s, %eax
	#movl $s, %eax

	movl $0, %ebx
loop1:
	movl $s, %edi
	movl values(, %ebx, 4), %esi
	call printf

	inc %ebx
	cmpl $9, %ebx
	jne loop1

	push %rcx

	movl $1, %eax
	int $0x80
