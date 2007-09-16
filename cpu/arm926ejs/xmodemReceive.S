#include <config.h>
#define L1(a)          ((CH_##a))
#define L2(a,b)     ((CH_##a)+(CH_##b<<8))
#define L3(a,b,c)   ((CH_##a)+(CH_##b<<8)+(CH_##c<<16))
#define L4(a,b,c,d) ((CH_##a)+(CH_##b<<8)+(CH_##c<<16)+(CH_##d<<24))

#define C2(a,b)     ((CH_##a<<8)+(CH_##b))
#define C3(a,b,c)   ((CH_##a<<16)+(CH_##b<<8)+(CH_##c))
#define C4(a,b,c,d) ((CH_##a<<24)+(CH_##b<<16)+(CH_##c<<8)+(CH_##d))
#include <asm/BigMacro.h>
#include <asm/char.inc>
#define LITTLE_ENDIAN 1
#include <asm/xmodemReceive.inc>
	.global load_xmodem

//r8 is a global register, YUCK
//ulong load_xmodem(ulong loadAddress,ulong* pEndAddress);
load_xmodem:
	stmdb	sp!,{r1,r4,r5,r6,r7,r9,sl,fp,ip,lr}
	mov		ip,r0	//ip is only input to macro, unchanged on output
	xmodemReceive r3,r4,r5,r6,r7,r9,sl,fp,ip,TransmitX,ReceiveX

	ldmia	sp!,{r1,r4,r5,r6,r7,r9,sl,fp,ip,lr}
	str		r3,[r1]		//save end address
	moveq	r0,#0
	movne	r0,#1
	mov		pc,lr

TransmitX:
	tst		r0,#0xff00
	bne		1f
	stmdb	sp!,{r2,r3,ip,lr}
	bl		putc
	ldmia	sp!,{r2,r3,ip,pc}
	
1:	mov		r1,#0
	stmdb	sp!,{r0,r1,r2,r3,ip,lr}
	mov		r0,sp
	bl		puts
	ldmia	sp!,{r0,r1,r2,r3,ip,pc}
ReceiveX:
	stmdb	sp!,{r2,r3,r4,ip,lr}
	mov		r4,#0x80000
1:	subs	r4,r4,#1
	ldmeqia	sp!,{r2,r3,r4,ip,pc}
	bl		tstc
	movs	r0,r0
	beq		1b
	bl		getc
	movs	r1,#1
	ldmia	sp!,{r2,r3,r4,ip,pc}
