#include "cpMacro.h"
//  ************************************************************************************************
//  ************************************************************************************************
	.ifdef __ARMASM
	GBLA	CPU_PXA270
	GBLA	RomWidthIsRamWidth
	.endif
#if (PLATFORM_TYPE==HALOGEN)||(PLATFORM_TYPE==ARGON)||(PLATFORM_TYPE==NEON270)
	.set	CPU_PXA270,1
#else
	.set	CPU_PXA270,0
#endif

#if (PLATFORM_TYPE==NEONB)||(PLATFORM_TYPE==HALOGEN)||(PLATFORM_TYPE==ARGON)||(PLATFORM_TYPE==NEON270)||(PLATFORM_TYPE==NEON)
	.set	RomWidthIsRamWidth,0	//use 32 bit ram always
#else
	.set	RomWidthIsRamWidth,1
#endif
// *******************************************************************************************

	.equiv	numBankAddrBits, 2
	.equiv	ClkSelect, 1	//0 : tRP = 2, tRCD = 1, tRAS = 3, tRC = 4
			//1 : tRP = 2, tRCD = 2, tRAS = 5, tRC = 8
			//2 : tRP = 3, tRCD = 3, tRAS = 7, tRC = 10
			//3 : tRP = 3, tRCD = 3, tRAS = 7, tRC = 11
//BM big memory option
#if (PLATFORM_TYPE==NEON270)
	//64 meg option
	.equiv	SM_numColumnAddrBits, 9
	.equiv	SM_numRowAddrBits, 13	//for MT48LC8M16A2 - 75 B
	.equiv	SM_ADDRESS_TEST_MASK,(1<<(1+SM_numColumnAddrBits+SM_numRowAddrBits+numBankAddrBits))
	.equiv	SM_LARGE_MAP_ENABLE (0)
	//128 meg option
	.equiv	BM_numColumnAddrBits, 10
	.equiv	BM_numRowAddrBits, 13	//for k4s561632a
	.equiv	BM_ADDRESS_TEST_MASK,(1<<(1+BM_numColumnAddrBits))
	.equiv	BM_LARGE_MAP_ENABLE (1<<31)
#else
	//32 meg option
	.equiv	SM_numColumnAddrBits, 9
	.equiv	SM_numRowAddrBits, 12	//for MT48LC8M16A2 - 75 B
	.equiv	SM_ADDRESS_TEST_MASK,(1<<(1+SM_numColumnAddrBits+SM_numRowAddrBits+numBankAddrBits))
	.equiv	SM_LARGE_MAP_ENABLE (0)
	//64 meg option
	.equiv	BM_numColumnAddrBits, 9
	.equiv	BM_numRowAddrBits, 13	//for k4s561632a
	.equiv	BM_ADDRESS_TEST_MASK,(1<<(1+BM_numColumnAddrBits+BM_numRowAddrBits+numBankAddrBits))
	.equiv	BM_LARGE_MAP_ENABLE (0)
#endif
	.equiv	BM_SA1111_mask, 0		//(1<<12)
	.equiv	BM_DRI_cnt,  (((99530*64)>>BM_numRowAddrBits)>>5)	//(# of cycles/ms  * # of ms for entire refresh period)/ # of rows/refresh period /32
	.equiv	BM_MDCNFG_VAL, 1+((BM_numColumnAddrBits-8)<<3)+((BM_numRowAddrBits-11)<<5)+((numBankAddrBits-1)<<7)+(ClkSelect<<8)+(1<<11)+(BM_SA1111_mask)+BM_LARGE_MAP_ENABLE	//DLATCH0, latch return data with return clock
	.equiv	BM_MDREFR_VAL, (1<<16)+(1<<15)+(BM_DRI_cnt&0xfff)		//don't set bit 20: APD (buggy), bit 16: K1RUN, 15:E1PIN
//	.equiv	BM_MDREFR_VAL, (1<<20)+(1<<16)+(1<<15)+(BM_DRI_cnt&0xfff)		//20: APD, bit 16: K1RUN, 15:E1PIN
//			 13		9		  2	       2 (4bytes per address)=2**26=64 MB
	.equiv	BM_MEM_SIZE, (1<<(2+BM_numColumnAddrBits+BM_numRowAddrBits+numBankAddrBits))

//SM small memory option
	.equiv	SM_SA1111_mask, 0
	.equiv	SM_DRI_cnt,  (((99530*64)>>SM_numRowAddrBits)>>5)	//(# of cycles/ms  * # of ms for entire refresh period)/ # of rows/refresh period /32
	.equiv	SM_MDCNFG_VAL, 1+((SM_numColumnAddrBits-8)<<3)+((SM_numRowAddrBits-11)<<5)+((numBankAddrBits-1)<<7)+(ClkSelect<<8)+(1<<11)+(SM_SA1111_mask)+SM_LARGE_MAP_ENABLE	//DLATCH0, latch return data with return clock
	.equiv	SM_MDREFR_VAL, (1<<16)+(1<<15)+(SM_DRI_cnt&0xfff)		//don't set bit 20: APD (buggy), bit 16: K1RUN, 15:E1PIN
//			 12		9		  2	       2 (4bytes per address)=2**25=32 MB
	.equiv	SM_MEM_SIZE, (1<<(2+SM_numColumnAddrBits+SM_numRowAddrBits+numBankAddrBits))
// *******************************************************************************************

//In: c-0 try 64meg, c-1 try 32meg
// or if 16 bit mode
//    c-0 try 32meg, c-1 try 16meg
//Out: z-0 if 16 bit mode if RomWidthIsRamWidth
.macro InitRam	rBase,rTemp
	BigMov	\rBase,MEMORY_CONTROL_BASE
	.if RomWidthIsRamWidth
	ldr	\rTemp,[\rBase,#BOOT_DEF]
	tst	\rTemp,#1			//bit 0 - 1 means 16 bit mode
	.endif
	BigMov	\rTemp,SM_MDCNFG_VAL
	BigEor2Cc \rTemp,(SM_MDCNFG_VAL)^(BM_MDCNFG_VAL)
	.if RomWidthIsRamWidth
	BigOrr2Ne \rTemp,(1<<2)			//select 16 bit width
	.endif
//pxa270 requires the next 3 stores be in the same cache line+8 bytes??????
	.ifdef __ARMASM
	nop
	nop
	nop
	nop
	nop
	nop
	.else
	.balignl        32,0xe1a00000	//nop code
	.endif
	str	\rTemp,[\rBase,#MDCNFG]//str cannot be at cache line offset 10,14,18, or 1c
								//str can be at 0,04,08,1c

	mov	\rTemp,#0
	str	\rTemp,[\rBase,#MDMRS] 

	BigMov	\rTemp,BM_MDREFR_VAL
	BigEor2Cs \rTemp,(BM_MDREFR_VAL)^(SM_MDREFR_VAL)
	str	\rTemp,[\rBase,#MDREFR]
.endm


//In: c-0 try BM meg, c-1 try SM meg
//	  z-1 32 bit wide, z-0 16 bit wide
//out: rTemp - memory size
.macro CheckRam	rBase,rTemp,rTemp2
	// Issue read requests to disabled bank to start refresh
	BigMov	\rBase,MEM_START+0x0C000000
	ldr	\rTemp, [\rBase]
	mov	\rTemp,#SM_MEM_SIZE
	movcc	\rTemp,#BM_MEM_SIZE
	.if RomWidthIsRamWidth
	movne	\rTemp,\rTemp,LSR #1
	mrs		\rTemp2,CPSR			//copy Z to overflow
	orreq	\rTemp2,\rTemp2,#1<<28	//overflow flag
	bicne	\rTemp2,\rTemp2,#1<<28	//overflow flag
	msr		CPSR_f,\rTemp2
	.endif
	BigMov	\rBase,MEM_START
#if 1
	mov	\rTemp2,#0x24<<2	//0x24 seems to work, but keep it safe
81:	sub	\rTemp2,\rTemp2,#1
	str	\rTemp2,[\rBase]
	movs	\rTemp2,\rTemp2	//don't affect carry flag, or overflow
	bne	81b
#endif
	str	\rTemp,[\rBase]

	mov	\rTemp2,#SM_ADDRESS_TEST_MASK
	movcc	\rTemp2,#BM_ADDRESS_TEST_MASK
	.if RomWidthIsRamWidth
	movvc	\rTemp2,\rTemp2,LSR #1
	.endif
	str	\rBase,[\rBase,\rTemp2]
	ldr	\rTemp2,[\rBase]
	movcs	\rTemp2,\rTemp	//if 2nd time through, force match
	cmp	\rTemp2,\rTemp
#if 1
	strne	\rTemp2,[\rBase,#4]
//	teq	\rTemp2,\rTemp2
#endif
.endm

	.equiv	tEHEL,	0	//R14
	.equiv	tAPA,	3	//r15 25ns/10ns rounded up

	.equiv	RDFSelect, 13	//R2 : tAVQV : 110ns/10ns = 11; 11-1=10=RDF
//	.equiv	RDFSelect, 10	//R2 : tAVQV : 110ns/10ns = 11; 11-1=10=RDF
			//0-11 map to 0-11
			//12 -> 13, 13 ->15, 14->18, 15->23

//#if (PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)
#if (PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)||(PLATFORM_TYPE==GAME_CONTROLLER)||(PLATFORM_TYPE==GAME_WITH_SMC)
	.equiv	BurstSelect, 0	//if plaited bug, burst mode will no longer work.
#else
	.equiv	BurstSelect, 2	//0->nonburst,1->SRAM,2->burst of4, 3->burst of 8, 4->variable latency i/o
#endif


#if (PLATFORM_TYPE==NEONB)
//access time 70ns, 25ns after CS data becomes valid
//							RRR					RDN					RDF					RBW			RTX
//						  gap between	     Address to
//				1-fast    chip selects	     data valid
//				0-slow    recovery           2nd burst access     1st access delay   16-bit bus    non-burst(0), sram(1), 4cycle(2), 8cycle(3), VLIO(4)
	.equiv	CS0_MSC, (1<<15)+   (3<<12)+           (12<<8)+        ((12-1)<<4)+                       0
	.equiv	CS1_MSC, (1<<15)+(((tEHEL+1)>>1)<<12)+((tAPA-1)<<8)+     (RDFSelect<<4)+                 (BurstSelect)			//fast device
#else
	.equiv	CS0_MSC, (1<<15)+(((tEHEL+1)>>1)<<12)+((tAPA-1)<<8)+     (RDFSelect<<4)+                 (BurstSelect)			//fast device
#if (PLATFORM_TYPE==NEON270)
	.equiv	CS1_MSC, (1<<15)+   (1<<12)+            (2<<8)+            ((4-1)<<4)+        (0<<3) +       4		//SM501
#else
	.equiv	CS1_MSC, (1<<15)+   (3<<12)+            (2<<8)+             ((3-1)<<4)+        (1<<3) +       4		//SMC chip
#endif
//	.equiv	CS1_MSC, (1<<15)+   (6<<12)+           ((11-1)<<8)+        ((4-1)<<4)+        (1<<3) +       4		//SMC chip
#endif

.macro InitCS0_CS1	rBase,rTemp
	BigMov	\rBase,MEMORY_CONTROL_BASE
	ldr		\rTemp,[\rBase,#BOOT_DEF]
	tst		\rTemp,#1			//bit 0 - 1 means 16 bit mode
	BigMov	\rTemp,(CS0_MSC)|((CS1_MSC)<<16)
///////	BigMov	\rTemp,0x7ff07ff0
	orrne	\rTemp,\rTemp,#1<<3		//16 bit bus
	str		\rTemp,[\rBase,#MSC0]
.endm


.macro InitMemory	rBase,rTemp,rTemp2
	cmp	pc,#MEM_START
	bhs	92f		//exit if running from ram
#if 0	//1 to force smaller memory
	subs	\rTemp,\rTemp,\rTemp	//set carry flag
#endif

1:
	InitRam \rBase, \rTemp		//out: \rBase - MEMORY_CONTROL_BASE

#if (PLATFORM_TYPE==BOUNDARY_OLD_BOARD)
	.equiv	CHIP_MODE, 0		//don't use VIO_READY
#else
	.equiv	CHIP_MODE, 4		//gp18 is VIO_READY
#endif

//							RRR					RDN					RDF					RBW			RTX
//						  gap between	     Address to
//				1-fast    chip selects	     data valid
//				0-slow    recovery           2nd burst access     1st access delay   16-bit bus    non-burst(0), sram(1), 4cycle(2), 8cycle(3), VLIO(4)
#if 1
	.equiv	CS2_MSC, (1<<15)+   (4<<12)+           ((6-2)<<8)+         ((4-1)<<4)+        (1<<3) +       CHIP_MODE		//for USB dma
//	.equiv	CS2_MSC, (1<<15)+   (7<<12)+           ((16-1)<<8)+         ((16-1)<<4)+        (1<<3) +       CHIP_MODE		//for USB dma

//											vlio min 2             vlio min 3
#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	.equiv	CS3_MSC, (1<<15)+   (1<<12)+            (2<<8)+            ((4-1)<<4)+        (0<<3) +       4		//SM501
	.equiv	CS4_MSC, (1<<15)+   (3<<12)+            (2<<8)+            ((4-1)<<4)+        (0<<3) +       4		//SMC chip
#else
	.if CPU_PXA270
	.equiv	CS3_MSC, (1<<15)+   (1<<12)+            (2<<8)+            ((4-1)<<4)+        (0<<3) +       4		//NC
	.equiv	CS4_MSC, (1<<15)+   (3<<12)+            (2<<8)+            ((5-1)<<4)+        (0<<3) +       4		//SMC chip
	.else
	.equiv	CS3_MSC, (1<<15)+   (4<<12)+           ((6-2)<<8)+         ((4-1)<<4)+        (1<<3) +       CHIP_MODE		//for USB dma
//	.equiv	CS3_MSC, (1<<15)+   (7<<12)+           ((16-1)<<8)+        ((16-1)<<4)+        (1<<3) +       CHIP_MODE		//for USB dma
	.equiv	CS4_MSC, (1<<15)+   (6<<12)+           ((11-1)<<8)+        ((4-1)<<4)+        (1<<3) +       CHIP_MODE		//for USB IO
	.endif
#endif

	.equiv	CS5_MSC, (0<<15)+   (7<<12)+           ((8-1)<<8)+         ((8-1)<<4)+       (1<<3) +       (0)			//for USB IO delay after CMD write
#else
	.equiv	CS2_MSC, (1<<15)+   (7<<12)+           ((16-1)<<8)+        ((16-1)<<4)+       (1<<3) +       CHIP_MODE		//for USB dma
	.equiv	CS3_MSC, (1<<15)+   (7<<12)+           ((16-1)<<8)+        ((16-1)<<4)+       (1<<3) +       CHIP_MODE		//for USB dma
	.equiv	CS4_MSC, (1<<15)+   (7<<12)+           ((16-1)<<8)+        ((16-1)<<4)+       (1<<3) +       CHIP_MODE		//for USB IO
	.equiv	CS5_MSC, (0<<15)+   (7<<12)+           ((16-1)<<8)+        ((16-1)<<4)+       (1<<3) +       (0)			//for USB IO delay after CMD write
#endif
	BigMov	\rTemp,(CS2_MSC)|((CS3_MSC)<<16)
	str	\rTemp,[\rBase,#MSC1]
	BigMov	\rTemp,(CS4_MSC)|((CS5_MSC)<<16)
	str	\rTemp,[\rBase,#MSC2]



	.equiv	PCMCIA_MEM_SETUP_ADDRESS, 5	//0-127
	.equiv	PCMCIA_MEM_COMMAND_CODE,	3	//0-31
	.equiv	PCMCIA_MEM_HOLD_ADDRESS,	2	//0-63
	BigMov	\rTemp,(PCMCIA_MEM_HOLD_ADDRESS<<14)+(PCMCIA_MEM_COMMAND_CODE<<7)+(PCMCIA_MEM_SETUP_ADDRESS<<0)
	str	\rTemp,[\rBase,#MCMEM0]
	str	\rTemp,[\rBase,#MCMEM1]
	.equiv	PCMCIA_ATT_SETUP_ADDRESS, 5	//0-127
	.equiv	PCMCIA_ATT_COMMAND_CODE,	3	//0-31
	.equiv	PCMCIA_ATT_HOLD_ADDRESS,	2	//0-63
	BigMov	\rTemp,(PCMCIA_ATT_HOLD_ADDRESS<<14)+(PCMCIA_ATT_COMMAND_CODE<<7)+(PCMCIA_ATT_SETUP_ADDRESS<<0)
	str	\rTemp,[\rBase,#MCATT0]
	str	\rTemp,[\rBase,#MCATT1]
	.equiv	PCMCIA_IO_SETUP_ADDRESS,	5	//0-127
	.equiv	PCMCIA_IO_COMMAND_CODE,	3	//0-31
	.equiv	PCMCIA_IO_HOLD_ADDRESS,	2	//0-63
	BigMov	\rTemp,(PCMCIA_IO_HOLD_ADDRESS<<14)+(PCMCIA_IO_COMMAND_CODE<<7)+(PCMCIA_IO_SETUP_ADDRESS<<0)
	str	\rTemp,[\rBase,#MCIO0]
	str	\rTemp,[\rBase,#MCIO1]
	mov	\rTemp,#2
	str	\rTemp,[\rBase,#MECR]

	CheckRam \rBase, \rTemp, \rTemp2
	cmpne	\rTemp,#0x0		//set carry flag, keep z-0 (memory size!=0)
	bne	1b
92:
.endm

// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
// *******************************************************************************************
	.equiv	CKEN_PWM0,	0
	.equiv	CKEN_PWM1,	1
	.equiv	CKEN_AC97,	2
	.equiv	CKEN_SSP,	3
	.equiv	CKEN_HWUART,4
	.equiv	CKEN_STUART,5
	.equiv	CKEN_FFUART,6
	.equiv	CKEN_BTUART,7
	.equiv	CKEN_I2S,	8
	.equiv	CKEN_USB,	11
	.equiv	CKEN_MMC,	12
	.equiv	CKEN_FICP,	13
	.equiv	CKEN_I2C,	14
	.equiv	CKEN_LCD,	16

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==BD2003) || (PLATFORM_TYPE==BOUNDARY_OLD_BOARD) || (PLATFORM_TYPE==OLD_GAME_CONTROLLER)
	.equiv	__ENABLED_BTUART_MASK, (1<<CKEN_BTUART)
	.equiv	__ENABLED_STUART_MASK, (1<<CKEN_STUART)
#endif
	.if CPU_PXA270
	.equiv	__ENABLED_BTUART_MASK, (1<<CKEN_BTUART)
	.equiv	__ENABLED_STUART_MASK, (1<<CKEN_STUART)
	.endif

#if (PLATFORM_TYPE==BD2003) || (PLATFORM_TYPE==BOUNDARY_OLD_BOARD) || (PLATFORM_TYPE==OLD_GAME_CONTROLLER) || (PLATFORM_TYPE==HALOGEN)|| (PLATFORM_TYPE==ARGON) || (PLATFORM_TYPE==NEON270)
	.equiv	__ENABLED_LCD_MASK, (1<<CKEN_LCD)
#endif

	.ifndef __ENABLED_BTUART_MASK
	.equiv	__ENABLED_BTUART_MASK, 0
	.endif

	.ifndef __ENABLED_STUART_MASK
	.equiv	__ENABLED_STUART_MASK, 0
	.endif

	.ifndef __ENABLED_LCD_MASK
	.equiv	__ENABLED_LCD_MASK, 0
	.endif

.macro InitIC_Clocks rBase,rTemp,cpuClock
	BigMov	\rBase,IC_BASE
	mov	\rTemp,#0
	str	\rTemp,[\rBase,#ICMR]			//disable all interrupts

	BigMov	\rBase,CLK_MANAGER_BASE
	.if CPU_PXA270
	.equiv	CKEN_MEMORY_CONTROLLER, 22
	.equiv	CKEN_OS_TIMER, 			9

	.equiv	CCCR_L_BIT,		0
	.equiv	CCCR_2N_BIT,	7
	.equiv	CCCR_A_BIT,		25

	.equiv	CLKCFG_TURBO_BIT,		0
	.equiv	CLKCFG_FREQUENCY_CHANGE_BIT,	1
	.equiv	CLKCFG_HALF_TURBO_BIT,	2
	.equiv	CLKCFG_FAST_BUS_BIT,	3

	.if		(\cpuClock==104)	//13*8 = 104MHz
	.equiv	CCCR_2N,	2
	.equiv	CCCR_L,	8
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 0
	.equiv	CLKCFG_FAST_BUS, 0
	.else
	.if		(\cpuClock==208)	//13*16 = 208MHz
	.equiv	CCCR_2N,	2
	.equiv	CCCR_L,	16
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 0
	.equiv	CLKCFG_FAST_BUS, 0
	.else
	.if		(\cpuClock==312)	//312MHz
	.equiv	CCCR_2N,	3
	.equiv	CCCR_L,	16
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 1
	.equiv	CLKCFG_FAST_BUS, 0
	.else
	.if		(\cpuClock==416)	//416MHz
	.equiv	CCCR_2N,	4
	.equiv	CCCR_L,	16
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 1
	.equiv	CLKCFG_FAST_BUS, 1
	.else
	.if		(\cpuClock==520)	//520MHz
	.equiv	CCCR_2N,	5
	.equiv	CCCR_L,	16
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 1
	.equiv	CLKCFG_FAST_BUS, 1
	.else
	.if		(\cpuClock==624)	//624MHz
	.equiv	CCCR_2N,	6
	.equiv	CCCR_L,	16
	.equiv	CCCR_A, 0
	.equiv	CLKCFG_TURBO, 1
	.equiv	CLKCFG_FAST_BUS, 1

	.else
	.err	//CPU_CLOCK selection not made
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif

	BigMov	\rTemp,(CCCR_L<<CCCR_L_BIT)+(CCCR_2N<<CCCR_2N_BIT)+(CCCR_A<<CCCR_A_BIT)
	str	\rTemp,[\rBase,#CCCR]

	BigMov	\rTemp,(1<<CKEN_OS_TIMER)+(1<<CKEN_MEMORY_CONTROLLER)+(1<<CKEN_FFUART)+__ENABLED_BTUART_MASK+__ENABLED_STUART_MASK+__ENABLED_LCD_MASK
	str	\rTemp,[\rBase,#CKEN]

	.else
	.equiv	tRP,  20
	.equiv	tRCD, 20
	.equiv	tRAS, 45		//45 ns
	.equiv	tRC,  65
// CRYSTALns 10000000/ 36864	//271ns

	.equiv	LSelect, 1	//1 : *27 = 10.0469 ns memory clk time, 99.53 MHz
			//	tRP_clk = tRCD_clk= 1.99066 -> 2 clks
			//	tRAS_clk= 4.47899 -> 5 clks
			//	tRC_clk= 6.46966 -> 7 clks
			//2 : *32 = 8.47711 ns, 117.96 MHz
			//	tRP_clk = tRCD_clk= 2.35929 -> 3 clks
			//	tRAS_clk=5.30841 -> 6
			//	tRC_clk =7.66771 -> 8 clks
			//3 : *36 = 7.5352 ns, 132.71 MHz
			//	tRP_clk = tRCD_clk=  2.65421 -> 3 clks
			//	tRAS_clk=5.97197 -> 6
			//	tRC_clk = 8.62618 -> 9 clks
			//4 : *40 = 6.78168ns, 147.46 MHz
			//	tRP_clk = tRCD_clk=  2.94912 -> 3 clks
			//	tRAS_clk=6.63522 -> 7
			//	tRC_clk = 9.58465 -> 10 clks
			//5 : *45 = 6.02816ns, 165.89 MHz
			//	tRP_clk = tRCD_clk=  3.31776 -> 4 clks
			//	tRAS_clk=7.46496 -> 8
			//	tRC_clk = 10.7827 -> 11 clks
//MSelect
//1 : *1
//2 : *2

//NSelect
//2 : *1;
//3 : *1.5;
//4 : *2;
//6 : *3

	.if		(\cpuClock==100)
	.equiv	MSelect,	1	//1 : *1	100MHz
	.equiv	NSelect,	4	//4 : *2;	turbo 200 MHZ
	.equiv	FCS_MASK,	2	//		turbo off
	.else
	.if		(\cpuClock==200)
	.equiv	MSelect,	2	//2 : *2	200 MHz
	.equiv	NSelect,	3	//3 : *1.5;	turbo 300 MHZ
	.equiv	FCS_MASK,	2	//		turbo off
	.else
	.if		(\cpuClock==300)
	.equiv	MSelect,	2	//2 : *2	200 MHz
	.equiv	NSelect,	3	//3 : *1.5;	turbo 300 MHZ
	.equiv	FCS_MASK,	3	//		turbo on
	.else
	.if		(\cpuClock==400)

	.if 0	//only pxa255 runs with 200Mhz internal bus, pxa250 doesn't
	.equiv	MSelect,	2	//2 : *2	200 MHz
	.equiv	NSelect,	4	//4 : *2;	turbo 400 MHZ
	.equiv	FCS_MASK,	3	//		turbo on
	.else
	.equiv	MSelect,	3	//3 : *4	400 MHz
	.equiv	NSelect,	2	//2 : *1;	turbo 400 MHZ
	.equiv	FCS_MASK,	2	//		turbo off
	.endif

	.else
	.err	//CPU_CLOCK selection not made
	.endif
	.endif
	.endif
	.endif

	BigMov	\rTemp,(NSelect<<7)+(MSelect<<5)+(LSelect)
	str	\rTemp,[\rBase,#CCCR]

	BigMov	\rTemp,(1<<CKEN_FFUART)+__ENABLED_BTUART_MASK+__ENABLED_STUART_MASK+__ENABLED_LCD_MASK
	str	\rTemp,[\rBase,#CKEN]
	.endif	//not CPU_PXA270

	mov \rTemp,#0					//disable 32.768khz oscillator
//	mov	\rTemp,#2					//enable 32.768khz oscillator
	str	\rTemp,[\rBase,#OSCC]
.endm

.macro	InitChangeCPUSpeed rTemp,rBase,rTemp2
	.if CPU_PXA270
	mov	\rTemp,#(CLKCFG_FAST_BUS<<CLKCFG_FAST_BUS_BIT)+(CLKCFG_TURBO<<CLKCFG_TURBO_BIT)+(1<<CLKCFG_FREQUENCY_CHANGE_BIT)
	CP14_CCLKCFG	mcr,\rTemp
	.else
	mov	\rTemp,#FCS_MASK
	CP14_CCLKCFG	mcr,\rTemp
	.endif
.endm
.macro	FinalInitChangeCPUSpeed rBase,rTemp,rTemp2
.endm

// *******************************************************************************************
// *******************************************************************************************

.macro InitUART rBase,rTemp,uartaddr,baudrate
	BigMov	\rBase,\uartaddr

	mov	\rTemp,#0x83			//DLAB, 8-bit characters
	str	\rTemp,[\rBase,#UART_LCR]
	mov	\rTemp,#(14745600/((\baudrate)<<4))&0xff
	str	\rTemp,[\rBase,#UART_DLL]
	mov	\rTemp,#((14745600/((\baudrate)<<4))>>8)
	str	\rTemp,[\rBase,#UART_DLH]

	mov	\rTemp,#3				//8-bit characters
	str	\rTemp,[\rBase,#UART_LCR]

	mov	\rTemp,#0xc1			//enable fifo, 32 byte level
	str	\rTemp,[\rBase,#UART_FCR]

	mov	\rTemp,#0x40			//UUE- UART unit enable
	str	\rTemp,[\rBase,#UART_IER]
.endm

.macro InitUARTs rBase,rTemp,baudrate1,baudrate2,baudrate3
	InitUART	\rBase,\rTemp,FFUART_BASE,\baudrate1
	InitUART	\rBase,\rTemp,BTUART_BASE,\baudrate2
	InitUART	\rBase,\rTemp,STUART_BASE,\baudrate3
.endm

.macro InitGPIO rBase,rTemp
	BigMov	\rBase,GPIO_BASE

	BigMov	\rTemp,~SRVAL0
	str	\rTemp,[\rBase,#GPCR0]
	mvn	\rTemp,\rTemp
	str	\rTemp,[\rBase,#GPSR0]

	BigMov	\rTemp,~SRVAL32
	str	\rTemp,[\rBase,#GPCR1]
	mvn	\rTemp,\rTemp
	str	\rTemp,[\rBase,#GPSR1]

	BigMov	\rTemp,~SRVAL64
	str	\rTemp,[\rBase,#GPCR2]
	mvn	\rTemp,\rTemp
	str	\rTemp,[\rBase,#GPSR2]
	.if CPU_PXA270
	BigMov	\rTemp,~SRVAL96
	str	\rTemp,[\rBase,#GPCR3]
	mvn	\rTemp,\rTemp
	str	\rTemp,[\rBase,#GPSR3]
	.endif
////////
	BigMov	\rTemp,DRVAL0
	str	\rTemp,[\rBase,#GPDR0]

	BigMov	\rTemp,DRVAL32
	str	\rTemp,[\rBase,#GPDR1]

	BigMov	\rTemp,DRVAL64
	str	\rTemp,[\rBase,#GPDR2]
	.if CPU_PXA270
	BigMov	\rTemp,DRVAL96
	str	\rTemp,[\rBase,#GPDR3]
	.endif
////////
	BigMov	\rTemp,AFVAL0
	str	\rTemp,[\rBase,#GAFR0_L]
	BigMov	\rTemp,AFVAL16
	str	\rTemp,[\rBase,#GAFR0_U]

	BigMov	\rTemp,AFVAL32
	str	\rTemp,[\rBase,#GAFR1_L]
	BigMov	\rTemp,AFVAL48
	str	\rTemp,[\rBase,#GAFR1_U]

	BigMov	\rTemp,AFVAL64
	str	\rTemp,[\rBase,#GAFR2_L]
	BigMov	\rTemp,AFVAL80
	str	\rTemp,[\rBase,#GAFR2_U]
	.if CPU_PXA270
	BigMov	\rTemp,AFVAL96
	str	\rTemp,[\rBase,#GAFR3_L]
	BigMov	\rTemp,AFVAL112
	str	\rTemp,[\rBase,#GAFR3_U]
	.endif
	BigMov	\rBase,PWR_MANAGER_BASE
	mov	\rTemp,#0x30
	str	\rTemp,[\rBase,#PSSR]
	
	.if CPU_PXA270
	BigMov	\rBase,GPIO_BASE
	mov		\rTemp,#(1<<11)		//gp11 reset for SMSC lan91c111
	str		\rTemp,[\rBase,#GPSR0]
	mov		\rTemp,#0x10000
99:	subs	\rTemp,\rTemp,#1
	bne		99b
	mov	\rTemp,#(1<<11)		//gp11 reset for SMSC lan91c111
	str	\rTemp,[\rBase,#GPCR0]
	
#if (PLATFORM_TYPE==NEON270)
	BigMov	\rTemp,(AFVAL112)|(3<<((115-112)<<1))	//MBREQ alternate function 3
	str	\rTemp,[\rBase,#GAFR3_U]
#endif
	.endif

.endm



.macro TransMacro val
	BigMov	r12,UART_BASE
91:
	BigMov	r0,\val
	bl	Transmit
	b	91b
.endm
// *******************************************************************************************
.macro InitMMU rBase,rVal
//	mov	\rBase,#PSR_NOINTS_MASK+PSR_MODE_SVC
//	msr	cpsr_c,\rBase
// ********************************************************************
// Disable the MMU and gang regardless of why we are here.
	BigMov \rBase,0x2001			//; enable access to all coprocessors
	CP15_CP_ACCESS	mcr,\rBase
	CPWAIT \rBase

	mov	\rBase,#0x00000078			//; get a zero to turn things off (must write bits[6:3] as 1's)
	CP15_CONTROL	mcr,\rBase		//; Turn Off MMU, I&D Caches, WB.
	CPWAIT \rBase 

	mov	\rBase,#0x00000000         //; get a zero to turn things off
	cmp	pc,#MEM_START
	CP15_TLB_INVAL_BOTH	mcr,\rBase	//; flush (invalidate) I/D tlb's
	CP15_CF_INVAL_BTB	mcr,\rBase	//invalidate Branch target buffer
	CP15_CF_INVAL_D		mcr,\rBase
	CP15_CF_INVAL_I		mcrcs,\rBase		//I might be running from cache only, invalidate if in ram
	CP15_CF_DRAIN		mcr,\rBase		//; Drain the write buffer
	nop
	nop
	nop
	mvn	\rBase, #0					//; grant manager access to all domains
	CP15_DACR	mcr,\rBase
.endm

//out rTemp1 reset reason
.macro InitPWR rBase,rTemp1,rTemp2
	BigMov	\rBase,PWR_MANAGER_BASE
// ********************************************************************
	ldr     \rTemp1,[\rBase,#RCSR]		// Read & Init Reset Cause bits in RCSR.
	and     \rTemp1,\rTemp1,#RCSR_MASK	// extract the reset cause bits
	str     \rTemp1,[\rBase,#RCSR]		// clear the reset cause bits (they're sticky)
// ********************************************************************
// Read and store PSSR, too - it will be reset later, after GPIOs are initialized.  
// Unclear when we'll need this information, but don't throw a good status away.
//	ldr     \rTemp2, [\rBase,#PSSR]                       
// extract the reset cause bits
//	and     \rTemp2,\rTemp2,#PSSR_MASK		//; r12 now holds the conditioned PSSR
//	orr     \rTemp1,\rTemp1,\rTemp2,lsl #16			//; R10 now has RCSR in lower half and PSSR in upper.
// ********************************************************************
// Reasons for being here:
//  1) Hard Reset
//  2) Wake from Sleep
//  3) GPIO Reset
//  4) Watchdog Reset
//  5) Eboot Handoff     
// If Sleep_Reset:     reinit all but RTC, PWRMAN, CLKS (except cp14)... so reinit: OST, INTC, GPIO
// If GPIO_Reset:      reinit all but MEMC.Config, RTC, CLKS (except cp14)... so reinit: treat like sleep
// If Watchdog_Reset:  reinit all but RTC.RTTR, CLK.OSCC... so reinit: treat like a hard reset, minus OSCC and RTTR init.
// If Hard_Reset:      reinit all 
.endm

// *******************************************************************************************
// *******************************************************************************************
	.list
