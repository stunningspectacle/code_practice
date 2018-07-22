.LC0:
	.string	"Hello World!"
	.text
	.globl	_start
	.type	_start, @function
_start:
.LFB0:
	pushq	%rbp
	movq	%rsp, %rbp
	movl	$.LC0, %edi
	call	puts
	call	_exit
