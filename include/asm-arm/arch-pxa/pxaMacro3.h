#include "pxaGpio.h"
#include "pxaMacro2.h"
	.equiv	WORKAROUND_FLAGS_BUG,1	//work around bug where flags(NZCV) aren't restored when returning to user mode

	.ifdef __ARMASM
	GBLA	TAGGED_LIST
	.endif
	.set	TAGGED_LIST,0xa0000100	//physical address of start of list

	.ifdef __ARMASM
	GBLA STACKS_VALID
	GBLA CONFIG_STACKS_VALID
	.set	CONFIG_STACKS_VALID,1
	.endif

#if (SOFTWARE_TYPE==WINCE)
	.set	STACKS_VALID,1
	.ifdef __ARMASM
	.else
#ifndef CONFIG_STACKS_VALID
#define CONFIG_STACKS_VALID 1
#endif
	.endif

	.equiv	SDRAM_BASE_C_VIRTUAL, 0xA0000000		//0x80000000 is cached mapped, 0xa0000000 is uncacheable
	.equiv	UART_VIRT_BASE, 0xAA100000
	.equiv	VMA_DEBUG, (0xfff00000)
	.equiv	VIRTUAL_CS0, 0xa8000000
	.equiv	VIRTUAL_CS1, 0xa8000000
	.equiv	DL_RANGE2_BASE, 0x04000000	//physical CS1
	.equiv	DL_RANGE2_END,  0x18000000	//end of physical CS5 space

#else
#ifdef CONFIG_STACKS_VALID
	.set	STACKS_VALID,1
#else
	.set	STACKS_VALID,0
#endif
	.equiv	SDRAM_BASE_C_VIRTUAL, 0xC0000000
	.equiv	UART_VIRT_BASE, 0xf8100000
//	.equiv	VMA_DEBUG, (0xff000000)
//!!!!!for some reason the above base causes bizarre problems
	.equiv	VMA_DEBUG, (0xfff00000)
	.equiv	VIRTUAL_CS0, 0xff000000
	.equiv	VIRTUAL_CS1, 0xff000000		//changed in 2.6.16 0xff100000
#endif

.macro EmulateJmpFIQ rWork,rBranch
	CheckBranch \rWork,\rBranch
	V_VectorExitCC \rWork,\rBranch,eq,eqia,ne
	CheckLdr  \rWork,\rBranch
	V_VectorExitCC1 \rWork,\rBranch,cc,ccia,cs
.endm
.macro EmulateJmpIRQ rWork,rBranch
	CheckBranch \rWork,\rBranch
	I_VectorExitCC \rWork,\rBranch,eq,eqia,ne
	CheckLdr  \rWork,\rBranch
	I_VectorExitCC1 \rWork,\rBranch,cc,ccia,cs
.endm

//Out: Z-1 breakpoint
.macro ResetVectorEntry MainInitCode
//this is either a normal processor reset, or a debug exception in halt mode
	mrs	sp,cpsr
	and	sp,sp,#0x1f
	cmp	sp,#0x15
	bne	2f
//this is a debug exception in halt mode
	CP14_DCSR mrc,sp
	and	sp,sp,#0x1c
	cmp	sp,#0		//reset
	cmpne	sp,#5<<2	//vector trap (reset vector)
	bne	10f		//initialization already done

	BigMov	sp,PWR_MANAGER_BASE
	ldr	sp,[sp,#RCSR]
	tst	sp,#2
	tsteq	pc,#0xff000000
	bne	2f		//br if watchdog reset or if not nCS0 address

//signal debugger that he can download into main instruction cache now
	CP14_TX mcr,sp
1:
	CP14_TXRXCTRL mrc,r15	//r15 means update condition codes
	bpl	1b
	CP14_RX mrc,sp		//just read and discard
2:
	CP14_DCSR mrc,sp
	orr	sp,sp,#1<<31		//set global debug enable bit
	CP14_DCSR mcr,sp

//	InitGPIO r0,sp
//	InitIC_Clocks r0,sp
//	InitUART r0,sp,UART_BASE,BAUDRATE
//	TransMacro L1(U)
	b	\MainInitCode	//this can be in the main instruction cache not locked
					//because it is only executed upon reset

10:
	cmp	sp,#1<<2		//Instruction breakpoint
.endm


.macro HandleRestoreBP rRestoreBP
	cmn	\rRestoreBP,#1
	moveq	\rRestoreBP,#0
	CP15_IBCR1 mrc,sp
	sub	r2,lr,#3
	cmp	sp,r2
	CP15_IBCR1 mcreq,\rRestoreBP		//remove breakpoint if single-stepping
	beq	99f
	CP15_IBCR0 mrc,sp
	cmp	sp,r2
	CP15_IBCR0 mcreq,\rRestoreBP
99:
.endm


//Out: r1 - Method of entry
.macro GetMethodOfEntry
	CP14_DCSR mrc,sp
//	str	sp,[r0,#DBG_Temp]	//the mcr CP14_DCSR instruction below will trash sp in SDS (special debug state)
	and	r1,sp,#0x1c
	orr	sp,sp,#0x1c
	CP14_DCSR mcr,sp		//change MOE to reserved
	mov	r1,r1,LSR #2
.endm

//Out: Z-1 yes
.macro IsInstructionBreakpoint
	CP14_DCSR mrc,sp
	and	sp,sp,#0x1c
	cmp	sp,#1<<2		//Instruction breakpoint
.endm

.macro GetIfEq_DCSR
	CP14_DCSR mrceq,r0
.endm

.macro StoreIfEq_DCSR
	CP14_DCSR mcreq,r0
.endm

//Out: z-0 abort happened
.macro ClearDCSR_ABORT rTmp
	CP14_DCSR mrc,\rTmp
	tst	\rTmp,#1<<5
	bic	\rTmp,\rTmp,#1<<5		//clear sticky abort bit
	CP14_DCSR mcrne,\rTmp
.endm

.macro EnableCoprocessorAccess rTmp
	BigMov	\rTmp,0x3fff
	CP15_CP_ACCESS mcr,\rTmp		//enable access to all coprocessors
.endm

.macro TransmitChar rChar,rUART,rTmp
111:
	ldr		\rTmp,[\rUART,#UART_LSR]
	ands	\rTmp,\rTmp,#0x20
	beq		111b
	and		\rTmp,\rChar,#0xff
	str		\rTmp,[\rUART,#UART_THR]
.endm

.macro WaitUartTxEmpty rUART,rTmp
111:
	ldr		\rTmp,[\rUART,#UART_LSR]
	ands	\rTmp,\rTmp,#0x40
	beq		111b
.endm

//Out: z-0 if good char
.macro ReceiveChar rChar,rUART
	ldr		\rChar,[\rUART,#UART_LSR]
	ands	\rChar,\rChar,#0x1
	ldrne	\rChar,[\rUART,#UART_RBR]
.endm

.macro ClearUart_DLAB rUART,rtDBG,rTmp
	ldr		\rTmp,[\rUART,#UART_LCR]
	strb	\rTmp,[\rtDBG,#DBG_FFUART_LCR]
	bic		\rTmp,\rTmp,#0x80			//make sure DLAB is clear
	str		\rTmp,[\rUART,#UART_LCR]
.endm

.macro RestoreUart_DLAB rUART,rtDBG,rTmp
	ldrb	\rTmp,[\rtDBG,#DBG_FFUART_LCR]
	str		\rTmp,[\rUART,#UART_LCR]	//restore value
.endm

.macro Get_FP0_reg	rLow,rHigh
//MRA{<cond>} <RdLo>,<RdHi>,acc0
	mra	\rLow,\rHigh,acc0
.endm

.macro	Set_FP0_reg rLow,rHigh
//MAR{<cond>} acc0,<RdLo>,<RdHi>
	mar	acc0,\rLow,\rHigh
.endm


.macro	EnableFlashReadMode rTemp0,rTemp1,rUART
	rsbs	\rTemp0,pc,#0x800
	BigMov	\rTemp0,FLASH_READ_CMD

#if (PLATFORM_TYPE==NEONB)
	BigMov	\rTemp1,VIRTUAL_CS1	//remember, although instruction fetches are physical, data are still virtual
#else
	BigMov	\rTemp1,VIRTUAL_CS0	//remember, although instruction fetches are physical, data are still virtual
#endif
	cmphs	\rUART,#0x41000000
	strhs	\rTemp0,[\rTemp1]		//if (pc is in minicache) & (rUart is in virtual memory) then
				//make sure flash is in read mode, we are almost off the minicache
.endm

//Out: z-0 means 16 bit width
.macro GetFlashWidth rFlash,rTmp
	BigMov	\rTmp,MEMORY_CONTROL_BASE
	tst		\rFlash,#1<<26
	ldr		\rTmp,[\rTmp,#MSC0]
	movne	\rTmp,\rTmp,LSR #16	//nCS1 being used for flash
	tst		\rTmp,#1<<3		//bit 3 - 1 means 16 bit mode
.endm


.macro ResetTLB rTmp
	CP15_TLB_UNLOCK_I mcr,\rTmp
	CP15_TLB_UNLOCK_D mcr,\rTmp
	CP15_TLB_INVAL_BOTH mcr,\rTmp
.endm

.macro ResetInstructionCache rTmp
	CP15_CF_UNLOCK_I mcr,\rTmp
//	CP15_CF_INVAL_I mcr,\rTmp		//I might be running from cache only, don't invalidate
.endm

.macro ResetDataCache rTmp
	CP15_CF_UNLOCK_D mcr,\rTmp
	CP15_CF_INVAL_D mcr,\rTmp		//this will also drain write buffer
.endm

.macro Lock_TLB_DENTRY rEntry,rTmp
	CP15_TLB_LOCK_DENTRY mcr,\rEntry
.endm

.macro Lock_Data_Cache_Range	rStart,rCacheLines,rTmp
	mov	\rTmp,#1
	CP15_CF_LOCK_D_CSR mcr,\rTmp		//data cache is into lock mode
	CPWAIT	\rTmp
115:
	pld	[\rStart]
	add	\rStart,\rStart,#32			//lock and load
	CP15_CF_DRAIN mcr,\rTmp
	subs	\rCacheLines,\rCacheLines,#1
	bne	115b

	mov	\rTmp,#0
	CP15_CF_LOCK_D_CSR mcr,\rTmp		//data cache is out of lock mode
	CPWAIT	\rTmp
.endm

	.equiv	cachelinecount, 2048		//1024 if baseaddress not used for another purpose
	.equiv	cachelinesize, 32

.macro	CleanInvalidateDataCache	rVaddr,rTmp1
// Set baseaddress as the 1st virtual address of a 32k range used only
// to flush the data cache. baseaddress should be aligned on a 32 byte(cache-line) boundary.
// If the virtual range beginning with baseaddress is used for any purpos other than flushing
// the data cache, then cachelinecount must be doubled from 1024  to 2048
	mov	\rTmp1,#cachelinecount
1:
	CP15_CF_ALLOC_LINE mcr,\rVaddr	//allocate a cache line for rVaddr
	add	\rVaddr,\rVaddr,#cachelinesize
	subs	\rTmp1,\rTmp1,#1
	bne	1b

//	teq	r2, #0			// if running only from cache, an invalidate could be fatal if not done from minicache
						// and then, a return to caller would not be allowed, so invalidate is postponed
//	CP15_CF_INVAL_I mcrne,\rTmp0
	CP15_CF_DRAIN mcr,\rTmp1
	CP15_CF_INVAL_D mcr,\rTmp1
// The instruction cache is guaranteed to be invalidated at this point. The
// next instruction sees the result of the invalidate command.
.endm

.macro	CleanInvalidateDataCache1	rTmp0,rTmp1,rDbg
	ldr	\rTmp0,[\rDbg,#DBG_PC]	//I hope my return address is mapped
	BigBic2	\rTmp0,0xffff
	CleanInvalidateDataCache	\rTmp0,\rTmp1
.endm

//No L2 cache
.macro	DisableL2Cache	rBase,rVal
.endm

//rTmp is trashed
.macro	InvalidateICacheRange rStart,rEnd,rTmp
	bic	\rTmp,\rStart,#0x1f
91:
	CP15_CF_INVAL_ILINE	mcr,\rTmp
	add	\rTmp,\rTmp,#32
	cmp	\rTmp,\rEnd
	blo	91b
	CP15_CF_INVAL_BTB	mcr,\rTmp
.endm

//Out: z-1 : at breakpoint, rTmp1 bkpt enable value
.macro CheckForAndRemoveBkpt rPc,rTmp1,rTmp2,rTmp3,rZero
	orr	\rTmp1,\rPc,#1
	CP15_IBCR0 mrc,\rTmp2
	CP15_IBCR1 mrc,\rTmp3
	mov	\rZero,#0
	cmp	\rTmp1,\rTmp2
	CP15_IBCR0 mcreq,\rZero
	cmp	\rTmp1,\rTmp3
	CP15_IBCR1 mcreq,\rZero
	cmpne	\rTmp1,\rTmp2
.endm

.macro AddBkpt	rPc,rTmp1,rTmp2,rTmp3,rDbg,trace
	orr	\rTmp1,\rPc,#1
	CP15_IBCR0 mrc,\rTmp2
	tst	\rTmp2,#1
	CP15_IBCR0 mcreq,\rTmp1
	beq	99f
	cmp	\rTmp1,\rTmp2
	streq	\rTmp2,[\rDbg,#\trace]	//this breakpoint needs to remain after trace
	beq	99f

	CP15_IBCR1 mrc,\rTmp2
	tst	\rTmp2,#1
	CP15_IBCR1 mcr,\rTmp1
	strne	\rTmp2,[\rDbg,#\trace]	//need to steal this breakpoint momentarily
99:
.endm

.macro	SearchBkpt rPc,rTmp0,rTmp1
	CP15_IBCR0 mrc,\rTmp0
	CP15_IBCR1 mrc,\rTmp1
	orr	\rPc,\rPc,#1
	cmp	\rTmp0,\rPc
	cmpne	\rTmp1,\rPc
.endm

//Out: z-1 breakpoint just set
.macro	SetBkpt	rPc,rTmp0,rTmp1,rTmp2
	tst	\rTmp0,#1
	CP15_IBCR0 mcreq,\rPc
	beq	99f
	tst	\rTmp1,#1
	CP15_IBCR1 mcreq,\rPc
99:
.endm

.macro	PrintBkpts
	CP15_IBCR0 mrc,r0
	tst	r0,#1
	bic	r0,r0,#1
	blne	PrintHex
	bl	TransmitSPACE

	CP15_IBCR1 mrc,r0
	tst	r0,#1
	bic	r0,r0,#1
	blne	PrintHex
.endm

.macro PrintWatchpts rwText,wText,rText
	CP15_DBCON mrc,r5
	tst	r5,#0x0f
	beq	99f
	tst	r5,#0x03
	bne	96f
	tst	r5,#0x100
	bne	99f		//br if dbr1 is mask (nothing being watched)
	b	98f

//this is a subroutine
90:
	BigMov	r0,\rwText
	tst	r5,#1
	beq	91f
	tst	r5,#2
	BigEor2Eq r0,\rwText^\wText	//store only
	BigEor2Ne r0,\rwText^\rText	//load only
91:
	b	Transmit
/////////////////////

96:
	CP15_DBR0 mrc,r0
	bl	PrintHex
	bl	90f
	tst	r5,#0x100
	beq	97f
	BigMov	r0,(CH_M+(CH_COLON<<8))
	bl	Transmit
	CP15_DBR1 mrc,r0	//read mask
	bl	PrintHex
	b	99f

97:
	tst	r5,#3<<2
	beq	99f
98:
	CP15_DBR1 mrc,r0
	bl	PrintHex
	mov	r5,r5,LSR #2
	bl	90f
99:
.endm


.macro	WatchClear rTmp
	mov	\rTmp,#0
	CP15_DBCON mcr,\rTmp
.endm


//In: z-0 means rNum2 is valid
//rMode - 1:w, 2:rw, 3:r
.macro	SetWatchpt	rNum1,rNum2,rMode,rZero,rCtl
	mov	\rZero,#0
	beq	3f
	orr	\rMode,\rMode,#0x100	//mask
1:
	CP15_DBCON mcr,\rZero
	CP15_DBR0 mcr,\rNum1
	CP15_DBR1 mcr,\rNum2
	b	2f
3:
	CP15_DBCON mrc,\rCtl
	mov	\rNum2,#0
	tst	\rCtl,#0x100
	bne	1b		//br if mask was used previously
	tst	\rCtl,#0x0f
	beq	1b
	CP15_DBCON mcr,\rZero
	tst	\rCtl,#3
	orreq	\rMode,\rCtl,\rMode
	bicne	\rCtl,\rCtl,#3<<2
	orrne	\rMode,\rCtl,\rMode,LSL #2
	CP15_DBR0 mcreq,\rNum1
	CP15_DBR1 mcrne,\rNum1
2:
	CP15_DBCON mcr,\rMode
.endm

//r1 - CP15_CONTROL
.macro GetPageTableStart rTmp,physToVirtOffset
	tst	r1,#1<<13		//assume 1:1 mapping for TTBR if relocation vector is off
// see if identity mapping is enabled
	CP15_TTBR mrc,r1
//  using Big will generate no instructions if Wince, throwing off my reloc vector
//	BigAdd2Ne r1,\physToVirtOffset
	addne	r1,r1,#\physToVirtOffset	//convert this physical address to a virtual address if relocation vector ON
.endm

.macro SetupTTBR rBase,rTmp
	CP15_TTBR mcr,\rBase
.endm

.macro	GetDebugMagicPhys	rTemp
	CalcMemSize	\rTemp,MEMORY_CONTROL_BASE				//out: \rTemp - mem size
	BigAdd	\rTemp,\rTemp,MEM_START-0x1000+((DEBUG_BASE+DBG_MAGIC)&0xfff)		//last 4k of memory
.endm

.macro	GetDebugMagic	rTemp
	BigMov \rTemp,DEBUG_BASE+DBG_MAGIC
.endm
.macro	GetDebugMagicCC	cc,ncc,rTemp
	BigMovCC \cc,\rTemp,DEBUG_BASE+DBG_MAGIC
.endm

.macro	GetDebugBase	rTemp
	BigMov \rTemp,DEBUG_BASE
.endm
.macro	GetDebugBaseStickyCheck rTemp,rEnd
	CP14_DCSR mrc,\rTemp
	tst     \rTemp,#1<<5                //test sticky abort bit
	BigMovEq \rTemp,DEBUG_BASE
	BigAddNe \rTemp,\rEnd,(-0x1000+((DEBUG_BASE)&0xfff)) //don't use virtual address if memory isn't working right
.endm

.macro SPS_ReturnBugsWorkAroundCode
ReturnWithIndirection:
	mov	r0,r0			//!!!! make sure this instruction is in the 1st 4k of flash so that BigOrr2Ne is guaranteed to work
	.if	WORKAROUND_FLAGS_BUG
	msr	cpsr_f,r2
	ldr	r2,[sp],#4
	.endif
	ldr	pc,[sp],#4
.endm

//Out: rPhys identity mapping flag,  don't change carry flag, z-1 identity mapping
.macro	ReturnBugsWorkAround1	rPhys,rTmp,rSpTmp,rDbg,rUart1,uartBase
	BigMov	\rPhys,\uartBase		//physical address
	eors	\rPhys,\rPhys,\rUart1		//this lets me know if identity mapping is used, or MMU is off, C & V are not affected by instruction
	ldrne	\rSpTmp,[\rDbg,#DBG_SP]
	ldrne	\rTmp,[\rDbg,#DBG_PC]
	strne	\rTmp,[\rSpTmp,#-4]!		//store pc for return indirect, (exit sds bug workaround)
	.if	WORKAROUND_FLAGS_BUG
	ldrne	\rTmp,[\rDbg,#DBG_R2]	//work around bug where flags(NZCV) aren't restored when returning to user mode
	strne	\rTmp,[\rSpTmp,#-4]!
	.endif
	strne	\rSpTmp,[\rDbg,#DBG_SP]
.endm

//In: r0 - @DBG_R2, z-0 use indirect return
.macro	ReturnBugsWorkAround2
	movne	lr,#ReturnWithIndirection-StartUp	//z-0 return indirect
	ldreq	lr,[r0,#DBG_PC-DBG_R2]			//z-1 return direct
	BigOrr2Ne lr,VMA_DEBUG	//used if MMU is enabled
	.if	WORKAROUND_FLAGS_BUG
	ldrne	r2,[r0,#DBG_CPSR-DBG_R2]
	.endif
.endm

.macro ResetCPU rBase,rVal
	BigMov	\rBase,0x40a00000	//OSTIMER_BASE
	/* We set OWE:WME (watchdog enable) and wait until timeout happens  */

	ldr	\rVal, [\rBase, #OWER]
	orr	\rVal, \rVal, #0x0001			/* bit0: WME                */
	str	\rVal, [\rBase, #OWER]

	/* OS timer does only wrap every 1165 seconds, so we have to set    */
	/* the match register as well.                                      */

	ldr	\rVal, [\rBase, #OSCR]			/* read OS timer            */
	add	\rVal, \rVal, #0x800			/* let OSMR3 match after    */
	add	\rVal, \rVal, #0x800			/* 4096*(1/3.6864MHz)=1ms   */
	str	\rVal, [\rBase, #OSMR3]
.endm
