.section .data
s:
	.asciz "The processor Vendor ID is '%s'\n" 

.section .bss
	.lcomm buffer, 13

.section .text
.global _start
_start:
	mov $0, %eax
	cpuid

	mov $buffer, %edi
	mov %ebx, 0(%edi)
	mov %edx, 4(%edi)
	mov %ecx, 8(%edi)

	#movb $10, 12(%edi) #put '\n' to end

	#push $id
	#pushq $buffer
	#pushq $s
	movl $buffer, %esi # put first arg to %edi, second to %esi
	movl $s, %edi
	#mov $output, %eax
	call printf

	add $16, %rsp

	pushq $0
	call exit
