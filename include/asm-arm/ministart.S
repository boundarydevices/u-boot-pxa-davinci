/*
 *  armboot - Startup Code for XScale
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003  Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2003  Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>
#include "BigMacro.h"
#include "lcdPanels2.h"
#include "arch/sysMacros.h"

#define Mode_SVC 0x13
#define NoIntsMask 0xc0
.globl _bss_start
.globl _bss_end

//defined globals
.globl HeadStart
.globl _armboot_start
.globl DbgBreak

//r3 - display type
HeadStart:
	mov	r8,r0	//there is a check for this instruction before jumping here

// ********************************************************************
// Set processor into Supervisior mode (SVC) and disable IRQ & FIQ
//
	mrs		r0, CPSR
	bic		r0, r0,#0x1f
	orr		r0, r0,#(Mode_SVC | NoIntsMask)
	msr		cpsr_c, r0
//exit SDS, if currently active
	msr		SPSR, r0
	adr		lr,1f
	movs	pc,lr
1:
	b		relocate
// ********************************************************************
_armboot_start:
_TEXT_BASE:		.word	TEXT_BASE

_start_armboot: .word	start_armboot
/*
 * These are defined in the board-specific linker script.
 */
_bss_start:		.word __bss_start
_bss_end:		.word _end

#ifdef CONFIG_USE_IRQ
/* IRQ stack memory (calculated at run-time) */
.globl IRQ_STACK_START
IRQ_STACK_START:
	.word	0x0badc0de

/* IRQ stack memory (calculated at run-time) */
.globl FIQ_STACK_START
FIQ_STACK_START:
	.word 0x0badc0de
#endif

relocate:				/* relocate U-Boot to RAM	    */
	adr		r0, HeadStart		/* r0 <- current position of code   */
	bic		r0,r0,#0xff
	bic		r0,r0,#0xff00		//64k aligned
	ldr		r1, _TEXT_BASE
	cmp     r0, r1                  /* don't reloc during debug         */
	beq     stack_setup

	ldr		r2, _bss_start

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0]    */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1]    */
	cmp		r1, r2			/* until dest end addreee [r2]    */
	ble	copy_loop

	/* Set up the stack						    */
stack_setup:
	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	BigSub2 r0,(CFG_MALLOC_LEN+CFG_GBL_DATA_SIZE+CFG_MMU_SPACE_RESERVED) //malloc area,bdinfo
#ifdef CONFIG_USE_IRQ
	sub	r0, r0, #(CONFIG_STACKSIZE_IRQ+CONFIG_STACKSIZE_FIQ)
#endif
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */

clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000		/* clear                            */

clbss_l:str	r2, [r0]		/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	bne	clbss_l
	bl	dcache_enable

	ldr	pc, _start_armboot


DbgBreak:
	mov	r1,lr
	swi	#0
	bx	r1

FlushCache:
	CP15_CONTROL mrc,r0
	tst	r0, #4		//is data cache enabled
	beq	2f
//	b	2f
	add	r0,pc,#0x20000	//make sure I don't alloc a line in this subroutine
	BigBic2	r0, 0xffff
	CleanInvalidateDataCache	r0,r1
2:	CP15_CF_INVAL_BOTH mcr,r0
	CPWAIT	r0
	mov	pc, lr

//void dcache_disable (void)
.globl dcache_disable
dcache_disable:
	mov	r3,lr
	bl	FlushCache
	CP15_CONTROL mrc,r1

	BigBic2	r1, 0x2805	//disable high vector, branch target buffer, disable data cache, MMU
	.balignl 32,0xe1a00000	//cacheline boundary (32 bytes)
			//Needed so that if new flash is burnt
			//and flash != ram copy,
			//mov pc, r3 will be in cache and execute correctly
			//(r3 is 0) return to the reset vector
	CP15_CONTROL mcr,r1
	CPWAIT	r0
	mov	pc, r3


//int dcache_status (void)
.globl dcache_status
dcache_status:
	CP15_CONTROL mrc,r0		//get the control register
	and	r0,r0,#1
	mov	pc,lr	


CalcMemEnd:
	CalcMemSize	r0,MEMORY_CONTROL_BASE			//out: r0 - mem size
	BigAdd2	r0,MEM_START		//32 meg
	mov	pc,lr

//void dcache_enable (void)
.globl dcache_enable
dcache_enable:
	stmdb	sp!,{r5,r6,lr}
	bl	FlushCache
#if !defined(CFG_MMU_SPACE_RESERVED) || (CFG_MMU_SPACE_RESERVED<(1<<14))
	mov	r0,#1<<15	//16k plus alignment of 16k
	bl	malloc
	add	r6,r0,#(1<<14)	//16k alignment
	BigBic2	r6,(1<<14)-1
#else
	ldr	r6, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	BigSub2 r6,(CFG_MMU_SPACE_RESERVED) //malloc area,bdinfo
#endif
	mov	r2,r6
	add	r5,r6,#(1<<14)	//16k 1st level page table
	BigMov	r0,0x0402			//section descriptor, ap-01, privileged r/w
						//map virt to phys 1-to-1, non-cache, non-bufferable
1:	str	r0,[r2],#4
	add	r0,r0,#1<<20
	cmp	r2,r5
	bne	1b


	bl	CalcMemEnd
	mov	r3,r0
	add	r5,r6,r3,LSR #(20-2)
	add	r1,r6,#((MEM_START>>20)&0xfff)<<2	//microsoft assembler bug propagates the sign bit
	BigMov	r0,(MEM_START&0xfff00000)+0x040A	//cacheable (write through)
2:	str	r0,[r1],#4
	cmp	r1,r5
	addne	r0,r0,#1<<20
	bne	2b

	mov	r0,pc,LSR #20
	mov	r0,r0,LSL #20
#define MEM_READONLY_SECTION 0x000a		//map as read-only(ap=0,r=1,s=0), cacheable, section
//#define MEM_READONLY_SECTION 0x840a	//set imx31 APX bit
	BigOrr2	r0,MEM_READONLY_SECTION
	str	r0,[r6]			//remap 1st meg to this code
	
	BigMov	r0,0xffffffff
	CP15_DACR mcr,r0

	CP15_TTBR mcr,r6
//enable MMU
	ResetTLB r0

	CP15_CONTROL mrc,r1		//get the control register
	BigOrr2	r1,0x201	// enable MMU, r bit
	BigBic2 r1,0x100	// disable S bit, ap -0 means read only by all
	CPWAIT	r0
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

//enable the instruction cache
	ResetInstructionCache r0
	orr	r1,r1,#0x1000			//set bit 12 -- the I bit
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

//enable the data cache
//	CP15_CF_UNLOCK_D mcr,r0
	CP15_CF_INVAL_D mcr,r0		//this will also drain write buffer
	CP15_CF_DRAIN mcr,r0		//make sure it is drained just to be very safe

	orr	r1,r1,#0x4			//set bit 4 -- the D bit
	CP15_CONTROL mcr,r1		//set the control register
	CPWAIT	r0

	EnableCoprocessorAccess r0
	ldmia	sp!,{r5,r6,pc}



/****************************************************************************/
/*                                                                          */
/* Reset function:															*/
/*                                                                          */
/****************************************************************************/
.globl reset_cpu
reset_cpu:
	ResetCPU	r0,r1
reset_endless:
	b	reset_endless
