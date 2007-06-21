#include "pxa250Base.h"
#include "pxaHardware.h"
	.nolist

//CP14 registers
.macro	CP14_PMNC	ins,rx
	\ins	p14,0,\rx,c0,c0,0
.endm
.macro	CP14_CCNT	ins,rx
	\ins	p14,0,\rx,c1,c0,0
.endm
.macro	CP14_PMN0	ins,rx
	\ins	p14,0,\rx,c2,c0,0
.endm
.macro	CP14_PMN1	ins,rx
	\ins	p14,0,\rx,c3,c0,0
.endm
.macro	CP14_CCLKCFG	ins,rx
	\ins	p14,0,\rx,c6,c0,0
.endm
.macro	CP14_PWRMODE	ins,rx
	\ins	p14,0,\rx,c7,c0,0
.endm
.macro	CP14_TX	ins,rx
	\ins	p14,0,\rx,c8,c0,0
.endm
.macro	CP14_RX	ins,rx
	\ins	p14,0,\rx,c9,c0,0
.endm
.macro	CP14_DCSR	ins,rx
	\ins	p14,0,\rx,c10,c0,0	//debug control and status register
.endm
.macro	CP14_TBREG	ins,rx
	\ins	p14,0,\rx,c11,c0,0	//trace buffer register
.endm
.macro	CP14_CHKPT0	ins,rx
	\ins	p14,0,\rx,c12,c0,0	//checkpoint register 0
.endm
.macro	CP14_CHKPT1	ins,rx
	\ins	p14,0,\rx,c13,c0,0	//checkpoint register 1
.endm
.macro	CP14_TXRXCTRL	ins,rx
	\ins	p14,0,\rx,c14,c0,0
.endm


//CP15 registers
.macro	CP15_ID	ins,rx
	\ins	p15,0,\rx,c0,c0,0
.endm
.macro	CP15_CACHETYPE	ins,rx
	\ins	p15,0,\rx,c0,c0,1
.endm
.macro	CP15_CONTROL	ins,rx
	\ins	p15,0,\rx,c1,c0,0
.endm
.macro	CP15_AUXCONTROL	ins,rx
	\ins	p15,0,\rx,c1,c0,1
.endm
.macro	CP15_TTBR	ins,rx
	\ins	p15,0,\rx,c2,c0,0		//translation table base
.endm
.macro	CP15_DACR	ins,rx
	\ins	p15,0,\rx,c3,c0,0		//domain access control register
.endm
.macro	CP15_FSR	ins,rx
	\ins	p15,0,\rx,c5,c0,0		//fault status register
.endm
.macro	CP15_FAR	ins,rx
	\ins	p15,0,\rx,c6,c0,0		//fault address register
.endm

.macro	CP15_CF_ALLOC_LINE	ins,rx
	\ins	p15,0,\rx,c7,c2,5		//allocate line in data cache
.endm
.macro	CP15_CF_INVAL_I	ins,rx
	\ins	p15,0,\rx,c7,c5,0		//invalidate instruction cache
.endm
.macro	CP15_CF_INVAL_ILINE	ins,rx
	\ins	p15,0,\rx,c7,c5,1
.endm
.macro	CP15_CF_INVAL_BTB	ins,rx
	\ins	p15,0,\rx,c7,c5,6		//invalidate Branch target buffer
.endm
.macro	CP15_CF_INVAL_D	ins,rx
	\ins	p15,0,\rx,c7,c6,0
.endm
.macro	CP15_CF_INVAL_DLINE	ins,rx
	\ins	p15,0,\rx,c7,c6,1
.endm
.macro	CP15_CF_INVAL_BOTH	ins,rx
	\ins	p15,0,\rx,c7,c7,0		//invalidate instruction & data cache & BTB
.endm
.macro	CP15_CF_CLEAN_DLINE	ins,rx
	\ins	p15,0,\rx,c7,c10,1
.endm
.macro	CP15_CF_DRAIN	ins,rx
	\ins	p15,0,\rx,c7,c10,4
.endm

.macro	CP15_TLB_INVAL_I	ins,rx
	\ins	p15,0,\rx,c8,c5,0
.endm
.macro	CP15_TLB_INVAL_IENTRY	ins,rx
	\ins	p15,0,\rx,c8,c5,1
.endm
.macro	CP15_TLB_INVAL_D	ins,rx
	\ins	p15,0,\rx,c8,c6,0
.endm
.macro	CP15_TLB_INVAL_DENTRY	ins,rx
	\ins	p15,0,\rx,c8,c6,1
.endm
.macro	CP15_TLB_INVAL_BOTH	ins,rx
	\ins	p15,0,\rx,c8,c7,0
.endm

.macro	CP15_CF_LOCK_ILINE	ins,rx
	\ins	p15,0,\rx,c9,c1,0		//mva to fetch and lock
.endm
.macro	CP15_CF_UNLOCK_I	ins,rx
	\ins	p15,0,\rx,c9,c1,1		//unlock all lines
.endm
.macro	CP15_CF_LOCK_D_CSR	ins,rx
	\ins	p15,0,\rx,c9,c2,0
.endm
.macro	CP15_CF_UNLOCK_D	ins,rx
	\ins	p15,0,\rx,c9,c2,1		//unlock all lines in data cache
.endm

.macro	CP15_TLB_LOCK_IENTRY	ins,rx
	\ins	p15,0,\rx,c10,c4,0
.endm
.macro	CP15_TLB_UNLOCK_I	ins,rx
	\ins	p15,0,\rx,c10,c4,1
.endm
.macro	CP15_TLB_LOCK_DENTRY	ins,rx
	\ins	p15,0,\rx,c10,c8,0
.endm
.macro	CP15_TLB_UNLOCK_D	ins,rx
	\ins	p15,0,\rx,c10,c8,1
.endm

.macro	CP15_PID	ins,rx
	\ins	p15,0,\rx,c13,c0,0
.endm

.macro	CP15_DBR0	ins,rx
	\ins	p15,0,\rx,c14,c0,0	//Data Breakpoint address register 0
.endm
.macro	CP15_DBR1	ins,rx
	\ins	p15,0,\rx,c14,c3,0	//Data Breakpoint address/mask register 1
.endm
.macro	CP15_DBCON	ins,rx
	\ins	p15,0,\rx,c14,c4,0	//Data Breakpoint control register
.endm
.macro	CP15_IBCR0	ins,rx
	\ins	p15,0,\rx,c14,c8,0	//Instruction Breakpoint Control Register 0
.endm
.macro	CP15_IBCR1	ins,rx
	\ins	p15,0,\rx,c14,c9,0	//Instruction Breakpoint Control Register 1
.endm

.macro	CP15_CP_ACCESS	ins,rx
	\ins	p15,0,\rx,c15,c1,0	//Coprocessor access register, set bit n to enable access to coprocessor n
.endm


.macro CPWAIT dest
	CP15_ID mrc,\dest		//read some register in CP15
	mov	\dest,\dest		//wait for the read to complete
	sub	pc,pc,#4		//branch to the next instruction, flushing the instruction pipeline
.endm

// *******************************************************************************************
// *******************************************************************************************
//out: rTmp0 - memory size
.macro CalcMemSize rTmp0,rTmp1,mem_control_base
	BigMov	\rTmp0,\mem_control_base
	ldr		\rTmp1,[\rTmp0,#MDCNFG]
	movs	\rTmp0,\rTmp1,LSR #3		//bits 14,4,3 column address bits,
										//bit 2 - 1 means 16 bit mode, 0 means 32 bit mode, mov to carry flag
	and		\rTmp0,\rTmp0,#3
//	tst		\rTmp1,#1<<14				//extended column address bit
//	orrne	\rTmp0,\rTmp0,#8
	tst		\rTmp1,#1<<7				//# of bank address bits, 0-1, 1-2
	addne	\rTmp0,\rTmp0,#1
	mov		\rTmp1,\rTmp1,LSR #5		//bits 6,5 row address bits
	and		\rTmp1,\rTmp1,#3
	add		\rTmp0,\rTmp0,\rTmp1

	addcc	\rTmp1,\rTmp0,#1			//one more if 32 bits wide
	mov		\rTmp0,#1<<(1+8+11+1)		//offset of 1 bank bit, 8 column bits, 11 row bits, 1 for low/high byte select
	mov		\rTmp0,\rTmp0, LSL \rTmp1
.endm

