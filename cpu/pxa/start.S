/*
 *  armboot - Startup Code for XScale
 *
 *  Copyright (C) 1998	Dan Malek <dmalek@jlc.net>
 *  Copyright (C) 1999	Magnus Damm <kieraypc01.p.y.kie.era.ericsson.se>
 *  Copyright (C) 2000	Wolfgang Denk <wd@denx.de>
 *  Copyright (C) 2001	Alex Zuepke <azu@sysgo.de>
 *  Copyright (C) 2002	Kyle Harris <kharris@nexus-tech.net>
 *  Copyright (C) 2003	Robert Schwebel <r.schwebel@pengutronix.de>
 *  Copyright (C) 2003	Kai-Uwe Bloem <kai-uwe.bloem@auerswald.de>
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

.globl _start
_start: b	reset
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:	.word prefetch_abort
_data_abort:		.word data_abort
_not_used:		.word not_used
_irq:			.word irq
_fiq:			.word fiq

_textBase: .word TEXT_BASE
	.balignl 16,0xdeadbeef

/****************************************************************************/
/*									    */
/* the actual reset code						    */
/*									    */
/****************************************************************************/

reset:
	mrs	r0,cpsr			/* set the cpu to SVC32 mode	    */
	bic	r0,r0,#0x1f		/* (superviser mode, M=10011)	    */
	orr	r0,r0,#0x13
	msr	cpsr,r0

	/*
	 * we do sys-critical inits only at reboot,
	 * not when booting from ram!
	 */
#ifndef CONFIG_SKIP_LOWLEVEL_INIT
	bl	lowlevel_init
#endif
	b	HeadStart


/****************************************************************************/
/*									    */
/* Interrupt handling							    */
/*									    */
/****************************************************************************/

/* IRQ stack frame							    */

#define S_FRAME_SIZE	72

#define S_OLD_R0	68
#define S_PSR		64
#define S_PC		60
#define S_LR		56
#define S_SP		52

#define S_IP		48
#define S_FP		44
#define S_R10		40
#define S_R9		36
#define S_R8		32
#define S_R7		28
#define S_R6		24
#define S_R5		20
#define S_R4		16
#define S_R3		12
#define S_R2		8
#define S_R1		4
#define S_R0		0

#define MODE_SVC 0x13

	/* use bad_save_user_regs for abort/prefetch/undef/swi ...	    */

	.macro	bad_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			/* Calling r0-r12	    */
	add	r8, sp, #S_PC

	ldr	r2, _textBase
	sub	r2, r2, #(CONFIG_STACKSIZE+CFG_MALLOC_LEN)
	sub	r2, r2, #(CFG_GBL_DATA_SIZE+8)	@ set base 2 words into abort stack
	ldmia	r2, {r2 - r4}			/* get pc, cpsr, old_r0	    */
	add	r0, sp, #S_FRAME_SIZE		/* restore sp_SVC	    */

	add	r5, sp, #S_SP
	mov	r1, lr
	stmia	r5, {r0 - r4}			/* save sp_SVC, lr_SVC, pc, cpsr, old_r */
	mov	r0, sp
	.endm


	/* use irq_save_user_regs / irq_restore_user_regs for		     */
	/* IRQ/FIQ handling						     */

	.macro	irq_save_user_regs
	sub	sp, sp, #S_FRAME_SIZE
	stmia	sp, {r0 - r12}			/* Calling r0-r12	     */
	add	r8, sp, #S_PC
	stmdb	r8, {sp, lr}^			/* Calling SP, LR	     */
	str	lr, [r8, #0]			/* Save calling PC	     */
	mrs	r6, spsr
	str	r6, [r8, #4]			/* Save CPSR		     */
	str	r0, [r8, #8]			/* Save OLD_R0		     */
	mov	r0, sp
	.endm

	.macro	irq_restore_user_regs
	ldmia	sp, {r0 - lr}^			@ Calling r0 - lr
	mov	r0, r0
	ldr	lr, [sp, #S_PC]			@ Get PC
	add	sp, sp, #S_FRAME_SIZE
	subs	pc, lr, #4			@ return & move spsr_svc into cpsr
	.endm



/****************************************************************************/
/*									    */
/* exception handlers							    */
/*									    */
/****************************************************************************/

	.align	5
undefined_instruction:
	bad_save_user_regs
	bl	do_undefined_instruction

	.align	5
software_interrupt:
	bad_save_user_regs
	bl	do_software_interrupt

	.align	5
prefetch_abort:
	bad_save_user_regs
	bl	do_prefetch_abort

	.align	5
data_abort:
	bad_save_user_regs
	bl	do_data_abort

	.align	5
not_used:
	bad_save_user_regs
	bl	do_not_used

#ifdef CONFIG_USE_IRQ

	.align	5
irq:
	irq_save_user_regs
	bl	do_irq
	irq_restore_user_regs

	.align	5
fiq:
	irq_save_user_regs		/* someone ought to write a more    */
	bl	do_fiq			/* effiction fiq_save_user_regs	    */
	irq_restore_user_regs

#else

	.align	5
irq:
	bad_save_user_regs
	bl	do_irq

	.align	5
fiq:
	bad_save_user_regs
	bl	do_fiq

#endif

