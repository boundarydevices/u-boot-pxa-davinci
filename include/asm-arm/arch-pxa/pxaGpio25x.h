//gpios for PLATFORM_TYPE== NEON,NEONB,BD2003, or BOUNDARY_OLD_BOARD

#define LCD_CS_STATE HIGH

//CP  - Clock and Power Management Unit
//MMC - Multimedia Card Controller
//MC  - Memory Controller
//SIU - System Integration Unit
//SSP - Synchronous Serial Port
//AC  - Audio Controller (AC97)
//FF  - Full Function UART
//BT  - Blue Tooth UART
//ST  - standard UART Port
//LCD - LCD Controller
	SPEC_GP  0,IN,HIGH,0		//        flash ready low 16, or magStripe T1 Clk(SMC)
	SPEC_GP  1,IN,HIGH,0		//CP_RST, flash ready high 16, or magStripe T2 Clk(SMC)

#if (PLATFORM_TYPE==NEONB)
	SPEC_GP  2,IN,HIGH,0		//USB client connection status
	SPEC_GP  3,IN,HIGH,0		//float means USB Slave not ready to accept data
								//out 1 means ready (D+ signal)
	SPEC_GP  4,OUT,LOW,0		//low means don't provide 12 volts to I2C bus
#else
#if (PLATFORM_TYPE==NEON)
	SPEC_GP  2,IN,HIGH,0		//USB client connection status
	SPEC_GP  3,IN,HIGH,0		//float means USB Slave not ready to accept data
	SPEC_GP  4,IN,HIGH,0		//UCB1400 irq on NEON board
#else
	SPEC_GP  2,OUT,LOW,0		//output to transistor (unused), OUT_DRY_CONTACT2
	SPEC_GP  3,OUT,HIGH,0
	SPEC_GP  4,IN,HIGH,0		//interrupt for USB irq 1
#endif
#endif

	SPEC_GP  5,IN,HIGH,0		//interrupt for USB irq 2,  or SM501
	SPEC_GP  6,OUT,HIGH,1		//MMC_CLK
	SPEC_GP  7,OUT,LOW,0		//CP_48MHZ	!!! red led, NEON:J13,pin3
	SPEC_GP  8,OUT,HIGH,1		//MMC_CCS0
	SPEC_GP  9,OUT,LOW,0		//MMC_CCS1, !!! doorlock, or OUT_DRY_CONTACT1
	SPEC_GP  10,IN,HIGH,0		//SIU_RTCCLK, Neon/Neonb data 1 for SDIO interrupt

#if (PLATFORM_TYPE==NEON)
	SPEC_GP  11,OUT,HIGH,0		//CP_3600KHZ, NEON:J12,pin1
#else
	SPEC_GP  11,IN,HIGH,0		//CP_3600KHZ, suspend USB slave
#endif
	SPEC_GP  12,IN,HIGH,0		//CP_32KHZ, suspend USB host

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  13,OUT,HIGH,2		//MC_MBGNT
	SPEC_GP  14,IN,HIGH,1		//MC_MBREQ
#else
	SPEC_GP  13,OUT,HIGH,0		//USB wakeup slave
	SPEC_GP  14,IN,HIGH,0		//UCB1400 IRQ
#endif


#if (PLATFORM_TYPE==NEONB)
	SPEC_GP  15,OUT,HIGH,2		//MC_nCS1, eeprom
#else
#if (PLATFORM_TYPE==NEON)
	SPEC_GP  15,OUT,LOW,0		//MC_nCS1,  NEON:J13, pin 1
#else
	SPEC_GP  15,OUT,LOW,0		//MC_nCS1, !!! amber led
#endif
#endif


#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  16,OUT,HIGH,0		//LCD backlight brightness control
	SPEC_GP  17,OUT,HIGH,0		//LCD backlight ON/OFF
#else
	SPEC_GP  16,IN,HIGH,0		//SIU_PWM0, !!! feedback2, left in
	SPEC_GP  17,IN,HIGH,0		//SIU_PWM1, !!! feedback1, right in
#endif

#if (PLATFORM_TYPE==BOUNDARY_OLD_BOARD)
	SPEC_GP  18,OUT,HIGH,0		//MC_RDY, !!! turnstile
#else
	SPEC_GP  18,IN,HIGH,1		//MC_RDY, VIO_READY
#endif

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  19,OUT,HIGH,0		//MC_DREQ1, nc
#else
	SPEC_GP  19,IN,HIGH,1		//MC_DREQ1, DMA request for USB DC
#endif

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  20,OUT,HIGH,1		//MC_DREQ0, nc
#else
	SPEC_GP  20,IN,HIGH,1		//MC_DREQ0, DMA request for USB HC
#endif

	SPEC_GP  21,IN,HIGH,0		//	pcmcia card detect
	SPEC_GP  22,IN,HIGH,0		//	pcmcia intr (active low)

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  23,IN,LOW,0		//UCB1400 irq for NEONB, nc for NEW NEON
#else
	SPEC_GP  23,OUT,HIGH,0		//SSP_SCLK, nc
#endif

#if (PLATFORM_TYPE==NEON) || (PLATFORM_TYPE==NEONB)
	SPEC_GP  24,IN,LOW,0		//SSP_SFRM, SMSC interrupt (active high)
#else
	SPEC_GP  24,OUT,HIGH,0		//SSP_SFRM, nc
#endif

	SPEC_GP  25,OUT,HIGH,0		//SSP_TXD, nc
	SPEC_GP  26,OUT,HIGH,0		//SSP_RXD, nc
	SPEC_GP  27,OUT,LOW,0		//SSP_EXTCLK, DC1 (SMC)

	SPEC_GP  28,IN,HIGH,1		//AC_BITCLK,	ac97 bitclk
	SPEC_GP  29,IN,HIGH,1		//AC_SDATAIN0,	ac97 datain0
	SPEC_GP  30,OUT,HIGH,2		//AC_SDATAOUT,	ac97 data out
	SPEC_GP  31,OUT,HIGH,2		//AC_SYNC,	ac97 sync
////////////////////////////////////////////////////////////////////////////////////////////


	SPEC_GP  32,OUT,HIGH,0		//AC_SDATAIN1, wet contact
	SPEC_GP  33,OUT,LOW,0		//MC_nCS5, green led (left), NEON:J13,pin 2
	SPEC_GP  34,IN,HIGH,1		//FF_RXD
	SPEC_GP  35,IN,HIGH,1		//FF_CTS
	SPEC_GP  36,IN,HIGH,0		//FF_DCD, MMC Card Detect
	SPEC_GP  37,IN,HIGH,1		//FF_DSR
	SPEC_GP  38,IN,HIGH,0		//FF_RI, MMC Write Protect, MMC/SDIO IRQ
	SPEC_GP  39,OUT,HIGH,2		//FF_TXD
	SPEC_GP  40,OUT,HIGH,2		//FF_DTR
	SPEC_GP  41,OUT,HIGH,2		//FF_RTS
	SPEC_GP  42,IN,HIGH,1		//BT_RXD
	SPEC_GP  43,OUT,HIGH,2		//BT_TXD
#if (PLATFORM_TYPE==NEON)
	SPEC_GP  44,IN,HIGH,0		//BT_CTS, NEON:J12,pin 2
#else
	SPEC_GP  44,OUT,HIGH,0		//BT_CTS, USB wakeup host
#endif
	SPEC_GP  45,OUT,LOW,0		//BT_RTS, HIGH 2
#if (PLATFORM_TYPE==NEON)
	SPEC_GP  46,IN,HIGH,0		//ST_RXD, NEON:J12,pin 3
#else
	SPEC_GP  46,IN,HIGH,2		//ST_RXD
#endif
	SPEC_GP  47,OUT,HIGH,1		//ST_TXD
	SPEC_GP  48,OUT,HIGH,2		//MC_nPOE,    pcmcia
	SPEC_GP  49,OUT,HIGH,2		//MC_nPWE,    pcmcia
	SPEC_GP  50,OUT,HIGH,2		//MC_nPIOR,   pcmcia
	SPEC_GP  51,OUT,HIGH,2		//MC_nPIOW,   pcmcia
	SPEC_GP  52,OUT,HIGH,2		//MC_nPCE1,   pcmcia
	SPEC_GP  53,OUT,HIGH,2		//MC_nPCE2,   pcmcia
	SPEC_GP  54,OUT,HIGH,2		//MC_nPSKTSEL, nc
	SPEC_GP  55,OUT,HIGH,2		//MC_nPREG,   pcmcia attribe vs Io space
	SPEC_GP  56,IN,HIGH,1		//MC_nPWAIT   pcmcia busy
	SPEC_GP  57,IN,HIGH,1		//MC_nIOIS16, pcmcia 16 bit wide
	SPEC_GP  58,OUT,HIGH,ALT_LCD	//LCD_LDD0, also GP_PIX_D0
	SPEC_GP  59,OUT,HIGH,ALT_LCD	//LCD_LDD1, also GP_PIX_D1
	SPEC_GP  60,OUT,HIGH,ALT_LCD	//LCD_LDD2, also GP_PIX_D2
	SPEC_GP  61,OUT,HIGH,ALT_LCD	//LCD_LDD3, also GP_PIX_D3
	SPEC_GP  62,OUT,HIGH,ALT_LCD	//LCD_LDD4, also GP_PIX_D4
	SPEC_GP  63,OUT,HIGH,ALT_LCD	//LCD_LDD5, also GP_PIX_D5


////////////////////////////////////////////////////////////////////////////////////////////


	SPEC_GP  64,OUT,HIGH,ALT_LCD	//LCD_LDD6, also GP_PIX_D6
	SPEC_GP  65,OUT,HIGH,ALT_LCD	//LCD_LDD7, also GP_PIX_D7
	SPEC_GP  66,OUT,HIGH,ALT_LCD	//LCD_LDD8, also GP_PIX_RESET
	SPEC_GP  67,OUT,HIGH,ALT_LCD	//LCD_LDD9
	SPEC_GP  68,OUT,HIGH,ALT_LCD	//LCD_LDD10
	SPEC_GP  69,OUT,HIGH,ALT_LCD	//LCD_LDD11
	SPEC_GP  70,OUT,HIGH,ALT_LCD	//LCD_LDD12

	SPEC_GP  71,OUT,HIGH,ALT_LCD	//LCD_LDD13

	SPEC_GP  72,OUT,HIGH,ALT_LCD	//LCD_LDD14
	SPEC_GP  73,OUT,HIGH,ALT_LCD	//LCD_LDD15
	SPEC_GP  74,OUT,HIGH,ALT_LCD	//LCD_FCLK, also GP_PIX_READ
	SPEC_GP  75,OUT,LCD_CS_STATE,ALT_LCD	//LCD_LCLK, also GP_PIX_CS1
	SPEC_GP  76,OUT,LCD_CS_STATE,ALT_LCD	//LCD_PCLK, also GP_PIX_CS0
	SPEC_GP  77,OUT,HIGH,ALT_LCD	//LCD_ACBIAS, also GP_PIX_A0
	SPEC_GP  78,OUT,HIGH,2		//nCS2, DMA acknowledge channel 1 for USB, SMC91c111 Chip Select nDATACS
	SPEC_GP  79,OUT,HIGH,2		//nCS3, DMA acknowledge channel 2 for USB, SM501 Chip Select
	SPEC_GP  80,OUT,HIGH,2		//nCS4, USB chip select, SMC91c111 Chip Select
	SPEC_GP  81,IN,LOW,0		//GND (pin F16), pxa255 has 9 extra gpios
	SPEC_GP  82,IN,LOW,0		//GND (pin E16)
	SPEC_GP  83,IN,LOW,0		//GND (pin E15)
	SPEC_GP  84,IN,LOW,0		//GND (pin D16)
	SPEC_GP  85,IN,LOW,0		//GND (pin F15)
//to maintain compatibility with code written for the pxa250
//the meaning of gp86-gp89's direction bit is reversed, and alternate function is forced to the SDRAM/AC97 unit's control
	SPEC_GP  86,IN,HIGH,0		//SDCS2 (pin G3) set as OUTPUT!!!
	SPEC_GP  87,IN,HIGH,0		//SDCS3 (pin F2) set as OUTPUT!!!
	SPEC_GP  88,IN,HIGH,0		//old RDnWR(pin D3) set as OUTPUT!!!
	SPEC_GP  89,IN,LOW,0		//old ac97_reset(pin D10), set as OUTPUT!!!
	SPEC_GP  90,IN,LOW,0		//undefined
	SPEC_GP  91,IN,LOW,0		//undefined
	SPEC_GP  92,IN,LOW,0		//undefined
	SPEC_GP  93,IN,LOW,0		//undefined
	SPEC_GP  94,IN,LOW,0		//undefined
	SPEC_GP  95,IN,LOW,0		//undefined

// ****************************************************************************
	CREATE_MASK_DIR		DRVAL0, SPEC_, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	CREATE_MASK_LEVEL	SRVAL0, SPEC_, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	CREATE_MASK_ALT		AFVAL0, SPEC_, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
	CREATE_MASK_ALT		AFVAL16,SPEC_,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31

	CREATE_MASK_DIR		DRVAL32,SPEC_,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
	CREATE_MASK_LEVEL	SRVAL32,SPEC_,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
	CREATE_MASK_ALT		AFVAL32,SPEC_,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47
	CREATE_MASK_ALT		AFVAL48,SPEC_,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63

	CREATE_MASK_DIR		DRVAL64,SPEC_,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95
	CREATE_MASK_LEVEL	SRVAL64,SPEC_,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95
	CREATE_MASK_ALT		AFVAL64,SPEC_,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
	CREATE_MASK_ALT		AFVAL80,SPEC_,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95

