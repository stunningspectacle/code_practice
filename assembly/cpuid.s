.section .data
output:
	.ascii "The processor Vendor ID is " 
id:
	.fill 3, 4, 0
	.ascii "\n"
len:
	.long . - output

.section .text
.global _start
_start:
	mov $0, %eax
	cpuid

mov $id, %edi
mov %ebx, 0(%edi)
mov %edx, 4(%edi)
mov %ecx, 8(%edi)

mov $4, %eax
mov $1, %ebx
mov $output, %ecx
mov $len, %edx
mov (%edx), %edx
int $0x80

mov $1, %eax
mov $0, %ebx
int $0x80
