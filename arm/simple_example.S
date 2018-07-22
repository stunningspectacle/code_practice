/***** func 1 *****/
.text
.syntax unified
.thumb
.type _main, %function
.global _START

_START:
	mov r0, #10
	mov r1, #0

loop:
	add r1, r0
	subs r0, #1
	bne loop

deadloop:
	b deadloop
.end

/*****func 2 *********/
myfunction:
	push {r4, r5}
	sub sp, sp, #8
	mov r4, sp
	ldr r5, =0x00001234
	str r5, [r4, #0]
	ldr r5, =0x55cc
	strh r5, [r4, #4]
	movs r5, #0xaa
	strb r5, [r4, #6]

	add sp, #8
	pop {r4, r5}
	bx lr
	
/***** hello world ******/
.text
.syntax unified
.thumb
.type _main, %function
.global _main

_main:
	movs r0, #'\n'
	bl putc

main_loop:
	ldr r0, =HELLO_TXT
	bl puts
	bl delay
	b main_loop

.type puts, %function
puts:
	push {r4, lr}
	movs r4, r0
puts_loop:
	ldrb r0, [r4], #1	
	cbz r0, puts_loop_exit
	bl putc
	b puts_loop
puts_loop_exit:
	pop {r4, pc}

.equ ITM_BASE,		0xe0000000
.equ ITM_PORT0, 	(ITM_BASE + 0x0)
.equ ITM_TER,		(ITM_BASE + 0xe00)
.equ ITM_TCR		(ITM_BASE + 0xe80)

.type putc, %function
putc:
	ldr r1, =ITM_TCR
	ldr r2, [r1]
	movs r3, #1
	tst r2, r3
	beq putc_exit
	ldr r1, =ITM_TER
	ldr r2, [r1]
	tst r2, r3
	beq putc_exit
	ldr r1, =ITM_PORT0
putc_wait:
	ldr r2, [r1]
	cmp r2, #0
	beq putc_wait
	strb r0, [r1]
putc_exit:
	bx lr

.type delay, %function
delay:
	ldr r0, =0x01000000
delay_loop:
	subs r0, r0, #1
	bne delay_loop
	bx lr

HELLO_TXT:
.string "Hello World\n"
.align 4

/***** hex ******/
.text
.thumb
.syntax unified
.type put_hex, %function
.global put_hex

put_hex:
	push {r4 - r6, lr}
	mov r4, r0
	mov r0, #'0'
	bl putc
	mov r0, #'x'
	bl putc
	mov r5, #8
	mov r6, #28
put_hex_loop:
	ror r4, r6
	and r0, r4
	cmp r0, #0xA   /* convert to ASCII */
	ite ge
	addge r0, #55
	addlt r0, #48
	bl putc
	subs r5, #1
	bne put_hex_loop
	pop {r4 - r6, pc}

.type put_dec, %function
put_dec:
	push {r4, lr}
	mov r12, sp
	sub sp, sp, #12
	movs r1, #0
	strb r1, [r12, #-1]!
	movs r1, #10
put_dec_loop:
	udiv r2, r0, r1 /* r2 = r0/r1 */
	mul r4, r2, r1	/* r4 = r2 * r1 */
	sub r3, r0, r4
	adds r3, #48
	strb r3, [r12, #-1]!
	movs r0, r2
	bne put_dec_loop
	mov r0, r12
	bl puts
	add sp, sp, #12
	pop {r4, pc}


/******* Enable interrupt ******/
.type enableIRQ, %function
enableIRQ:
	/* r0 has the irq number */
	push {r0 - r2, lr}
	and r1, r0, #0x1f
	mov r2, #1
	lsl r2, r2, r1  /* bit: (0x1 << (N & 0x1f))  */
	and r1, r0, #0xE0
	lsr r1, r1, #3  /* offset: (N/32) * 4  */
	ldr r0, =0xE000E100  /* CLREN register */
	str r2, [r0, r1] 
	pop {r0 - r2, pc}

.type disableIRQ, %function
disableIRQ:
	push {r0 - r2, lr}
	and r1, r0, #0x1f
	mov r2, #1
	lsl r2, r2, r1
	and r1, r0, #0xE0 		/* lsr r1, r0, #5 */
	lsr r1, r1, #3			/* lsl r1, r1, #2 */
	ldr r0 =0xE000E180		/* CLREN register */
	str r2, [r0, r1]
	pop {r0 - r2, pc}

set_int_priority:
	ldr r0, =0xE000E400		/* start of interrupt priority register */
	movs r1, #0xc0		/* priority is 0xc0 */
	strb r1, [r0, #4]	/* set the priority of IRQ4 to 0xc0 */

get_priority_width:
	ldr r2, =0xE000E400
	ldr r1, =0xff
	strb r1, [r2]
	ldrb r1, [r2]
	rbit r1, r1			/* reverse the bit in r1 */
	clz	r1, r1			/* count the leading zero */
	mov r0, #8
	sub r0, r0, r1		/* get the width, if it's 3: 8 - 5 = 3 */
	movs r1, #0x0
	strb r1, [r2]

/***** square root ******/
.type simple_sqrt, %function
/* input r0 */
simple_sqrt:
	movw r1, #0x8000
	movs r2, #0
simple_sqrt_loop:
	adds r2, r2, r1
	muls r3, r2, r2
	cmp r3, r0
	it hi
	subhi r2, r2, r1
	lsrs r1, r1, #1
	bne simple_sqrt_loop
	mov r0, r2
	bx lr
	

