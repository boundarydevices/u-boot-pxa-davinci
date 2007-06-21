#include "platformTypes.h"
#define SUB_LR_FALL_THRU_FOR_FIQ 1		//0 for branch
#define LITTLE_ENDIAN 1		//describes memory system
#define DDEBUG 0
#define DO_GPTEST 0

	.equiv	USE_SUPERVISOR,0
	.equiv	USE_HCPS,0

	.equiv	MACH_TYPE_SCANPASS,332
	.equiv	ATAG_CORE,	0x54410001
	.equiv	ATAG_MEM,	0x54410002
#if (SOFTWARE_TYPE==WINCE)
//0x80000000 is cached mapped, 0xa0000000 is uncacheable
	.equiv	PHYS_TO_VIRT, (0xA0000000-MEM_START)
#else
	.equiv	PHYS_TO_VIRT, (0xC0000000-MEM_START)
#endif

#define V_rWork r2
#define V_rBranch r3
#define I_rWork r2
#define I_rBranch sp	//r13

#ifdef CONFIG_STACKS_VALID
#define rWork V_rWork
#define rBranch V_rBranch

#else
#define rWork I_rWork
#define rBranch I_rBranch
#endif

//	.equiv	BAUDRATE, 9600
//	.equiv	BAUDRATE, 38400
//	.equiv	BAUDRATE, 57600
	.equiv	BAUDRATE, 115200
//	.equiv	BAUDRATE, 230400

#if (CPU_CLOCK==100)
	.equiv	COUNT_MULT, 1
#else
#if (CPU_CLOCK==200)
	.equiv	COUNT_MULT, 2
#else
#if (CPU_CLOCK==300)
	.equiv	COUNT_MULT, 3
#else
	.equiv	COUNT_MULT, 4
#endif
#endif
#endif

#if 1		//(SOFTWARE_TYPE==WINCE)
	.equiv	RECEIVE_LOOP_COUNT, 0x10000*COUNT_MULT
#else
#if (PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)||(PLATFORM_TYPE==GAME_CONTROLLER)||(PLATFORM_TYPE==GAME_WITH_SMC)
	.equiv	RECEIVE_LOOP_COUNT, 0x100000*COUNT_MULT
#else
	.equiv	RECEIVE_LOOP_COUNT, 0x300000*3*COUNT_MULT
#endif
#endif

// **********************************************************
	.equiv	DBG_MAGIC,95 //this allows BigMov sp,DEBUG_BASE+DBG_MAGIC to generate just 1 instruction
//	.equiv	DBG_START, -95
	.equiv	DBG_START,0
	.equiv	DBG_R0,((0<<2)+DBG_START)
	.equiv	DBG_R1,((1<<2)+DBG_START)
	.equiv	DBG_R2,((2<<2)+DBG_START)
	.equiv	DBG_R3,((3<<2)+DBG_START)
	.equiv	DBG_R4,((4<<2)+DBG_START)
	.equiv	DBG_R5,((5<<2)+DBG_START)
	.equiv	DBG_R6,((6<<2)+DBG_START)
	.equiv	DBG_R7,((7<<2)+DBG_START)
	.equiv	DBG_R8,((8<<2)+DBG_START)
	.equiv	DBG_R9,((9<<2)+DBG_START)
	.equiv	DBG_SL,((10<<2)+DBG_START)
	.equiv	DBG_FP,((11<<2)+DBG_START)
	.equiv	DBG_IP,((12<<2)+DBG_START)
	.equiv	DBG_SP,((13<<2)+DBG_START)
	.equiv	DBG_LR,((14<<2)+DBG_START)
	.equiv	DBG_PC,((15<<2)+DBG_START)
	.equiv	DBG_CPSR,	((16<<2)+DBG_START)	//this and above have corresponding symbol #s

	.equiv	DBG_HCPSR,	((17<<2)+DBG_START)	//interrupt handler original CPSR

	.equiv	DBG_TRACE,	((18<<2)+DBG_START)
	.equiv	DBG_LastSignal,	((19<<2)+DBG_START)		//only 1 byte
	.equiv	DBG_Mode,	(((19<<2)+1)+DBG_START)	//only 1 byte, bit 0 -1 means gdb mode for control breaks
	.equiv	DBG_FFUART_LCR,	(((19<<2)+2)+DBG_START) //only 1 byte
	.equiv	DBG_HSP,	((20<<2)+DBG_START)		//handler stack pointer on entry, for aborts
	.equiv	DBG_TEMP,	((21<<2)+DBG_START)
//22 free
	.equiv	DBG_ABORT_PC,	((23<<2)+DBG_START)
	.equiv	DBG_INDIRECT_R0, ((24<<2)+DBG_START)	//it needs it's own space in case a debug interrupt
	.equiv	DBG_INDIRECT_LR, ((25<<2)+DBG_START)	//happens in the return code, but memory is mapped if used

	.if USE_SUPERVISOR
	.equiv	DBG_SUPERVISOR_SP, ((26<<2)+DBG_START)
	.equiv	DBG_SUPERVISOR_LR, ((27<<2)+DBG_START)
	.endif

	.equiv	DEBUG_SPACE, (((23<<2)|0x1f)+1)			//a multiple of 32 bytes

// *********************************************************

	.equiv	DEBUG_START, (VMA_DEBUG+0x3000-DEBUG_SPACE)	//this saves on memory, but the else is easier to debug
	.equiv	VMA_DEBUG_OFFSET, (0xffff0000-VMA_DEBUG)
	.equiv	DEBUG_BASE, (DEBUG_START-DBG_START)
	.equiv	DEBUG_SYM, DEBUG_START

	.equiv	SYM_R0,	0
	.equiv	SYM_R1,	1
	.equiv	SYM_R2,	2
	.equiv	SYM_R3,	3
	.equiv	SYM_R4,	4
	.equiv	SYM_R5,	5
	.equiv	SYM_R6,	6
	.equiv	SYM_R7,	7
	.equiv	SYM_R8,	8
	.equiv	SYM_R9,	9
	.equiv	SYM_SL,	10
	.equiv	SYM_FP,	11
	.equiv	SYM_IP,	12
	.equiv	SYM_SP,	13
	.equiv	SYM_LR,	14
	.equiv	SYM_PC,	15
	.equiv	SYM_CPSR,16

	.if	USE_HCPS
	.equiv	SYM_HCPS,17
	.equiv	NN,18
	.else
	.equiv	NN,17
	.endif

//	.equiv	SYM_SPSR,NN	//nice to have, but do it later
//	.equiv	SYM_FPS,NN
	.equiv	SYM_FP0,(NN)		//40 bits, 5

	.ifdef CONFIG_ARM1136
	.equiv	SYM_TTB0,(NN+1)
	.equiv	SYM_TTB1,(NN+2)
	.equiv	SYM_TTBC,(NN+3)
	.equiv	NN2,(NN+4)
	.else
	.equiv	SYM_TTBR,(NN+1)
	.equiv	NN2,(NN+2)
	.endif

	.equiv	SYM_FSR,(NN2)
	.equiv	SYM_FAR,(NN2+1)
	.equiv	SYM_DCSR,(NN2+2)
	.equiv	SYM_CTRL,(NN2+3)

	.equiv	SYM_LAST,(NN2+3)

	.equiv	SYM_FIRST_SPECIAL,SYM_FP0	//1st symbol not in memory
	.equiv	SYM_LAST_RCMD,(SYM_CPSR)	//last symbol printed by R cmd


	.equiv	SIG_RESET,			0
	.equiv	SIG_UNDEFINED_INSTRUCTION,	1
	.equiv	SIG_SWI,			2
	.equiv	SIG_PREFETCH_ABORT,		3
	.equiv	SIG_DATA_ABORT,			4
	.equiv	SIG_UNUSED,			5
	.equiv	SIG_IRQ,			6
	.equiv	SIG_FIQ,			7
	.equiv	SIG_DBG,			8
	.equiv	SIG_DBG_RESET,			8+0
	.equiv	SIG_DBG_INSTRUCTION_BKPT,	8+1
	.equiv	SIG_DBG_DATA_BKPT,		8+2
	.equiv	SIG_DBG_BKPT_SOFTWARE,		8+3
	.equiv	SIG_DBG_EXTERNAL,		8+4
	.equiv	SIG_DBG_VECTOR_TRAP,		8+5
	.equiv	SIG_DBG_TRACE_BUFFER_FULL,	8+6
	.equiv	SIG_DBG_RESERVED,		8+7

	.equiv	GDB_EXIT_CHAR, 0x0d
// ******************************************************************
#define rFieldStart r4
#define rField	    r5
#define rValidCnt   r6
#define rCommand    r7
#define rNum1	    r8
#define rNum2	    r9
#define rSymbol     sl	//r10
#define rPrevNum1   fp	//r11
#define rUart       ip	//r12
#define rDBG        sp

//in GDB routines
#define rGdbCmd		r5
#define rGdbNum1	r6
#define rGdbTermChar1	r7
#define rGdbNum2	r8
#define rGdbTermChar2	r9
#define rGdbNum3 	sl	//r10
#define rGdbChkSum	fp	//r11

#define rSP r8			//register which contains L4(' ',' ',' ',' '), all spaces
// ********

	.equiv	CR, 0x0d
	.equiv	LF, 0x0a

// ******************************
	.equiv	F_INC, 4
	.equiv	F_NUM1_BIT, (F_INC*3)
	.equiv	F_NUM2_BIT, (F_INC*5)

	.equiv	F_COMMAND, 1<<(F_INC)
	.equiv	F_NUM1,	  1<<(F_NUM1_BIT)
	.equiv	F_NUM2,	  1<<(F_NUM2_BIT)
	.equiv	F_UNDEF,  1<<(F_INC*7)

	.equiv	F_COMMAND_MASK,	((1<<F_INC)-1)<<(F_INC)
	.equiv	F_NUM1_MASK,	((1<<F_INC)-1)<<(F_INC*3)
	.equiv	F_NUM2_MASK,	((1<<F_INC)-1)<<(F_INC*5)
// ******************************
//	.equiv	baseaddress, 0		//physical memory does not have to exist here, but a valid descriptor table entry IS required

// *****************************

// divide 0x1000 bytes among the stacks
	.equiv	SS_SUPERVISOR,	0x0800
	.equiv	SS_IRQ,		0x0400
	.equiv	SS_FIQ,		0x0100
	.equiv	SS_SYSTEM,	0x0100
	.equiv	SS_UNDEFINED,	0x0100
	.equiv	SS_ABORT,	0x0100

	.equiv	SS_TOTAL, (SS_SUPERVISOR+SS_IRQ+SS_FIQ+SS_SYSTEM+SS_UNDEFINED+SS_ABORT)
	.equiv	SS_MEM_OFFSET, (0x8000-0x4400-SS_TOTAL)	//-17k, 16k for 1st level page table, 1k for 2nd level page table
// *****************************

.macro mac_AfterPCPrint branch		//vector table at 0xffff0000, br to vector table at VMA_DEBUG
	.ifdef __ARMASM
	LCLA cnt
	.set	cnt,$branch
	WHILE cnt>0
		b	93f
		.set	cnt,cnt-1
	WEND
	.else
	.rept (\branch)
	b	93f
	.endr
	.endif

AfterPCPrint:
	mov		r0,#0x55
	b		AfterPCPrint1
InitializeCont:
	b		MainInitializationCode
93:
.endm


.macro RelocationVector branch		//vector table at 0xffff0000, br to vector table at VMA_DEBUG
					//warning, do not use relocated vectors unless memory management is enabled and VMA_DEBUG is mapped
	.ifdef __ARMASM
	LCLA cnt
	.set	cnt,$branch
	WHILE cnt>0
		b	71f
		.set	cnt,cnt-1
	WEND
	.else
	.rept (\branch)
	b	71f
	.endr
	.endif

	mov	pc,#0x00		//0    - reset always goes to 0 because it will be in physical memory mode for instructions
	b	.-VMA_DEBUG_OFFSET	//4    - UndefinedInstr
	b	.-VMA_DEBUG_OFFSET	//8    - SWI
	b	.-VMA_DEBUG_OFFSET	//0x0c - PrefetchAbort
	b	.-VMA_DEBUG_OFFSET	//0x10 - DataAbort
	b	.-VMA_DEBUG_OFFSET	//0x14 - Unused
	b	.-VMA_DEBUG_OFFSET	//0x18 - IRQ
	.if SUB_LR_FALL_THRU_FOR_FIQ
	sub	lr,lr,#4		//0x1c - FIQ
	.else
	b	.-VMA_DEBUG_OFFSET	//0x1c - FIQ
	.endif

	.ifdef __ARMASM
	align 32
	.else
	.balignl        32,0xffffffff
	.endif
//these instructions are always at this address
//to minimize the effect of a mismatch of minicache and flash, also the invalidate does not affect minicache
	SPS_ReturnBugsWorkAroundCode

//c-0 invalidate caches, c-1 skip cache invalidate
InvalidateAndReturn:
//Invalidate the data/instruction cache and branch target buffer
	CP15_CF_INVAL_BOTH mcrcc,r1
	CPWAIT	r1
	ldr	sp,[r0,#DBG_HSP-DBG_R2]
	ldr	r1,[r0,#DBG_R1-DBG_R2]
	str	r0,[r0,#DBG_ABORT_PC-DBG_R2]	//reset error flag so abort can be retried
	ldr	r0,[r0,#DBG_R0-DBG_R2]
	movs	pc,lr
71:
.endm

.macro  CheckBranch rTemp,rAddr
	movs	\rTemp,pc		//don't redirect if running from flash
	submi	\rAddr, \rAddr, #0x00010000
	ldrmi	\rTemp, [\rAddr,#-8]	//load high vector
	eor		\rTemp, \rTemp, #0xea000000	//unconditionally done (ensures the CheckLdr fails if in flash)
	tstmi	\rTemp, #0xff000000
	moveq	\rTemp, \rTemp, LSL #8
	addeq	\rAddr, \rAddr, \rTemp, ASR #6
.endm
.macro  CheckLdr rTemp,rAddr		//check for instruction LDR pc,[pc,#nnn]
	eor	\rTemp, \rTemp, #0x0f900000
	eor	\rTemp, \rTemp, #0x000ff000
	cmp	\rTemp, #0x1000
	ldrcc	\rAddr,[\rAddr,\rTemp]
.endm


//this is case where stacks are assumed valid
.macro  V_VectorEntrance Work,Branch,code
	stmdb	sp!,{\Work,\Branch,lr}
	mov	\Branch,#(\code<<2)+8
.endm
.macro  V_VectorExitCC Work,Branch,cc,ccia,ncc
	str\cc	\Branch,[sp,#8]
	ldm\ccia	sp!,{\Work,\Branch,pc}
.endm
.macro  V_VectorExitCC1 Work,Branch,cc,ccia,ncc
	str\cc	\Branch,[sp,#8]
	ldm\ccia	sp!,{\Work,\Branch,pc}
.endm

//this is case where stacks are assumed invalid
.macro  I_VectorEntrance Work,Branch,code
	GetDebugMagic 	\Branch
	str	\Work,[\Branch,#DBG_TEMP-DBG_MAGIC]
	mov	\Branch, #(\code<<2)+8
.endm
.macro  I_VectorExitCC Work,Branch,cc,ccia,ncc
	GetDebugMagicCC 	\cc,\ncc,\Work
	ldr\cc	\Work,[\Work,#DBG_TEMP-DBG_MAGIC]
	bx\cc	\Branch
.endm
.macro  I_VectorExitCC1 Work,Branch,cc,ccia,ncc
	GetDebugMagic 	\Work
	ldr\cc	\Work,[\Work,#DBG_TEMP-DBG_MAGIC]
	bx\cc	\Branch
.endm
.macro	VectorEntrance Work,Branch,code
	.if STACKS_VALID
	V_VectorEntrance \Work,\Branch,\code
	.else
	I_VectorEntrance \Work,\Branch,\code
	.endif
.endm
.macro	JOIN brcc
	.if STACKS_VALID
	\brcc	join_fiq
	.else
	\brcc	join_irq
	.endif
.endm
.macro JOIN2 brcc
	.if STACKS_VALID
	\brcc	join_fiq2
	.else
	\brcc	join_irq2
	.endif
.endm
// *****************************************************

.macro SaveRegisters rBase,rTemp
	GetDebugMagicPhys \rBase,\rTemp
	mov	\rTemp,#0
//great, now memory should be working, let's save registers, only r0(rBase),sp(rTemp) have been lost
//don't trust LDM,STM instructions in debug mode....
//or LDR w/Rd=PC, LDR w/RRX addressing mode, SWP, LDC, STC
// *******************************************************
// *******************************************************
	str	\rTemp,[\rBase,#DBG_START-32  -DBG_MAGIC]
	str	\rTemp,[\rBase,#DBG_R0        -DBG_MAGIC]
	str	r1,    [\rBase,#DBG_R1        -DBG_MAGIC]
	str	r2,    [\rBase,#DBG_R2        -DBG_MAGIC]
	str	r3,    [\rBase,#DBG_R3        -DBG_MAGIC]
	str	\rTemp,[\rBase,#DBG_TRACE     -DBG_MAGIC]
	str	\rTemp,[\rBase,#DBG_LastSignal-DBG_MAGIC]
.endm
.macro	ReadHexE dest,rCnt1

#if LITTLE_ENDIAN
	mov	\rCnt1,#4
	mov	\dest,#0
1:	bl	ReadHex
	bcc	CheckSumError
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	mov	r1,r1,LSL #28
	add	\dest,r1,\dest,LSR #8

	bl	ReadHex
	bcc	CheckSumError
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	add	\dest,\dest,r1,LSL #24
#else
	mov	\rCnt1,#8
	mov	\dest,#0
1:	bl	ReadHex
	bcc	CheckSumError
	add	rGdbChkSum,rGdbChkSum,r0		//update checksum
	add	\dest,r1,\dest,LSL #4
#endif
	subs	\rCnt1,\rCnt1,#1
	bne	1b
.endm



