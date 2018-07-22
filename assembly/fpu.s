.section .data
v1:
.float 20.5
v2:
.float 10.90

.section .bss

.section .text
.global _start

_start:
finit
flds v1
flds v2
fcomi %st(1), %st(0)
fcmovb %st(1), %st(0)

mov $1, %eax
int $0x80
