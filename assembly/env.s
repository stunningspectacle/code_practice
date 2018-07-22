.section .data
envs:
	.asciz "environments are:\n"
envs_num:
	.asciz "%d envs in total\n"
args:
	.asciz "%d args in total, they are:\n"
p:
	.asciz "%s\n"

.section .bss

.section .text

.global _start
_start:
	pushl %ebp
	movl %esp, %ebp
	# 0(%ebp): old %ebp; 4(%ebp): argc; 8(%ebp): argv[0]

	subl $12, %esp
	# 0(%esp): count_tmp; 4(%esp): index; 8(%esp): pointer; 

	pushl 4(%ebp)
	pushl $args
	call printf
	addl $8, %esp
	
	movl %ebp, 8(%esp)
	addl $8, 8(%esp)

	movl 4(%ebp), %ecx
print_args:
	movl %ecx, 0(%esp)	
	movl 8(%esp), %esi
	pushl (%esi)
	pushl $p
	call printf
	addl $8, %esp

	addl $4, 8(%esp)
	movl 0(%esp), %ecx
	loop print_args

	jmp exit0
# print envs
	pushl $envs
	call printf

	addl $4, %ebp
	xor %ecx, %ecx
print_envs:
	cmpl $0, (%ebp) 
	je finish

	incl %ecx
	pushl %ecx
	pushl (%ebp)
	pushl $p
	call printf
	addl $8, %esp
	addl $4, %ebp
	popl %ecx
	jmp print_envs
finish:
	pushl %ecx
	pushl $envs_num
	call printf
		
exit0:
	pushl $1
	call exit

	movl %ebp, %esp
	popl %ebp
	ret
