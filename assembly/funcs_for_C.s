.section .data
s:
	.asciz "This is a good example: %d\n"
.section .bss
	.lcomm cpu_id, 13

.section .text

.global myadd
.type myadd @function

.global get_cpuid
.type get_cpuid @function

myadd:
	pushl %ebp
	movl %esp, %ebp
	subl $12, %esp
	pushl %ebx
	pushl %esi
	pushl %edi

	movl 8(%ebp), %eax
	addl 12(%ebp), %eax
	pushl %eax
	pushl $s
	call printf
	addl $4, %esp
	popl %eax
	incl %eax

	popl %edi
	popl %esi
	popl %ebx
	movl %ebp, %esp
	popl %ebp
	ret

get_cpuid:
	pushl %ebp
	movl %esp, %ebp
	subl $12, %esp
	pushl %ebx

	movl $0, %eax
	cpuid
	movl $cpu_id, %eax
	movl %ebx, (%eax)
	movl %edx, 4(%eax)
	movl %ecx, 8(%eax)
	movb $0, 12(%eax)

	popl %ebx
	movl %ebp, %esp
	popl %ebp
	ret
