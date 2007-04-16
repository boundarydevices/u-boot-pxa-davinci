
	.equiv	IPU_BASE, 0x53fc0000
	.equiv	IPU_CONF, 			0x00
	.equiv	IPU_CHAN_BUF0_RDY,	0x04
	.equiv	IPU_CHAN_BUF1_RDY,	0x08
	.equiv	IPU_CHAN_DB_MODE_SEL, 0x0c	//Double buffer mode select
	.equiv	IPU_CHAN_CUR_BUF,	0x10	//which double buffer is active
	.equiv	IPU_FS_PROC_FLOW,	0x14
	.equiv	IPU_FS_DISP_FLOW,	0x18
	.equiv	IPU_TASKS_STAT,		0x1c
	.equiv	IPU_IMA_ADDR,		0x20	//internal memory access address
	.equiv	IPU_IMA_DATA,		0x24
	.equiv	IPU_INT_CTRL1,		0x28	//Interrupt for end of frame on channel
	.equiv	IPU_INT_CTRL2,		0x2c	//Interrupt for new frame and Ack of channel
	.equiv	IPU_INT_CTRL3,		0x30	//misc ints
	.equiv	IPU_INT_CTRL4,		0x34	//New frame before end of frame error???
	.equiv	IPU_INT_CTRL5,		0x38	//misc error ints
	.equiv	IPU_INT_STAT1,		0x3c	//corresponds to CTRL1
	.equiv	IPU_INT_STAT2,		0x40
	.equiv	IPU_INT_STAT3,		0x44
	.equiv	IPU_INT_STAT4,		0x48
	.equiv	IPU_INT_STAT5,		0x4c
	.equiv	IPU_BRK_CTRL1,		0x50
	.equiv	IPU_BRK_CTRL2,		0x54
	.equiv	IPU_BRK_STAT,		0x58
	.equiv	IPU_DIAG_CTRL,		0x5c
	.equiv	CSI_SENS_CONF,		0x60
	.equiv	CSI_SENS_FRM_SIZE,	0x64
	.equiv	CSI_ACT_FRM_SIZE,	0x68
	.equiv	CSI_OUT_FRM_CTRL,	0x6c
	.equiv	CSI_TST_CTRL,		0x70
	.equiv	CSI_CCIR_CODE1,		0x74
	.equiv	CSI_CCIR_CODE2,		0x78
	.equiv	CSI_CCIR_CODE3,		0x7c
	.equiv	CSI_FLASH_STROBE1,	0x80
	.equiv	CSI_FLASH_STROBE2,	0x84
	.equiv	IC_CONF,			0x88		//task enables
	.equiv	IC_PRP_ENC_RSC,		0x8c		//preprocessing encoder resizing coefficients
	.equiv	IC_PRP_VF_RSC,		0x90		//preprocessing view-finder resizing coefficients
	.equiv	IC_PP_RSC,			0x94		//postprocessing resizing coefficients
	.equiv	IC_CMBP1,			0x98		//combining parameters register,alphas
	.equiv	IC_CMBP2,			0x9c		//key color
	.equiv	PF_CONF,			0xa0		//Post filter configuration register
	.equiv	IDMAC_CONF,			0xa4
	.equiv	IDMAC_CHAN_EN,		0xa8
	.equiv	IDMAC_CHAN_PRI,		0xac		//channel priority
	.equiv	IDMAC_CHAN_BUSY,	0xb0
	.equiv	SDC_COM_CONF,		0xb4	//select graphics plane foreground/background
	.equiv	SDC_GRAPH_WIND_CTRL, 0xb8
	.equiv	SDC_FG_POS,			0xbc		//Foreground window position (10 bit x,y)
	.equiv	SDC_BG_POS,			0xc0		//Background window position (10 bit x,y)
					//25-16 : x # of pix clks from HSYNC start and data (left margin)
					// 9- 0 : y # of lines from VSYNC start and data  (top margin)
	.equiv	SDC_CURSOR_POS,		0xc4
	.equiv	SDC_CURSOR_BLINK_PWM_CTRL,	0xc8
	.equiv	SDC_CURSOR_MAP,		0xcc		//solid cursor color
	.equiv	SDC_HORIZONTAL_CONF, 0xd0			//horizontal configuration
					//31-26 : HSYNC width-1
					//25-16 : Screen width-1, actually HSYNC signal width i.e. 320+HSync width+LeftMargin+rightMargin-1
	.equiv	SDC_VERTICAL_CONF,	0xd4			//horizontal configuration
					//31-26 : VSYNC width-1
					//25-16 : Screen Height-1, actually VSYNC signal width i.e. 240+VSync width+TopMargin+BottomMargin-1
					//0     : VSync width units, 1-rows, 0 pixels
	.equiv	SDC_SHARP_CONF1,	0xd8
	.equiv	SDC_SHARP_CONF2,	0xdc
	.equiv	ADC_CONF,			0xe0
	.equiv	ADC_SYSCHA1_SA,		0xe4		//start address of channel1
	.equiv	ADC_SYSCHA2_SA,		0xe8		//start address of channel2
	.equiv	ADC_PRPCHAN_SA,		0xec		//start address of preprocessing channel
	.equiv	ADC_PPCHAN_SA,		0xf0		//start address of postprocessing channel
	.equiv	ADC_DISP0_CONF,		0xf4
	.equiv	ADC_DISP0_RD_AP,	0xf8		//read acknowledge pattern
	.equiv	ADC_DISP0_RDM,		0xfc		//read mask
	.equiv	ADC_DISP0_SS,		0x100		//screen size
	.equiv	ADC_DISP1_CONF,		0x104
	.equiv	ADC_DISP1_RD_AP,	0x108		//read acknowledge pattern
	.equiv	ADC_DISP1_RDM,		0x10c		//read mask
	.equiv	ADC_DISP12_SS,		0x110		//screen size
	.equiv	ADC_DISP2_CONF,		0x114
	.equiv	ADC_DISP2_RD_AP,	0x118		//read acknowledge pattern
	.equiv	ADC_DISP2_RDM,		0x11c		//read mask
	.equiv	ADC_DISP_VSYNC,		0x120		//screen size
	.equiv	DI_DISP_IF_CONF,	0x124
	.equiv	DI_DISP_SIG_POL,	0x128		//Display signals polarity
	.equiv	DI_SER_DISP1_CONF,	0x12c
	.equiv	DI_SER_DISP2_CONF,	0x130
	.equiv	DI_HSP_CLK_PER,		0x134
	.equiv	DI_DISP0_TIME_CONF1, 0x138
	.equiv	DI_DISP0_TIME_CONF2, 0x13c
	.equiv	DI_DISP0_TIME_CONF3, 0x140
	.equiv	DI_DISP1_TIME_CONF1, 0x144
	.equiv	DI_DISP1_TIME_CONF2, 0x148
	.equiv	DI_DISP1_TIME_CONF3, 0x14c
	.equiv	DI_DISP2_TIME_CONF1, 0x150
	.equiv	DI_DISP2_TIME_CONF2, 0x154
	.equiv	DI_DISP2_TIME_CONF3, 0x158
	.equiv	DI_DISP3_TIME_CONF,	0x15c
	.equiv	DI_DISP0_DB0_MAP,	0x160
	.equiv	DI_DISP0_DB1_MAP,	0x164
	.equiv	DI_DISP0_DB2_MAP,	0x168
	.equiv	DI_DISP0_CB0_MAP,	0x16c
	.equiv	DI_DISP0_CB1_MAP,	0x170
	.equiv	DI_DISP0_CB2_MAP,	0x174
	.equiv	DI_DISP1_DB0_MAP,	0x178
	.equiv	DI_DISP1_DB1_MAP,	0x17c
	.equiv	DI_DISP1_DB2_MAP,	0x180
	.equiv	DI_DISP1_CB0_MAP,	0x184
	.equiv	DI_DISP1_CB1_MAP,	0x188
	.equiv	DI_DISP1_CB2_MAP,	0x18c
	.equiv	DI_DISP2_DB0_MAP,	0x190
	.equiv	DI_DISP2_DB1_MAP,	0x194
	.equiv	DI_DISP2_DB2_MAP,	0x198
	.equiv	DI_DISP2_CB0_MAP,	0x19c
	.equiv	DI_DISP2_CB1_MAP,	0x1a0
	.equiv	DI_DISP2_CB2_MAP,	0x1a4
	.equiv	DI_DISP3_B0_MAP,	0x1a8
	.equiv	DI_DISP3_B1_MAP,	0x1ac
	.equiv	DI_DISP3_B2_MAP,	0x1b0
	.equiv	DI_DISP_ACC_CC,		0x1b4
	.equiv	DI_DISP_LLA_CONF,	0x1b8	//low level config
	.equiv	DI_DISP_LLA_DATA,	0x1bc
	
	
#if 0
	.equiv	BLUE_VAL,  0x0
	.equiv	GREEN_VAL, 0x0
	.equiv	RED_VAL,   0x0

	.equiv	BLUE_VAL2,  0x0
	.equiv	GREEN_VAL2, 0x0
	.equiv	RED_VAL2,   0x0
#else
#if 0
	.equiv	BLUE_VAL,  0x3f
	.equiv	GREEN_VAL, 0x3f
	.equiv	RED_VAL,   0x3f

	.equiv	BLUE_VAL2,  0x3f
	.equiv	GREEN_VAL2, 0x3f
	.equiv	RED_VAL2,   0x3f
#else
	.equiv	BLUE_VAL,  0x0a		//5
	.equiv	GREEN_VAL, 0x15		//6
	.equiv	RED_VAL,   0x15		//5

	.equiv	BLUE_VAL2,  0x15
	.equiv	GREEN_VAL2, 0x2a
	.equiv	RED_VAL2,   0x0a
#endif
#endif

.macro GetFrameSize rTmp1,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\rTmp1,\xres*\yres*BYTES_PER_PIXEL
.endm

.macro	GetSignalPolarity reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
//bit 28 vsync polarity(vsPol) 0 - active low
//bit 27 hsync polarity(hsPol)
//bit 25 pixel clock polarity (pPol)
	BigMov	\reg,(\vsPol<<28)+(\hsPol<<27)+((\oePol^1)<<26)+((\pPol^1)<<25)+(\dPol<<24)
.endm

.macro GetTimeConf	rTemp,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	.ifdef __ARMASM
	GBLA PixClksPerSec
	GBLA HspClksPerPixClk
	.endif
	.set	PixClksPerSec,(\xres+\xsyncWidth+\xbegin+\xend)*(\yres+\ysyncWidth+\ybegin+\yend)*(\vSyncHz)
	.set	HspClksPerPixClk16,(100663296*16) / PixClksPerSec
//bit 22 - falling edge*4,(from (Up + 1/2 period) to period)
//bit 12 - rising edge *4 (from 0 to (period - 1/2 HSP_CLK_PERIOD)*16)
//bit 0 - clock period *16 (# of HSP_CLKs * 16)
	BigMov	\rTemp,( (HspClksPerPixClk16>>(2+1))<<22)|(0<<12)|(HspClksPerPixClk16<<0)
//DA640X240
//pclk freq 12.58 Mhz (640+64+34+1) * (240+20+8+3) *60 = 739*271*60=12Mhz
//hsync 17Khz good
//vsync refresh 62.8Hz
//drdy0 lbias  17k khz
//
.endm

.macro	GetHorizontalSync reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\xsyncWidth-1)<<26)+((\xsyncWidth-1+\xres+\xbegin+\xend)<<16)
.endm
.macro	GetVerticalSync reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\ysyncWidth-1)<<26)+((\ysyncWidth-1+\yres+\ybegin+\yend)<<16)
.endm
.macro	GetBeginWaits reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\xbegin)<<16)+((\ybegin)<<0)
.endm

.macro	GetFrameSize1 reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
//108-96=12,   120-96=24
	BigMov	\reg,((\xres-1)<<12)|((\yres-1)<<24)
.endm
.macro	GetFrameSize2 reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\yres-1)>>8)
.endm

	.equiv	BBP_CODE32,0
	.equiv	BBP_CODE24,1
	.equiv	BBP_CODE16,2
	.equiv	BBP_CODE8,3

	.equiv	PIXEL_FORMAT_PALETTE,0	//interleaved fields
	.equiv	PIXEL_FORMAT_YUV444,1	//non-interleaved (all Y's together, all U's, all V's)
	.equiv	PIXEL_FORMAT_YUV422,2
	.equiv	PIXEL_FORMAT_YUV420,3
	.equiv	PIXEL_FORMAT_RGB,4		//interleaved fields (rgb then next pixel)
	.equiv	PIXEL_FORMAT_YUV422I,6	//(yyuv then next pixel)

#if (BYTES_PER_PIXEL==1)
//#warning BYTES_PER_PIXEL==1
	.equiv	PIXELS_PER_BURST,16	//32 make it drop pixels
	.equiv	BPP_CODE,BBP_CODE8
	.equiv	PIXEL_FORMAT,PIXEL_FORMAT_PALETTE
	.equiv	RED_MSB_OFFSET,0
	.equiv	GREEN_MSB_OFFSET,3
	.equiv	BLUE_MSB_OFFSET,6
	.equiv	ALPHA_MSB_OFFSET,8
	.equiv	RED_WIDTH,3
	.equiv	GREEN_WIDTH,3
	.equiv	BLUE_WIDTH,2
	.equiv	ALPHA_WIDTH,1		//>= 1, 0 not allowed
#endif

#if (BYTES_PER_PIXEL==2)
//#warning BYTES_PER_PIXEL==2
	.equiv	PIXELS_PER_BURST,16
	.equiv	BPP_CODE,BBP_CODE16
	.equiv	PIXEL_FORMAT,PIXEL_FORMAT_RGB		//rgb
	.equiv	RED_MSB_OFFSET,0
	.equiv	GREEN_MSB_OFFSET,5
	.equiv	BLUE_MSB_OFFSET,11
	.equiv	ALPHA_MSB_OFFSET,16
	.equiv	RED_WIDTH,5
	.equiv	GREEN_WIDTH,6
	.equiv	BLUE_WIDTH,5
	.equiv	ALPHA_WIDTH,1		//>= 1, 0 not allowed
#endif

#if (BYTES_PER_PIXEL==3)
//#warning BYTES_PER_PIXEL==3
	.equiv	PIXELS_PER_BURST,10
	.equiv	BPP_CODE,BBP_CODE24
	.equiv	PIXEL_FORMAT,PIXEL_FORMAT_RGB		//rgb
	.equiv	RED_MSB_OFFSET,6
	.equiv	GREEN_MSB_OFFSET,12
	.equiv	BLUE_MSB_OFFSET,18
	.equiv	ALPHA_MSB_OFFSET,0
	.equiv	RED_WIDTH,6
	.equiv	GREEN_WIDTH,6
	.equiv	BLUE_WIDTH,6
	.equiv	ALPHA_WIDTH,6
#endif

.macro	GetFrameStrideBurst reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
//bits 2-0 : bits per pixel (0=32,1=24,2=16,3=8,4=4,5=1)
//bits 16-3: stride-1 in bytes
//bits 19-17: format  0=code, 4=rgb, 6=yuv, 7=generic data
//bits 22-20: flipping, rotation
//bits 30-25: burst size in pixels
	BigMov	\reg,(BPP_CODE<<0)|(((\xres*BYTES_PER_PIXEL)-1)<<3)|(PIXEL_FORMAT<<17)|((PIXELS_PER_BURST-1)<<25)
.endm

.macro StoreData rVal,rBase,rCopy
	str		\rVal,[\rBase,#IPU_IMA_DATA]
//	str		\rVal,[\rCopy],#4
.endm

//In: rFrameBuffer
//Out: rFrameBuffer unchanged, other regs trashed
.macro	InitLCD rFrameBuffer,rBase,rTemp,rIndex,rCopy
//	mov		\rCopy,\rFrameBuffer
	BigMov	\rBase,IPU_BASE
	BigMov	\rTemp,0	//Disable Displays, little endian
	str		\rTemp,[\rBase,#IPU_CONF]

	str		\rTemp,[\rBase,#IPU_CHAN_BUF0_RDY]
	str		\rTemp,[\rBase,#IPU_CHAN_BUF1_RDY]
	
	BigMov	\rTemp,((8-1)<<4)		//8 consecutive requests, round robin
	str		\rTemp,[\rBase,#IDMAC_CONF]		//dual AHB mode

//Configuring Display Interface
	BigMov	\rTemp,0
	str		\rTemp,[\rBase,#DI_DISP_IF_CONF]

	GetSignalPolarity	\rTemp,DEF_P
	str		\rTemp,[\rBase,#DI_DISP_SIG_POL]
	GetTimeConf	\rTemp,DEF_P
	str		\rTemp,[\rBase,#DI_DISP3_TIME_CONF]

	BigMov	\rTemp,(5<<16)|(0xf)		//low 2 bits masked
	str		\rTemp,[\rBase,#DI_DISP3_B0_MAP]
	BigMov	\rTemp,(11<<16)|(0xf)		//low 2 bits masked
	str		\rTemp,[\rBase,#DI_DISP3_B1_MAP]
	BigMov	\rTemp,(17<<16)|(0xf)		//low 2 bits masked
	str		\rTemp,[\rBase,#DI_DISP3_B2_MAP]

	BigMov	\rTemp,0		//1 clock cyle per pixel
	str		\rTemp,[\rBase,#DI_DISP_ACC_CC]

	BigMov	\rTemp,(1<<6)|(1<<0)	//Background disable,Global Alpha,TFT Color
	str		\rTemp,[\rBase,#SDC_COM_CONF]
	
	GetHorizontalSync	\rTemp,DEF_P
	str		\rTemp,[\rBase,#SDC_HORIZONTAL_CONF]

	GetVerticalSync	\rTemp,DEF_P
	str		\rTemp,[\rBase,#SDC_VERTICAL_CONF]
	
//Configuring and Initializing Submodules
	BigMov	\rTemp,(1<<28)			//Global alpha
	str		\rTemp,[\rBase,#IC_CONF]

	BigMov	\rTemp,0
	str		\rTemp,[\rBase,#IC_CMBP1]	//global alpha
	str		\rTemp,[\rBase,#IC_CMBP2]	//key color

//IC Task Parameter Memory, (bits 15-3: max row 2239(0x8bf)) (bits 2-0: max word 1)
//	BigMov	\rTemp,0
//	str		\rTemp,[\rBase,#IPU_IMA_ADDR]
//	BigMov	\rTemp,0
//	str		\rTemp,[\rBase,#IPU_IMA_DATA]	//IC task parameters table 44-32

//SDC Configuring
	BigMov	\rTemp,(1<<6)|(1<<0)	//Background disable,Global Alpha,TFT Color
	str		\rTemp,[\rBase,#SDC_COM_CONF]

	BigMov	\rTemp,(255<<24)		//alpha = 1
	str		\rTemp,[\rBase,#SDC_GRAPH_WIND_CTRL]

	GetBeginWaits	\rTemp,DEF_P
	str		\rTemp,[\rBase,#SDC_BG_POS]
	str		\rTemp,[\rBase,#SDC_FG_POS]

//DI Configuring
//MCU_MAIN_CLK=201326592/ 2(PDR0:field HSP_PODF) = 100663296
	BigMov	\rTemp,(16<<16)|(16<<0)		//program for 100.6 Mhz
	str		\rTemp,[\rBase,#DI_HSP_CLK_PER]	//Initial HSP_CLK

//IDMAC Configuring
	BigMov	\rTemp,((8-1)<<4)		//8 consecutive requests, round robin
	str		\rTemp,[\rBase,#IDMAC_CONF]

	BigMov	\rTemp,0			//disable all channels
	str		\rTemp,[\rBase,#IDMAC_CHAN_EN]
	
//load IDMAC channel parameter, table 44-33 thru 44-37
//max row=63 (2 rows per channel), max word is 4
//SDC Channel	14 is background data (full refresh)
//				15 is Foreground data
//				16 is mask data
//				17 is background data (partial refresh)
	mov		\rIndex,#(1<<16)
	str		\rIndex,[\rBase,#IPU_IMA_ADDR]
90:	BigMov	\rTemp,0
	StoreData \rTemp,\rBase,\rCopy		//0
	BigMov	\rTemp,(1<<(46-32))
	StoreData \rTemp,\rBase,\rCopy		//1
	BigMov	\rTemp,0
	StoreData \rTemp,\rBase,\rCopy		//2
	GetFrameSize1	\rTemp,DEF_P
	StoreData \rTemp,\rBase,\rCopy		//3
	GetFrameSize2	\rTemp,DEF_P
	StoreData \rTemp,\rBase,\rCopy		//4

	StoreData \rFrameBuffer,\rBase,\rCopy		//0	EBA0
	StoreData \rFrameBuffer,\rBase,\rCopy		//1	EBA1
	GetFrameStrideBurst	\rTemp,DEF_P
	StoreData \rTemp,\rBase,\rCopy		//2
	BigMov	\rTemp,(2<<0)|(RED_MSB_OFFSET<<3)|(GREEN_MSB_OFFSET<<8)|(BLUE_MSB_OFFSET<<13)|(ALPHA_MSB_OFFSET<<18)|((RED_WIDTH-1)<<23)|((GREEN_WIDTH-1)<<26)|((BLUE_WIDTH-1)<<29)		//32 bit access
	StoreData \rTemp,\rBase,\rCopy		//3
	BigMov	\rTemp,((ALPHA_WIDTH-1)<<0)
	StoreData \rTemp,\rBase,\rCopy		//4
	add		\rIndex,\rIndex,#2<<3
	tst		\rIndex,#63<<3
	bne		90b

//load IDMAC decoding look-up-table, table 44-38
//palette used in 8 bit mode
	mov		\rIndex,#(2<<16)
	str		\rIndex,[\rBase,#IPU_IMA_ADDR]
	mov		\rTemp,#0
91:	StoreData \rTemp,\rBase,\rCopy
	add		\rTemp,\rTemp,#0x4000	//add to blue (2 bits)
	tst		\rTemp,#0xc000
	bne		91b
	add		\rTemp,\rTemp,#0x200000-0x10000	//add to green(3 bits)
	tst		\rTemp,#0xe00000
	bne		91b
	add		\rTemp,\rTemp,#0x20000000-0x1000000	//add to red(3 bits)
	tst		\rTemp,#0xe0000000
	bne		91b

//CM Configuring
	BigMov	\rTemp,0	//no double buffers
	str		\rTemp,[\rBase,#IPU_CHAN_DB_MODE_SEL]

	BigMov	\rTemp,-1		//reset all to buffer 0
	str		\rTemp,[\rBase,#IPU_CHAN_CUR_BUF]	//write 1 to clear

	BigMov	\rTemp,0
	str		\rTemp,[\rBase,#IPU_FS_PROC_FLOW]

	BigMov	\rTemp,0		//background source is MCU
	str		\rTemp,[\rBase,#IPU_FS_DISP_FLOW]	//table 44-168,169 (44.4.8.2)

//Initializing Display
	BigMov	\rTemp,(1<<6)	//Enable Display Interface
	str		\rTemp,[\rBase,#IPU_CONF]

//	BigMov	\rTemp,0
//	str		\rTemp,[\rBase,#DI_DISP_LLA_CONF]
//	BigMov	\rTemp,
//	str		\rTemp,[\rBase,#DI_DISP_LLA_DATA]

//Enable tasks
	BigMov	\rTemp,(1<<6)|(1<<4)	//Enable Display Interface & Synchronous display controller
	str		\rTemp,[\rBase,#IPU_CONF]

	BigMov	\rTemp,(1<<14)
	str		\rTemp,[\rBase,#IDMAC_CHAN_EN]

//	BigMov	\rTemp,(1<<28)			//Global alpha
//	str		\rTemp,[\rBase,#IC_CONF]
//Fill Frame buffer


	BigMov	\rTemp,(1<<14)
	str		\rTemp,[\rBase,#IPU_CHAN_BUF0_RDY]	//section 44.4.8.2.2

	BigMov	\rTemp,(1<<9)|(1<<6)|(1<<0)	//Background Enable,Global Alpha,TFT Color
	str		\rTemp,[\rBase,#SDC_COM_CONF]
.endm

.macro dbgChar	char
	BigMov	ip,UART_BASE		//this is right if MMU is off
	mov	r4,r0
	mov	r0,#\char
	bl Transmit
	mov	r0,r4
.endm

//rFrameBuffer is start of frame buffer in memory
.macro SetupLcd	rFrameBuffer,rTmp0,rTmp1,rTmp2,rTmp3,rTmp4
	mov	\rTmp0,\rFrameBuffer
	GetFrameSize \rTmp1,DEF_P

#if (BYTES_PER_PIXEL==1)
//	dbgChar 0x41
	BigMov	\rTmp2,(((BLUE_VAL&0x3)<<0)|((GREEN_VAL&0x7)<<2)|((RED_VAL&0x7)<<5)) | ((((BLUE_VAL2&0x3)<<0)|((GREEN_VAL2&0x7)<<2)|((RED_VAL2&0x7)<<5))<<8)		//rgb value
	orr		\rTmp2,\rTmp2,\rTmp2,LSL #16
79:	subs	\rTmp1,\rTmp1,#4
	str		\rTmp2,[\rTmp0],#4
	bne		79b
#endif
#if (BYTES_PER_PIXEL==2)
//	dbgChar 0x42
	BigMov	\rTmp2,((BLUE_VAL<<0)|(GREEN_VAL<<5)|(RED_VAL<<11)) | (((BLUE_VAL2<<0)|(GREEN_VAL2<<5)|(RED_VAL2<<11))<<16)		//rgb value
//	orr		\rTmp2,\rTmp2,\rTmp2,LSL #16
79:	subs	\rTmp1,\rTmp1,#4
	str		\rTmp2,[\rTmp0],#4
	bne		79b
#endif
#if (BYTES_PER_PIXEL==3)
//	dbgChar 0x43
	BigMov	\rTmp2,(BLUE_VAL<<0)|(GREEN_VAL<<6)|(RED_VAL<<12)		//rgb value
	orr		\rTmp2,\rTmp2,\rTmp2,LSL #24

	mov		\rTmp3,\rTmp2,LSR #8
	orr		\rTmp3,\rTmp3,\rTmp3,LSL #24

	mov		\rTmp4,\rTmp3,LSR #8
	orr		\rTmp4,\rTmp4,\rTmp4,LSL #24
79:	subs	\rTmp1,\rTmp1,#12
	stmia	\rTmp0!,{\rTmp2,\rTmp3,\rTmp4}
	bne		79b
#endif

	InitLCD	\rFrameBuffer,\rTmp0,\rTmp1,\rTmp2,\rTmp3
.endm



