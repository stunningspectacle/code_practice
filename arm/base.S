/* put # before immediate number */
movs r0, #0x12  
movs r1, #'a'

.equ NVIC_IRQ_STEEN. 0xE000E100
.equ NVIC_IRQ0_ENABLE, 0x1

ldr r0, =nvic_irq_seten  /* save address 0xe000e100 to r0 */
movs r1, #nvic_irq0_enable /* move num 0x1 to r1 */
str r1, [r0]			   /* save 0x1 to 0xe000e100 */

ldr r3, =my_number
ldr r4, [r3]

ldr r0, =hello_text
bl PrintText

.algin 4
MY_NUMBER:
.word 0x12345678		/* word is 4 bytes length! */
HELLO_TEXT:
.asciz "Hello\n"

adds r0, #1
adds.n r0, #1 /* 16 bits thumb */
adds.w r0, #1 /* 32 bits thumb-2 */

0x1000: ldr r0, [r1]  /* 16 bits thumb insruction in 0x1000~0x1001 */
0x1002: rbit.w r0    /* 32 bits thumb-2 instruction in 0x1002~0x1005 */

adr r0, datatable /* save address of datatable to r0 */

movw r0, #0x789a 			/* save 0x789a to lower 16 bits of r0 */
movt r0, #0x3456 			/* save 0x3456 to higher 16 bits of r0 */
mrs r7, PRIMASK				/* move primask to r7 */
msr CONTROL, r2	 			/* move r2 to control */

ldr r0, [r1, #0x8]!  /* load 32bits from address r1+0x8, then update r1 to r1+0x8 */

ldr r3, [r0, r2]			/* load r0 + r2 to r3 */
ldr r3, [r0, r2, lsl #2]	/* load r0+(r2<<2) to r3 */
str r5, [r0, r7]			/* save r5 to r0+r7 */

ldr r0, [r1], #2			/* load r1 to r0, then update r1 to r1+2 */

ldr r4, =0x20000000
ldmia r4, {r0-r3} 			/* read 4 words from 0x20000000 and save them to r0 ~ r3 */
ldmdb r4, {r0-r3}			/* read 4 words decreases from 0x20000000 and save them to r0 ~ r3 */
stmia r4, {r0-r3}			/* save r0 ~ r3 to 0x20000000 ~ (0x2000000 + 4 * 4) */
stmdb r4, {r0-r3}			/* save r0 ~ r3 to 0x20000000 ~ (0x2000000 - 4 * 4) */
ldr r8, =0x8000
stmia r8!, {r0-r3}			/* save r0 ~ r3 to 0x8000, then update r8 to 0x8010 */

push {r0, r4-r7, r9}		/* push r0, r4, r5, r6, r7, r9 to stack */
pop {r2, r3}				/* pop stack to r2, r3 */
push {r4-r6, lr}
pop {r4-r6, pc}

ldrt rd, [rn, #offset]		/* load rn+offset to rd, "t" means 16bits instruction */

ldrex rt, [rn, #offset]		/* load rn+offset to rt exclusively */
strex rd, [rn, #offset]		/* save rd to rn+offset exclusively */
clrex						/* clear exclusion monitor */

add r0, r0, r1				/* r0 = r0 + r1 */
adds r0, r0, #0x12			/* r0 = r0 + 0x12, update APSR */
adc r0, r1, r2				/* r0 = r1 + r2 + carry */
adc r0, r1, #0x12
addw Rd, Rn, #immed			/* Rd = Rn + #immed */
sub Rd, Rn, Rm				/* Rd = Rn - Rm */
sub Rd, #immed				/* Rd = Rd - #immed */
sub Rd, Rn #immed
sbc Rd, Rn, #immed
sbc Rd, Rn, Rm
subw Rd, Rn, #immed
rsb Rd, Rn, #immed			/* Rd = #immed - Rn */
rsb Rd, Rn, Rm				/* Rd = Rm - Rn */
mul Rd, Rn, Rm				/* Rd = Rn * Rm */
udiv Rd, Rn, Rm				/* Rd = Rn / Rm, unsigned */
sdiv Rd, Rn, Rm				/* Rd = Rn / Rm, signed */

mla Rd, Rn, Rm, Ra			/* Rd = Ra + Rn * Rm */
mls Rd, Rn, Rm, Ra			/* Rd = Ra - Rn * Rm */
smull RdLo, RdHi, Rn, Rm	/* {RdHi, RdLow} = Rn * Rm, signed */
smlal RdLo, RdHi, Rn, Rm	/* {RdHi, RdLow} += Rn * Rm signed */
umull RdLo, RdHi, Rn, Rm	/* {RdHi, RdLo} = Rn * Rm, unsigned */
umlal RdLo, RdHi, Rn, Rm	/* {RdHi, RdLo} += Rn * Rm, unsigned */

and Rd, Rn					/* Rd = Rd & Rn */
and Rd, Rn, #imm
and Rd, Rn, Rm
orr Rd, Rn					/* Rd = Rd | Rn */
orr Rd, Rn, #imm
orr Rd, Rn, Rm
bic Rd, Rn					/* Rd = Rd & (~Rn) */
bic Rd, Rn, #imm
bic Rd, Rn, Rm
orn Rd, Rn, #imm			/* Rd = Rn | (w#immed) */
orn Rd, Rn, Rm				/* Rd = Rn | wRm */
eor Rd, Rn					/* Rd = Rd ^ Rn */
eor Rd, Rn, #imm
eor Rd, Rn, Rm

asr Rd, Rn, #imm			/* Rd = Rn >> imm */
asr Rd, Rn					/* Rd = Rd >> Rn */
asr	Rd, Rn, Rm
lsl Rd, Rn, #imm			/* Rd = Rn << imm */
lsl Rd, Rn
lsl Rd, Rn, Rm
lsr Rd, Rn, #imm			/* Rd = Rn >> imm */
lsr Rd, Rn
lsr Rd, Rn, Rm
ror Rd, Rn					/* Rd rotation shift right Rn bits */
ror Rd, Rn, Rm				/* Rd = Rn rotation shift right Rm bits */
rrx Rd, Rn					/* {C, Rd} = {Rn, C}

sxtb Rd, Rm					/* Rd = extention of Rn[7:0], signed */
sxth Rd, Rm					/* Rd = extention of Rn[15:0], signed */
uxtb Rd, Rm
uxth Rd, Rm
sxtb Rd, Rm {, ROR #n } 	/* Rd = Rn rotation shift right n bits, then extention Rn[0:7]. n = 8/16/24 */
sxth Rd, Rm {, ROR #n }
uxtb Rd, Rm {, ROR #n }
uxth Rd, Rm {, ROR #n }

rev Rd, Rn					/* Rd = rev(Rn), 0x11223344 -> 0x44332211 */
revh Rd, Rn					/* Rd = revh(Rn), 0x11223344 -> 0x22114433 */
revsh Rd, Rn				/* Rd = revsh(Rn), 0x11223344 -> 0x4433, 0x33448899 -> 0xffff9988*/

bfc Rd, #<lsb>, #<width>	/* Clear the bits from lsb to lsb+width-1 */
bfi Rd, Rn, #<lsb>, #<width> /* insert the bits of Rn from lsb to lsb+width-1 into Rd */
clz Rd, Rn					/* count the leading zero, Rd = CLZ(Rn), CLZ(0xffffff) is 8 */
rbit R0, R1					/* 1011 0100 1110 0001 0000 1100 0010 0011 -> 1100 0100 0011 0000 1000 0111 0010 1101 */
LDR R0, =0x5678ABCD
ubfx R1, R0, #4, #8			/* R1 = 0x0000BC, get the bits from 4 to 4+8-1 of R0, unsigned extention to R1 */
sbfx R1, R0, #4, #8			/* R1 = 0xffffBC */

cmp Rn, Rm
cmp Rn, #imm
cmn Rn, Rm					/* calculate Rn + Rm and update APSR, won't save the outcome */
cmn Rn, #imm
tst Rn, Rm					/* calculate Rn & Rm and update N and Z of APSR */
tst Rn, #imm
teq Rn, Rm					/* calculate Rn ^ Rm and update the N and Z of APSR */
teq Rn, #imm

b <label>					/* short jump in +-2KB */
b.w <label>					/* jump farther than b */
bx Rm						/* jump to the address in Rm, and the bit 0 of Rm must be 1 to indicate thumb */
bl <label>					/* jump to label and save the return address to LR */
blx Rm						/* jump to the address in Rm, save the return address to LR and update the T bit of EPSR to the bit 0 of Rm */
b<cond><label>				/* cond: EQ, NE, CS/HS, CC/LO, MI, PL, VS, VC, HI, LS, GE, LT, GT, LE */
b<cond>.W<label>
cbz Rm, <label>				/* if Rm is zero, jump to label, only support jumping forward */
cbnz Rm, <label>			/* if Rm is not zero, jump to label, only support jumping forward */

it<t><e>					/* if else */
itett ne
addne r0, r0, r1
addeq r0, r0, r3
addne r2, r4, #1
movne r5, r3

svc #imm					/* supervise control, generate an exception, usually for status change */
cps							/* change processor state */
cpsie i						/* enable interrupt in primask */
cpsid i
cpsie f						/* enable interrupt in faultmask */
cpsid f

wfi							/* sleep and waiting for interrupt, cmsis: _wfi() */
wfe							/* sleep conditionally, cmsis: _wfe() */
sev							/* send event, cmsis: _sev() */

dmb							/* data memory barrior */
dsb							/* data sync barrior */
isb							/* instruction sync barrior */

nop							/* cmsis: _nop() */
bkpt #imm					/* breakpoint */	
