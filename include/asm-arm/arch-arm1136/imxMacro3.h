#include "cpMacro.h"
	.ifdef __ARMASM
	GBLA	TAGGED_LIST
	GBLA	CONFIG_ARM1136
	.endif
	.set	CONFIG_ARM1136,1
	.set	TAGGED_LIST,0x80000100		//physical address of start of list

	.ifdef __ARMASM
	GBLA STACKS_VALID
	GBLA CONFIG_STACKS_VALID
	.set	CONFIG_STACKS_VALID,1
	.endif
//force stacks valid
#define CONFIG_STACKS_VALID 1
	.set	STACKS_VALID,1

#if (SOFTWARE_TYPE==WINCE)
	.equiv	SDRAM_BASE_C_VIRTUAL, 0xA0000000		//0x80000000 is cached mapped, 0xa0000000 is uncacheable
	.equiv	UART_VIRT_BASE, 0xAA190000
	.equiv	VMA_DEBUG, (0xfff00000)
#else
	.equiv	SDRAM_BASE_C_VIRTUAL, 0xC0000000
	.equiv	UART_VIRT_BASE, 0xD4090000
//	.equiv	VMA_DEBUG, (0xff000000)
//!!!!!for some reason the above base causes bizarre problems
	.equiv	VMA_DEBUG, (0xfff00000)
#endif

	.equiv	MEM_START,	   0x80000000
	.equiv	LOCAL_RAM,     0x1fffc000
	.equiv	LOCAL_RAM_END, 0x20000000	//16k
	.equiv	NFC_RAM,	   0xb8000000
	.equiv	NFC_RAM_END,   0xb8000800	//2k
#ifndef UART_BASE
	.equiv	UART1_BASE, 0x43f90000
	.equiv	UART2_BASE, 0x43f94000
	.equiv	UART3_BASE, 0x5000c000
	.equiv	UART4_BASE, 0x43fb0000
	.equiv	UART5_BASE, 0x43fb4000
	.equiv	UART_BASE, UART1_BASE
#endif
	.equiv	UART_URXD,  0x0000
	.equiv	UART_UTXD,  0x0040
	.equiv	UART_UCR1,  0x0080
	.equiv	UART_UCR2,  0x0084
	.equiv	UART_UCR3,  0x0088
	.equiv	UART_UCR4,  0x008c
	.equiv	UART_UFCR,  0x0090
	.equiv	UART_USR1,  0x0094
	.equiv	UART_USR2,  0x0098
	.equiv	UART_UESC,  0x009c
	.equiv	UART_UTIM,  0x00a0
	.equiv	UART_UBIR,  0x00a4
	.equiv	UART_UBMR,  0x00a8
	.equiv	UART_UBRC,  0x00ac
	.equiv	UART_ONEMS, 0x00b0
	.equiv	UART_UTS,   0x00b4

	.equiv	ESD_BASE,	0xb8001000
	.equiv	ESDCTL0,	0x00
	.equiv	ESDCFG0,	0x04

	.equiv	IOMUX_BASE,	0x43fac000
	.equiv	IOMUX_SW, 0x00c				//switch control (alternate functions)
	
	.equiv	SW_REG_SD1_D1_2_3, (0x18-IOMUX_SW)		//index 3
	.equiv	SW_REG_SD1_CMD_CLK_D0, (0x1c-IOMUX_SW)	//index 4
	
	.equiv	SW_REG_VSYNC3_CONTRAST_D3REV_D3CLS,(0x20-IOMUX_SW)
	.equiv	SW_REG_HSYNC_FPSHIFT_DRDY_SDDI,(0x2c-IOMUX_SW)	//sddi=1, FPSHIFT is PCLK
	.equiv	SW_REG_LD15_16_17_VSYNC0,(0x30-IOMUX_SW)
	.equiv	SW_REG_LD11_12_13_14,(0x34-IOMUX_SW)
	.equiv	SW_REG_LD7_8_9_10,(0x38-IOMUX_SW)
	.equiv	SW_REG_LD3_4_5_6,(0x3c-IOMUX_SW)
	.equiv	SW_REG_USBH2Data1_LD0_1_2,(0x40-IOMUX_SW)
	.equiv	SW_REG_RTS1_CTS1_DTR1_DSR1,(0x7c-IOMUX_SW)
	.equiv	SW_REG_CSPI2_SCLK_CSPI2_RDY_RXD1_TXD1,(0x80-IOMUX_SW)
//MUXING on gpio1_0,1,2,3 is setup as gpios on reset
	.equiv	IOMUX_CS1_T0_CS4,	0xe8	//all chip selects muxed out correctly on reset

	.equiv	L2CC_BASE,	0x30000000
	.equiv	L2CC_CONTROL, 0x100
	.equiv	L2CC_AUXCTL,  0x104
	.equiv	L2CC_INVALIDATE,	0x77c
	.equiv	L2CC_CLEAN_INVALIDATE, 0x7fc


	.equiv	GPIO_BASE1, 0x53fcc000
	.equiv	GPIO_BASE2, 0x53fd0000
	.equiv	GPIO_BASE3, 0x53fa4000
	.equiv	GPIO_DR,	0x00
	.equiv	GPIO_DIR,	0x04
	.equiv	GPIO_PSR,	0x08
	.equiv	GPIO_ICR1,	0x0c
	.equiv	GPIO_ICR2,	0x10
	.equiv	GPIO_IMR,	0x14
	.equiv	GPIO_ISR,	0x18


	.equiv	CSCR_BASE,	0xb8002000
	.equiv	CSCR4U,	0x40	//Chip Select 4 upper
	.equiv	CSCR4L,	0x44	//Chip Select 4 lower
	.equiv	CSCR4A,	0x48	//Chip Select 4 additional control

	.equiv	CS4A_FCE,	0			//bit 0: data captured with AHB clock
	.equiv	CS4A_CNC2,	1			//bit 1: slow, maximize CS negation clock cycles 1(5-8) 0 (0,2,3, or 4) based on CNC
	.equiv	CS4A_AGE,	0			//bit 2: external DTACK glue enable
	.equiv	CS4A_WWU,	0			//bit 3: prevent wrap during write access
	.equiv	CS4A_DCT,	3			//bit 5-4: DTACK check delay time
	.equiv	CS4A_DWW,	0			//bit 7-6: Decrease write wait state
	.equiv	CS4A_LBA,	0			//bit 9-8: # of AHB half clocks to wait before LBA assertion
	.equiv	CS4A_LBN,	0			//bit 12-10: # of AHB half clocks between LBA negation and CS negation
	.equiv	CS4A_LAH,	0			//bit 14-13: Address hold time after LBA negation
	.equiv	CS4A_MUM,	0			//bit 15: 0 is non-muxed mode
	.equiv	CS4A_RWN,	0			//bit 19-16: half AHB cycles between EB negation and end of accesson writes
	.equiv	CS4A_RWA,	1			//bit 23-20: half AHB cycles RW is delayed from CS
	.equiv	CS4A_EBRN,	0			//bit 27-24: half AHB cycles between EB negation and end of access on reads
	.equiv	CS4A_EBRA,	1			//bit 31-28: half AHB cycles EB is delayed from CS

	.equiv	CS4A_DEFAULT,	((CS4A_EBRA<<28)|(CS4A_EBRN<<24)|(CS4A_RWA<<20)|(CS4A_RWN<<16)|(CS4A_MUM<<15)|(CS4A_LAH<<13)|(CS4A_LBN<<10)|(CS4A_LBA<<8)|(CS4A_DWW<<6)|(CS4A_DCT<<4)|(CS4A_WWU<<3)|(CS4A_AGE<<2)|(CS4A_CNC2<<1)|(CS4A_FCE<<0))

	.equiv	CS4L_CSEN,	1		//bit 0: enabled
	.equiv	CS4L_WRAP,	0		//bit 1: not using wrap mode
	.equiv	CS4L_CRE,	0		//bit 2: control register enble, A23 state for write access time (PSR also 1)
	.equiv	CS4L_PSR,	0		//bit 3: Pseudo SRAM Enable
	.equiv	CS4L_CSN,	1		//bit 7-4: Chip Select Negate, Half AHB ccles between CS negation and end of access
	.equiv	CS4L_DSZ,	5		//bit 10-8:  5 means 16 bit port
	.equiv	CS4L_EBC,	0		//bit 11: Enable Byte Control, 0- both read and write assert EB[3:0]
	.equiv	CS4L_CSA,	1		//bit 15-12: Half AHB cycles between Address stable and CS asserted
	.equiv	CS4L_EBWN,	0		//bit 19-16: Half AHB cycles between EB negation and end of access
	.equiv	CS4L_EBWA,	0		//bit 23-20: Half AHB cycles before EB asserted
	.equiv	CS4L_OEN,	0		//bit 27-24: Half AHB cycles between OE negation and end of access
	.equiv	CS4L_OEA,	2		//bit 31-28: Half AHB cycles before OE asserted

	.equiv	CS4L_DEFAULT,	((CS4L_OEA<<28)|(CS4L_OEN<<24)|(CS4L_EBWA<<20)|(CS4L_EBWN<<16)|(CS4L_CSA<<12)|(CS4L_EBC<<11)|(CS4L_DSZ<<8)|(CS4L_CSN<<4)|(CS4L_PSR<<3)|(CS4L_CRE<<2)|(CS4L_WRAP<<1)|(CS4L_CSEN<<0))

	.equiv	CS4U_EDC,	0		//bit 3-0: Extra Dead Cycles, prevent bus contention
	.equiv	CS4U_WWS,	0		//bit 6-4: additional write wait states
	.equiv	CS4U_EW,	0		//bit 7: ECB/WAIT,
	.equiv	CS4U_WSC,	11		//bit 13-8: Wait State Control, # cycles before ECB checked to terminate
	.equiv	CS4U_CNC,	2		//bit 15-14: Chip select negation Clock Cycles
	.equiv	CS4U_DOL,	11		//bit 19-16: Data Output Length, wait states during burst access
	.equiv	CS4U_SYNC,	0		//bit 20: Synchronous Burst Mode Enable
	.equiv	CS4U_PME,	0		//bit 21: Page Mode Emulation
	.equiv	CS4U_PSZ,	1		//bit 23-22: Page Size, 8 word burst
	.equiv	CS4U_BCS,	0		//bit 27-24: Burst Clock Start, # half cycles after address assertion before rising BCLK
	.equiv	CS4U_BCD,	0		//bit 29-28: Burst Clock Divisor, by 1
	.equiv	CS4U_WP,	0		//bit 30: Write Protect, 0-writes are allowed
	.equiv	CS4U_SP,	0		//bit 31: Supervisor Protect, 0-user mode access allowed


	.equiv	CS4U_DEFAULT,	((CS4U_SP<<31)|(CS4U_WP<<30)|(CS4U_BCD<<28)|(CS4U_BCS<<24)|(CS4U_PSZ<<22)|(CS4U_PME<<21)|(CS4U_SYNC<<20)|(CS4U_DOL<<16)|(CS4U_CNC<<14)|(CS4U_WSC<<8)|(CS4U_EW<<7)|(CS4U_WWS<<4)|(CS4U_EDC<<0))

//No mini instruction cache, so don't need to emulate
.macro EmulateJmpFIQ rWork,rBranch
.endm
.macro EmulateJmpIRQ rWork,rBranch
.endm


//Out: Z-1 breakpoint
.macro ResetVectorEntry MainInitCode
	b	\MainInitCode	//this can be in the main instruction cache not locked
					//because it is only executed upon reset
.endm

.macro HandleRestoreBP rRestoreBP
.endm

//Out: r1 - Method of entry
.macro GetMethodOfEntry
	CP14_DSCR mrc,sp
//	str	sp,[r0,#DBG_Temp]	//the mcr CP14_DCSR instruction below will trash sp in SDS (special debug state)
	and	r1,sp,#0x1c
	orr	sp,sp,#0x1c
	CP14_DSCR mcr,sp		//change MOE to reserved
	mov	r1,r1,LSR #2
.endm

//Out: Z-1 yes
.macro IsInstructionBreakpoint
	CP14_DSCR mrc,sp
	and	sp,sp,#0x1c
	cmp	sp,#1<<2		//Instruction breakpoint
.endm

.macro GetIfEq_DCSR
	CP14_DSCR mrceq,r0
.endm

.macro StoreIfEq_DCSR
	CP14_DSCR mcreq,r0
.endm

//Out: z-0 abort happened
.macro ClearDCSR_ABORT rTmp
//sorry, this is read only on imx, cleared through jtag
	movs	\rTmp,#0	//z-1, assume no abort
.endm


.macro InitCS0_CS1	rBase,rTemp
.endm

	.equiv	SW_IO_FUNC,((1<<4)|2)		//function for input and output
	.equiv	SW_OUT_FUNC, (1<<4)
.macro InitGPIO rBase,rTemp
	BigMov	\rBase,IOMUX_BASE
//setup sdhc1 signals
	BigMov	\rTemp,(SW_IO_FUNC<<24)|(SW_IO_FUNC<<16)|(SW_IO_FUNC<<8)|1
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_SD1_D1_2_3]
	mov		\rTemp,\rTemp,LSR #8
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_SD1_CMD_CLK_D0]
//setup UART1 pins
	BigMov	\rTemp,(SW_IO_FUNC<<24)|(SW_OUT_FUNC<<16)
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_RTS1_CTS1_DTR1_DSR1]
	mov		\rTemp,\rTemp,LSR #16
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_CSPI2_SCLK_CSPI2_RDY_RXD1_TXD1]
	
//setup LCD pins
	BigMov	\rTemp,(SW_OUT_FUNC<<24)
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_VSYNC3_CONTRAST_D3REV_D3CLS]
	BigOrr2	\rTemp,(SW_OUT_FUNC<<16)|(SW_OUT_FUNC<<8)|1
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_HSYNC_FPSHIFT_DRDY_SDDI]	//sddi=1, FPSHIFT is PCLK, DRDY is LBIAS (enab)
	BigBic2	\rTemp,1
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_LD15_16_17_VSYNC0]
	BigOrr2	\rTemp,(SW_OUT_FUNC<<0)
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_LD11_12_13_14]
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_LD7_8_9_10]
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_LD3_4_5_6]
	BigBic2	\rTemp,(SW_OUT_FUNC<<24)
	str		\rTemp,[\rBase,#IOMUX_SW+SW_REG_USBH2Data1_LD0_1_2]

//MUXING on gpio1_0,1,2,3 is setup as gpios on reset
	BigMov	r4,GPIO_BASE1
	mov		r1,#0x0				//set all as input
	str		r1,[r4,#GPIO_DIR]
.endm
	.equiv	Fref_K,32768		//Fref = 32768*1024 = 33,554,432 = 33.5M
//					   HSP	NFC IPG HCLK MCU
//PDR0 0xff870b48 : 00 001  011 01  001  000
//NOTE:!!! MFI  must be >= 5, 5 : 15
//NOTE:!!! MFN can be negative, -512 : 511
//NOTE:!!! if (MFN) if ((abs(MFN/MFD)<0.1)||(abs(MFN/MFD)>0.9) BRMO should be set
#if (CPU_CLOCK==100)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(4-1)	//Fref/(PD+1) = input frequency
	.equiv	MP_MFi,6
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 100.663296MHz
	.equiv	HCLK_PODF,(1-1)
	.equiv	HSP_PODF,(1-1)
#else
#if (CPU_CLOCK==104)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(4-1)
	.equiv	MP_MFi,6	//continued fraction for 6 +.198883056640625 (104000000/(32768*1024*2/4))
//   5  35   1   1   1   1    2    1    2     1     2
//0  1  35  36  71 107 178  463  641 1745  2386  6517
//1  5 176 181 357 538 895 2328 3223 8774 11997 32768
	.equiv	MP_MFn,178
	.equiv	MP_MFd,(895-1)		//yields 103.999993MHz
	.equiv	HCLK_PODF,(1-1)
	.equiv	HSP_PODF,(1-1)
#else
#if (CPU_CLOCK==200)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(2-1)
	.equiv	MP_MFi,6
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 201.326592MHz
	.equiv	HCLK_PODF,(2-1)
	.equiv	HSP_PODF,(2-1)
#else
#if (CPU_CLOCK==208)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(2-1)
	.equiv	MP_MFi,6	//continued fraction for 6 +.198883056640625 (208000000/(32768*1024*2/2))
	.equiv	MP_MFn,178
	.equiv	MP_MFd,(895-1)	//yields 207.999987MHz
	.equiv	MP_PD,(1-1)

	.equiv	HCLK_PODF,(2-1)
	.equiv	HSP_PODF,(2-1)
#else
#if (CPU_CLOCK==300)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(2-1)
	.equiv	MP_MFi,9
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 301.989888MHz
	.equiv	HCLK_PODF,(3-1)
	.equiv	HSP_PODF,(3-1)
#else
#if (CPU_CLOCK==312)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(2-1)
	.equiv	MP_MFi,9	//continued fraction for 9 +.2983245849609375 (312000000/(32768*1024*2/2))
//   3 2  1  5   3   1   2   1    1    2    16
//0  1 2  3 17  54  71 196 267  463 1193 19551
//1  3 7 10 57 181 238 657 895 1552 3999 65536
	.equiv	MP_MFn,267
	.equiv	MP_MFd,(895-1)	//yields 311.999981MHz
	.equiv	HCLK_PODF,(3-1)
	.equiv	HSP_PODF,(3-1)
#else
#if (CPU_CLOCK==400)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(1-1)
	.equiv	MP_MFi,6
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 402.653184MHz
	.equiv	HCLK_PODF,(4-1)
	.equiv	HSP_PODF,(4-1)
#else
#if (CPU_CLOCK==416)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(1-1)
	.equiv	MP_MFi,6	//continued fraction for 6 +.198883056640625 (416000000/(32768*1024*2))
//   5  35   1   1   1   1    2    1    2     1   2
//0  1  35  36  71 107 178  463  641 1745  2386  6517
//1  5 176 181 357 538 895 2328 3223 8774 11997 32768
	.equiv	MP_MFn,178
	.equiv	MP_MFd,(895-1)	//yields 415.999974MHz
	
	.equiv	HCLK_PODF,(4-1)
	.equiv	HSP_PODF,(4-1)
#else
#if (CPU_CLOCK==500)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(2-1)
	.equiv	MP_MFi,15
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 503.316480MHz
	.equiv	HCLK_PODF,(5-1)
	.equiv	HSP_PODF,(5-1)
#else
#if (CPU_CLOCK==520)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(1-1)
	.equiv	MP_MFi,7	//continued fraction for 7 +.74860382080078125 (520000000/(32768*1024*2))
//   1 2 1  44    66     1     1      5
//0  1 2 3 134  8847  8981 17828  98121
//1  1 3 4 179 11818 11997 23815 131072
	.equiv	MP_MFn,134
	.equiv	MP_MFd,(179-1)	//yields 519.999968MHz
	.equiv	HCLK_PODF,(5-1)
	.equiv	HSP_PODF,(5-1)
#else
#if (CPU_CLOCK==600)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(1-1)
	.equiv	MP_MFi,9
	.equiv	MP_MFn,0
	.equiv	MP_MFd,0	//yields 603.979776MHz
	.equiv	HCLK_PODF,(6-1)
	.equiv	HSP_PODF,(6-1)
#else
#if (CPU_CLOCK==624)
	.equiv	MP_BRMO,0
	.equiv	MP_PD,(1-1)
	.equiv	MP_MFi,9	//continued fraction for 9 +.2983245849609375 (624000000/(32768*1024*2))
//   3 2  1  5   3   1   2   1    1    2    16
//0  1 2  3 17  54  71 196 267  463 1193 19551
//1  3 7 10 57 181 238 657 895 1552 3999 65536
	.equiv	MP_MFn,267
	.equiv	MP_MFd,(895-1)	//yields 623.999962MHz
	.equiv	HCLK_PODF,(6-1)
	.equiv	HSP_PODF,(6-1)
#endif	//624
#endif	//600
#endif	//520
#endif	//500
#endif	//416
#endif	//400
#endif	//312
#endif	//300
#endif	//208
#endif	//200
#endif	//104
#endif	//100
	.equiv	MP_VAL,(MP_BRMO<<31)|(MP_PD<<26)|(MP_MFd<<16)|(MP_MFi<<10)|MP_MFn		//MPCTL=0x04001800
// Fref x 2 x ((MFi + MFn/(MFd+1))/(PD+1)) = Fvco
// Fref_K x ((MFi*2048 + MFn*2048/(MFd+1))/(PD+1)) = Fvco
	.equiv	MP_CLKRATE,(Fref_K*( (MP_MFi*2048)+( (MP_MFn*2048)/(MP_MFd+1))))/(MP_PD+1)	//33.5M x 2 x (6/2) = 201326592 Hz = 201.3 MHz
	.equiv	NFC_PODF,(4-1)
	.equiv	IPG_PODF,(2-1)
	.equiv	MCU_PODF,(1-1)

//USB PLL control
	.equiv	UP_MFi,7
	.equiv	UP_MFd,58
	.equiv	UP_MFn,9		//MFn <= MFd
	.equiv	UP_PD,1
	.equiv	UP_VAL,(UP_PD<<26)|(UP_MFd<<16)|(UP_MFi<<10)|UP_MFn		//was UPCTL=0x04112014
	.equiv	UP_CLKRATE,(Fref_K*( (UP_MFi*2048)+( (UP_MFn*2048)/(UP_MFd+1))))/(UP_PD+1)	//about 240MHz

	.equiv	SP_MFi,12
	.equiv	SP_MFd,4
	.equiv	SP_MFn,1		//MFn <= MFd
	.equiv	SP_PD,1
	.equiv	SP_VAL,(SP_PD<<26)|(SP_MFd<<16)|(SP_MFi<<10)|SP_MFn		//SPCTL=0x04043001
	.equiv	SP_CLKRATE,(Fref_K*( (SP_MFi*2048)+( (SP_MFn*2048)/(SP_MFd+1))))/(SP_PD+1)	//33.5M x 12.2 = 409364070 Hz = 409.3 MHz

	.equiv	PER_PODF,(8-1)	//this is reset value in CLK_PDR0
	.equiv	PERIF_CLOCK,(SP_CLKRATE/(PER_PODF+1))	//409.3/(7+1) = 51.170508 MHz
	.equiv	PDR0_VAL,(0xff800000|MCU_PODF|(HCLK_PODF<<3)|(IPG_PODF<<6)|(NFC_PODF<<8)|(HSP_PODF<<11)|(PER_PODF<<16))

	.equiv	CLOCK_BASE,0x53f80000 //Romval   reset value
	.equiv	CLK_CCMR, 0x00		//0x074b0b7b 0x074b0b79 choose FPM clock source
	.equiv	CLK_PDR0, 0x04		//0xff870b48 same
	.equiv	CLK_PDR1, 0x08		//0x09fcfe7f 0x49fcfe7f USB pre divider
	.equiv	CLK_RCSR, 0x0c		//0x007f0000 same, need to set NANDF 16 bit, 2K Page
	.equiv	CLK_MPCTL,0x10		//0x04001800 same
	.equiv	CLK_UPCTL,0x14		//0x04112014 0x04051c03 USB PLL
	.equiv	CLK_SPCTL,0x18		//0x04043001 same
	.equiv	CLK_COSR, 0x1c		//0x00000280 same
	.equiv	CLK_CGR0, 0x20		//0xffffffff same
	.equiv	CLK_CGR1, 0x24		//0xffffffff same
	.equiv	CLK_CGR2, 0x28		//0xffffffff same
	.equiv	CLK_WIMR, 0x2c		//0xffffffff same
	.equiv	CLK_LDC,  0x30		//0x0000000f same
	.equiv	CLK_DCVR0,0x34		//0x00000000 same
	.equiv	CLK_DCVR1,0x38		//0x00000000 same
	.equiv	CLK_DCVR2,0x3c		//0x00000000 same
	.equiv	CLK_DCVR3,0x40		//0x00000000 same
	.equiv	CLK_LTR0, 0x44		//0x00000000 same
	.equiv	CLK_LTR1, 0x48		//0x00004040 same
	.equiv	CLK_LTR2, 0x4c		//0x00000000 same
	.equiv	CLK_LTR3, 0x50		//0x00000000 same
	.equiv	CLK_LTBR0,0x54		//0x00000000 same
	.equiv	CLK_LTBR1,0x58		//0x00000000 same
	.equiv	CLK_PMCR0,0x5c		//0x802d9828 same
	.equiv	CLK_PMCR1,0x60		//0x00aa0000 same
	.equiv	CLK_PDR2, 0x64		//0x00000285 same

.macro InitIC_Clocks rBase,rTemp
	BigMov	\rBase,CLOCK_BASE
	BigMov	\rTemp,UP_VAL
	str		\rTemp,[\rBase,#CLK_UPCTL]
.endm
.macro IPL_InitIC_Clocks rBase,rTemp
	ldr		\rTemp,[\rBase,#CLK_CCMR]
	bic		\rTemp,\rTemp,#0x8
	str		\rTemp,[\rBase,#CLK_CCMR]	//disable MCU PLL
	bic		\rTemp,\rTemp,#0xe
	orr		\rTemp,\rTemp,#0x3
	str		\rTemp,[\rBase,#CLK_CCMR]	//switch to FPM
	orr		\rTemp,\rTemp,#0xb
	str		\rTemp,[\rBase,#CLK_CCMR]	//reenable
.endm

.macro	InitChangeCPUSpeed rBase,rTemp,rTemp2
.endm

.macro	FinalInitChangeCPUSpeed rBase,rTemp,rTemp2
	.if 1
//	BigMov	\rBase,ESD_BASE
//	ldr		\rTemp,[\rBase,#ESDCTL0]
//	bic		\rTemp,\rTemp,0xf0000000
//	orr		\rTemp,\rTemp,0xa0000000
//	str		\rTemp,[\rBase,#ESDCTL0]

	BigMov	\rBase,CLOCK_BASE
	movs	\rTemp,#0	//set Z, instructions not in cache yet

	ldr		\rTemp,[\rBase,#CLK_CCMR]
98:	bic		\rTemp,\rTemp,#0x8
	orr		\rTemp,\rTemp,#0x80
	BigMov	\rTemp2,0xff800000|(IPG_PODF<<6)	//IPG should not = HCLK
	strne	\rTemp,[\rBase,#CLK_CCMR]	//disable MCU PLL
	strne	\rTemp2,[\rBase,#CLK_PDR0]	//speed up the slow clocks while waiting for LOCK
	BigMov	\rTemp2,MP_VAL
	strne	\rTemp2,[\rBase,#CLK_MPCTL]

	orr		\rTemp,\rTemp,#0x8
	strne	\rTemp,[\rBase,#CLK_CCMR]	//reenable pll
	bicne	\rTemp,\rTemp,#0x80			//select pll as source
//wait for lock
	mov		\rTemp2,#0x1000
99:	subnes	\rTemp2,\rTemp2,#1			//delay after stores
	bne		99b
	tst		\rTemp,#0x80	//pll as source?
	BigMov	\rTemp2,PDR0_VAL
	streq	\rTemp2,[\rBase,#CLK_PDR0]
	streq	\rTemp,[\rBase,#CLK_CCMR]	//maybe select pll as source
	bne		98b							//br if 1st time through
90:
	.endif
.endm

.macro InitUART rBase,rTemp,uartaddr,baudrate
//setings for 115200,8,o,2
	BigMov	\rBase,\uartaddr
	mov		\rTemp,#0x01
	str		\rTemp,[\rBase,#UART_UCR1]
	mov		\rTemp,#0x20
	str		\rTemp,[\rBase,#UART_UTS]
	BigMov	\rTemp,0x0a81
	str		\rTemp,[\rBase,#UART_UFCR]

//Baudrate = PERIF_CLOCK/(16 * (UBMR+1)/(UBIR+1))
	BigMov	\rTemp,0x000f
	str		\rTemp,[\rBase,#UART_UBIR]
	BigMov	\rTemp,(PERIF_CLOCK/\baudrate)	//49M/115200 = 425.34 (0x01a9), 51M/115200 = 444(0x01bc)
	str		\rTemp,[\rBase,#UART_UBMR]

	.equiv	UCR2_IGNORE_RTS,(1<<14)
	.equiv	UCR2_CTS,(1<<13)			//receiver level controls CTS pin level
	.equiv	UCR2_PARITY_NONE,(0<<7)
	.equiv	UCR2_PARITY_EVEN,(2<<7)
	.equiv	UCR2_PARITY_ODD,(3<<7)
	.equiv	UCR2_1STOP_BITS,(0<<6)
	.equiv	UCR2_2STOP_BITS,(1<<6)
	.equiv	UCR2_DATA7,(0<<5)
	.equiv	UCR2_DATA8,(1<<5)
	.equiv	UCR2_TXEN,(1<<2)
	.equiv	UCR2_RXEN,(1<<1)
	.equiv	UCR2_NORESET,(1<<0)
	.if 1
	.equiv	UCR2_VAL,UCR2_IGNORE_RTS|UCR2_CTS|UCR2_PARITY_NONE|UCR2_1STOP_BITS|UCR2_DATA8|UCR2_TXEN|UCR2_RXEN|UCR2_NORESET
	.else
	.equiv	UCR2_VAL,UCR2_IGNORE_RTS|UCR2_CTS|UCR2_PARITY_ODD|UCR2_2STOP_BITS|UCR2_DATA8|UCR2_TXEN|UCR2_RXEN|UCR2_NORESET
	.endif
		
	BigMov	\rTemp,UCR2_VAL
	str		\rTemp,[\rBase,#UART_UCR2]
	
	mov		\rTemp,#4
	str		\rTemp,[\rBase,#UART_UCR3]
	BigMov	\rTemp,0x8000
	str		\rTemp,[\rBase,#UART_UCR4]
.endm

.macro InitUARTs rBase,rTemp,baudrate1,baudrate2,baudrate3
	InitUART	\rBase,\rTemp,UART_BASE,\baudrate1
.endm

	.equiv	RCSR_SLEEP_RESET, 4

//out rTemp1 reset reason
.macro InitPWR rBase,rTemp1,rTemp2
	mov		\rTemp1,#0
.endm

.macro EnableCoprocessorAccess rTmp
	CP15_CP_ACCESS_CONTROL	mrc,\rTmp
	BigOrr2 \rTmp,0x0fffffff
	CP15_CP_ACCESS_CONTROL mcr,\rTmp		//enable access to all coprocessors
.endm

.macro InitMMU rBase,rVal
//	mov	\rBase,#PSR_NOINTS_MASK+PSR_MODE_SVC
//	msr	cpsr_c,\rBase
// ********************************************************************
// Disable the MMU and gang regardless of why we are here.
	CP15_CP_ACCESS_CONTROL	mrc,\rBase
	BigOrr2 \rBase,0x003
	CP15_CP_ACCESS_CONTROL	mcr,\rBase
	CPWAIT \rBase

	mov	\rBase,#0x00000078			//; get a zero to turn things off (must write bits[6:3] as 1's)
	CP15_CONTROL	mcr,\rBase		//; Turn Off MMU, I&D Caches, WB.
	CPWAIT \rBase 

	mov	\rBase,#0x00000000         //; get a zero to turn things off
	cmp	pc,#MEM_START
	CP15_TLB_INVAL_UNIFIED_ALL	mcr,\rBase	//; flush (invalidate) I/D tlb's
	CP15_CF_INVAL_BTB	mcr,\rBase	//invalidate Branch target buffer
	CP15_CF_INVAL_D		mcr,\rBase
	CP15_CF_INVAL_I		mcrcs,\rBase		//I might be running from cache only, invalidate if in ram
	CP15_CF_DRAIN		mcr,\rBase		//; Drain the write buffer
	nop
	nop
	nop
	mvn	\rBase, #0					//; grant manager access to all domains
	CP15_DACR	mcr,\rBase


	BigMov	\rBase,L2CC_BASE
	ldr		\rVal,[\rBase,#L2CC_CONTROL]
	tst		\rVal,#1
	bne		99f						//br if L2 cache is enabled
	BigMov	\rVal,0x0003001b		//16k way size,8-way associativity, 1 clk dirty,4 clk data latency (400)mhz
	str		\rVal,[\rBase,#L2CC_AUXCTL]
	mov		\rVal,#0xff
	str		\rVal,[\rBase,#L2CC_INVALIDATE]
98:	ldr		\rVal,[\rBase,#L2CC_INVALIDATE]
	cmp		\rVal,#0
	bne		98b
	mov		\rVal,#1
	str		\rVal,[\rBase,#L2CC_CONTROL]	//enable L2 cache
99:	
.endm



//d b8001004 79e73a
//d b8001000 92126180
//d 80000f00 92126180
//d b8001000 a2126180
//d 80000000 a2126180
//d 80000000 92126180
//d 80000000 82126180
//d 80000000 72126180
//d 80000000 62126180
//d 80000000 52126180
//d 80000000 42126180
//d 80000000 32126180
//d b8001000 22126180
//d b8001000 12126180
//d b8001000 02126180
//d 80000037 80
//d b8001000 82126180

.macro InitRam	rBase,rVal,rMem
	BigMov	\rBase,ESD_BASE
//esdcfg0: 4 clocks before new command,2 clocks changing from write to read,
//3 clocks row precharge,2 clocks after load mode,2clocks write to precharge,
//7 clocks active to precharge,2 clocks switching banks,3 clocks CAS latency
//4 clocks row to column delay,10 clocks row cycle delay
	BigMov	\rVal,0x0079e73a
	str		\rVal,[\rBase,#ESDCFG0]

//esdctl0: sde-enabled,pre-charge,13 rows, 9 cols
	BigMov	\rVal,0x92126180		//SMODE=001
	str		\rVal,[\rBase,#ESDCTL0]

	BigMov	\rMem,MEM_START
	str		\rVal,[\rMem,#0xf00]	//make sure A10 on SDRAM is high for charging all

//esdctl0: auto refresh
	eor		\rVal,\rVal,#0xa0000000^0x90000000	//change 9 to A (SMODE=010)
	str		\rVal,[\rBase,#ESDCTL0]

90:	str		\rVal,[\rMem]		//start auto-refresh cycle, 11 writes
	subs	\rVal,\rVal,#0x10000000
	bhs		90b					//br if NO borrow (c-1)

//esdctl0: load mode register
	eor		\rVal,\rVal,#0xb0000000^0xf0000000	//change F to B(SMODE=011)
	str		\rVal,[\rBase,#ESDCTL0]

	strb	\rVal,[\rMem,#0x37]		//set mode register with address line values
									//Full page burst, 3 cas latency
//	BigMov	\rVal,0xff
//	BigMov	\rMem,MEM_START+(16<<20)
//	strb	\rVal,[\rMem]			//huh?????


//esdctl0: sde-enabled,normal,13 rows,9 cols, 32bit width, 
//4 rows each refresh clock,full page burst, burst of 8,
	eor		\rVal,\rVal,#0x80000000^0xb0000000	//change B to 8
	str		\rVal,[\rBase,#ESDCTL0]
	
//	BigMov	\rMem,MEM_START
//	str		\rVal,[\rMem]
.endm

.macro InitMemory	rBase,rVal,rMem
	cmp	pc,#MEM_START+(512<<20)		//sdram is max of 512 MEG
	bhs	1f
	cmp	pc,#MEM_START
	bhs	99f		//skip if running from ram
1:
	InitRam	\rBase,\rVal,\rMem

99:
	BigMov	\rBase,CSCR_BASE
	BigMov	\rVal,CS4A_DEFAULT
	str		\rVal,[\rBase,#CSCR4A]
	BigMov	\rVal,CS4U_DEFAULT
	str		\rVal,[\rBase,#CSCR4U]
	BigMov	\rVal,CS4L_DEFAULT
	str		\rVal,[\rBase,#CSCR4L]
.endm


.macro TransmitChar rChar,rUART,rTmp
111:
	ldr	\rTmp,[\rUART,#UART_USR1]
	ands	\rTmp,\rTmp,#1<<13
	beq	111b
	and	\rTmp,\rChar,#0xff
	str	\rTmp,[\rUART,#UART_UTXD]
.endm

.macro WaitUartTxEmpty rUART,rTmp
111:
	ldr		\rTmp,[\rUART,#UART_USR2]
	tst		\rTmp,#1<<3
	beq		111b
.endm

.macro	DEBUG_CHAR	Char
	BigMov	ip,UART_BASE
	BigMov	r0,\Char
	TransmitChar	r0,ip,r1
	WaitUartTxEmpty ip,r0
.endm

//Out: z-0 if good char
.macro ReceiveChar rChar,rUART
	ldr		\rChar,[\rUART,#UART_USR2]
	ands	\rChar,\rChar,#0x1
	ldrne	\rChar,[\rUART,#UART_URXD]
	andne	\rChar,\rChar,#0xff
.endm

.macro ClearUart_DLAB rUART,rtDBG,rTmp
//no DLAB, so nothing to do
.endm

.macro RestoreUart_DLAB rUART,rtDBG,rTmp
//no DLAB, so nothing to do
.endm

.macro Get_FP0_reg	rLow,rHigh
//no FP0, so return 0
	mov	\rLow,#0
	mov	\rHigh,#0
.endm

.macro	Set_FP0_reg rLow,rHigh
//no FP0, so nothing to do
.endm

.macro	EnableFlashReadMode rTemp0,rTemp1,rUART
.endm

//Out: z-0 means 16 bit width, z-1 : 32 bit width
.macro GetFlashWidth rFlash,rTmp
	movs	\rTmp,#0
.endm

.macro ResetTLB rTmp
	CP15_TLB_INVAL_UNIFIED_ALL mcr,\rTmp
.endm

.macro ResetInstructionCache rTmp
.endm

.macro ResetDataCache rTmp
	CP15_CF_INVAL_D mcr,\rTmp		//this will also drain write buffer
.endm

.macro Lock_TLB_DENTRY rEntry,rTmp
	BigBic	\rTmp,\rEntry,0x3ff
	CP15_TLB_INVAL_UNIFIED_SINGLE mcr,\rTmp
	CP15_TLB_LOCK mrc,\rTmp
	orr	\rTmp,\rTmp,#1
	CP15_TLB_LOCK mcr,\rTmp

	ldr	\rTmp,[\rEntry]

	CP15_TLB_LOCK mrc,\rTmp	//victim will have incremented
	bic	\rTmp,\rTmp,#1
	CP15_TLB_LOCK mcr,\rTmp
.endm

.macro Lock_Data_Cache_Range	rStart,rCacheLines,rTmp
.endm

.macro	CleanInvalidateDataCache	rBase,rVal
	BigMov	\rBase,L2CC_BASE
	ldr		\rVal,[\rBase,#L2CC_CONTROL]
	tst		\rVal,#1
	beq		99f						//br if L2 cache is disabled
	mov		\rVal,#0xff
	str		\rVal,[\rBase,#L2CC_CLEAN_INVALIDATE]
98:	ldr		\rVal,[\rBase,#L2CC_CLEAN_INVALIDATE]
	cmp		\rVal,#0
	bne		98b
99:	
	CP15_CF_CLEAN_INVAL_DCACHE	mcr,\rBase
.endm
.macro	CleanInvalidateDataCache1	rBase,rVal,rDbg
//	CleanInvalidateDataCache \rBase,\rVal
//Virtual memory is probably on but 1 to 1 mapping for L2 cache control
//may not be. So, don't worry about L2 cache
	CP15_CF_CLEAN_INVAL_DCACHE	mcr,\rBase
.endm

.macro	DisableL2Cache	rBase,rVal
	BigMov	\rBase,L2CC_BASE
	mov		\rVal,#0
	str		\rVal,[\rBase,#L2CC_CONTROL]	//disable L2 cache
.endm

//rTmp is trashed
.macro	InvalidateICacheRange rStart,rEnd,rTmp
	sub	\rTmp,\rEnd,#1
	CP15_CF_INVAL_ICACHE_RANGE \rStart,\rTmp
.endm
//	.equiv	MONITOR_MODE, 1

//rNum is 0 to 5, only 4,5 can hold context ids
.macro CheckForAndRemoveBkptNum rPc,rTmp1,rTmp2,rTmp3,rMatch,rNum
	.ifdef MONITOR_MODE
	CP14_BKPT mrc,\rTmp2,\rNum
	eor	\rTmp1,\rPc,\rTmp2
	bics	\rTmp1,\rTmp1,#3
	CP14_BKPT_CONTROL mrceq,\rTmp2,\rNum
	eoreq	\rTmp2,\rTmp2,#1
	tsteq	\rTmp2,#1
	CP14_BKPT_CONTROL mcreq,\rTmp2,\rNum
	biceq	\rMatch,\rMatch,#1
	.endif
.endm

//Out: z-1 at breakpoint, rTmp1 bkpt enable value
.macro CheckForAndRemoveBkpt rPc,rTmp1,rTmp2,rTmp3,rMatch
	mov		\rMatch,#1
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,0
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,1
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,2
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,3
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,4
	CheckForAndRemoveBkptNum	\rPc,\rTmp1,\rTmp2,\rTmp3,\rMatch,5
	tst	\rMatch,#1
	mov		\rTmp1,\rPc
.endm

//rNum is 0 to 5
//Out:Z-1 done
.macro AddBkptNum	rVal,rTmp2,rCtl,rNum
	CP14_BKPT_CONTROL mrc,\rTmp2,\rNum
	tst	\rTmp2,#1
	CP14_BKPT mcreq,\rVal,\rNum
	CP14_BKPT_CONTROL mcreq,\rCtl,\rNum
.endm

.macro AddBkpt	rPc,rTmp1,rTmp2,rCtl,rDbg,trace
	.ifdef MONITOR_MODE
	BigMov	\rCtl,0x1e7
	bic		\rTmp1,\rPc,#3
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,1
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,2
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,3
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,4
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,5
	beq	99f

	CP14_BKPT_CONTROL mrc,\rTmp2,0	//only 4,5 can hold contextID so steal 0
	tst	\rTmp2,#1
	CP14_BKPT mrcne,\rTmp2,0
	CP14_BKPT mcr,\rTmp1,0
	CP14_BKPT_CONTROL mcr,\rCtl,0
	strne	\rTmp2,[\rDbg,#\trace]	//need to steal this breakpoint momentarily
99:
	.endif
.endm


//rNum is 0 to 5
.macro CheckForBkptNum rPc,rTmp,rMatch,rNum
	.ifdef MONITOR_MODE
	CP14_BKPT mrc,\rTmp,\rNum
	eor		\rTmp,\rPc,\rTmp
	bics	\rTmp,\rTmp,#3
	CP14_BKPT_CONTROL mrceq,\rTmp,\rNum
	tsteq	\rTmp,#1
	bicne	\rMatch,\rMatch,#1
	.endif
.endm

//Out: z-1 breakpoint found
.macro	SearchBkpt rPc,rTmp0,rMatch
	mov		\rMatch,#1
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,0
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,1
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,2
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,3
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,4
	CheckForBkptNum	\rPc,\rTmp0,\rMatch,5
	tst	\rMatch,#1
.endm

//Out: z-1 breakpoint just set
.macro	SetBkpt	rPc,rTmp1,rTmp2,rCtl
	.ifdef MONITOR_MODE
	BigMov	\rCtl,0x1e7
	bic		\rTmp1,\rPc,#3
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,0
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,1
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,2
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,3
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,4
	beq	99f
	AddBkptNum	\rTmp1,\rTmp2,\rCtl,5
99:
	.else
	movs		\rTmp1,#1
	.endif
.endm

.macro PrintBkptNum rNum
	.ifdef MONITOR_MODE
	CP14_BKPT_CONTROL mrc,r0,\rNum
	tst	r0,#1
	CP14_BKPT mrcne,r0,\rNum
	blne	PrintHex
	bl	TransmitSPACE
	.endif
.endm

.macro	PrintBkpts
	PrintBkptNum 0
	PrintBkptNum 1
	PrintBkptNum 2
	PrintBkptNum 3
	PrintBkptNum 4
	PrintBkptNum 5
.endm

.macro PrintWatchNum	rNum, rwText,wText,rText
	.ifdef MONITOR_MODE
	CP14_WATCHPT_CONTROL mrc,r5,\rNum
	tst	r5,#1
	beq	99f
	BigMov	r0,0x20202020	//L4(SPACE,SPACE,SPACE,SPACE)
	tst	r5,#1<<3
	BigEor2Ne r0,0x20202020^\rText	//Load
	tst	r5,#1<<4
	BigEor2Ne r0,0x20202020^\wText	//Store
	bl	Transmit1

	CP14_WATCHPT mrc,r0,\rNum
	bl	PrintHex
99:
	.endif
.endm

.macro PrintWatchpts  rwText,wText,rText
	PrintWatchNum	0, \rwText,\wText,\rText
	PrintWatchNum	1, \rwText,\wText,\rText
.endm

.macro	WatchClear rTmp
	.ifdef MONITOR_MODE
	mov	\rTmp,#0
	CP14_WATCHPT_CONTROL mcr,\rTmp,0
	CP14_WATCHPT_CONTROL mcr,\rTmp,1
	.endif
.endm

//In: z-0 means rNum2 is valid
//rMode - 1:w, 2:rw, 3:r
.macro	SetWatchpt	rNum1,rNum2,rMode,rTmp,rCtl
	.ifdef MONITOR_MODE
	orrne	\rMode,\rMode,#(1<<20)|(4<<16)	//linked with contextID
	BigMovNe	\rTmp,0x3001e7
	CP14_BKPT mcrne,\rNum2,4	//only 4,5 can hold contextID
	CP14_BKPT_CONTROL mcrne,\rTmp,4	//only 4,5 can hold contextID

	cmp	\rMode,#2
	movhi	\rMode,#1<<3	//read
	movlo	\rMode,#2<<3	//write
	moveq	\rMode,#3<<3	//read/write

	ands	\rTmp,\rNum1,#3
	orreq	\rMode,\rMode,#0xf<<5
	movne	\rCtl,#1<<5
	orrne	\rMode,\rMode,\rCtl, LSL \rTmp
	bicne	\rNum1,\rNum1,#3
	orr		\rMode,\rMode,#7
	
	CP14_WATCHPT_CONTROL mrc,\rCtl,0
	tst		\rCtl,#1
	CP14_WATCHPT mrceq,\rNum1,0
	CP14_WATCHPT mrcne,\rNum1,1
	CP14_WATCHPT_CONTROL mcreq,\rMode,0
	CP14_WATCHPT_CONTROL mcrne,\rMode,1
	.endif
.endm

//r1 - CP15_CONTROL
.macro GetPageTableStart rTmp,physToVirtOffset
	movs	r1,r1,LSR #13+1		//assume 1:1 mapping for TTBR if relocation vector is off
								//mov bit 13 to carry flag
// see if identity mapping is enabled
	CP15_TTBR_CONTROL	mrc,r1
	tst	r1,#0x7
	CP15_TTBR0 mrceq,r1
	CP15_TTBR1 mrcne,r1
	bic	r1,r1,#0xff		//clear caching attributes, and shared memory flag
	
//  using Big will generate no instructions if Wince, throwing off my reloc vector
//	BigAdd2Ne r1,\physToVirtOffset
	addcs	r1,r1,#\physToVirtOffset	//convert this physical address to a virtual address if relocation vector ON
.endm

.macro SetupTTBR rBase,rTmp
	CP15_TTBR0 mcr,\rBase
	mov	\rTmp,#0
	CP15_TTBR_CONTROL	mrc,\rTmp
.endm

.macro	GetDebugMagicPhys	rTemp
	CalcMemSize	\rTemp,MEMORY_CONTROL_BASE				//out: \rTemp - mem size
	BigAdd	\rTemp,\rTemp,MEM_START-0x1000+((DEBUG_BASE+DBG_MAGIC)&0xfff)		//last 4k of memory
.endm

.macro	GetDebugMagic	rTemp
	CP15_CONTROL mrc,\rTemp		//get the control register
	tst	\rTemp,#0x1			//tst bit 0 - enable MMU
	BigMovNe \rTemp,DEBUG_BASE+DBG_MAGIC
	bne		98f
	GetDebugMagicPhys	\rTemp
98:
.endm


.macro	GetDebugMagicCC	cc,ncc,rTemp
	b\ncc	99f
	GetDebugMagic \rTemp
99:
.endm

.macro	GetDebugBase	rTemp
	GetDebugMagic	\rTemp
	BigSub2		\rTemp,DBG_MAGIC
.endm
.macro	GetDebugBaseStickyCheck rTemp,rEnd
//MMU is off, so use physical address, (don't lock into cache)
	BigAdd \rTemp,\rEnd,(-0x1000+((DEBUG_BASE)&0xfff)) //don't use virtual address if memory isn't working right
.endm

.macro SPS_ReturnBugsWorkAroundCode
.endm

//Out: rPhys identity mapping flag,  don't change carry flag
.macro	ReturnBugsWorkAround1	rPhys,rTmp,rSpTmp,rDbg,rUart1,uartBase
.endm

//In: z-1 use indirect return
.macro	ReturnBugsWorkAround2
	ldr		lr,[r0,#DBG_PC-DBG_R2]			//no bug to work around
.endm

	.equiv	WATCHDOG_BASE,0x53fdc000
	.equiv	WD_WCR,0x0
	
.macro ResetCPU rBase,rVal
	BigMov	\rBase,WATCHDOG_BASE
	mov		\rVal,#1<<5		//clear 1<<4, SRS bit, leave 1<<5 WDA alone
	str		\rVal,[\rBase,#WD_WCR]
.endm
