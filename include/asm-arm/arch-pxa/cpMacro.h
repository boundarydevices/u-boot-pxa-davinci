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
// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
//  ************************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************

	.equiv	numColumnAddrBits, 9
	.equiv	numBankAddrBits, 2
	.equiv	ClkSelect, 1	//0 : tRP = 2, tRCD = 1, tRAS = 3, tRC = 4
			//1 : tRP = 2, tRCD = 2, tRAS = 5, tRC = 8
			//2 : tRP = 3, tRCD = 3, tRAS = 7, tRC = 10
			//3 : tRP = 3, tRCD = 3, tRAS = 7, tRC = 11
//64 meg option
	.equiv	M64_numRowAddrBits, 13	//for k4s561632a
	.equiv	M64_SA1111_mask, 0		//(1<<12)
	.equiv	M64_DRI_cnt,  (((99530*64)>>M64_numRowAddrBits)>>5)	//(# of cycles/ms  * # of ms for entire refresh period)/ # of rows/refresh period /32
	.equiv	M64_MDCNFG_VAL, 1+((numColumnAddrBits-8)<<3)+((M64_numRowAddrBits-11)<<5)+((numBankAddrBits-1)<<7)+(ClkSelect<<8)+(1<<11)+(M64_SA1111_mask)	//DLATCH0, latch return data with return clock
	.equiv	M64_MDREFR_VAL, (1<<16)+(1<<15)+(M64_DRI_cnt&0xfff)		//don't set bit 20: APD (buggy), bit 16: K1RUN, 15:E1PIN
//	.equiv	M64_MDREFR_VAL, (1<<20)+(1<<16)+(1<<15)+(M64_DRI_cnt&0xfff)		//20: APD, bit 16: K1RUN, 15:E1PIN
//			 13		9		  2	       2 (4bytes per address)=2**26=64 MB
	.equiv	M64_MEM_SIZE, (1<<(M64_numRowAddrBits+numColumnAddrBits+numBankAddrBits+2))
	.equiv	M64_MEM_END, ((MEM_START)+M64_MEM_SIZE)

//32 meg option
	.equiv	M32_numRowAddrBits, 12	//for MT48LC8M16A2 - 75 B
	.equiv	M32_SA1111_mask, 0
	.equiv	M32_DRI_cnt,  (((99530*64)>>M32_numRowAddrBits)>>5)	//(# of cycles/ms  * # of ms for entire refresh period)/ # of rows/refresh period /32
	.equiv	M32_MDCNFG_VAL, 1+((numColumnAddrBits-8)<<3)+((M32_numRowAddrBits-11)<<5)+((numBankAddrBits-1)<<7)+(ClkSelect<<8)+(1<<11)+(M32_SA1111_mask)	//DLATCH0, latch return data with return clock
	.equiv	M32_MDREFR_VAL, (1<<16)+(1<<15)+(M32_DRI_cnt&0xfff)		//don't set bit 20: APD (buggy), bit 16: K1RUN, 15:E1PIN
//			 12		9		  2	       2 (4bytes per address)=2**25=32 MB
	.equiv	M32_MEM_SIZE, (1<<(M32_numRowAddrBits+numColumnAddrBits+numBankAddrBits+2))
	.equiv	M32_MEM_END, ((MEM_START)+M32_MEM_SIZE)
// *******************************************************************************************
//out: rTemp - memory size
.macro CalcMemSize rTemp,mem_control_base
	BigMov	\rTemp,\mem_control_base
	ldr	\rTemp,[\rTemp,#MDCNFG]
	movs	\rTemp,\rTemp,LSR #2+1		//bit 2 - 1 means 16 bit mode, 0 means 32 bit mode, mov to carry flag
	tst	\rTemp,#1<<(5-3)				//is number of row address bits 12 or 13 ?
	moveq	\rTemp,#M64_MEM_SIZE
	movne	\rTemp,#M32_MEM_SIZE
	movcs	\rTemp,\rTemp,LSR #1		//half as much if 16 bit mode
.endm

