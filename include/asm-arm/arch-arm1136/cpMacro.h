//CP14 registers
.macro	CP14_DIDR	ins,rx
	\ins	p14,0,\rx,c0,c0,0
.endm
.macro	CP14_DSCR	ins,rx
	\ins	p14,0,\rx,c0,c1,0
.endm
.macro	CP14_DTR	ins,rx
	\ins	p14,0,\rx,c0,c5,0
.endm
.macro	CP14_DTR_LS	ins,addressMode
	\ins	p14,c5,\addressMode	//LDC{cc}/STC{cc}
.endm
.macro	CP14_VECTOR_CATCH	ins,rx
	\ins	p14,0,\rx,c0,c7,0
.endm
.macro	CP14_BKPT		ins,rx,num
	\ins	p14,0,\rx,c0,c\num,4	//num to 0 to 5
.endm
.macro	CP14_BKPT_CONTROL	ins,rx,num
	\ins	p14,0,\rx,c0,c\num,5	//num to 0 to 5
.endm
.macro	CP14_WATCHPT		ins,rx,num
	\ins	p14,0,\rx,c0,c\num,6	//num to 0 to 1
.endm
.macro	CP14_WATCHPT_CONTROL	ins,rx,num
	\ins	p14,0,\rx,c0,c\num,7	//num to 0 to 1
.endm





//CP15 registers
.macro	CP15_ID	ins,rx
	\ins	p15,0,\rx,c0,c0,0
.endm
.macro	CP15_FEATURE_ID	ins,rx,num
	\ins	p15,0,\rx,c0,c1,\num	//num is 0-7
.endm
.macro	CP15_INSTRUCTION_SET_ATTR	ins,rx,num
	\ins	p15,0,\rx,c0,c2,\num	//num is 0-5
.endm

.macro	CP15_CONTROL	ins,rx
	\ins	p15,0,\rx,c1,c0,0
.endm
.macro	CP15_AUXCONTROL	ins,rx
	\ins	p15,0,\rx,c1,c0,1
.endm
.macro	CP15_CP_ACCESS_CONTROL	ins,rx
	\ins	p15,0,\rx,c1,c0,2
.endm

.macro	CP15_TTBR	ins,rx
	\ins	p15,0,\rx,c2,c0,0		//translation table base
.endm
.macro	CP15_TTBR1	ins,rx
	\ins	p15,0,\rx,c2,c0,1		//translation table base1
.endm
.macro	CP15_TTBR_CONTROL	ins,rx
	\ins	p15,0,\rx,c2,c0,2		//translation table base control
.endm
.macro	CP15_DACR	ins,rx
	\ins	p15,0,\rx,c3,c0,0		//domain access control register
.endm
.macro	CP15_FSR	ins,rx
	\ins	p15,0,\rx,c5,c0,0		//fault status register
.endm
.macro	CP15_IFSR	ins,rx
	\ins	p15,0,\rx,c5,c0,1		//instruction fault status register
.endm
.macro	CP15_FAR	ins,rx
	\ins	p15,0,\rx,c6,c0,0		//fault address register
.endm
.macro	CP15_WFAR	ins,rx
	\ins	p15,0,\rx,c6,c0,1		//watchpoint fault address register
.endm

//////////////////////////////////////////////
.macro	CP15_WAIT_FOR_INT	ins,rx
	\ins	p15,0,\rx,c7,c0,4
.endm
.macro	CP15_CF_INVAL_I	ins,rx
	\ins	p15,0,\rx,c7,c5,0		//invalidate instruction cache
.endm
.macro	CP15_CF_INVAL_ILINE	ins,rx
	\ins	p15,0,\rx,c7,c5,1
.endm
.macro	CP15_CF_INVAL_ILINE_WAYSET	ins,rx
	\ins	p15,0,\rx,c7,c5,2
.endm
.macro	CP15_CF_FLUSH_PREFETCH	ins,rx
	\ins	p15,0,\rx,c7,c5,4
.endm
.macro	CP15_CF_INVAL_BTB	ins,rx
	\ins	p15,0,\rx,c7,c5,6		//invalidate Branch target buffer
.endm
.macro	CP15_CF_INVAL_BTB_ENTRY	ins,rx
	\ins	p15,0,\rx,c7,c5,7
.endm

.macro	CP15_CF_INVAL_D	ins,rx
	\ins	p15,0,\rx,c7,c6,0
.endm
.macro	CP15_CF_INVAL_DLINE	ins,rx
	\ins	p15,0,\rx,c7,c6,1
.endm
.macro	CP15_CF_INVAL_DLINE_WAYSET	ins,rx
	\ins	p15,0,\rx,c7,c6,2
.endm

.macro	CP15_CF_INVAL_BOTH	ins,rx
//	\ins	p15,0,\rx,c7,c7,0		//invalidate instruction & data cache & BTB
.endm

.macro	CP15_CF_CLEAN_DCACHE ins,rx	//Clean entire Data Cache
	\ins	p15,0,\rx,c7,c10,0
.endm
.macro	CP15_CF_CLEAN_DLINE	ins,rx
	\ins	p15,0,\rx,c7,c10,1
.endm
.macro	CP15_CF_CLEAN_DLINE_WAYSET	ins,rx
	\ins	p15,0,\rx,c7,c10,2
.endm
.macro	CP15_CF_DRAIN	ins,rx
	\ins	p15,0,\rx,c7,c10,4	//Data Synchronization Barrier, accessible in user mode
.endm
.macro	CP15_CF_MEMORY_BARRIER	ins,rx
	\ins	p15,0,\rx,c7,c10,5	//Data Memory Barrier, accessible in user mode
.endm
.macro	CP15_CF_DIRTY_STATUS	ins,rx
	\ins	p15,0,\rx,c7,c10,6	//read only
.endm
.macro	CP15_CF_BLOCK_TRANSFER_STATUS ins,rx
	\ins	p15,0,\rx,c7,c12,4	//read only, accessible in user mode
.endm
.macro	CP15_CF_STOP_PREFETCH_RANGE ins,rx
	\ins	p15,0,\rx,c7,c12,5	//accessible in user mode
.endm
.macro	CP15_CF_PREFETCH_ILINE ins,rx
	\ins	p15,0,\rx,c7,c13,1	//Prefetch Instruction Cache Line
.endm
.macro	CP15_CF_CLEAN_INVAL_DCACHE ins,rx
	\ins	p15,0,\rx,c7,c14,0	//Clean and Invalidate Entire Data Cache
.endm
.macro	CP15_CF_CLEAN_INVAL_DLINE ins,rx
	\ins	p15,0,\rx,c7,c14,1	//Clean and Invalidate Data Cache Line
.endm
.macro	CP15_CF_CLEAN_INVAL_DLINE_WAYSET ins,rx
	\ins	p15,0,\rx,c7,c14,2	//Clean and Invalidate Data Cache Line
.endm

////
//MCRR instructions (CRn is c7) (Opcode2 is 0)
.macro	CP15_CF_INVAL_ICACHE_RANGE rStart,rEnd
	mcrr	p15,0,\rEnd,\rStart,c5
.endm
.macro	CP15_CF_INVAL_DCACHE_RANGE rStart,rEnd
	mcrr	p15,0,\rEnd,\rStart,c6
.endm
.macro	CP15_CF_CLEAN_DCACHE_RANGE rStart,rEnd
	mcrr	p15,0,\rEnd,\rStart,c12		//accessible in user mode
.endm
.macro	CP15_CF_CLEAN_INVAL_DCACHE_RANGE rStart,rEnd
	mcrr	p15,0,\rEnd,\rStart,c14
.endm
.macro	CP15_CF_PREFETCH_ICACHE_RANGE rStart,rEnd
	mcrr	p15,1,\rEnd,\rStart,c12		//accessible in user mode
.endm
.macro	CP15_CF_PREFETCH_DCACHE_RANGE rStart,rEnd
	mcrr	p15,2,\rEnd,\rStart,c12	//accessible in user mode
.endm

//////////////////////////////////////////////
.macro	CP15_TLB_INVAL_UNIFIED_ALL	ins,rx
	mov		\rx,#0
	\ins	p15,0,\rx,c8,c7,0
.endm
.macro	CP15_TLB_INVAL_UNIFIED_SINGLE	ins,rx
	\ins	p15,0,\rx,c8,c7,1
.endm
.macro	CP15_TLB_INVAL_UNIFIED_ASID	ins,rx
	\ins	p15,0,\rx,c8,c7,2
.endm

.macro	CP15_TLB_LOCK	ins,rx
	\ins	p15,0,\rx,c10,c0,0
.endm
.macro	CP15_TEX_REMAP_PRIMARY	ins,rx
	\ins	p15,0,\rx,c10,c2,0
.endm
.macro	CP15_TEX_REMAP_NORMAL	ins,rx
	\ins	p15,0,\rx,c10,c2,1
.endm



//////////////////////////////////////////////

.macro	CP15_FCSE_PID	ins,rx
	\ins	p15,0,\rx,c13,c0,0
.endm
.macro	CP15_CONTEXT_ID	ins,rx
	\ins	p15,0,\rx,c13,c0,1
.endm
.macro	CP15_THREAD_PID_URW	ins,rx
	\ins	p15,0,\rx,c13,c0,2
.endm
.macro	CP15_THREAD_PID_UR	ins,rx
	\ins	p15,0,\rx,c13,c0,3
.endm
.macro	CP15_THREAD_PID_P	ins,rx
	\ins	p15,0,\rx,c13,c0,4
.endm

.macro	CP15_DATA_REMAP	ins,rx
	\ins	p15,0,\rx,c15,c2,0	//Data Memory Remap Register
.endm
.macro	CP15_INSTRUCTION_REMAP	ins,rx
	\ins	p15,0,\rx,c15,c2,1	//Instruction Memory Remap Register
.endm
.macro	CP15_DMA_REMAP	ins,rx
	\ins	p15,0,\rx,c15,c2,2	//Dma Memory Remap Register
.endm
.macro	CP15_PERIPHERAL_REMAP	ins,rx
	\ins	p15,0,\rx,c15,c2,4	//Peripheral Memory Remap Register
.endm

//////////////////////////////////////////////

.macro CPWAIT dest
	CP15_ID mrc,\dest		//read some register in CP15
	mov	\dest,\dest		//wait for the read to complete
	sub	pc,pc,#4		//branch to the next instruction, flushing the instruction pipeline
.endm

//out: rTemp - memory size
.macro CalcMemSize rTemp,mem_control_base
	mov		\rTemp,#64<<20
.endm


