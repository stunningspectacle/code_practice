	.file	"volatile.c"
	.text
	.p2align 4,,15
	.globl	read32
	.type	read32, @function
read32:
.LFB1:
	movl	4(%esp), %eax
	movl	(%eax), %eax
	ret
.LFE1:
	.size	read32, .-read32
	.p2align 4,,15
	.globl	read32_volatile
	.type	read32_volatile, @function
read32_volatile:
.LFB2:
	subl	$16, %esp
.LCFI0:
	movl	20(%esp), %eax
	movl	(%eax), %eax
	movl	%eax, 12(%esp)
	movl	12(%esp), %eax
	addl	$16, %esp
.LCFI1:
	ret
.LFE2:
	.size	read32_volatile, .-read32_volatile
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0
	.byte	0x1
	.string	""
	.uleb128 0x1
	.sleb128 -4
	.byte	0x8
	.byte	0xc
	.uleb128 0x4
	.uleb128 0x4
	.byte	0x88
	.uleb128 0x1
	.align 4
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.long	.LFB1
	.long	.LFE1-.LFB1
	.align 4
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.long	.LFB2
	.long	.LFE2-.LFB2
	.byte	0x4
	.long	.LCFI0-.LFB2
	.byte	0xe
	.uleb128 0x14
	.byte	0x4
	.long	.LCFI1-.LCFI0
	.byte	0xe
	.uleb128 0x4
	.align 4
.LEFDE3:
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
