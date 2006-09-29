	.nolist
	.ifdef __ARMASM
CONFIG_UNSCRAMBLE_LCD EQU 0
LCD_REORDER_BLUE EQU 1
	.endif

#define L1(a)          ((CH_##a))
#define L2(a,b)     ((CH_##a)+(CH_##b<<8))
#define L3(a,b,c)   ((CH_##a)+(CH_##b<<8)+(CH_##c<<16))
#define L4(a,b,c,d) ((CH_##a)+(CH_##b<<8)+(CH_##c<<16)+(CH_##d<<24))

#define C2(a,b)     ((CH_##a<<8)+(CH_##b))
#define C3(a,b,c)   ((CH_##a<<16)+(CH_##b<<8)+(CH_##c))
#define C4(a,b,c,d) ((CH_##a<<24)+(CH_##b<<16)+(CH_##c<<8)+(CH_##d))
#include "char.inc"

#include "platformTypes.h"
#include "common.h"
#include "BigMacro.h"
#include "lcdPanels2.h"
#include "arch/sysMacros.h"
#include "miniMac.inc"
#include "burn.inc"
#include "xmodemReceive.inc"

	.ifdef __ARMASM
	STARTUPTEXT
	EXTERN HeadStart
	.endif

	.list
	.global StartUp


// *******************************************************************************************
// *******************************************************************************************


//.global _start
//_start:
StartUp:
	b	V_Reset			//0
	b	V_UndefinedInstr	//4
	b	V_SWI			//8
	b	V_PrefetchAbort		//0x0c
	b	V_DataAbort		//0x10
	b	V_Unused		//0x14, not used
	b	V_IRQ			//0x18
//	b	V_FIQ			//0x1c

//In Linux, all modes switch almost immediately to the SVC mode
//and it is the only one with a valid stack
//FIQ & SWI stacks are always assumed valid
V_FIQ:
	V_VectorEntrance V_rWork,V_rBranch,SIG_FIQ

join_fiq:
	EmulateJmpFIQ V_rWork,V_rBranch
join_fiq2:
	sub	sp,sp,#(DEBUG_SPACE-DBG_R0)		//the extra 12 bytes already on stack are needed for indirect return for SDS bug
	stmia	sp,{r0,r1}
	sub	r1,V_rBranch,#8		//restore to SIG_xxx value
	add	r0,sp,#(DEBUG_SPACE-DBG_R0)
	ldr	V_rWork,[r0],#4
	ldr	V_rBranch,[r0],#8
	str	r0,[sp,#DBG_HSP-DBG_R0]
	sub	r0,sp,#DBG_R0
join_fiq3:
	mov	r1,r1,LSR #2
	and	r1,r1,#0xf
	b	SaveRest2
V_Reset:
	ResetVectorEntry MainInitializationCode	//z-1 if breakpoint
	mov	sp,r0			//save r0
	GetDebugBase r0
	str	sp,[r0,#DBG_R0]
	str	r1,[r0,#DBG_R1]
	str	r2,[r0,#DBG_R2]
SSDebug:
	bne	11f
//this is a breakpoint
	ldr	r1,[r0,#DBG_TRACE]
	tst	r1,#1
	beq	11f
	HandleRestoreBP r1
11:
	mrs	r2,spsr
	sub	lr,lr,#4

	str	r3,[r0,#DBG_R3]
	ldrb	r3,[r0,#DBG_Mode]
SaveDebug:
	GetMethodOfEntry		//Out: r1 method of entry
	add	r1,r1,#SIG_DBG
	b	SaveR4andUp

V_UndefinedInstr:
	VectorEntrance rWork,rBranch,SIG_UNDEFINED_INSTRUCTION
	JOIN b
V_IRQ:
	VectorEntrance rWork,rBranch,SIG_IRQ

	.if STACKS_VALID
	JOIN b
	.else
join_irq:
	EmulateJmpIRQ I_rWork,I_rBranch
join_irq2:
	strd	r0,[I_rWork,#DBG_R0-DBG_MAGIC]
	sub	r0,I_rWork,#DBG_MAGIC
	sub	r1,I_rBranch,#8		//restore to SIG_xxx value
	ldr	I_rWork,[r0,#DBG_TEMP]
	add	sp,r0,#DBG_INDIRECT_R0+8	//sp is trashed, if stacks assumed invalid
	str	sp,[r0,#DBG_HSP]
	b	join_fiq3
	.endif

//sp is valid for this!!!!!, what a treat
V_SWI:
	V_VectorEntrance V_rWork,V_rBranch,SIG_SWI
	b	join_fiq

//monitor mode also has instruction breakpoint, bkpt instruction
V_PrefetchAbort:
	VectorEntrance rWork,rBranch,SIG_PREFETCH_ABORT
	CP15_FSR mrc,rWork
	tst	rWork,#1<<9
	bne	DEBUG_EVENT		//br if a debug event, in monitor mode

//the 1st instruction after exiting special debug state can cause an abort or possibly the wrong instruction to execute.
//the Immu is not turned on until the 2nd instruction.
	tst	rWork,#1<<10
	JOIN beq
	and	rWork,rWork,#0xf
	cmp	rWork,#0x6
	JOIN bne		//br if not an external abort

	CP15_CONTROL mrc,rWork	//get the control register
	tst	rWork,#1		//test MMU

	BigMovNe	rWork,DEBUG_BASE+DBG_ABORT_PC	//only if mmu is on
	swpne	rBranch,lr,[rWork]	//save lr as a flag so not infinite errors, if mmu on
	cmpne	rBranch,lr

	.if STACKS_VALID
	ldrne	rBranch,[sp,#4]
	ldrne	rWork,[sp],#12
	subnes	pc,lr,#4		//retry access
	.else
	//rBranch is sp, so need to restore invalid stack
	ldrne	rWork,[rWork,#DBG_TEMP-DBG_ABORT_PC]
	subnes	pc,lr,#4		//retry access
	GetDebugMagic 	rWork
	.endif

	mov		rBranch,#(SIG_PREFETCH_ABORT<<2)+8
	JOIN2 b

//stacks aren't kept valid
//monitor mode also has data breakpoint, external debug break, trace-buffer full break
V_DataAbort:
	VectorEntrance rWork,rBranch,SIG_DATA_ABORT
	CP15_FSR mrc,rWork
	tst	rWork,#1<<9
	JOIN beq
	sub	lr,lr,#4		//+8 of offending instruction for most data aborts instead of +4
DEBUG_EVENT:
	GetDebugMagic 	rWork
	strd	r0,[rWork,#DBG_R0-DBG_MAGIC]
	sub	r0,rWork,#DBG_MAGIC

	.if STACKS_VALID
	ldr	rWork,[sp],#4
	ldr	rBranch,[sp],#8
	.else
	ldr	rWork,[r0,#DBG_TEMP]
	add	sp,r0,#DBG_INDIRECT_R0+8	//sp is trashed, if stacks assumed invalid
	.endif
	str	sp,[r0,#DBG_HSP]
	IsInstructionBreakpoint		//Out: Z-1 yes
	str	r2,[r0,#DBG_R2]
	b	SSDebug

V_Unused:
	mov	sp,r0			//save r0
	GetDebugBase 	r0
	str	sp,[r0,#DBG_R0]
	str	r1,[r0,#DBG_R1]
	mov	r1,#SIG_UNUSED
//	b	SaveRest2

//r0 - debug storage
//r1 - reason for getting here
SaveRest2:
	str	r2,[r0,#DBG_R2]
	sub	lr,lr,#4
	mrs	r2,spsr
	str	r3,[r0,#DBG_R3]
	ldrb	r3,[r0,#DBG_Mode]
//r2 - spsr
//r3 - DBG_MODE - don't assume that ram is working upon reset
SaveR4andUp:
	str	r4,[r0,#DBG_R4]
	str	lr,[r0,#DBG_PC]	//return PC
	str	r2,[r0,#DBG_CPSR]	//return CPSR
	mrs	r4,cpsr
	str	r4,[r0,#DBG_HCPSR]	//mode to return to before exception return

	tst	r2,#0xf
	.if USE_SUPERVISOR
	orreq	r2,r2,#0xd3		//if user mode, switch to supervisor mode
	.else
	orreq	r2,r2,#0xdf		//if user mode, switch to system mode
	.endif
	orrne	r2,r2,#0xc0		//set I, F
	msr	cpsr_c,r2

	add	r2,r0,#DBG_R5
	.if USE_SUPERVISOR
	stmneia	r2,{r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}
	stmeqia r2,{r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}^	//user mode registers
	streq	sp,[r0,#DBG_SUPERVISOR_SP]
	streq	lr,[r0,#DBG_SUPERVISOR_LR]
	.else
	stmia	r2,{r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}
	.endif
// **********************************
// **********************************

	mov	rDBG,r0
	mov	r6,r1			//reason
	mov	r5,r3			//DBG_MODE
	strb	r2,[rDBG,#DBG_LastSignal]
	CP15_CONTROL mrc,r1		//get the control register
	tst	r1,#1
	BigMov	rUart,UART_BASE		//this is right if MMU is off
	blne GetUartAddress			//if MMU is on
	b	gdb_lastSignal1

//r1 - CP15_CONTROL
GetUartAddress:
	GetPageTableStart	r1,PHYS_TO_VIRT
	bic	r3,rUart,#0x000ff000	//clear to meg boundary (needed for imx31)

	ldr	r2,[r1,r3,lsr #18]
	sub	r2,r2,r3
	cmp	r2,#1<<20
	movlo	pc,lr				//identity mapping on

	BigMov	rUart,(UART_VIRT_BASE&0xfff00000)
1:	ldr	r2,[r1,rUart,lsr #18]
	tst	r2,#3
	beq	2f
	sub	r2,r2,r3
	cmp	r2,#1<<20
	BigOrr2Lo	rUart,(UART_VIRT_BASE&0x000fffff)	//this won't generate an instruction on the pxa
	movlo	pc,lr				//br if found virtual address

	adds	rUart,rUart,#1<<20
	moveq	rUart,#0xE0000000
	b	1b
2:
	BigOrr2	r3,0xc02
	str	r3,[r1,rUart,lsr #18]	//steal this unused entry
	BigOrr2Lo	rUart,(UART_VIRT_BASE&0x000fffff)	//this won't generate an instruction on the pxa
//	b	data_cache_clean_invalidate_all

data_cache_clean_invalidate_all:
//flush data cache, if data cache is enabled
	CP15_CONTROL mrc,r0		//get the control register
	tst	r0,#4
	moveq	pc,lr				//return if disabled

	CleanInvalidateDataCache1	r0,r1,rDBG
	mov	pc,lr


//r0 - value to printm r9 - chksum
PrintHexEndian:
#if LITTLE_ENDIAN
	mov	r2,#8
1:	mov	r4,r0
	mov	r3,lr
2:	tst	r2,#1
	moveq	r0,r4,LSR #4
	andne	r0,r4,#0xf
	andeq	r0,r0,#0xf
	movne	r4,r4,ROR #8
	cmp	r0,#0xA
	addcs	r0,r0,#L1(a)-0x0a
	addcc	r0,r0,#L1(0)
	add	r9,r9,r0
	bl	Transmit
	subs	r2,r2,#1
	bne	2b
	mov	pc,r3

PrintHexByteEndian:
	mov	r2,#2
	b	1b
#else
	mov	r2,#8
	mov	r4,r0
1:	mov	r3,lr
2:	mov	r4,r4,ROR #28
	and	r0,r4,#0xf
	cmp	r0,#0xA
	addcs	r0,r0,#L1(a)-0x0a
	addcc	r0,r0,#L1(0)
	add	r9,r9,r0
	bl	Transmit
	subs	r2,r2,#1
	bne	2b
	mov	pc,r3
PrintHexByteEndian:
	mov	r4,r0,LSL #24
	mov	r2,#2
	b	1b
#endif

PrintHexByte:
	mov	r4,r0,LSL #24
	mov	r2,#2
	b	PrintHex1

	.if STACKS_VALID
	.else
// *******************************************************
	RelocationVector 0
// *******************************************************
	.endif

TransmitCRLF:
	BigMov  r0,L2(CR,LF)
	b	Transmit

//IN: r0 - value to print
//OUT: r0-r4 trashed
PrintHex:
	mov	r2,#8
	mov	r4,r0
PrintHex1:
	mov	r3,lr
1:	mov	r4,r4,ROR #28
	and	r0,r4,#0xf
	cmp	r0,#0xA
	addcs	r0,r0,#L1(A)-0x0a
	addcc	r0,r0,#L1(0)
	bl	Transmit
	subs	r2,r2,#1
	bne	1b
	mov	pc,r3


TransmitSPACE:
	mov	r0,#L1(SPACE)
//	b Transmit

//IN: r0 - character to transmit
//OUT: r0 - last character transmitted, r1 - trashed
Transmit:
1:
	TransmitChar	r0,rUart,r1
	movs	r1,r0,LSR #8
	movne	r0,r1
	bne	1b
	mov	pc,lr			//return

TransmitChkSum:
	add     r9,r9,r0
	b       Transmit

	.if STACKS_VALID
// *******************************************************
	RelocationVector 2 
// *******************************************************
	.endif

//OUT:
//z-0 good data in r0
//z-1 timeout, r0 is 0
//r1 - loop cnt time remaining
Receive:
	mov	r1,#RECEIVE_LOOP_COUNT
Receive1:
1:
	ReceiveChar	r0,rUart	//Out: z-0 good char
	movne	pc,lr			//return

	subs	r1,r1,#1
	bne	1b
	mov	pc,lr			//return




//IN: r2 - symbol #
//OUT: r1:r0 - value,
GetRegVal:
	cmp	r2,#SYM_FIRST_SPECIAL
	bcs	GetSpecialReg
#if DBG_R0
	add	r1,rDBG,#DBG_R0
	ldr	r0,[r1,r2,LSL #2]
#else
	ldr	r0,[rDBG,r2,LSL #2]
#endif
	mov	r1,#0
	mov	pc,lr

// ********************************************************************
// ********************************************************************
// ********************************************************************


gdb_lastSignal:
	ldrb	r6,[rDBG,#DBG_LastSignal]
	ldrb	r5,[rDBG,#DBG_Mode]
gdb_lastSignal1:
	ClearUart_DLAB rUart,rDBG,r0

1:	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
//	mov	r0,#L1(T)
	mov	r0,#L1(S)
	bl	TransmitChkSum
	mov	r0,r6
	bl	PrintHexByteEndian
//	mov	r0,#L1(F)
//	bl	TransmitChkSum
//	mov	r0,#L1(COLON)
//	bl	TransmitChkSum
//	mov	r2,#SYM_PC
//	bl	GetRegVal
//	bl	PrintHexEndian

	mov	r0,#L1(POUND)
	bl	Transmit
	and	r0,r9,#0xff
	bl	PrintHexByteEndian
	mov	r3,#0
	mov	r1,#RECEIVE_LOOP_COUNT
10:
	bl	Receive1		//z-0 good data in r0, z-1 timeout, r0 is 0
	moveq rNum1,#0
	beq	12f		//br if timeout
	cmpne	r6,#SIG_RESET 		//disable cr abort on power up
//	beq	10b		//uncomment to stop <cr> from stopping
	
	cmp	r0,#L1(PLUS)
	beq	WaitForDollarSign
	cmp	r0,#L1(DOLLAR)
	beq	ReadGDB
	
	cmp	r0,#L1(MINUS)
	beq	1b
	tst	r5,#1
	cmpeq	r0,#GDB_EXIT_CHAR
	bne	10b		//br if in GDB mode or NOT <cr>
	mov	rNum1,#1

//r6 -SIG_xxx
12:
	bl	TransmitCRLF
	cmp	r6,#SIG_DBG
	BigMov	r0,L4(D,B,G,MINUS)
	blcs	Transmit

	mov	r2,#0
	mov	r3,#0
	mov	r4,#0
	cmp	r6,#SIG_RESET
	cmpne	r6,#SIG_DBG_RESET
	BigMovEq r0,L4(R,e,s,e)
	BigMovEq r2,L1(t)
	cmp	r6,#SIG_UNDEFINED_INSTRUCTION
	BigMovEq r0,L4(U,n,d,e)
	BigMovEq r2,L1(f)
//	BigMovEq r3,L1(d)
	cmp	r6,#SIG_SWI
	BigMovEq r0,L3(S,w,i)
	cmp	r6,#SIG_PREFETCH_ABORT
	BigMovEq r0,L4(P,r,e,f)
	BigMovEq r2,L4(e,t,c,h)
	cmp	r6,#SIG_DATA_ABORT
	BigMovEq r0,L4(D,a,t,a)
	cmpne	r6,#SIG_PREFETCH_ABORT
	BigMovEq r3,L4(SPACE,A,b,o)
	BigMovEq r4,L2(r,t)
	cmp	r6,#SIG_UNUSED
	BigMovEq r0,L4(U,n,u,s)
	BigMovEq r2,L2(e,d)
	cmp	r6,#SIG_IRQ
	BigMovEq r0,L3(I,r,q)
	cmp	r6,#SIG_FIQ
	BigMovEq r0,L3(F,i,q)
	cmp	r6,#SIG_DBG_INSTRUCTION_BKPT
	BigMovEq r0,L4(I,n,s,t)
	BigMovEq r2,L4(SPACE,B,r,k)
	cmp	r6,#SIG_DBG_DATA_BKPT
	BigMovEq r0,L4(D,a,t,a)
	BigMovEq r2,L4(SPACE,B,r,k)
	cmp	r6,#SIG_DBG_BKPT_SOFTWARE
	BigMovEq r0,L4(B,k,p,t)
	cmp	r6,#SIG_DBG_EXTERNAL
	BigMovEq r0,L4(E,x,t,e)
	BigMovEq r2,L4(r,n,a,l)
	cmp	r6,#SIG_DBG_VECTOR_TRAP
	BigMovEq r0,L4(V,e,c,t)
	BigMovEq r2,L4(o,r,SPACE,T)
	BigMovEq r3,L3(r,a,p)
	cmp	r6,#SIG_DBG_TRACE_BUFFER_FULL
	BigMovEq r0,L4(T,r,a,c)
	BigMovEq r2,L4(e,SPACE,F,u)
	BigMovEq r3,L2(l,l)
	cmp	r6,#SIG_DBG_RESERVED
	BigMovEq r0,L4(R,e,s,e)
	BigMovEq r2,L4(r,v,e,d)

	bl	Transmit
	movs	r0,r2
	blne	Transmit
	movs	r0,r3
	blne	Transmit
	movs	r0,r4
	blne	Transmit

	bl	TransmitSPACE
	mov	r2,#SYM_PC
	bl	GetRegVal
	bl	PrintHex
//	bl	TransmitSPACE
	bl	TransmitCRLF

	EnableFlashReadMode	r0,r1,rUart

#if (PLATFORM_TYPE==NEONB)
	adr		lr,AfterPCPrint
	add		r1,r1,lr		//virtual address of AfterPCPrint if in Linux for data fetch
	cmp		lr,#0x800
	BigOrr2Eq lr,FLASH_BASE_ADDRESS 		//if in low part of CS0 flash, try CS1 flash
	BigMov	r0,0xe3a00055		//instruction mov	r0,#55
	cmp		rUart,#0x41000000
	movlo	r1,lr			//use physical address of AfterPCPrint for data fetch
	ldr		r2,[r1]
	cmp		r2,r0
	moveq	pc,lr		//br if CS1 valid
	b		AfterPCPrint_error
#else
	b		AfterPCPrint1
#endif

// ******************************************************************************************
//A MAIN goal is for all the code above this point to fit into the 2k mini instruction cache
//So that if difficult errors occur and the flash isn't functioning properly, we can
//at least get the PC printed out before a crash.
// ******************************************************************************************


//IN: r2 - symbol #, z-1 if SYM_FIRST_SPECIAL
//OUT: r1:r0 - value,
GetSpecialReg:
	bne	2f
	Get_FP0_reg r0,r1
	mov	pc,lr

	.if STACKS_VALID
	.else
//Must keep this aligned at 0x800 if Bal
	mac_AfterPCPrint 1
	.endif

2:
	.ifdef CONFIG_ARM1136
	cmp	r2,#SYM_TTB0
	CP15_TTBR0 mrceq,r0
	cmp	r2,#SYM_TTB1
	CP15_TTBR1 mrceq,r0
	cmp	r2,#SYM_TTBC
	CP15_TTBR_CONTROL mrceq,r0

	.else
	cmp	r2,#SYM_TTBR
	CP15_TTBR mrceq,r0
	.endif

	cmp	r2,#SYM_FSR
	CP15_FSR mrceq,r0
	cmp	r2,#SYM_FAR
	CP15_FAR mrceq,r0
	cmp	r2,#SYM_DCSR
	GetIfEq_DCSR
	cmp	r2,#SYM_CTRL
	CP15_CONTROL mrceq,r0
	mov	r1,#0
	mov	pc,lr

//out: c-1 means hex char, r0 - character read, r1 - 0:15 if r0 is a hex character
ReadHex:
1:
	ReceiveChar r0,rUart
	beq	1b			//br on timeout
	rsbs	r1,r0,#L1(9)	//reverse subtract
	subcss	r1,r0,#L1(0)
	movcs	pc,lr

	rsbs	r1,r0,#L1(f)	//reverse subtract
	subcss	r1,r0,#L1(a)
	addcs	r1,r1,#10

	movcs	pc,lr

	rsbs	r1,r0,#L1(F)	//reverse subtract
	subcss	r1,r0,#L1(A)
	addcs	r1,r1,#10
	mov	pc,lr

	.if STACKS_VALID
//Must keep this aligned at 0x800 if Bal
	mac_AfterPCPrint 0
	.endif


//r2:r1:r0 value to print, r9 - chksum
PrintHexEndian12:
	.if 1
	orr	r3,r0,r1
	orrs	r3,r3,r2
	bne	1f
	mov	r3,lr
	mov	r0,#L1(0)
	bl	TransmitChkSum
	mov	r0,#L1(ASTERISK)
	bl	TransmitChkSum
	mov	r0,#29+24-1
	mov	lr,r3
	b	TransmitChkSum
	.endif
1:
#if LITTLE_ENDIAN
	mov	r7,lr
	mov	r5,r1
	mov	r6,r2
	bl	PrintHexEndian	//r0
	mov	r0,r5
	bl	PrintHexEndian	//r1
	mov	r0,r6
	mov	lr,r7
	b	PrintHexEndian	//r2

#else
	mov	r7,lr
	mov	r5,r0
	mov	r6,r1
	mov	r0,r2
	bl	PrintHexEndian	//r2
	mov	r0,r6
	bl	PrintHexEndian	//r1
	mov	r0,r5
	mov	lr,r7
	b	PrintHexEndian	//r0
#endif


MainInitializationCode:
	cmp		pc,#MEM_START
	biclo	sp,pc,#FLASH_BASE_ADDRESS
	rsblos	sp,sp,#0x4000
	movlo	r1,#0
	blo		InitializeCont2		//br if not ram and not from reset vector

//pc - 0-0x4000, 0x04000000-0x04004000, MEM_START-0xffffffff
	InitCS0_CS1	r0,sp
	InitGPIO	r0,sp

#if (PLATFORM_TYPE==NEONB)
	adr		lr,AfterPCPrint
	cmp		lr,#0x800
	bne		InitializeCont1
	BigOrr2 lr,FLASH_BASE_ADDRESS 		//if in low part of CS0 flash, try CS1 flash
	BigMov	r0,0xe3a00055		//instruction mov	r0,#55
	ldr		r2,[lr]
	cmp		r2,r0
	addeq	pc,lr,#InitializeCont-AfterPCPrint		//br if CS1 valid
#endif
	b		InitializeCont1

//OUT: r0,r2 value, r7 trashed
ReadHexEndian:
	mov	r3,lr
	ReadHexE r2,r7
	mov	r0,r2
	mov	pc,r3


//OUT: r2:r1:r0 value
ReadHexEndian12:
	mov	r3,lr

#if LITTLE_ENDIAN
	ReadHexE r4,r7
	ReadHexE r5,r7
	ReadHexE r2,r7
	mov	r0,r4
	mov	r1,r5
#else
	ReadHexE r2,r7
	ReadHexE r5,r7
	ReadHexE r4,r7
	mov	r0,r4
	mov	r1,r5
#endif
	mov	pc,r3



ReadGDB:
	mov	r8,#0
	mov	r9,#0
	mov	rGdbNum3,#0
	mov	rGdbChkSum,#0
1:	bl	Receive
	beq	1b			//br on timeout
	cmp	r0,#L1(G)
	beq	gdb_G
//	cmp	r0,#L1(q)
//	beq	gdb_q
	mov	rGdbCmd,r0			//r5 - command being requested
	add	rGdbChkSum,rGdbChkSum,r0	//update checksum
	bl	GetNumber
	mov	r6,r2
	mov	r7,r0
	cmp	r0,#L1(POUND)
	beq	2f
	cmp	rGdbCmd,#L1(P)
	beq	gdb_P
//	cmp	r0,#L1(COMMA)
//	cmpne	r0,#L1(SEMICOLON)
//	cmpne	r0,#L1(COLON)
//	cmpne	r0,#L1(EQUAL)
	bl	GetNumber
	mov	r8,r2
	mov	r9,r0
	cmp	r0,#L1(POUND)
	beq	2f
	cmp	rGdbCmd,#L1(M)
	beq	gdb_M
	cmp	rGdbCmd,#L1(X)
	beq	gdb_X

	bl	GetNumber
	mov	rGdbNum3,r2
	cmp	r0,#L1(POUND)
	bne	WaitForPound

//# found, check chksum
2:	bl	ReadChksum
	and	rGdbChkSum,rGdbChkSum,#0xff
	cmp	rGdbChkSum,r2
	bne	CheckSumError
	mov	r0,#L1(PLUS)
	bl	Transmit

	cmp	rGdbCmd,#L1(g)
	beq	gdb_g
	cmp	rGdbCmd,#L1(p)
	beq	gdb_p
	cmp	rGdbCmd,#L1(m)
	beq	gdb_m
	cmp	rGdbCmd,#L1(c)
	beq	gdb_c
	cmp	rGdbCmd,#L1(C)
	beq	gdb_C
	cmp	rGdbCmd,#L1(s)
	beq	gdb_s
	cmp	rGdbCmd,#L1(S)
	beq	gdb_S
	cmp	rGdbCmd,#L1(z)
	beq	gdb_z
	cmp	rGdbCmd,#L1(Z)
	beq	gdb_Z
	cmp	rGdbCmd,#L1(t)
	beq	gdb_t
	cmp	rGdbCmd,#L1(QUESTION_MARK)
	beq	gdb_lastSignal
	cmp	rGdbCmd,#L1(D)
	beq	gdb_D
//this is an unimplemented command
UnImplemented:
	BigMov	r0,L4(DOLLAR,POUND,0,0)
	bl	Transmit
	b	WaitForDollarSign


SendError01:
	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
	mov	r0,#L1(E)
	bl	TransmitChkSum
	mov	r0,#L1(0)
	bl	TransmitChkSum
	mov	r0,#L1(1)
	bl	TransmitChkSum
	sub	r5,pc,#.+8-SendError01
	b	FinishPacket


SendOK:
	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
	mov	r0,#L1(O)
	bl	TransmitChkSum
	mov	r0,#L1(K)
	bl	TransmitChkSum
	sub	r5,pc,#.+8-SendOK
	b	FinishPacket

CheckSumError:
	mov	r0,#L1(MINUS)
	bl	Transmit
WaitForDollarSign:
11:	bl	GetNumber		//$ will branch to ReadGDB
	b	11b
WaitForPound:
11:	bl	GetNumber
	cmp	r0,#L1(POUND)
	bne	11b
	bl	ReadChksum
	and	rGdbChkSum,rGdbChkSum,#0xff
	cmp	rGdbChkSum,r2
	bne	CheckSumError
	mov	r0,#L1(PLUS)
	bl	Transmit
	b	UnImplemented



FinishPacket:
	mov	r0,#L1(POUND)
	bl	Transmit
	and	r0,r9,#0xff
	bl	PrintHexByte
10:	mov	r2,#5
11:	bl	ReadHex
	cmp	r0,#L1(DOLLAR)
	beq	ReadGDB
	cmp	r0,#L1(PLUS)
	beq	WaitForDollarSign
	cmp	r0,#L1(MINUS)
	

	moveq	pc,r5			//retransmit packet
	cmp	r0,#GDB_EXIT_CHAR
	bne	10b
	subs	r2,r2,#1
	beq	Prompt
	b	11b

	.if 0
gdb_q:
	mov	r2,#0
	mov	r3,#0
1:	bl	ReadHex
	cmp	r0,#L1(DOLLAR)
	beq	ReadGDB
	cmp	r0,#GDB_EXIT_CHAR
	beq	Prompt
	cmp	r0,#L1(POUND)
	beq	2f
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	mov	r1,r2,LSR #24
	orr	r3,r1,r3,LSL #8
	orr	r2,r0,r2,LSL #8
	b	1b
2:
	bl	ReadChksum
	and	rGdbChkSum,rGdbChkSum,#0xff
	cmp	rGdbChkSum,r2
	bne	CheckSumError
	mov	r0,#L1(PLUS)
	bl	Transmit
	b	UnImplemented
	.endif


//Detach command - gdb is exiting
gdb_D:
//clear break/watch points
	b	Prompt

//r6 - addr, r8 - PP, rGdbNum3 - mm
gdb_t:
	b	UnImplemented


//read general registers
gdb_g:
	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
	mov	r8,#0
1:	mov	r2,r8
	bl	GetRegVal
	bl	PrintHexEndian
	add	r8,r8,#1
	cmp	r8,#SYM_PC+1
	bne	1b

	mov	r2,#SYM_FP0
	bl	GetRegVal
	mov	r8,#0
2:	mov	r2,#0
	bl	PrintHexEndian12	//fp0-7
	mov	r0,#0
	mov	r1,#0
	add	r8,r8,#1
	cmp	r8,#8
	bne	2b

	bl	PrintHexEndian		//fps

	mov	r2,#SYM_CPSR
	bl	GetRegVal
	bl	PrintHexEndian		//cpsr

	sub	r5,pc,#.+8-gdb_g
	b	FinishPacket


//Read general register
//r6 - reg #
gdb_p:
	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
	mov	r2,r6
	cmp	r6,#SYM_PC+1
	bcs	2f

1:	bl	GetRegVal
3:	bl	PrintHexEndian
4:	sub	r5,pc,#.+8-gdb_p
	b	FinishPacket
2:	cmp	r6,#25
	mov	r2,#SYM_CPSR
	beq	1b
	cmp	r6,#24			//fps
	mov	r0,#0
	beq	3b
	cmp	r6,#16			//fp0
	mov	r0,#0
	mov	r1,#0
	mov	r2,#SYM_FP0
	mov	rGdbNum3,r6
	bleq	GetRegVal
	mov	r2,#0
	bl	PrintHexEndian12	//fp0-7
	mov	r6,rGdbNum3
	b	4b

//read memory
//r6 - addr, r8 - length
gdb_m:
	mov	r9,#0			//chksum
	mov	r0,#L1(DOLLAR)
	bl	Transmit
	mov	r5,r6
	movs	r7,r8
	beq	4f
1:
	tst	r5,#3
	bne	3f
	cmp	r7,#4
	bcc	3f
	ldr	r0,[r5],#4
	bl	PrintHexEndian
	subs	r7,r7,#4
	bne	1b
	b	4f

3:	ldrb	r0,[r5],#1
	bl	PrintHexByteEndian
	subs	r7,r7,#1
	bne	1b
4:	sub	r5,pc,#.+8-gdb_m
	b	FinishPacket


//continue
//r6 - address, 0- current PC
gdb_c:
	mov	r8,r6
//r8 - address, 0- current PC
gdb_C:
	movs	rNum1,r8
	movne	rValidCnt,#F_NUM1_MASK
	moveq	rValidCnt,#0
	tst	rNum1,#3
	bne	SendError01
	mov	r4,#1
	b	Go_Cmd1

//step
//r6 - address, 0- current PC
gdb_s:
	mov	r8,r6
//r8 - address, 0- current PC
gdb_S:
	movs	rNum1,r8
	movne	rValidCnt,#F_NUM1_MASK
	moveq	rValidCnt,#0
	tst	rNum1,#3
	bne	SendError01
	mov	r4,#1
	b	Trace_Cmd1



//watchpoints/breakpoints - remove
//r6 - type, r8 - address
gdb_z:
	cmp	r6,#0		//software breakpoint
	cmpne	r6,#1		//hardware breakpoint
	bne	UnImplemented
//	mov	rNum1,r8		//same reg
	bl	BC_Do
	b	SendOK



//watchpoints/breakpoints - insert
gdb_Z:
	cmp	r6,#0		//software breakpoint
	cmpne	r6,#1		//hardware breakpoint
	bne	UnImplemented
//	mov	rNum1,r8		//same reg
	bl	BS_Do			//	z-0 if r0==rNum1, breakpoint was already set
	cmpne	r0,rNum1
	beq	SendOK
	b	SendError01


//gdb routines below have not done checksum yet
//write general registers
gdb_G:
	mov	r8,#0
1:	bl	ReadHexEndian
	mov	r2,r8
	mov	r1,#0
	bl	StoreRegVal		//r2 - symbol #, r1:r0 - value
	add	r8,r8,#1
	cmp	r8,#SYM_PC+1
	bne	1b

	bl	ReadHexEndian12
	mov	r2,#SYM_FP0
	bl	StoreRegVal		//r2 - symbol #, r1:r0 - value

	mov	r8,#1
2:	bl	ReadHexEndian12
	add	r8,r8,#1
	cmp	r8,#8
	bne	2b

	bl	ReadHexEndian		//fps
	bl	ReadHexEndian		//cpsr

	mov	r2,#SYM_CPSR
	bl	StoreRegVal		//r2 - symbol #, r1:r0 - value
FinishWritePacket:
	bl	FinishWriteP
	b	SendOK
FinishWriteP:
	mov	r4,lr
	bl	ReadHex
	cmp	r0,#L1(POUND)
	bne	CheckSumError
	bl	ReadChksum
	and	rGdbChkSum,rGdbChkSum,#0xff
	cmp	rGdbChkSum,r2
	bne	CheckSumError
	mov	r0,#L1(PLUS)
	bl	Transmit
	mov	pc,r4




//write general register
//r6 - reg#
gdb_P:
	mov	r8,r6
	cmp	r6,#SYM_PC+1
	bcs	2f
1:
	bl	ReadHexEndian
4:	mov	r6,r0
	mov	r7,r1
	bl	FinishWriteP
	mov	r0,r6
	mov	r1,r7
	mov	r2,r8
	bl	StoreRegVal		//r2 - symbol #, r1:r0 - value
	b	SendOK

3:	bl	ReadHexEndian
	b	FinishWritePacket

2:	cmp	r6,#25
	mov	r8,#SYM_CPSR
	beq	1b
	cmp	r6,#24			//fps
	beq	3b
	bl	ReadHexEndian12
	cmp	r6,#16			//fp0
	bne	FinishWritePacket
	mov	r8,#SYM_FP0
	b	4b


//write memory
//r6 - addr, r8 - length
gdb_M:
	movs	r8,r8
	beq	FinishWritePacket

1:	tst	r6,#3
	beq	3f
2:	bl	ReadHexByteEndian
	strb	r2,[r6],#1
	subs	r8,r8,#1
	bne	1b
	b	FinishWritePacket
3:	cmp	r8,#4
	bcc	2b
	bl	ReadHexEndian
	str	r2,[r6],#4
	subs	r8,r8,#4
	bne	3b
	b	FinishWritePacket


//write memory binary with 0x7d, "$","#", escaped with 0x7d
//r6 - addr, r8 - length
gdb_X:
	movs	r8,r8
	beq	FinishWritePacket

1:	tst	r6,#3
	beq	3f
2:	bl	ReadByteEndian
	strb	r0,[r6],#1
	subs	r8,r8,#1
	bne	1b
	b	FinishWritePacket
3:	cmp	r8,#4
	bcc	2b
	bl	ReadEndian
	str	r0,[r6],#4
	subs	r8,r8,#4
	bne	3b
	b	FinishWritePacket

	

ReadEndian:
	mov	r4,lr
	mov	r5,#4
1:	bl	ReadByteEndian

#if LITTLE_ENDIAN
	mov	r0,r0,LSL #24
	orr	r2,r0,r2,LSR #8
#else
	orr	r2,r0,r2,LSL #8
#endif

	subs	r5,r5,#1
	bne	1b
	mov	r0,r2
	mov	pc,r4


ReadByteEndian:
	mov	r3,lr
	bl	ReadHex
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	cmp	r0,#0x7d
	beq	1f
	cmp	r0,#L1(DOLLAR)
	cmpne	r0,#L1(POUND)
	beq	CheckSumError
	mov	pc,r3
1:
	bl	ReadHex
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	mov	pc,r3


ReadHexByteEndian:
	mov	r3,lr
	bl	ReadHex
	bcc	CheckSumError
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	mov	r2,r1
	bl	ReadHex
	bcc	CheckSumError
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	add	r2,r1,r2,LSL #4
	mov	pc,r3

//OUT: r2 - checksum
ReadChksum:
	mov	r3,lr
	bl	ReadHex
	movcs	r2,r1
	blcs	ReadHex
	addcs	r2,r1,r2,LSL #4
	mvncc	r2,#0
	mov	pc,r3

//out: r0 exit character, r2 - number, rGdbChkSum - updated
GetNumber:
	mov	r2,#0
	mov	r3,lr
1:	bl	ReadHex
	bcc	3f
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
2:	tst	r2,#0xf0000000
	movne	pc,r3		//return on number overflow
	add	r2,r1,r2,LSL #4
	b	1b
3:	cmp	r0,#L1(DOLLAR)
	beq	ReadGDB
	cmp	r0,#GDB_EXIT_CHAR
	beq	Prompt
	cmp	r0,#L1(POUND)
	addne	rGdbChkSum,rGdbChkSum,r0		//update checksum
	mov	pc,r3

// *****************************************************************
#if (PLATFORM_TYPE==NEONB)
AfterPCPrint_error:
	bl	TransmitCRLF
	BigMov	r0,L4(F,l,a,s)
	bl	Transmit
	BigMov	r0,L4(h,SPACE,i,s)
	bl	Transmit
	BigMov	r0,L4(SPACE,i,n,v)
	bl	Transmit
	BigMov	r0,L4(a,l,i,d)
	bl	Transmit
	bl	TransmitCRLF
#endif
AfterPCPrint1:
	mov		rPrevNum1,#MEM_START
	movs	rNum1,rNum1
	tsteq	r6,#7		//make sure it's a SIG_RESET, or SIG_DBG_RESET, and timeout
	bne		1f
	BigBic	r0,pc,FLASH_BASE_ADDRESS
	cmp		r0,#0x4000
	blo		Gl_Cmd		//auto start program if running in flash
1:
	b		R_Cmd
//	b		Prompt2


Invalid:
	bl	TransmitCRLF
	BigMov	r0,L4(i,n,v,a)
	bl	Transmit
	BigMov	r0,L3(l,i,d)
	bl	Transmit
	b	Prompt
Prompt1:
	mov	rPrevNum1,#MEM_START
Prompt:
1:	bl	TransmitCRLF
Prompt2:
	mov	r0,#L1(PERIOD)
	bl	Transmit
	bl	TransmitSPACE
	mov	rFieldStart,#1
	bl	ReadCommand

	mov	r2,r0			//save exit character
	cmp	r0,#0x0d
	blne	Transmit
	bl	TransmitCRLF

	cmp	rCommand,#0
	bne	2f
	cmp	r2,#0x0d
	beq	1b
	cmp	r2,#L1(PLUS)
	addeq	rPrevNum1,rPrevNum1,#4
	beq	Examine_Cmd
	cmp	r2,#L1(MINUS)
	subeq	rPrevNum1,rPrevNum1,#4
	beq	Examine_Cmd
	b	1b

2:	tst	rSymbol,#F_NUM1_MASK
	mov	r0,rNum1
	blne	GetSymbolNumber
	bne	Invalid
	mov	rNum1,r0

	tst	rSymbol,#F_NUM2_MASK
	mov	r0,rNum2
	blne	GetSymbolNumber
	bne	Invalid
	mov	rNum2,r0

	cmp	rCommand,#L1(E)
	beq	Examine_Cmd
	cmp	rCommand,#L1(D)
	beq	Deposit_Cmd
	cmp	rCommand,#L1(G)
	beq	Go_Cmd
	cmp	rCommand,#L1(R)
	beq	R_Cmd
	cmp	rCommand,#L1(T)
	beq	Trace_Cmd
	BigMov	r0,C2(D,L)
	cmp	rCommand,r0
	beq	Download_Cmd
	cmp	rCommand,#L1(QUESTION_MARK)
	beq	Help_Cmd

	mov	r0,r0,LSL #8
	orr	r0,r0,#L1(W)
	cmp	rCommand,r0
	beq	Download_Wireless_Cmd

	BigMov	r0,C4(S,S,I,D)
	cmp	rCommand,r0
	beq	SSID_Cmd

	BigMov	r0,C3(M,A,C)
	cmp	rCommand,r0
	beq	MAC_Cmd

	BigMov	r0,C4(B,U,R,N)
	cmp	rCommand,r0
	beq	Burn_Cmd

	BigMov	r0,C2(B,B)
	cmp	rCommand,r0
	beq	Burn2_Cmd

	BigMov	r0,C4(B,A,L,L)	//Burn all of flash, erase end.
	cmp	rCommand,r0
	beq	BurnAll_Cmd

	cmp	rCommand,#L1(V)
	beq	Verify_Cmd

	BigMov	r0,C2(V,V)
	cmp	rCommand,r0
	beq	Verify2_Cmd

	BigMov	r0,C2(G,L)
	cmp	rCommand,r0
	beq	Gl_Cmd			//go linux!!

	BigEor2	r0,C2(G,L)^C2(G,G)
	cmp	rCommand,r0
	beq	GG_Cmd			//go without cache invalidate

	BigMov	r0,C2(T,T)
	cmp	rCommand,r0
	beq	TT_Cmd			//Trace without cache invalidate

	BigMov	r0,C2(B,S)
	cmp	rCommand,r0
	beq	BS_Cmd

	BigEor2	r0,C2(B,S)^C2(B,E)
	cmp	rCommand,r0
	beq	BE_Cmd

	BigEor2	r0,C2(B,E)^C2(B,C)
	cmp	rCommand,r0
	beq	BC_Cmd

	BigEor2	r0,C2(B,C)^C2(W,C)
	cmp	rCommand,r0
	beq	WC_Cmd

	BigEor2	r0,C2(W,C)^C2(W,W)
	cmp	rCommand,r0
	beq	WW_Cmd

	BigEor2	r0,C2(W,W)^C2(W,R)
	cmp	rCommand,r0
	beq	WR_Cmd

	mov	r0,r0,LSL #8
	orr	r0,r0,#L1(W)
	cmp	rCommand,r0
	beq	WRW_Cmd

	BigMov	r0,C3(M,M,U)
	cmp	rCommand,r0
	beq	MMU_Cmd
//	mov	r0,rCommand
//	bl	PrintHex
	b	Invalid

//IN: rFieldStart - starting field (this is a blank counting field)
//OUT:
ReadCommand:
	mov	r3,lr
	mov	rSymbol,#0
	mov	rField,rFieldStart
	mov	rValidCnt,#0		//low nibble: # of blanks before command
					//next: # of chars in command
					//next: # of blanks
					//next: # of hex digits in num1
					//next: # of blanks
					//next: # of hex digits in num2
	mov	rCommand,#0		//command
	mov	rNum1,#0		//1st number
	mov	rNum2,#0		//2nd number

1:	bl	Receive
	beq	1b			//br on timeout
	cmp	r0,#L1(DOLLAR)
	beq	ReadGDB
	cmp	r0,#L1(PLUS)
	cmpne	r0,#L1(MINUS)
	cmpne	r0,#L1(AT_SIGN)
	cmpeq	rValidCnt,#0
	cmpne	r0,#0x0d
	moveq	pc,r3			//return on -,+, 1st on line or <cr>

	cmp	r0,#L1(SPACE)
	bne	2f
	bl	Transmit
	tst	rValidCnt,#0x80000000
	eorne	rValidCnt,rValidCnt,#0x80000000	//clear bit 31 to mark as blank field
	movne	rField,rField,LSL #F_INC
//now we are in a counting blanks field
	rsb	r2,rField,rField,LSL #F_INC	//get a field mask
	and	r1,rValidCnt,r2
	cmp	r1,r2
	beq	Invalid				//br if field is full
	add	rValidCnt,rValidCnt,rField
	b	1b
2:
	cmp	r0,#0x08		//Backspace
	cmpne	r0,#0x7f		//del or Backspace
	bne	4f
	cmp	rValidCnt,#0
	beq	1b			//br if nothing to remove
	BigMov	r0,L3(BACKSPACE,SPACE,BACKSPACE)
	bl	Transmit
	sub	rValidCnt,rValidCnt,rField
	mov	r0,#4
	tst	rSymbol,rField
	movne	r0,#8
	cmp	rField,#F_COMMAND
	moveq	rCommand,rCommand,LSR #8
	cmp	rField,#F_NUM1
	moveq	rNum1,rNum1,LSR r0
	cmp	rField,#F_NUM2
	moveq	rNum2,rNum2,LSR r0

3:	rsb	r0,rField,rField,LSL #F_INC	//get a field mask
	tst	rValidCnt,r0
	bne	1b				//br if still more in this field
	bic	rSymbol,rSymbol,rField
	cmp	rField,rFieldStart
	beq	1b
//move back to lower field
	mov	rField,rField,LSR #F_INC
	eors	rValidCnt,rValidCnt,#0x80000000
	b	3b

4:	bl	Transmit
	tst	rValidCnt,#0x80000000
	orreq	rValidCnt,rValidCnt,#0x80000000	//set bit 31 to mark as data field
	moveq	rField,rField,LSL #F_INC
	cmp	rField,#F_UNDEF
	beq	Invalid
//now we are in a data field
	rsbs	r1,rField,#F_COMMAND
	rsbges	r1,r0,#L1(9)		//reverse subtract
	subges	r1,r0,#L1(0)
	movge	rField,rField,LSL #F_INC+F_INC	//advance to number field if in range "0"-"9"

	rsb	r2,rField,rField,LSL #F_INC	//get a field mask
	and	r1,rValidCnt,r2
	cmp	r1,r2
	beq	Invalid				//field is full
	add	rValidCnt,rValidCnt,rField

	rsbs	r1,r0,#L1(z)	//reverse subtract
	subges	r1,r0,#L1(a)
	andge	r0,r0,#0xdf	//convert to uppercase if was "a"-"z"
	cmp	rField,#F_COMMAND
	bne	6f
	tst	rCommand,#0xff000000
	bne	Invalid
	add	rCommand,r0,rCommand,LSL #8
	b	1b

6:	tst	rSymbol,rField
	bne	15f
	rsbs	r1,r0,#L1(9)	//reverse subtract
	subges	r1,r0,#L1(0)
	bge	5f
	rsbs	r1,r0,#L1(F)	//reverse subtract
	subges	r1,r0,#L1(A)
	add	r1,r1,#10
	bge	5f
	orr	rSymbol,rSymbol,rField
//now if number started with "A"-"F", convert back to ascii

	cmp	rField,#F_NUM1
	mov	r1,rValidCnt,LSR #F_NUM1_BIT	//# of nibbles +1 in rNum
	movne	r1,rValidCnt,LSR #F_NUM2_BIT
	and	r1,r1,#((1<<F_INC)-1)

	subs	r1,r1,#1			//# of nibbles in rNum
	beq	15f



	cmp	r1,#3
	bcs	Invalid			//br if too many leading hex characters
	mov	r1,r1,LSL #2		//# of bits valid in rNum
	cmp	rField,#F_NUM1
	rsb	lr,r1,#24
	moveq	r2,rNum1,ROR r1
	movne	r2,rNum2,ROR r1
	orr	r2,r2,r0,LSL lr		//save r0 character

	mov	lr,#0
26:	mov	r0,r2,LSR #28
	cmp	r0,#10
	addcs	r0,r0,#L1(A)-10
	addcc	r0,r0,#L1(0)
	orr	lr,r0,lr,LSL #8
	mov	r2,r2,LSL #4
	subs	r1,r1,#4
	bne	26b

	cmp	rField,#F_NUM1
	mov	r0,r2,LSR #24		//restore r0
	moveq	rNum1,lr
	movne	rNum2,lr
	b	15f

5:
	cmp	rField,#F_NUM1
	bne	7f
	tst	rNum1,#0xf0000000
	bne	Invalid
	add	rNum1,r1,rNum1,LSL #4
	b	1b
7:	tst	rNum2,#0xf0000000
	bne	Invalid
	add	rNum2,r1,rNum2,LSL #4
	b	1b

15:
	cmp	rField,#F_NUM1
	bne	17f
	tst	rNum1,#0xff000000
	bne	Invalid
	add	rNum1,r0,rNum1,LSL #8
	b	1b
17:	tst	rNum2,#0xff000000
	bne	Invalid
	add	rNum2,r0,rNum2,LSL #8
	b	1b

//rNum1 - 0 off, 1 on
MMU_Cmd:
	bl	data_cache_clean_invalidate_all
	tst	rNum1,#1
//now enable/disable MMU, Data Cache
	CP15_CONTROL mrc,r1		//get the control register
	biceq	r1,r1,#0x5
	orrne	r1,r1,#0x5
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0
	BigMov	rUart,UART_BASE		//this is right if MMU is off
	blne	GetUartAddress		//if mmu is on
	b	Prompt
	
// ********************************************************************
// ********************************************************************
// ********************************************************************
// r3 - -1 : Trace, 0 : Go
// r4 - 0 : normal, 1 : mark gdb mode
// rNum2 - 0 : invalidate cache, 1 : don't invalidate cache
//go without cache invalidate
GG_Cmd:
	mov	r4,#0
	mov	r3,#0
	mov	rNum2,#1
	b	Go2
Go_Cmd:
	mov	r4,#0
Go_Cmd1:
	mov	r3,#0
Go1:
	bl	data_cache_clean_invalidate_all
	.if	0
	mov	rNum2,#0
	.else
	mov	rNum2,#1
	.endif
Go2:
	ldrb	r2,[rDBG,#DBG_Mode]
	and	r2,r2,#0xfe
	orr	r2,r2,r4
	strb	r2,[rDBG,#DBG_Mode]	//mark gdb mode

	tst	rValidCnt,#F_NUM1_MASK
	strne	rNum1,[rDBG,#DBG_PC]	//return pc
	ldreq	rNum1,[rDBG,#DBG_PC]	//return pc

	ldr	r7,[rDBG,#DBG_CPSR]
	CheckForAndRemoveBkpt	rNum1,r0,r1,r2,r4
//z-1 current instruction matches one of the 2 breakpoint regs
	streq	r0,[rDBG,#DBG_TRACE]	//this is the current instruction breakpoint value
	beq	4f				//br if a bkpt matches current instruction
	cmp	r3,#0
	str	r3,[rDBG,#DBG_TRACE]	//-1 : Trace, 0 : Go
	beq	6f


4:
	bl	CalcNextPc			//IN: rNum1: current PC, r7- cpsr;    OUT: rNum1: next PC
	tst	r0,#1
	bne	Invalid
	AddBkpt	r0,r1,r2,r4,rDBG,DBG_TRACE
6:
	RestoreUart_DLAB rUart,rDBG,r0
	bl	ClearStickyAbort
	movs	rNum2,rNum2,LSR #1	//carry flag 1 : don't invalidate cache

	ReturnBugsWorkAround1	r1,r0,r2,rDBG,rUart,UART_BASE	//Out: r1 val for ReturnBugsWorkAround2
	add	r0,rDBG,#DBG_R2	

	.if	USE_SUPERVISOR
	ldr	r2,[rDBG,#DBG_CPSR]
	tst	r2,#0x0f		//C & V not affected
	ldmneia	r0,{r2,r3,r4,r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}
	ldmeqia	r0,{r2,r3,r4,r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}^	//load user mode registers
	ldreq	sp,[r0,#DBG_SUPERVISOR_SP-DBG_R2]
	ldreq	lr,[r0,#DBG_SUPERVISOR_LR-DBG_R2]
	teq	r1,#0			//doesn't affect C & V

	.else
	ldmia	r0,{r2,r3,r4,r5,r6,r7,r8,r9,sl,fp,ip,sp,lr}
	.endif

	ldr	r1,[r0,#DBG_HCPSR-DBG_R2]
	orr	r1,r1,#0xc0		//set I, F
	msr	cpsr_c,r1
//	ldr	r1,[r0,#DBG_CPSR-DBG_R2]	//don't allow changing return mode for now... to messy
//	msr	spsr,r1
	CP15_CF_DRAIN mcr,r1	//make sure data cache write buffers are drained
	ReturnBugsWorkAround2	//load lr to correct return val
	b	InvalidateAndReturn


//Out: z-0 : abort has happened since last clear
ClearStickyAbort:
	ClearDCSR_ABORT r1
	mov	pc,lr

TT_Cmd:
	mov	r4,#0
	mvn	r3,#0
	mov	rNum2,#1
	b	Go2

Trace_Cmd:
	mov	r4,#0
Trace_Cmd1:
	mvn	r3,#0
	b	Go1


//IN: rNum1: current PC, r7- cpsr
//OUT: r0: next PC
//trashed: r1,r2,r3,r4,r6
CalcNextPc:
	mov	r4,lr
	bl	ClearStickyAbort
	ldr	r2,[rNum1]
	add	r0,rNum1,#4
	nop				//let abort have time to signal
	bl	ClearStickyAbort
	movne	pc,r4
//1st check to see if condition codes allow instruction to execute
1:
	mov	r3,r2,LSR #28
	msr	CPSR_f,r7
	add	pc,pc,r3,LSL #3
	nop
	beq	2f		//0-eq
	mov	pc,r4
	bne	2f		//1-ne
	mov	pc,r4
	bcs	2f		//2-cs
	mov	pc,r4
	bcc	2f		//3-cc
	mov	pc,r4
	bmi	2f		//4-mi
	mov	pc,r4
	bpl	2f		//5-pl
	mov	pc,r4
	bvs	2f		//6-vs
	mov	pc,r4
	bvc	2f		//7-vc
	mov	pc,r4
	bhi	2f		//8-hi
	mov	pc,r4
	bls	2f		//9-ls
	mov	pc,r4
	bge	2f		//a-ge
	mov	pc,r4
	blt	2f		//b-lt
	mov	pc,r4
	bgt	2f		//c-gt
	mov	pc,r4
	ble	2f		//d-le
	mov	pc,r4
	b	2f		//e-al
	mov	pc,r4		//just a spacer instruction, never executed
				//f-nv

	and	r3,r2,#0xfe000000
	cmp	r3,#0xfa000000
	movne	pc,r4
//BLX instruction changing into thumb mode
	movs	r3,r2,LSL #8		//mov H bit to carry flag
	orrcs	r3,r3,#1<<7
	add	r0,r0,#4
	add	r0,r0,r3,ASR #6
	mov	pc,r4


//instruction WILL execute
2:
	mov	r3,r2,LSR #25
	and	r3,r3,#0x7
	and	r1,r2,#0x0000f000
	cmp	r1,   #0x0000f000
	mov	r1,r2,LSL #20
	mov	r1,r1,LSR #20		//r1 gets low 12 bits of r2
	add	pc,pc,r3,LSL #2
	nop
	b	10f
	b	11f
	b	12f
	b	13f
	b	14f
	b	15f
	mov	pc,r4		//6
	mov	pc,r4		//7
//nnnn 000n
10:
	movne	pc,r4
	and	r3,r2,#0x01800000
	cmp	r3,   #0x01000000
	bne	51f
	and	r3,r2,#0x0ff00000
	cmp	r3,   #0x01200000
	andeq	r3,r2,#0x000000d0
	cmpeq	r3,   #0x00000010
//B{L} <Rm> instruction -eq
	andeq	r3,r2,#0xf
#if DBG_R0
	addeq	r0,rDBG,#DBG_R0
	ldreq	r0,[r0,r3,LSL #2]
#else
	ldreq	r0,[rDBG,r3,LSL #2]
#endif

	biceq	r0,r0,#1
	mov	pc,r4


51:	tst	r2,#1<<4
	bne	52f
	add	r0,r0,#4		//pc+8
	bl	RegShiftImmed
	b	50f
52:
	tst	r2,#1<<7
	movne	pc,r4			//return if LDR|STR<H|D|B|SB|SH>|MUL|MLA...
//this is a shift by a register value
	add	r0,r0,#4		//pc+8
	bl	RegShiftReg
	b	50f

//nnnn 001n	immediate
11:
	movne	pc,r4
	and	r3,r2,#0x01800000
	cmp	r3,   #0x01000000
	moveq	pc,r4			//return if TST|TEQ|CMP|CMN|MRS|MSR|SMLA.....
	and	r6,r1,#0xff
	mov	r3,r1,LSR #7
	BIC	r3,r3,#1
	mov	r1,r6,ROR r3
	add	r0,r0,#4		//pc+8
//r1 has 2nd operand
50:
	mov	r3,r2,LSR #16
	and	r3,r3,#0xf
	cmp	r3,#0xf
	moveq	r6,r0			//pc+8
#if DBG_R0
	addne	r6,rDBG,#DBG_R0
	ldrne	r6,[r6,r3,LSL #2]	//<Rn> - r6
#else
	ldrne	r6,[rDBG,r3,LSL #2]	//<Rn> - r6
#endif

//r6 has 1st operand
	mov	r3,r2,LSR #21
	and	r3,r3,#0xf
	msr	CPSR_f,r7
	add	pc,pc,r3,LSL #3
	nop
	and	r0,r6,r1		//0-AND
	mov	pc,r4
	eor	r0,r6,r1		//1-EOR
	mov	pc,r4
	sub	r0,r6,r1		//2-SUB
	mov	pc,r4
	rsb	r0,r6,r1		//3-RSB
	mov	pc,r4
	add	r0,r6,r1		//4-ADD
	mov	pc,r4
	adc	r0,r6,r1		//5-ADC
	mov	pc,r4
	sbc	r0,r6,r1		//6-SBC
	mov	pc,r4
	rsc	r0,r6,r1		//7-RSC
	mov	pc,r4
	sub	r0,r0,#4		//8-TST, cannot get here
	mov	pc,r4
	sub	r0,r0,#4		//9-TEQ, cannot get here
	mov	pc,r4
	sub	r0,r0,#4		//A-CMP, cannot get here
	mov	pc,r4
	sub	r0,r0,#4		//B-CMN, cannot get here
	mov	pc,r4
	orr	r0,r6,r1		//C-ORR
	mov	pc,r4
	mov	r0,r1			//D-MOV
	mov	pc,r4
	bic	r0,r6,r1		//E-BIC
	mov	pc,r4
	mvn	r0,r1			//F-MVN
	mov	pc,r4

//LDR
12:
13:
	tsteq	r2,#1<<22		//B bit 22  1 : Byte, 0: Word
	movne	pc,r4
	tst	r2,#1<<20		//L bit 20  1 : LDR, 0:STR
	moveq	pc,r4

	add	r0,r0,#4		//pc+8
	tst	r2,#1<<24		//P bit
	moveq	r1,#0
	beq	34f
	tst	r2,#1<<25
	beq	34f			//branch if immediate12bits value is in r1
	bl	RegShiftImmed
34:
	mov	r3,r2,LSR #16
	and	r3,r3,#0xf
	cmp	r3,#0xf
	moveq	r6,r0			//pc+8
#if DBG_R0
	addne	r6,rDBG,#DBG_R0
	ldrne	r6,[r6,r3,LSL #2]	//<Rn> - r6
#else
	ldrne	r6,[rDBG,r3,LSL #2]	//<Rn> - r6
#endif

	tst	r2,#1<<23		//U bit Up/Down, Add/Sub
	addne	r6,r6,r1
	subeq	r6,r6,r1
	ldr	r0,[r6]
	mov	pc,r4


//LDM
14:
	and	r3,r2,#(1<<15)+(1<<20)
	cmp	r3,   #(1<<15)+(1<<20)
	movne	pc,r4

//LDM to PC -eq
	add	r0,r0,#4		//pc+8
	mov	r3,r2,LSR #16
	and	r3,r3,#0xf
	cmp	r3,#0xf
	moveq	r1,r0			//pc+8, should never execute
#if DBG_R0
	addne	r1,rDBG,#DBG_R0
	ldrne	r1,[r1,r3,LSL #2]	//<Rn> - r1
#else
	ldrne	r1,[rDBG,r3,LSL #2]	//<Rn> - r1
#endif

//r1 is already correct address for downward from base (U==0), included(P==0)
	tst	r2,#1<<23		//U bit
	beq	4f
	mov	r6,r2,LSL #16
3:	movs	r6,r6,LSL #1
	addcs	r1,r1,#4
	bne	3b
	eor	r2,r2,#1<<24
4:	tst	r2,#1<<24		//P bit
	subne	r1,r1,#4
	ldr	r0,[r1]
	mov	pc,r4

//B{L} instruction
15:
	mov	r3,r2,LSL #8
	add	r0,r0,#4
	add	r0,r0,r3,ASR #6
	mov	pc,r4

//IN: r0 - pc+4, r2 instruction, r1 low 12 bits of instruction
//OUT: r1 - shifted value, r2 - unchanged, r0 - pc+8
RegShiftImmed:
	mov	r1,r1,LSR #7		//Shift_imm - r1
	and	r3,r2,#0xf
	cmp	r3,#0xf
	moveq	r6,r0			//pc+8
#if DBG_R0
	addne	r6,rDBG,#DBG_R0
	ldrne	r6,[r6,r3,LSL #2]	//<Rm> - r6
#else
	ldrne	r6,[rDBG,r3,LSL #2]	//<Rm> - r6
#endif

	tst	r2,#1<<6
	bne	32f
	tst	r2,#1<<5
	bne	31f
	mov	r1,r6,LSL r1		//<Rm> shf Shift_imm - r1
	mov	pc,lr

31:	cmp	r1,#0
	movne	r1,r6,LSR r1
	mov	pc,lr

32:	tst	r2,#1<<5
	bne	33f
	cmp	r1,#0
	moveq	r1,#32
	mov	r1,r6,ASR r1
	mov	pc,lr

33:	cmp	r1,#0
	movne	r1,r6,ROR r1
	movne	pc,lr

	msr	CPSR_f,r7
	mov	r1,r6, RRX
	mov	pc,lr


//IN: r2 instruction, r1 low 12 bits of instruction
//OUT: r1 - shifted value, r2 - unchanged, r0 - pc+8
RegShiftReg:
	mov	r3,r1,LSR #8
	cmp	r3,#0xf
	moveq	r1,r0			//pc+8
#if DBG_R0
	addne	r1,rDBG,#DBG_R0
	ldrne	r1,[r1,r3,LSL #2]	//<Rs> - r1
#else
	ldrne	r1,[rDBG,r3,LSL #2]	//<Rs> - r1
#endif

	and	r3,r2,#0xf
	cmp	r3,#0xf
	moveq	r6,r0			//pc+8
#if DBG_R0
	addne	r6,rDBG,#DBG_R0
	ldrne	r6,[r6,r3,LSL #2]	//<Rm> - r6
#else
	ldrne	r6,[rDBG,r3,LSL #2]	//<Rm> - r6
#endif
	tst	r2,#1<<6
	bne	32f
	tst	r2,#1<<5
	moveq	r1,r6,LSL r1		//<Rm> shf <Rs> - r1
	movne	r1,r6,LSR r1
	mov	pc,lr

32:	tst	r2,#1<<5
	moveq	r1,r6,ASR r1
	movne	r1,r6,ROR r1
	mov	pc,lr



//IN: r0 - register name
//OUT: r0 - location
GetSymbolNumber:
	BigMov	r1,C2(R,0)
	sub	r2,r0,r1
	cmp	r2,#10
	bcc	1f

	BigMov	r1,C3(R,1,0)
	sub	r2,r0,r1
	cmp	r2,#6
	bcc	2f

	BigMov	r1,C3(F,P,0)
	cmp	r1,r0
	moveq	r2,#SYM_FP0
	beq	1f

	BigMov	r1,C2(S,L)
	cmp	r1,r0
	moveq	r2,#SYM_SL
	beq	1f

	BigMov	r1,C2(F,P)
	cmp	r1,r0
	moveq	r2,#SYM_FP
	beq	1f

	BigEor2	r1,C2(F,P)^C2(I,P)
	cmp	r1,r0
	moveq	r2,#SYM_IP
	beq	1f

	BigEor2	r1,C2(I,P)^C2(S,P)
	cmp	r1,r0
	moveq	r2,#SYM_SP
	beq	1f

	BigMov	r1,C2(L,R)
	cmp	r1,r0
	moveq	r2,#SYM_LR
	beq	1f

	BigMov	r1,C2(P,C)
	cmp	r1,r0
	moveq	r2,#SYM_PC
	beq	1f

	BigMov	r1,C4(C,P,S,R)
	cmp	r1,r0
	moveq	r2,#SYM_CPSR
	beq	1f

	BigMov	r1,C3(F,S,R)
	cmp	r1,r0
	moveq	r2,#SYM_FSR
	beq	1f

	BigMov	r1,C3(F,A,R)
	cmp	r1,r0
	moveq	r2,#SYM_FAR
	beq	1f

	BigMov	r1,C4(D,C,S,R)
	cmp	r1,r0
	moveq	r2,#SYM_DCSR
	beq	1f

	.ifdef CONFIG_ARM1136
	BigMov	r1,C4(T,T,B,0)
	cmp	r1,r0
	moveq	r2,#SYM_TTB0
	beq	1f

	BigEor2	r1,C4(T,T,B,0)^C4(T,T,B,1)
	cmp	r1,r0
	moveq	r2,#SYM_TTB1
	beq	1f

	BigEor2	r1,C4(T,T,B,1)^C4(T,T,B,C)
	cmp	r1,r0
	moveq	r2,#SYM_TTBC
	beq	1f

	.else
	BigMov	r1,C4(T,T,B,R)
	cmp	r1,r0
	moveq	r2,#SYM_TTBR
	beq	1f
	.endif

	BigMov	r1,C4(C,T,R,L)
	cmp	r1,r0
	moveq	r2,#SYM_CTRL
	beq	1f

	.if	USE_HCPS
	BigMov	r1,C4(H,C,P,S)
	cmp	r1,r0
	moveq	r2,#SYM_HCPS
	beq	1f
	.endif

//	BigMov	r1,C3(F,P,S)
//	cmp	r1,r0
//	moveq	r2,#SYM_FPS-SYM_F0
	movne	pc,lr


//3:	add	r2,r2,#SYM_F0-SYM_SL
2:	add	r2,r2,#SYM_SL
1:	BigMov	r0,DEBUG_SYM
	add	r0,r0,r2
	subs	r2,r2,r2
	mov	pc,lr



//IN: r0 - symbol address or number to print
PrintHexOrSymbol:
	BigAdd	r1,r0,-DEBUG_SYM
	cmp	r1,#SYM_LAST+1
	bcs	PrintHex
	mov	r0,#0

//IN: r1 register #, r0 {31:16}-  2 characters to print after
PrintRegName:
	cmp	r1,#SYM_SL
	bcs	1f
	add	r0,r0,r1,LSL #8
	add	r0,r0,#L2(NULL,0)
//	cmp	r1,#10
//	subge	r0,r0,#10<<8
//	movge	r0,r0,LSL #8
//	addge	r0,r0,#L2(NULL,1)
	add	r0,r0,#L1(R)
20:	b	Transmit
1:	BigOrr2Eq r0,L2(S,L)
	cmp	r1,#SYM_FP
	BigOrr2Eq r0,L2(F,P)
	cmp	r1,#SYM_IP
	BigOrr2Eq r0,L2(I,P)
	cmp	r1,#SYM_SP
	BigOrr2Eq r0,L2(S,P)
	cmp	r1,#SYM_LR
	BigOrr2Eq r0,L2(L,R)
	cmp	r1,#SYM_PC
	BigOrr2Eq r0,L2(P,C)
	ble	20b
	cmp	r1,#SYM_FP0

	mov	r0,r0,LSL #8
	BigOrr2Eq r0,L3(F,P,0)
	cmp	r1,#SYM_FSR
	BigOrr2Eq r0,L3(F,S,R)
	cmp	r1,#SYM_FAR
	BigOrr2Eq r0,L3(F,A,R)

	cmp	r1,#SYM_CPSR
	BigMovEq r0,L4(C,P,S,R)
	cmp	r1,#SYM_DCSR
	BigMovEq r0,L4(D,C,S,R)

	.ifdef CONFIG_ARM1136
	cmp	r1,#SYM_TTB0
	cmpne	r1,#SYM_TTB1
	cmpne	r1,#SYM_TTBC
	BigMovEq r0,L4(T,T,B,0)
	cmp	r1,#SYM_TTB1
	BigEor2Eq r0,L4(T,T,B,0)^L4(T,T,B,1)
	cmp	r1,#SYM_TTBC
	BigEor2Eq r0,L4(T,T,B,0)^L4(T,T,B,C)
	.else
	cmp	r1,#SYM_TTBR
	BigMovEq r0,L4(T,T,B,R)
	.endif
	
	cmp	r1,#SYM_CTRL
	BigMovEq r0,L4(C,T,R,L)

	.if	USE_HCPS
	cmp	r1,#SYM_HCPS
	BigMovEq r0,L4(H,C,P,S)
	.endif

	b	Transmit


//r2 - symbol #
PrintRegVal:
	mov	r6,lr
	bl	GetRegVal
	cmp	r2,#SYM_FP0
	bne	1f
	mov	r5,r0
	mov	r0,r1
	bl	PrintHexByte		//40 bits
	mov	r0,r5
1:	mov	lr,r6
	b	PrintHex





//r2 - symbol #, r1:r0 - value
StoreRegVal:
	cmp	r2,#SYM_FP0
	bcs	1f
#if DBG_R0
	add	r1,rDBG,#DBG_R0
	str	r0,[r1,r2,LSL #2]
#else
	str	r0,[rDBG,r2,LSL #2]
#endif
	mov	pc,lr
1:
	bne	2f
	Set_FP0_reg r0,r1
	mov	pc,lr
2:	cmp	r2,#SYM_FSR
	CP15_FSR mcreq,r0
	cmp	r2,#SYM_FAR
	CP15_FAR mcreq,r0
	cmp	r2,#SYM_DCSR
	StoreIfEq_DCSR

	.ifdef CONFIG_ARM1136
	cmp	r2,#SYM_TTB0
	CP15_TTBR0 mcreq,r0
	cmp	r2,#SYM_TTB1
	CP15_TTBR1 mcreq,r0
	cmp	r2,#SYM_TTBC
	CP15_TTBR_CONTROL mcreq,r0
	
	.else
	cmp	r2,#SYM_TTBR
	CP15_TTBR mcreq,r0
	.endif
	
	cmp	r2,#SYM_CTRL
	CP15_CONTROL mcreq,r0
	mov	pc,lr

R_Cmd:
	mov	r7,#0
1:	mov	r1,r7
	BigMov	r0,L4(NULL,NULL,COLON,SPACE)
	bl	PrintRegName
	bl	TransmitSPACE
	mov	r2,r7
	bl	PrintRegVal
	bl	TransmitSPACE
	bl	TransmitSPACE
	add	r7,r7,#1
	tst	r7,#3
	bleq	TransmitCRLF
	cmp	r7,#SYM_LAST_RCMD+1
	bne	1b
	b	Prompt
BS_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	beq	BE_Cmd
	tst	rSymbol,#F_NUM1_MASK
	mov	r0,rNum1
	blne	GetSymValue
	mov	rNum1,r0
	bl	BS_Do
	beq	Prompt
	b	BE_Cmd

//out: z-1  breakpoint just set
//	z-0 if r0==rNum1, breakpoint was already set
BS_Do:
	SearchBkpt rNum1,r0,r1
	beq	2f		//branch if found
	SetBkpt	rNum1,r0,r1,r2
	mov	pc,lr

2:	mov	r0,rNum1
	movs r1,#1
	mov	pc,lr
	

BC_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	beq	BE_Cmd
	tst	rSymbol,#F_NUM1_MASK
	mov	r0,rNum1
	blne	GetSymValue
	mov	rNum1,r0
	bl	BC_Do
	beq	Prompt
	b	BE_Cmd

//out: z-1  breakpoint just cleared
BC_Do:
	CheckForAndRemoveBkpt rNum1,r0,r1,r2,r3
	mov	pc,lr

BE_Cmd:
	BigMov	r0,L4(B,K,P,T)
	bl	Transmit
	bl	TransmitSPACE
	BigMov	r0,L4(S,t,a,t)
	bl	Transmit
	BigMov	r0,L4(u,s,COLON,SPACE)
	bl	Transmit
	
	PrintBkpts
	b	Prompt

WatchStatus:
	BigMov	r0,L4(W,A,T,C)
	bl	Transmit
	BigMov	r0,L2(H,SPACE)
	bl	Transmit
	BigMov	r0,L4(S,t,a,t)
	bl	Transmit
	BigMov	r0,L4(u,s,COLON,SPACE)
	bl	Transmit
	PrintWatchpts L4(SPACE,R,W,SPACE),L4(SPACE,SPACE,W,SPACE),L4(SPACE,R,SPACE,SPACE)
	b	Prompt

WC_Cmd:
	WatchClear r0
	b	Prompt
WRW_Cmd:
	mov	r4,#2
	b	WW_JOIN
WR_Cmd:
	mov	r4,#3
	b	WW_JOIN
WW_Cmd:
	mov	r4,#1
WW_JOIN:
	tst	rValidCnt,#F_NUM1_MASK
	beq	WatchStatus
	tst	rSymbol,#F_NUM1_MASK
	mov	r0,rNum1
	blne	GetSymValue
	mov	rNum1,r0

	tst	rSymbol,#F_NUM2_MASK
	mov	r0,rNum2
	blne	GetSymValue
	mov	rNum2,r0

	tst	rValidCnt,#F_NUM2_MASK
//z-0 means rNum2 is valid
	SetWatchpt	rNum1,rNum2,r4,r0,r5
	b	Prompt


//IN: rNum2 address
//OUT: r1:r0 val
GetSymValue:
	BigAdd	r2,rNum2,-DEBUG_SYM
	cmp	r2,#SYM_LAST+1
	bcc	GetRegVal
	mov	r0,rNum2
	mov	r1,#0
	mov	pc,lr
//IN: rPrevNum1 address or symbol
//OUT: r1:r0 val
GetValue:
	BigAdd	r2,rPrevNum1,-DEBUG_SYM
	cmp	r2,#SYM_LAST+1
	bcc	GetRegVal
	ldr	r0,[rPrevNum1]
	mov	r1,#0
	mov	pc,lr


//IN: rNum1 address
PrintLocationOrRegVal:
	BigAdd	r2,rNum1,-DEBUG_SYM
	cmp	r2,#SYM_LAST+1
	bcc	PrintRegVal
	tst	r5,#3
	tsteq	rNum1,#3
	bne	1f
	ldr	r0,[rNum1]
	b	PrintHex
1:	ldrb	r0,[rNum1]
	b	PrintHexByte

//IN: rPrevNum1 address, r1:r0 - value
StoreLocationOrReg:
	BigAdd	r2,rPrevNum1,-DEBUG_SYM
	cmp	r2,#SYM_LAST+1
	bcc	StoreRegVal
	tst	r5,#3
	tsteq	rPrevNum1,#3
	bne	1f
	str	r0,[rPrevNum1]
	mov	pc,lr
1:	strb	r0,[rPrevNum1]
	mov	pc,lr

GetChangeAmount:
	BigAdd	r0,rPrevNum1,-DEBUG_SYM
	cmp	r0,#SYM_LAST+1
	movcs	r0,#4
	movcc	r0,#1
	tst	r5,#3
//	tsteq	rPrevNum1,#3
	movne	r0,#1
	mov	pc,lr

Examine_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	moveq	rNum1,rPrevNum1
	movne	rPrevNum1,rNum1
	mov	r5,rNum1
	tst	rValidCnt,#F_NUM2_MASK
	bne	12f
1:	mov	r0,rNum1
	bl	PrintHexOrSymbol
	mov	r0,#L1(FSLASH)
	bl	Transmit

	bl	PrintLocationOrRegVal

	bl	TransmitSPACE
	mov	rFieldStart,#1<<(F_INC+F_INC+F_INC+F_INC)	//blank before num2 field
	str	r5,[rDBG,#DBG_TEMP]
	bl	ReadCommand
	ldr	r5,[rDBG,#DBG_TEMP]
	mov	r3,r0			//save exit character
	mov	r0,rNum2
	mov	r1,#0
	tst	rSymbol,#F_NUM2_MASK
	beq	20f
	blne	GetSymbolNumber
	bne	Invalid
	mov	rNum2,r0
	bl	GetSymValue

20:
	cmp	r3,#L1(PLUS)
	cmpne	r3,#L1(MINUS)
	cmpne	r3,#L1(AT_SIGN)
	beq	2f
	tst	rValidCnt,#F_NUM2_MASK
	beq	Prompt
	bl	StoreLocationOrReg
	mov	r3,#L1(PLUS)
	b	3f

2:	mov	r0,r3
	bl	Transmit
3:	bl	TransmitCRLF
	bl	GetChangeAmount
	cmp	r3,#L1(AT_SIGN)
	beq	5f
	cmp	r3,#L1(PLUS)
	addeq	rPrevNum1,rPrevNum1,r0
	subne	rPrevNum1,rPrevNum1,r0
4:	mov	rNum1,rPrevNum1
	b	1b

5:	bl	GetValue
	mov	rPrevNum1,r0
	mov	r5,r0
	b	4b

11:	sub	r0,rNum1,r5
	cmp	r0,#0x1000
	bhs	Prompt		//no more than 128 lines per examine command
	bl	TransmitCRLF
12:	mov	r0,rNum1
	bl	PrintHexOrSymbol
	mov	r0,#L1(FSLASH)
	bl	Transmit
13:
	bl	GetChangeAmount
	bcc	14f
	add	r0,rNum1,#3
	cmp	r0,rNum2
	bhi	16f
	tst	r5,#3
	tsteq	rNum1,#3
	bne	16f
14:	bl	PrintLocationOrRegVal
	bl	GetChangeAmount
15:	add	rNum1,rNum1,r0
	cmp	rNum1,rNum2
	bhi	Prompt
	tst	rNum1,#0x1f
	beq	11b
	bl	TransmitSPACE
	tst	rNum1,#0x0f
	bleq	TransmitSPACE
	b	13b
16:
	ldrb	r0,[rNum1]
	bl	PrintHexByte
	mov	r0,#1
	b	15b
	
Deposit_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	beq	Invalid
	tst	rValidCnt,#F_NUM2_MASK
	beq	Invalid
	mov	r0,rNum2
	mov	r1,#0
	tst	rSymbol,#F_NUM2_MASK
	blne	GetSymValue
	mov	rPrevNum1,rNum1
	bl	StoreLocationOrReg
	b	Prompt


Download_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	beq	Invalid
	tst	rNum1,#3
	bne	Invalid
	.ifdef LOCAL_RAM
	sub	r0,rNum1,#LOCAL_RAM_END
	cmn	r0,#LOCAL_RAM_END-LOCAL_RAM
	bhs	2f
	.endif
	.ifdef NFC_RAM
	sub	r0,rNum1,#NFC_RAM
	cmp	r0,#NFC_RAM_END-NFC_RAM
	blo	2f
	.endif
	cmp	rNum1,#MEM_START
	bcc	Invalid			//br if rNum1 is below ram start,
2:					//NOTE: arm sets the carry to the opposite of most processors on subtract, compare

	mov	fp,rNum1	//fp is only input to macro, unchanged on output
	xmodemReceive r3,r4,r5,r6,r7,r8,r9,sl,fp,Transmit,Receive	//rPrevNum1 is fp
	beq	63f
	bl	data_cache_clean_invalidate_all
	BigMov	r0,L4(E,R,R,O)
	bl	Transmit
	mov	r0,#L1(R)
	bl	Transmit
	movs r0,#1
	b	Prompt
63:
	mov	r5,r3		//save end address

	bl	data_cache_clean_invalidate_all
	InvalidateICacheRange fp,r3,r0		//r0 is trashed

	BigMov	r0,L3(O,K,SPACE)
	bl	Transmit
	mov	r0,fp
	bl	PrintHex
	mov	r0,#L1(MINUS)
	bl	Transmit
	mov	r0,r5
	bl	PrintHex
	b	Prompt


// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
//Code above this point must fit in 2k mini cache or be burned on flash
//
//Code below this point need not be in the 2k mini-instruction cache,
//it can be temporarily placed in the main instruction cache unlocked
// *******************************************************************************************

InitializeCont1:
//pc - 0-0x4000, 0x04000000-0x04004000, MEM_START-0xffffffff
	InitIC_Clocks	r0,sp
	InitUARTs	r0,sp,BAUDRATE,BAUDRATE,9600
	InitChangeCPUSpeed r0
	InitMemory	r0,sp,r1
	BigMov	r1,MACH_TYPE_SCANPASS
	SaveRegisters	r0,sp
// *******************************************************
	InitMMU		r0,r1
	InitPWR		r0,r1,sp	//out: r1 RCSR
InitializeCont2:
	cmp	pc,#MEM_START
	bhs	89f				//if in ram, must be mdebug
	bic	sp,pc,#FLASH_BASE_ADDRESS
	cmp	sp,#0x4000
	mov	r2,#FUNC_REQ_GL
	bhs	87f				//br if not part of reset vector
	tst r1,#RCSR_SLEEP_RESET
	beq 89f		//;  Not sleep.
	mov	r2,#FUNC_REQ_WAKEUP
87:	adr	r7,88f
	adr	lr,89f
	mov	r10,r1	//RCSR
	b	TryRoutine1
88:	ldmia	sp,{rPrevNum1,rUart,rDBG}
89:
	mov	r2,lr
	bl	CalcMemEnd
	mov	r3,r0
	mov	lr,r2
// *******************************************************
//0xnnnn0000 - 16k  remapping of flash address 0 for relocated vector table
//0xnnnn4000 - 16k 16k 1st level descriptors table
//0xnnnn8000 - 16k  2 x 1k 2nd level descriptor tables
//0xnnnnc000 - 16k  12k skipped, 4k debug data
//end of ram
	BigSub	sp,r3,0x8000
	sub	r2,sp,#0x4000
	mov	r1,r2

	cmp	pc,#MEM_START
	bichs	r0,pc,#0x000ff
	bichs	r0,r0,#0x0ff00
	orrhs	r0,r0,#0x400
	orrhs	r0,r0,#0x002
	strhs	r0,[r2],#4		//if running from ram, remap page 0 to us
	
	BigMov	r0,0x0402			//section descriptor, ap-01, privileged r/w
						//map virt to phys 1-to-1, non-cache, non-bufferable
	addhs	r0,r0,#1<<20
1:	str	r0,[r2],#4
	add	r0,r0,#1<<20
	cmp	r2,sp
	bne	1b


	add	sp,r1,r3,LSR #(20-2)
	add	r1,r1,#((MEM_START>>20)&0xfff)<<2	//microsoft assembler bug propagates the sign bit
	BigMov	r0,(MEM_START&0xfff00000)+0x040A	//cacheable (write through)
2:	str	r0,[r1],#4
	cmp	r1,sp
	addne	r0,r0,#1<<20
	bne	2b
//r0 is physical address of last meg of memory
	orr	r1,r2,#1			//2nd level descriptor address (1k boundary), coarse page table

	.ifdef __ARMASM
	GBLA TABLE_CNT
	.endif
	.if  ((DEBUG_START&0xfff00000) - (0xffff0000&0xfff00000))
	.set	TABLE_CNT,2
	str	r1,[r2,#((DEBUG_START>>18)&0x3ffc)-0x4000] //r2 is the end of the 16k table
	add	r1,r1,#0x400

	.else
	.set	TABLE_CNT,1
	.endif

	str	r1,[r2,#-4]			//map (0xfff00000) to 2nd level page table
						//last meg of memory (contains relocated reset vector)
	add	sp,r2,#((256*TABLE_CNT)-16)<<2		//256 - 4byte entries each mapping 4k

//clear out 1 or 2  level 2 page tables
	mov	r1,#0
3:	str	r1,[r2],#4
	cmp	r2,sp
	bne	3b


	BigMov	r1,0x559			//map to flash in 1 - 64k page
	add	sp,sp,#16<<2
4:	str	r1,[r2],#4
	cmp	r2,sp
	bne	4b

	bic	r0,r0,#0x000ff
	orr	r0,r0,#0x0000e			//cacheable writeback 4k page
//	orr	r0,r0,#0x0000a			//cacheable writethru 4k page
	orr	r0,r0,#0x00aa0			//privileged r/w, user read
	orr	r0,r0,#0xff000			//last 4k of ram

	str	r0,[r2,#((DEBUG_START>>10)&0x3fc)-(TABLE_CNT*0x400)] //debug variables, r2 is end of 1k tables

//	DEBUG_CHAR	CH_A
	BigMov	r0,0xffffffff
	CP15_DACR mcr,r0

	BigSub	r0,r3,0xc000			//MEM_END-0xc000
	SetupTTBR	r0,r1
	
//enable MMU
	ResetTLB r0

	CP15_CONTROL mrc,r1		//get the control register
	orr	r1,r1,#0x1			//set bit 0 - enable MMU
	CPWAIT	r0
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

//enable the instruction cache
	ResetInstructionCache r0

	orr	r1,r1,#0x1000			//set bit 12 -- the I bit
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

//enable the data cache
	ResetDataCache r0
	CP15_CF_DRAIN mcr,r0		//make sure it is drained just to be very safe

	orr	r1,r1,#0x4			//set bit 4 -- the D bit
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

	.if 0
//if not in halt mode, lock 64k of flash remap in instruction TLB(contains override relocated vector table)
//halt mode uses the mini cache and physical addresses only for this
	CP14_DCSR mrc,r0
	BigMov	r1,0xffff0000
	tst	r0,#0x40000000
//I am currently executing from flash, marked as uncacheable
	CP15_TLB_LOCK_IENTRY mcreq,r1	//lock 64k if not in halt mode
	CPWAIT	r0
	.endif

//now lock debug data into data cache
	BigMov	r1,DEBUG_START
	mov	r2,#DEBUG_SPACE>>5		//3 or 4 cache lines to lock
	Lock_TLB_DENTRY r1,r0
	
	Lock_Data_Cache_Range	r1,r2,r0

	EnableCoprocessorAccess r0
//now disable MMU, Data Cache
	.if 1
	CP15_CONTROL mrc,r1		//get the control register
	bic	r1,r1,#0x5
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0
	.endif
#if (DO_GPTEST==0)
//	TransMacro L1(U)
#ifndef SKIP_LCD_SETUP
Setup_Lcd:
	sub	r3,r3,#0x100000		//last meg-8k for video ram
	SetupLcd	r3,r0,r1,r2,sp,lr		//r0-r2,sp,lr trashed
	add	r3,r3,#0x100000		//back to end of ram
#endif

// ****************************
	mrs	r1, CPSR
	bic	r2, r1,#PSR_MODE_MASK
	orr	r0, r2,#PSR_NOINTS_MASK+PSR_MODE_SVC	//change to supervisor stack
	msr	CPSR_c, r0
	BigMov	r0,MEM_START+SS_MEM_OFFSET+SS_SUPERVISOR
	mov	sp, r0

	orr	r2, r2, #PSR_NOINTS_MASK+PSR_MODE_IRQ	//irq stack
	msr	CPSR_c, r2
	BigAdd	sp, r0, SS_IRQ

	sub	r2, r2, #PSR_MODE_IRQ-PSR_MODE_FIQ		//fiq stack
	msr	CPSR_c, r2
	BigAdd	sp, r0, SS_IRQ+SS_FIQ

	add	r2, r2, #PSR_MODE_SYSTEM-PSR_MODE_FIQ	//system(user mode)
	msr	CPSR_c, r2
	BigAdd	sp, r0, SS_IRQ+SS_FIQ+SS_SYSTEM

	sub	r2, r2, #PSR_MODE_SYSTEM-PSR_MODE_UNDEF	//undefined stack
	msr	CPSR_c, r2
	BigAdd	sp, r0, SS_IRQ+SS_FIQ+SS_SYSTEM+SS_UNDEFINED

	sub	r2, r2, #PSR_MODE_UNDEF-PSR_MODE_ABORT	//Abort stack
	msr	CPSR_c, r2
	BigAdd	sp, r0, SS_IRQ+SS_FIQ+SS_SYSTEM+SS_UNDEFINED+SS_ABORT
//
	msr	CPSR_c, r1
	sub	r2, r2, #PSR_MODE_ABORT-PSR_MODE_SVC	//supervisor stack

// ****************************
	msr	spsr,r2						//make sure returning state is supervisor

	mov	sp,r0
	GetDebugBaseStickyCheck	r0,r3
	str	sp,[r0,#DBG_HSP]

	and	r1,r1,#PSR_MODE_MASK
	mov	r3,#0
	cmp	r1,#PSR_MODE_DEBUG
	BigMov	lr,MEM_START+0x8000		//default start pc
	beq	SaveDebug		//br if a debug exception in halt mode
	mov	r1,#SIG_RESET
	b	SaveR4andUp
#else
//#define GPBIT 2
//#define GPL GPLR1
#define GPBIT 9
#define GPL GPLR0
#define GPMASK (1<<GPBIT)
GPTest:
	BigMov	rUart,UART_BASE
	BigMov	fp,GPIO_BASE
	mov	r6,#GPMASK
	str	r6,[fp,#GPSR0]

2:	bl	TransmitCRLF
	mov	r3,#40
20:	mov	r2,#0x300000

1:	ldr	r0,[fp]
	and	r0,r1,#GPMASK
	cmp	r0,r6
	subnes	r2,r2,#1
	bne	1b			//wait for desired state

	mov	r1,#L1(0)
	add	r0,r1,r0,LSR #GPBIT
	bl	Transmit

	mov	r0,#GPMASK
	eors	r6,r6,#GPMASK		//get new desired state
	streq	r0,[fp,#GPCR0]
	strne	r0,[fp,#GPSR0]
	subs	r3,r3,#1
	bne	20b
	b	2b
#endif




	.if 0		//macro tests
	BigMov	r0,-1
	BigMov	r0,-5
	BigMov	r0,0x0ffffff0
	BigMov	r0,1
	BigMov	r0,0xff
	BigMov	r0,0xf000000f
	BigMov	r0,0x80010003
	.endif


TransLineR3:
	mov	r4,#0
TransLineR4:
	mov	r5,#0
TransLineR5:
	mov	r6,#0
TransLineR6:
	mov	r7,#0
TransLineR7:
	mov	fp,lr
	mov	r0,r2
	bl	Transmit

	BigEor	r0,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(SPACE,SPACE,MINUS,SPACE)
	bl	Transmit

	mov	r0,r3
	bl	Transmit
	movs	r0,r4
	blne	Transmit
	movs	r0,r5
	blne	Transmit
	movs	r0,r6
	blne	Transmit
	movs	r0,r7
	blne	Transmit
	mov	lr,fp
	b	TransmitCRLF

Help_Cmd:
	BigMov	rSP,L2(SPACE,SPACE)
	orr	rSP,rSP,rSP,LSL #16

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(B,C,SPACE,SPACE)	//BC  -Breakpoint clear
	BigEor	r3,r2, L4(B,C,SPACE,SPACE)^L4(B,r,e,a)
	BigMov	r4,			   L4(k,p,o,i)
	BigEor	r5,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(n,t,SPACE,c)
	BigMov	r6,			   L4(l,e,a,r)
	bl	TransLineR6

	BigEor2	r2,L4(B,C,SPACE,SPACE)    ^L4(B,E,SPACE,SPACE)	//BE  -Breakpoint examine
	BigEor2	r5,L4(n,t,SPACE,c)    ^L4(n,t,SPACE,e)
	BigMov	r6,			   L4(x,a,m,i)
	BigMov	r7,			   L2(n,e)
	bl	TransLineR7

	BigEor2	r2,L4(B,E,SPACE,SPACE)    ^L4(B,S,SPACE,SPACE)	//BS  -Breakpoint set
	BigEor2	r5,L4(n,t,SPACE,e)    ^L4(n,t,SPACE,s)
	BigMov	r6,			   L2(e,t)
	bl	TransLineR6

	BigEor2	r2,L4(B,S,SPACE,SPACE)	  ^L4(B,U,R,N)	//BURN-Write to flash
	BigMov	r3,			   L4(W,r,i,t)
	BigEor	r4,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(e,SPACE,t,o)
	BigMov	r5,			   L4(SPACE,f,l,a)
	BigMov	r6,			   L2(s,h)
	bl	TransLineR6

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(E,SPACE,SPACE,SPACE)	//E   -Examine
	BigEor	r3,r2, L4(E,SPACE,SPACE,SPACE)^L4(E,x,a,m)
	BigMov	r4,			   L3(i,n,e)
	bl	TransLineR4

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(D,SPACE,SPACE,SPACE)	//D   -Deposit
	BigEor	r3,r2, L4(D,SPACE,SPACE,SPACE)^L4(D,e,p,o)
	BigMov	r4,			   L3(s,i,t)
	bl	TransLineR4

	BigEor2	r2,L4(D,SPACE,SPACE,SPACE)    ^L4(D,L,SPACE,SPACE)	//DL  -Download
	BigEor2	r3,L4(D,e,p,o)    ^L4(D,o,w,n)
	BigMov	r4,			   L4(l,o,a,d)
	bl	TransLineR4

	BigEor2	r2,L4(D,L,SPACE,SPACE)    ^L4(D,L,W,SPACE)	//DLW -Download wireless
	BigEor	r5,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(SPACE,w,i,r)
	BigMov	r6,			   L4(e,l,e,s)
	BigMov	r7,			   L1(s)
	bl	TransLineR7

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(G,SPACE,SPACE,SPACE)	//G   -Go
	BigMov	r3,			   L2(G,o)
	bl	TransLineR3

	BigEor2	r2,L4(G,SPACE,SPACE,SPACE)    ^L4(G,L,SPACE,SPACE)	//GL  -Go Linux
	BigEor2	r3,L2(G,o)		  ^L4(G,o,SPACE,L)
	BigMov	r4,			   L4(i,n,u,x)
	bl	TransLineR4

	BigEor2	r2,L4(G,L,SPACE,SPACE)    ^L4(G,G,SPACE,SPACE)	//GG  -Go no cache clear
	BigEor2	r3,L4(G,o,SPACE,L)    ^L4(G,o,SPACE,n)
	BigEor	r4,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(o,SPACE,c,a)
	BigEor	r5,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(c,h,e,SPACE)
	BigMov	r6,			   L4(c,l,e,a)
	BigMov	r7,			   L1(r)
	bl	TransLineR7

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(R,SPACE,SPACE,SPACE)	//R   -Registers
	BigEor	r3,r2, L4(R,SPACE,SPACE,SPACE)^L4(R,e,g,i)
	BigMov	r4,			   L4(s,t,e,r)
	BigMov	r5,			   L1(s)
	bl	TransLineR5

	BigMov	r2,			   L4(S,S,I,D)	//SSID- Set SSID string
	BigEor	r3,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(S,e,t,SPACE)
	mov	r4,r2
	BigMov	r5,			   L4(SPACE,s,t,r)
	BigMov	r6,			   L3(i,n,g)
	bl	TransLineR6

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(T,SPACE,SPACE,SPACE)	//T   -Trace
	BigEor	r3,r2, L4(T,SPACE,SPACE,SPACE)^L4(T,r,a,c)
	BigMov	r4,L1(e)
	bl	TransLineR4

	BigEor2	r2,L4(T,SPACE,SPACE,SPACE)    ^L4(T,T,SPACE,SPACE)	//TT   -Trace
	BigEor2	r4,L1(e)		  ^L4(e,SPACE,n,o)
	BigEor	r5,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(SPACE,c,a,c)
	BigEor	r6,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(h,e,SPACE,c)
	BigMov	r7,			   L4(l,e,a,r)
	bl	TransLineR7

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(V,SPACE,SPACE,SPACE)	//V   -Verify
	BigEor	r3,r2, L4(V,SPACE,SPACE,SPACE)^L4(V,e,r,i)
	BigMov	r4,			   L2(f,y)
	bl	TransLineR4

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(W,C,SPACE,SPACE)	//WC  -Watch clear
	BigEor	r3,r2, L4(W,C,SPACE,SPACE)^L4(W,a,t,c)
	BigEor	r4,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(h,SPACE,c,l)
	BigMov	r5,			   L3(e,a,r)
	bl	TransLineR5

	BigEor2	r2,L4(W,C,SPACE,SPACE)    ^L4(W,R,SPACE,SPACE)	//WR  -Watch read
	BigEor2	r4,L4(h,SPACE,c,l)    ^L4(h,SPACE,r,e)
	BigMov	r5,			   L2(a,d)
	bl	TransLineR5

	BigEor2	r2,L4(W,R,SPACE,SPACE)    ^L4(W,R,W,SPACE)	//WRW -Watch read/write
	BigEor2	r5,L2(a,d)            ^L4(a,d,FSLASH,w)
	BigMov	r6,			   L4(r,i,t,e)
	bl	TransLineR6

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(W,W,SPACE,SPACE)	//WW  -Watch write
	BigEor2	r4,L4(h,SPACE,r,e)    ^L4(h,SPACE,w,r)
	BigMov	r5,			   L3(i,t,e)
	bl	TransLineR5

	BigEor	r2,rSP,L4(SPACE,SPACE,SPACE,SPACE)^L4(QUESTION_MARK,SPACE,SPACE,SPACE)	//?   -Help
	BigMov	r3,			   L4(H,e,l,p)
	bl	TransLineR3

	b	Prompt

//.global	CalcMemEnd
CalcMemEnd:
	CalcMemSize	r0,MEMORY_CONTROL_BASE			//out: r0 - mem size
	BigAdd2	r0,MEM_START		//32 meg
	mov	pc,lr


//rRamSector - start (rNum1)
//rRamEnd - end (rNum2)
Verify_Cmd:
	mov	r5,#CMD_VERIFY
	b	Burn1
Verify2_Cmd:
	mov	r5,#CMD_VERIFY
	b	Burn2
BurnAll_Cmd:
	mov	r5,#CMD_BURNALL
	b	Burn1
Burn2_Cmd:
	mov	r5,#CMD_BURN
Burn2:
	BigMov	r4,FLASH_BASE_ADDRESS+(16<<20)
	b	BurnContinue
Burn_Cmd:
	mov	r5,#CMD_BURN
Burn1:
	BigMov	r4,FLASH_BASE_ADDRESS
BurnContinue:
	tst	rValidCnt,#F_NUM1_MASK
	tstne	rValidCnt,#F_NUM2_MASK
	beq	Invalid
	tst	rRamSector,#3
	bne	Invalid
	tst	rRamEnd,#3
	bne	Invalid
	cmp	rRamSector,rRamEnd
	bhs	Invalid
	cmp	rRamSector,#MEM_START
	blo	Invalid			//br if rRamSector is below ram start,
	bl	CalcMemEnd
	mov	r1,r4
	cmp	rRamEnd,r0
	bhi	Invalid			//br if rRamEnd is above ram end,
	sub	lr,rRamEnd,rRamSector
	cmp	lr,#f128j3a_SIZE
	bhi	Invalid

	mvn		r2,#0
	tst		rRamEnd,#1
	strneb	r2,[rRamEnd],#1
	tst		rRamEnd,#2
	strneh	r2,[rRamEnd],#2

//1st verify that flash needs burned
	mov	lr,rRamSector
1:	ldr	r2,[lr],#4
	ldr	r3,[r1],#4
	cmp	r2,r3
	bne	2f
	cmp	lr,rRamEnd
	blo	1b

	cmp	r5,#CMD_BURNALL
	beq	3f
	BigMov	r0,L4(V,e,r,i)
	bl	Transmit
	BigMov	r0,L4(f,i,e,d)
	bl	Transmit
	b	Prompt
2:
	sub	r1,r1,#4
	cmp	r5,#CMD_VERIFY
	bne	3f
	sub	r0,lr,#4
20:
	bl	Burn_error
	b	Prompt
3:
	mov	rFlashSector,r1,LSR #18
	mov	rFlashSector,rFlashSector,LSL #18		//round to 256k boundary
	add	rRamSector,rRamSector,rFlashSector
	sub	rRamSector,rRamSector,r4

	GetFlashWidth rFlashSector,r1	//Out: z-0 means 16 bit width
	adr	r4,Burn_Rtn32
	ldr	r3,[r4]
	BigMov	r2,0xe58d5054	//NOTE: when 1st instruction of Burn_Rtn32 changes (or DBG_TEMP value), this must change as well
	adrne	r4,Burn_Rtn16

	cmp	r4,#MEM_START
	bhs	10f
//I must move the burn code, unless I'm only running from cache
	cmp	r2,r3
	bne	10f
	tst	r1,#1<<3		//bit 3 - 1 means 16 bit mode
	BigAdd	r1,r0,(-0x1000+((DEBUG_BASE+DBG_START)&0xfe0)-32)
	moveq	r3,#((Burn_Rtn_End-Burn_Rtn32)+0x1f)&(~0x1f)	//round to 32 byte (cache line)boundary
	movne	r3,#((Burn_Rtn_End-Burn_Rtn16)+0x1f)&(~0x1f)	//round to 32 byte (cache line)boundary
	add	r2,r4,r3
	sub	r1,r1,r3
	mov	r0,r1
4:	ldr	r3,[r4],#4
	str	r3,[r1],#4
	cmp	r4,r2
	blo	4b

	mov	r4,r0
5:
	CP15_CF_INVAL_ILINE mcr,r4
	add	r4,r4,#32
	cmp	r4,r1
	blo	5b

	CP15_CF_INVAL_BTB mcr,r4

	CP15_CF_DRAIN mcr,r4	//data cache is disabled, but it should still drain first
	CPWAIT	r1

	BigMov	rFlashBase,FLASH_BASE_ADDRESS
	mov	rRet,#0
	blx	r0
	b	20b		//unknow Man/dev Id if it returns from here

10:
//I'm running from ram, or from cache only
	BigMov	rFlashBase,FLASH_BASE_ADDRESS
	adr	rRet,11f
	blx	r4
	b	20b		//unknow Man/dev Id if it returns from here
11:	b	Prompt




//BurnRtn istr,ildr,mask,shift,inc,plait
Burn_Rtn32:
	BurnRtn str,ldr,0xffffffff,0,0,4,0
Burn_Rtn16:
#if 1
	BigMov	r0,FLASH_STATUS_CLEAR_CMD&0xffff
	strh	r0,[rFlashBase,#0]
	delay
	BigMov	r0,FLASH_ID_CMD&0xffff
	strh	r0,[rFlashBase,#0]
	delay
	ldrh	r2,[rFlashBase,#2]

	BigMov	r0,FLASH_READ_CMD&0xffff
	strh	r0,[rFlashBase,#0]
	delayCnt r0,((40/10)*COUNT_MULT)
	cmp	r2,#0x16
	cmpne	r2,#0x17
	cmpne	r2,#0x18
	beq	99f
//(PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)
	BurnRtn strh,ldrh,  0xffff,0,1,4,1		//the plait version (a1 jumpered to a high address line)
99:
	BurnRtn strh,ldrh,  0xffff,1,1,2,0		//the non-plait version (a1 correct)
#else

#if (PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)
	BurnRtn strh,ldrh,  0xffff,0,1,4,1		//the plait version (a1 jumpered to a high address line)
#else
99:
	BurnRtn strh,ldrh,  0xffff,1,1,2,0		//the non-plait version (a1 correct)
#endif

#endif

Timeout:
	BigMov	r0,L4(T,i,m,e)
	bl	TransmitR
	BigMov	r0,L4(d,SPACE,o,u)
	bl	TransmitR
	BigMov	r0,L2(t,SPACE)
	bl	TransmitR

	mov	r0,rRamSector
	mov	r1,rFlashSector
	ldr	r2,[r0]
	ldr	r3,[r1]
	bl	Burn_error
	b	Burn_return

ReturnError:
	sub	r0,r0,#4
	sub	r1,r1,#4
	bl	Burn_error
	b	Burn_return

PrintSector:
	mov	r5,lr
	bl	TransmitR_CRLF
	BigMov	r0,L4(S,e,c,t)
	bl	TransmitR
	BigMov	r0,L3(o,r,SPACE)
	bl	TransmitR
	mov	r0,rFlashSector
	bl	PrintHexR
	mov	r0,#L1(SPACE)
	bl	TransmitR
	mov	r0,rRamSector
	mov	r1,rFlashSector
	mov	pc,r5

PrintErasing:
	mov	r5,lr
	BigMov	r0,L4(E,r,a,s)
	bl	TransmitR
	BigMov	r0,L4(i,n,g,SPACE)
	mov	lr,r5
	b	TransmitR

PrintProgramming:
	mov	r5,lr
	BigMov	r0,L4(P,r,o,g)
	bl	TransmitR
	BigMov	r0,L4(r,a,m,m)
	bl	TransmitR
	BigMov	r0,L4(i,n,g,SPACE)
	bl	TransmitR
	mov	r0,rRamSector
	mov	r1,rFlashSector
	mov	pc,r5

PrintVerifying:
	mov	r5,lr
	BigMov	r0,L4(V,e,r,i)
	bl	TransmitR
	BigMov	r0,L4(f,y,i,n)
	bl	TransmitR
	BigMov	r0,L4(g,PERIOD,PERIOD,PERIOD)
	bl	TransmitR
	mov	r0,rRamSector
	mov	r1,rFlashSector
	mov	pc,r5

PrintSuccess:
	bl	TransmitR_CRLF
	BigMov	r0,L4(S,u,c,c)
	bl	TransmitR
	BigMov	r0,L3(e,s,s)
	bl	TransmitR
//	b	Burn_return

Burn_return:
	bl	TransmitR_CRLF
	bl	WaitTxEmpty
	movs	rRet,rRet
	CP15_CF_INVAL_BOTH mcreq,r1	//invalidate if return is reset vector
	CPWAIT	r1
	mov	pc,rRet

Burn_error:
	mov	r8,lr
	mov	r5,r1
	mov	sl,r2
	mov	r7,r3
	bl	PrintHexR
	mov	r0,#L1(COLON)
	bl	TransmitR
	mov	r0,sl
	bl	PrintHexR
	bl	TransmitSPACER

	mov	r0,r5
	bl	PrintHexR
	mov	r0,#L1(COLON)
	bl	TransmitR
	mov	r0,r7
	mov	lr,r8
	b	PrintHexR

	DEFINE_FLASH_GPIO_WAIT_FOR_IDLE


WaitTxEmpty:
	WaitUartTxEmpty rUart,r1
	mov	pc,lr			//return

TransmitSPACER:
	mov	r0,#L1(SPACE)
	b	TransmitR
TransmitR_CRLF:
	BigMov	r1,L2(CR,LF)
trR:
	mov	r0,r1
//IN: r0 - character to transmit
//OUT: r0 - last character transmitted, r1 - trashed
TransmitR:
	TransmitChar r0,rUart,r1
	movs	r1,r0,LSR #8
	bne	trR
	mov	pc,lr			//return
//IN: r0 - value to print
//OUT: r0-r4 trashed
PrintHexR:
	mov	r2,#8
	mov	r4,r0
	mov	r3,lr
1:	mov	r4,r4,ROR #28
	and	r0,r4,#0xf
	cmp	r0,#0xA
	addcs	r0,r0,#L1(A)-0x0a
	addcc	r0,r0,#L1(0)
	bl	TransmitR
	subs	r2,r2,#1
	bne	1b
	mov	pc,r3

Burn_Rtn_End:


SSID_Cmd:
	BigMov	r0,L4(I,d,COLON,SPACE)
	bl	Transmit
	bl	CalcMemEnd
	BigAdd	r5,r0,(-0x1000+((DEBUG_BASE+DBG_START)&0xfff)-32)
	mov	r6,#31
1:	bl	Receive
	beq	1b			//br on timeout
	cmp	r0,#CR
	beq	2f
	strb	r0,[r5],#1
	bl	Transmit
	subs	r6,r6,#1
	bne	1b
2:	mov	r0,#0
	strb	r0,[r5],#1
	b	Prompt

MAC_Cmd:
	mov	r2,#FUNC_REQ_MAC
	b	join_cmd
Download_Wireless_Cmd:
	tst	rValidCnt,#F_NUM1_MASK
	beq	Invalid
	tst	rNum1,#3
	bne	Invalid
	cmp	rNum1,#MEM_START
	bcc	Invalid			//br if rNum1 is below ram start,
					//NOTE: arm sets the carry to the opposite of most processors on subtract, compare
	mov	r2,#FUNC_REQ_DLW
join_cmd:
	bl	TryRoutine
	ldmia	sp,{rPrevNum1,rUart,rDBG}
	bl	SetupATAG		//dlw is now overwriting ATAG space, so set it again
	b	Prompt

//struct tag_mem32 {
//	u32	size;
//	u32	start;	/* physical start address */
//};

//out: r2,r7 - saved, r0 - CalcMemEnd return value
SetupATAG:
	mov	r3,lr
	bl	CalcMemEnd
	mov	lr,r3
	BigMov	r1,TAGGED_LIST
	BigMov	r3,2
	BigMov	r4,ATAG_CORE
	BigMov	r5,4
	BigEor	r6,r4,(ATAG_MEM^ATAG_CORE)
	stmia	r1!,{r3,r4,r5,r6}
	BigMov	r4,MEM_START
	BigMov	r5,0
	sub	r3,r0,r4
	stmia	r1,{r3,r4,r5}
	mov	pc,lr


Gl_Cmd:
	mov	r2,#FUNC_REQ_PIC
	bl	TryRoutine
	ldmia	sp,{rPrevNum1,rUart,rDBG}

	BigMov	r1,MACH_TYPE_SCANPASS
	str	r1,[rDBG,#DBG_R1]
	mov	r2,#FUNC_REQ_GL
	bl	TryRoutine
	ldmia	sp,{rPrevNum1,rUart,rDBG}
	movs	rNum1,r4
	bne	Go_Cmd
	b	Prompt

//r2- function select
TryRoutine:
	mov	r10,#0
	mov	r7,lr
	bl	TryRoutine1
	b	Invalid
//r10- RCSR
//r2- function select
//r7- main return
//lr- error return
TryRoutine1:
	adr	r0,MiniDebugEnd
	ldr	r1,[r0]
	BigMov	r3,0xe1a08000
	cmp	r1,r3
	movne	pc,lr	//failure return
	bl	SetupATAG	//out: r0 - calcMemEnd return value
	mov	lr,r7

	BigAdd	r1,r0,(-0x1000+((DEBUG_BASE+DBG_START)&0xfff)-32)	//@ssid
	mov	r0,r1
	stmdb	r0!,{rPrevNum1,rUart,rDBG}
	mov	sp,r0
	mov	fp,r0
	mov	r0,rNum1
	adr	r4,MiniDebugEnd
	mov	r3,#DEF_DISPLAY_INDEX
	b	HeadStart

	.ifdef __ARMASM
	DUP 6,0xffffffff
	align 256
	.else
	.balignl	256,0xffffffff
	.endif

MiniDebugEnd:
//This is defined in head.S or minidebug.lds
//HeadStart:
