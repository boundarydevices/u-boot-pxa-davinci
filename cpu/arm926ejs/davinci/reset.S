/*
 * Processor reset using WDT for TI TMS320DM644x SoC.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * -----------------------------------------------------
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <asm/BigMacro.h>
.globl reset_cpu
reset_cpu:
	ldr	r0, WDT_TGCR
	mov	r1, $0x08
	str	r1, [r0]
	ldr	r1, [r0]
	orr	r1, r1, $0x03
	str	r1, [r0]
	mov	r1, $0
	ldr	r0, WDT_TIM12
	str	r1, [r0]
	ldr	r0, WDT_TIM34
	str	r1, [r0]
	ldr	r0, WDT_PRD12
	str	r1, [r0]
	ldr	r0, WDT_PRD34
	str	r1, [r0]
	ldr	r0, WDT_TCR
	ldr	r1, [r0]
	orr	r1, r1, $0x40
	str	r1, [r0]
	ldr	r0, WDT_WDTCR
	ldr	r1, [r0]
	orr	r1, r1, $0x4000
	str	r1, [r0]
	ldr	r1, WDTCR_VAL1
	str	r1, [r0]
	ldr	r1, WDTCR_VAL2
	str	r1, [r0]
	nop
	nop
	nop
	nop
reset_cpu_loop:
	b	reset_cpu_loop

WDT_TGCR:
	.word	0x01c21c24
WDT_TIM12:
	.word	0x01c21c10
WDT_TIM34:
	.word	0x01c21c14
WDT_PRD12:
	.word	0x01c21c18
WDT_PRD34:
	.word	0x01c21c1c
WDT_TCR:
	.word	0x01c21c20
WDT_WDTCR:
	.word	0x01c21c28
WDTCR_VAL1:
	.word	0xa5c64000
WDTCR_VAL2:
	.word	0xda7e4000
/* PLL2-SYSTEM PLL MMRs */
	.equiv	PLL_BASE,		0x01c40000
	.equiv	_PLL2_CTL,		0x0d00
	.equiv	_PLL2_PLLM,		0x0d10
	.equiv	_PLL2_DIV1,		0x0d18
	.equiv	_PLL2_DIV2,		0x0d1c
	.equiv	_PLL2_PLLCMD,		0x0d38
	.equiv	_PLL2_PLLSTAT,		0x0d3c
	.equiv	CTL_PLLEN,	(1 << 0)
	.equiv	CTL_PLLPWRDN,	(1 << 1)
	.equiv	CTL_PLLRST,	(1 << 3)
	.equiv	CTL_PLLDIS,	(1 << 4)
	.equiv	CTL_PLLENSRC,	(1 << 5)
	.equiv	CTL_CLKMODE,	(1 << 8)	/* internal osc or CLKIN select*/
	.equiv	GOSET,		1
	.equiv	GOSTAT,		1
	.equiv	_PLL_LOCK_COUNT,	0x2000

	.equiv	MM_BASE,		0x20000000
	.equiv	MM_SDRSTAT,		0x0004
	.equiv	MM_SDBCR,		0x0008
	.equiv	MM_SDRCR,		0x000c
	.equiv	MM_SDTIMR,		0x0010
	.equiv	MM_SDTIMR2,		0x0014
	.equiv	MM_DDRPHYCR,		0x00e4
	.equiv	MM_VTPIOCR,		0x00f0	// VTP IO Control register
	.equiv	SDRCR_SR,		(1 << 31)
	.equiv	SDRCR_MCLKSTOPEN,	(1 << 30)
	.equiv	SDRSTAT_PHYRDY,		(1 << 2)
	.equiv	DDRPHYCR_DLLPWRDN,	(1 << 4)

	.equiv	M2_BASE,		0x01c41000
	.equiv	M2_PTCMD,		0x120
	.equiv	M2_PTSTAT,		0x128
	.equiv	M2_MDSTAT_,		0x800
	.equiv	M2_MDSTAT_DDR2,	0x834		//13
	.equiv	M2_MDSTAT_GEM,	0x89c		//39
	.equiv	M2_MDCTL_,		0xa00
	.equiv	M2_P1394,		0xa20		// For WDT reset chip bug
	.equiv	M2_MDCTL_DDR2,	0xa34		//13
	.equiv	M2_MDCTL_GEM,	0xa9c		//39 GEM Power Up & LPSC Control Register

	.equiv	SDTIM0_VAL_162MHz, 0x28923211
	.equiv	SDTIM1_VAL_162MHz, 0x0016c722
	.equiv	DUMMY_VAL, 0xa55aa55a
	.equiv	DDR2_MEM_BASE,	0x80000000


	.equiv	C_PSC_FLAG_MASK,	0x1f
	.equiv	C_VTP_LOCK_COUNT,	0x05b0
	.equiv	C_WAITCFG_VAL,	0
	.equiv	C_ACFG3_VAL,			0x3ffffffd
	.equiv	C_ACFG4_VAL,			0x3ffffffd
	.equiv	C_ACFG5_VAL,			0x3ffffffd
	.equiv	C_MMARG_BRF0_VAL,		0x00444400
	.equiv	C_DDRCTL_VAL,			0x50006405
	.equiv	C_SDREF_VAL,			0x000005c3
	.equiv	DAVINCI_LPSC_DDR2,	13

	.macro	DelayLoop rCnt,val
	BigMov	\rCnt,\val
99:	subs	\rCnt,\rCnt,#1
	bne		99b
	.endm

	.macro WaitForBitLow	rTmp,rBase,offset,mask
99:	ldr		\rTmp,[\rBase,#\offset]
	tst		\rTmp,#\mask
	bne		99b
	.endm

	.macro WaitForValue		rTmp,rBase,offset,mask,val
99:	ldr		\rTmp,[\rBase,#\offset]
	and		\rTmp,\rTmp,#\mask
	cmp		\rTmp,#\val
	bne		99b
	.endm
/*
	int i;
	// Select bypass mode
	REGVALUE(PLL2_CTL) &= ~CTL_PLLENSRC;
	REGVALUE(PLL2_CTL) &= ~CTL_PLLEN;
	for (i=0; i < 0x218; i++);
	REGVALUE(PLL2_CTL) &= ~(CTL_PLLRST);
	REGVALUE(PLL2_CTL) &= ~(CTL_PLLDIS);

	REGVALUE(PLL2_PLLM) = best_mult;
	REGVALUE(PLL2_DIV1) = best_div;
	REGVALUE(PLL2_DIV2) = ddr2_div;

	REGVALUE(PLL2_CMD) = GOSET;
	while (REGVALUE(PLL2_STAT) & GOSTAT);
	udelay(60);

	REGVALUE(PLL2_CTL) |= CTL_PLLRST;
	udelay(500);
	REGVALUE(PLL2_CTL) |= CTL_PLLEN;
*/
pll2_change_routine:
//;stop ddr2
	ldr	ip, [r5,#MM_SDBCR]	//save 128MB/256MB setting

	ldr	r3, [r5,#MM_SDRCR]
	orr	r3,r3,$SDRCR_SR
	str	r3, [r5,#MM_SDRCR]
	orr	r3,r3,$SDRCR_MCLKSTOPEN
	str	r3, [r5,#MM_SDRCR]

	ldr	r3, [r5,#MM_DDRPHYCR]
	orr	r3,r3,$DDRPHYCR_DLLPWRDN
	str	r3, [r5,#MM_DDRPHYCR]

1:	ldr	r3, [r5,#MM_SDRSTAT]
	tst	r3,$SDRSTAT_PHYRDY
	bne	1b
//;disable lpsc vclk
//;???

//; Select PLL bypass mode
	ldr	r3, [r6,#_PLL2_CTL]
	bic	r3, r3, $CTL_CLKMODE
	str	r3, [r6,#_PLL2_CTL]
	bic	r3, r3, $CTL_PLLENSRC
	str	r3, [r6,#_PLL2_CTL]
	bic	r3, r3, $CTL_PLLEN
	str	r3, [r6,#_PLL2_CTL]
	/* Wait for a few cycles to allow PLLEN Mux to switch properly to bypass Clock */
	DelayLoop r8,0x20

	bic	r3, r3, $CTL_PLLRST
	str	r3, [r6,#_PLL2_CTL]
	bic	r3, r3, #CTL_PLLPWRDN
	str	r3, [r6,#_PLL2_CTL]
	bic	r3, r3, $CTL_PLLDIS
	str	r3, [r6,#_PLL2_CTL]

	orr	r1, r1, $(1 << 15)
	orr	r2, r2, $(1 << 15)
	str	r0, [r6,#_PLL2_PLLM]
	str	r1, [r6,#_PLL2_DIV1]
	str	r2, [r6,#_PLL2_DIV2]

	/* Program the GOSET bit to take new divider values */
	mov	r0, $GOSET
	str	r0, [r6,#_PLL2_PLLCMD]

	WaitForBitLow r0,r6,_PLL2_PLLSTAT,GOSTAT	// Wait for Done

	/* Wait for PLL to Reset Properly */
	DelayLoop r0,0x218

	/* Bring PLL out of Reset */
	ldr	r0, [r6,#_PLL2_CTL]
	orr	r0, r0, $CTL_PLLRST
	str	r0, [r6,#_PLL2_CTL]

	/* Wait for PLL to Lock */
	DelayLoop r0,_PLL_LOCK_COUNT

	/* Enable the PLL */
	ldr	r0, [r6,#_PLL2_CTL]
	orr	r0, r0, $CTL_PLLEN
	str	r0, [r6,#_PLL2_CTL]

//;enable lpsc vclk
//;???
	ldr	r3, [r5,#MM_SDRCR]
	bic	r3,r3,$SDRCR_SR
	str	r3, [r5,#MM_SDRCR]
	bic	r3,r3,$SDRCR_MCLKSTOPEN
	str	r3, [r5,#MM_SDRCR]

	ldr	r3, [r5,#MM_DDRPHYCR]
	bic	r3,r3,$DDRPHYCR_DLLPWRDN
	str	r3, [r5,#MM_DDRPHYCR]
1:	ldr	r3, [r5,#MM_SDRSTAT]
	tst	r3,$SDRSTAT_PHYRDY
	beq	1b

	/*------------------------------------------------------*
	 * Issue Soft Reset to DDR Module			*
	 *------------------------------------------------------*/

	/* Shut down the DDR2 LPSC Module */
	ldr	r0, [r7,#M2_MDCTL_DDR2]
	bic	r0, r0, #C_PSC_FLAG_MASK
	orr	r0, r0, $0x03
	str	r0, [r7,#M2_MDCTL_DDR2]

	/* Enable the Power Domain Transition Command */
	ldr	r0, [r7,#M2_PTCMD]
	orr	r0, r0, $0x01
	str	r0, [r7,#M2_PTCMD]

	WaitForBitLow r0,r7,M2_PTSTAT,0x01	//Wait for Transition Complete(PTSTAT)
	WaitForValue r0,r7,M2_MDSTAT_DDR2,0x1f,0x03		//Wait for DDR2 Controller Enable Completion

	/*------------------------------------------------------*
	 * Program DDR2 MMRs for 162MHz Setting			*
	 *------------------------------------------------------*/

	/* Program PHY Control Register */
	BigMov	r0,C_DDRCTL_VAL
	str	r0, [r5,#MM_DDRPHYCR]

	orr	r1, ip, $(1<<15)
	str	r1, [r5,#MM_SDBCR]

	/* Program SDRAM TIM-0 Config Register */
	BigMov	r0, SDTIM0_VAL_162MHz
	str	r0, [r5,#MM_SDTIMR]

	/* Program SDRAM TIM-1 Config Register */
	BigMov	r0, SDTIM1_VAL_162MHz
	str	r0, [r5,#MM_SDTIMR2]

	/* Program the SDRAM Bank Config Control Register */
	bic	r0,r1,#1<<15		//Lock timing registers
	str	r0, [r5,#MM_SDBCR]

	/* Program SDRAM SDREF Config Register */
	BigMov	r0,C_SDREF_VAL
	str	r0, [r5,#MM_SDRCR]

	/*------------------------------------------------------*
	 * Issue Soft Reset to DDR Module			*
	 *------------------------------------------------------*/

	/* Issue a Dummy DDR2 read/write */
	BigMov	r1, DDR2_MEM_BASE
	BigMov	r0, DUMMY_VAL
	str	r0, [r1]
	ldr	r0, [r1]

	/* Shut down the DDR2 LPSC Module */
	ldr	r0, [r7,#M2_MDCTL_DDR2]
	bic	r0, r0, #C_PSC_FLAG_MASK
	orr	r0, r0, $0x01
	str	r0, [r7,#M2_MDCTL_DDR2]

	/* Enable the Power Domain Transition Command */
	ldr	r0, [r7,#M2_PTCMD]
	orr	r0, r0, $0x01
	str	r0, [r7,#M2_PTCMD]

	WaitForBitLow r0,r7,M2_PTSTAT,0x01		//Wait for Transition Complete(PTSTAT)
	WaitForValue r0,r7,M2_MDSTAT_DDR2,0x1f,0x01	//Wait for DDR2 Controller Enable Completion

	/*------------------------------------------------------*
	 * Turn DDR2 Controller Clocks On			*
	 *------------------------------------------------------*/
	mov	r0,#DAVINCI_LPSC_DDR2

/* Works on Always On power domain only (no PD argument) */
//In: r0 - (unsigned int id)
//	r7 - M2_BASE
lpsc_On:
	WaitForBitLow r1,r7,M2_PTSTAT,0x01	//Wait for Transition Complete(PTSTAT)
	add		r2,r7,#M2_MDSTAT_
	ldr		r1,[r2,r0,LSL #2]	//mdstat
	and		r1,r1,#0x1f
	cmp		r1,#3
	moveq	pc,lr		//return if already on and enabled

	add		r2,r7,#M2_MDCTL_
	ldr		r1,[r2,r0,LSL #2]	//mdctl
	orr		r1,r1,#3
	str		r1,[r2,r0,LSL #2]	//mdctl

#if 0
	cmp		r0,#DAVINCI_LPSC_GPIO
// Special treatment for some modules as for sprue14 p.7.4.2
	ldreq	r1,[r2,r0,LSL #2]	//mdctl
	orreq	r1,r1,#0x200
	streq	r1,[r2,r0,LSL #2]	//mdctl
#endif
	mov		r1,#1
	str		r1,[r7,#M2_PTCMD]

	WaitForBitLow r1,r7,M2_PTSTAT,0x03	//Wait for Transition Complete(PTSTAT)
	add		r2,r7,#M2_MDSTAT_
99:	ldr		r1,[r2,r0,LSL #2]	//mdstat
	and		r1,r1,#0x1f
	cmp		r1,#3
	bne		99b
	mov		pc,lr

pll2_change_routine_end:
	.globl reset_pll2
//reset_pll2(best_mult, best_div, ddr2_div);
reset_pll2:
	stmdb	sp!,{r4,r5,r6,r7,r8,ip,lr}
	adr	r4, pll2_change_routine
//skip the reset vector as a read from 0 is always branch instruction
	BigMov	r5, 0x8004	//;data IRAM address
	mov	r6, #(pll2_change_routine_end - pll2_change_routine)/4
1:	ldr	r3,[r4],#4
	str	r3,[r5],#4
	subs	r6, r6, #1
	bne	1b

	BigMov	r5, MM_BASE
	BigMov	r6, PLL_BASE
	BigMov	r7, M2_BASE
//0x0 and 0x8000 point to same 16k of TCM
//0x0 for instruction access, 0x8000 for data
	mov	r3,#4
	adr	lr,2f
	mov	pc,r3
2:
	ldmia	sp!,{r4,r5,r6,r7,r8,ip,pc}
