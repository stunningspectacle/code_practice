.section .data
d1:
.int 2, -2
d2:
.byte 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
d3:
.double 3.14159265358
d4:
.float 3.1415926

.section .bss
.section .text
.global _start
_start:
movq d1, %mm0
movq d2, %mm1

mov $1, %eax
int $0x80
