//#define GPIO_BASE 0x40E00000
#define GPLR0	0x00	//level
#define GPLR1	0x04
#define GPLR2	0x08
#define GPLR3	0x100	//level

#define GPDR0	0x0c	//direction
#define GPDR1	0x10
#define GPDR2	0x14
#define GPDR3	0x10c	//direction

#define GPSR0	0x18	//set
#define GPSR1	0x1C
#define GPSR2	0x20
#define GPSR3	0x118	//set

#define GPCR0	0x24	//clear
#define GPCR1	0x28
#define GPCR2	0x2c
#define GPCR3	0x124	//clear

#define GRER0	0x30	//enable rising edge detect
#define GRER1	0x34
#define GRER2	0x38
#define GRER3	0x130	//enable rising edge detect

#define GFER0	0x3C	//enable falling edge detect
#define GFER1	0x40
#define GFER2	0x44
#define GFER3	0x13C	//enable falling edge detect

#define GEDR0	0x48	//edge detect status
#define GEDR1	0x4C
#define GEDR2	0x50
#define GEDR3	0x148	//edge detect status

#define GAFR0_L	0x54	//alternate function
#define GAFR0_U	0x58
#define GAFR1_L	0x5C
#define GAFR1_U	0x60
#define GAFR2_L	0x64
#define GAFR2_U	0x68
#define GAFR3_L	0x6c
#define GAFR3_U	0x70


#define UART_RBR		0		//read, DLAB-0	:RECEIVE_BUFFER
#define UART_THR		0		//write, DLAB-0	:TRANSMIT_BUFFER
#define UART_DLL		0		//r/w, DLAB-1	:DIVISOR_LOW
#define UART_IER		4		//r/w, DLAB-0	:INTERRUPT_ENABLE
#define UART_DLH		4		//r/w, DLAB-1	:DIVISOR_HIGH

#define UART_IIR		8		//read		:INTERRUPT_INDENTIFICATION
#define UART_FCR		8		//write		:FIFO_CONTROL
#define UART_LCR		0x0c		//r/w		:LINE_CONTROL
#define UART_MCR		0x10		//r/w		:MODEM_CONTROL
#define UART_LSR		0x14		//read		:LINE_STATUS
#define UART_MSR		0x18		//read		:MODEM_STATUS
#define UART_SPR		0x1c		//r/w		:SCRATCH_PAD
#define UART_ISR		0x20		//r/w		:INFRARED

//#define IC_BASE 0x40D00000
#define ICIP	0x00	//IRQ pending status
#define ICMR	0x04	//interrupt controller mask
#define ICLR	0x08	//level, 0 - irq, 1 -fiq
#define ICFP	0x0C	//FIQ pending status
#define ICPR	0x10	//all interrupts pending, no mask
#define ICCR	0x14	//0 - ignore ICMR in idle mode, 1 - only unmasked interrupts will awaken processor


#define __SKIP 8
#define ICL1_GPIO0		(8 - __SKIP)
#define ICL1_GPIO1		(9 - __SKIP)
#define ICL1_GPIO2_80		(10 - __SKIP)
#define ICL1_USB		(11 - __SKIP)
#define ICL1_PMU		(12 - __SKIP)
#define ICL1_I2S		(13 - __SKIP)
#define ICL1_AC97		(14 - __SKIP)
#define ICL1_LCD		(17 - __SKIP)
#define ICL1_I2C		(18 - __SKIP)
#define ICL1_ICP		(19 - __SKIP)
#define ICL1_STUART		(20 - __SKIP)
#define ICL1_BTUART		(21 - __SKIP)
#define ICL1_FFUART		(22 - __SKIP)
#define ICL1_MMC		(23 - __SKIP)
#define ICL1_SSP		(24 - __SKIP)
#define ICL1_DMA		(25 - __SKIP)
#define ICL1_OS_TIMER0		(26 - __SKIP)
#define ICL1_OS_TIMER1		(27 - __SKIP)
#define ICL1_OS_TIMER2		(28 - __SKIP)
#define ICL1_OS_TIMER3		(29 - __SKIP)
#define ICL1_RTC_TICK		(30 - __SKIP)
#define ICL1_RTC_ALARM_MATCH	(31 - __SKIP)

//#define OS_TIMER_BASE 0x40a00000
#define OSMR0	0x00
#define OSMR1	0x04
#define OSMR2	0x08
#define OSMR3	0x0c
#define OSCR	0x10		//OS timer Count register, increments at 3.6864 Mhz
#define OSSR	0x14		//OS timer Status Register
#define OWER	0x18		//OS timer Watchdog Match enable register
#define OIER	0x1c

//3686400 ticks/seconds = 3.6864 ticks/usec = 1 tick/.271267361111 usec
#define TICK_PER_USEC_WHOLE 3
#define TICK_PER_USEC_FRAC 0xAFB7E910
//////////////////////////////////////////////////////////////////////////////////////////

//#define PWR_MANAGER_BASE 0x40F00000
#define PMCR  0
#define PSSR  4
#define PSPR  8
#define PWER  0x0c
#define PRER  0x10
#define PFER  0x14
#define PEDR  0x18
#define PCFR  0x1c
#define PGSR0 0x20
#define PGSR1 0x24
#define PGSR2 0x28
#define RCSR  0x30

#define PSSR_MASK 0x37
#define RCSR_MASK 0x0f
#define RCSR_SLEEP_RESET 4

//#define CLK_MANAGER_BASE 0x41300000
#define CCCR 0
#define CKEN 4
#define OSCC 8

//#define MEMORY_CONTROL_BASE 0x48000000
#define MDCNFG		0
#define MDREFR		4
#define MSC0		8
#define MSC1		0x0c
#define MSC2		0x10
#define MECR		0x14
#define SXCNFG		0x1c
#define SXMRS		0x24
#define MCMEM0		0x28
#define MCMEM1		0x2c
#define MCATT0		0x30
#define MCATT1		0x34
#define MCIO0		0x38
#define MCIO1		0x3c
#define MDMRS		0x40
#define BOOT_DEF	0x44


//#define LCD_CONTROL_BASE 0x44000000
#define LCD_CR0		0		//master enable last
#define LCD_CR1		4
#define LCD_CR2		8
#define LCD_CR3 	0x0c
#define LCD_FBR0	0x20			//Frame branch register
#define LCD_FBR1	0x24			//Frame branch register
#define LCD_SR		0x38
#define LCD_INT_ID	0x3c
#define LCD_TMEDS	0x40			//TMED RGB seed
#define LCD_TMEDC	0x44			//TMED control
#define LCD_FDADR0	0x200		//frame descriptor address register
#define LCD_FDADR1	0x210		//frame descriptor address register

#define LCD_FSADR0	0x204		//frame source address register
#define LCD_FSADR1	0x214		//frame source address register

#define LCD_FIDR0	0x208		//frame ID register
#define LCD_FIDR1	0x218		//frame ID register

#define LCD_DCMD0	0x20c		//dma command
#define LCD_DCMD1	0x21c		//dma command

#define CR0_LDDALT	26	//LDDALT	;0 selects RGB 555, 1 selects RGB 565
#define CR0_OUC		25	//OUC		;0 underlays, 1 overlays
#define CR0_CMDIM	24	//CMDIM		;command interrupt mask, 0 - enabled, 1 disabled
#define CR0_RDSTM	23	//RDSTM		;read status interrupt mask, 0 - enabled, 1 disabled
#define CR0_LCDT	22	//LCDT		;LCD Panel Type, 1 - internal frame buffer
#define CR0_IM_OUTPUT_FIFO_UNDERRUN		21	//OUM IM stands for Interrupt Mask
#define CR0_IM_BRANCH					20	//BSM0
#define CR0_PALETTE_DMA_REQUEST_DELAY	12	//PDD  8 bit field, 0-255
#define CR0_IM_LCD_QUICK_DISABLE		11	//QDM
#define CR0_DISABLE						10	//DIS
#define CR0_DOUBLE_PIXEL_DATA			9	//DPD
#define CR0_ACTIVE_SELECT				7	//PAS
#define CR0_IM_END_OF_FRAME				6	//EOFM0
#define CR0_IM_INPUT_FIFO_UNDERRUN		5	//IUM
#define CR0_IM_START_OF_FRAME			4	//SOFM0
#define CR0_IM_DISABLE_DONE				3	//LDM
#define CR0_DUAL_PANEL_SELECT			2	//SDS
#define CR0_MONOCHROME_SELECT			1	//CMS
#define CR0_ENABLE						0	//ENB




#define CR1_BEGINNING_OF_LINE_WAIT		24	//8 bit field +1
#define CR1_END_OF_LINE_WAIT			16	//8 bit field +1
#define CR1_HORIZONTAL_SYNC_PULSE_WIDTH		10	//6 bit field +1
#define CR1_PIXELS_PER_LINE			0	//10 bit field +1



#define CR2_BEGINNING_OF_FRAME_WAIT		24	//8 bit field
#define CR2_END_OF_FRAME_WAIT			16	//8 bit field
#define CR2_VERTICAL_SYNC_PULSE_WIDTH		10	//6 bit field +1
#define CR2_LINES_PER_PANEL			0	//10 bit field +1


#define CR3_PDFOR 					30
#define CR3_DOUBLE_PCLK				27
#define CR3_BITS_PER_PIXEL			24	//3 bit field
//0 - 1 bit
//1 - 2 bits
//2 - 4 bits
//3 - 8 bits
//4 - 16 bits,
//5-7 reserved
#define CR3_BIAS_POLARITY			23
#define CR3_PCLK_POLARITY			22
#define CR3_LCLK_POLARITY			21
#define CR3_FCLK_POLARITY			20
#define CR3_API					16	//4 bits field, ac bias transitions per interrupt, 0 disable
#define CR3_ACBIAS_TOGGLE			8	//8 bit field +1
#define CR3_PCLK_DIVISOR			0	//8 bit field +1



#define FBR_BRANCH_INT	1
#define FBR_BRANCH	0


#define SR_MISSED_INT		10
#define SR_BRANCH_INT		9
#define SR_END_OF_FRAME_INT	8
#define SR_QUICK_DISABLE_INT	7
#define SR_OUTPUT_FIFO_UNDERRUN	6
#define SR_INPUT_FIFO_UNDERRUN0 5
#define SR_INPUT_FIFO_UNDERRUN1 4
#define SR_ACBIAS_COUNT_INT	3
#define SR_BUS_ERROR		2
#define SR_START_OF_FRAME	1
#define SR_DISABLE_DONE		0

#define TMEDS_BLUE		16	//8 bit field
#define TMEDS_GREEN		8	//8 bit field
#define TMEDS_RED		0	//8 bit field


#define TMEDC_MATRIX2_SELECT			14
#define TMEDC_RESERVED				12		//2 bit field
#define TMEDC_HORIZONTAL_BEAT_SUPPRESSION	8		//4 bit field
#define TMEDC_VERTICAL_BEAT_SUPPRESSION		4		//4 bit field
#define TMEDC_FRAME_NUMBER_ADJUST_EN		3
#define TMEDC_COLOR_OFFSET_ADJUST_EN		2
#define TMEDC_FRAME_NUMBER_ADJUST_MATRIX2	1
#define TMEDC_COLOR_OFFSET_ADJUST_MATRIX2	0


#define DCMD_PALETTE			26
#define DCMD_START_OF_FRAME_INTERRUPT	22
#define DCMD_END_OF_FRAME_INTERRUPT	21
#define DCMD_LENGTH			0		//21 bit field


#define FDESC_FDADR	0
#define FDESC_FSADR	4
#define FDESC_FIDR	8
#define FDESC_DCMD	0x0c

