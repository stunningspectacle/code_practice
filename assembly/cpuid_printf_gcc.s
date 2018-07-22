.section .data
s:
	.asciz "The processor Vendor ID is '%s'\n" 

.section .bss
	.lcomm buffer, 14

.section .text
.global main
main:
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
	#mov $output, %eax

	movl $buffer, %esi
	movl $s, %edi
	call printf

	#add $16, %rsp

	movl $0, %edi
	call exit
