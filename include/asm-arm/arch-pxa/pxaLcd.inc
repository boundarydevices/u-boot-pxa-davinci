#if 1
	.equiv	RED_VAL,   0x0
	.equiv	GREEN_VAL, 0x0
	.equiv	BLUE_VAL,  0x0
#else
#if 0
	.equiv	RED_VAL,   0x1f
	.equiv	GREEN_VAL, 0x1f
	.equiv	BLUE_VAL,  0x1f
#else
	.equiv	RED_VAL,   0x15
	.equiv	GREEN_VAL, 0x2a
	.equiv	BLUE_VAL,  0x0a
#endif
#endif

.macro GetFrameSize rTmp1,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\rTmp1,\xres*\yres*BYTES_PER_PIXEL
.endm

.macro	CR0_INIT_VAL reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,(\enable<<CR0_ENABLE)+(1<<CR0_IM_DISABLE_DONE)+(1<<CR0_IM_START_OF_FRAME)+\
	(1<<CR0_IM_INPUT_FIFO_UNDERRUN)+(1<<CR0_IM_END_OF_FRAME)+(\active<<CR0_ACTIVE_SELECT)+\
	(1<<CR0_IM_LCD_QUICK_DISABLE)+(1<<CR0_IM_BRANCH)+(1<<CR0_IM_OUTPUT_FIFO_UNDERRUN)
.endm

.macro	CR1_INIT_VAL reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\xres-1)<<CR1_PIXELS_PER_LINE)+((\xsyncWidth-1)<<CR1_HORIZONTAL_SYNC_PULSE_WIDTH)+\
	((\xend-1)<<CR1_END_OF_LINE_WAIT)+((\xbegin-1)<<CR1_BEGINNING_OF_LINE_WAIT)
.endm

.macro	CR2_INIT_VAL reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
	BigMov	\reg,((\yres-1)<<CR2_LINES_PER_PANEL)+((\ysyncWidth-1)<<CR2_VERTICAL_SYNC_PULSE_WIDTH)+\
	(\yend<<CR2_END_OF_FRAME_WAIT)+(\ybegin<<CR2_BEGINNING_OF_FRAME_WAIT)
.endm

.macro	CR3_INIT_VAL reg,xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend, vsPol,hsPol,pPol,oePol,dPol, enable,unscramble,rotate,active,vSyncHz,type
//LCD Clock is same as memory clock
	.ifdef __ARMASM
	GBLA PixClksPerSec
	GBLA HspClksPerPixClk
	GBLA PCD_VAL
	.endif
	.set	PixClksPerSec,(\xres+\xsyncWidth+\xbegin+\xend)*(\yres+\ysyncWidth+\ybegin+\yend)*(\vSyncHz)
	.set	HspClksPerPixClk,100000000 / PixClksPerSec
	.set	PCD_VAL,((HspClksPerPixClk/2)-1)&0xff
	BigMov	\reg,(PCD_VAL<<CR3_PCLK_DIVISOR)+((256-1)<<CR3_ACBIAS_TOGGLE)+\
	(0<<CR3_API)+(\vsPol<<CR3_FCLK_POLARITY)+(\hsPol<<CR3_LCLK_POLARITY)+\
	(\pPol<<CR3_PCLK_POLARITY)+(\oePol<<CR3_BIAS_POLARITY)+\
	(BPP_FORMAT<<CR3_BITS_PER_PIXEL)+(0<<CR3_DOUBLE_PCLK)+(PDFOR<<CR3_PDFOR)
.endm

	.equiv	TMEDS_INIT_VAL,	(0xaa<<TMEDS_BLUE)+(0x55<<TMEDS_GREEN)+(0x00<<TMEDS_RED)
	.equiv	TMEDC_INIT_VAL,	(1<<TMEDC_MATRIX2_SELECT)+(3<<TMEDC_RESERVED)+(5<<TMEDC_HORIZONTAL_BEAT_SUPPRESSION)+(4<<TMEDC_VERTICAL_BEAT_SUPPRESSION)+(1<<TMEDC_FRAME_NUMBER_ADJUST_EN)+(1<<TMEDC_COLOR_OFFSET_ADJUST_EN)+(1<<TMEDC_FRAME_NUMBER_ADJUST_MATRIX2)+(1<<TMEDC_COLOR_OFFSET_ADJUST_MATRIX2)


.macro	InitLCD rDescript,rBase,rTemp

	BigMov	\rBase,LCD_CONTROL_BASE

	CR1_INIT_VAL	\rTemp,DEF_P
	str	\rTemp,[\rBase,#LCD_CR1]

	CR2_INIT_VAL	\rTemp,DEF_P
	str	\rTemp,[\rBase,#LCD_CR2]

	CR3_INIT_VAL	\rTemp,DEF_P
	str	\rTemp,[\rBase,#LCD_CR3]

	BigMov	\rTemp,0
	str	\rTemp,[\rBase,#LCD_FBR0]
	str	\rTemp,[\rBase,#LCD_FBR1]

	BigMov	\rTemp,TMEDS_INIT_VAL
	str	\rTemp,[\rBase,#LCD_TMEDS]
	BigMov	\rTemp,TMEDC_INIT_VAL
	str	\rTemp,[\rBase,#LCD_TMEDC]

	str	\rDescript,[\rBase,#LCD_FDADR0]

	CR0_INIT_VAL	\rTemp,DEF_P
	str	\rTemp,[\rBase,#LCD_CR0]
.endm


//rFrameBuffer is start of frame buffer in memory
.macro SetupLcd	rFrameBuffer,rTmp0,rTmp1,rTmp2,rTmp3,rTmp4
	.if __ENABLED_LCD_MASK
	mov	\rTmp0,\rFrameBuffer
	GetFrameSize \rTmp1,DEF_P
	add	\rTmp4,\rFrameBuffer,\rTmp1
	add	\rTmp4,\rTmp4,#0x3fc			//round up to a nice boundary
	bic	\rTmp4,\rTmp4,#0x3fc
	str	\rTmp4,[\rTmp4,#FDESC_FDADR]		//next frame descriptor address, loop to self
	str	\rFrameBuffer,[\rTmp4,#FDESC_FSADR]		//start of frame
	str	\rFrameBuffer,[\rTmp4,#FDESC_FIDR]		//id of frame, anything I want
	str	\rTmp1,[\rTmp4,#FDESC_DCMD]		//length of dma transfer

#if (BYTES_PER_PIXEL==2)
	BigMov	\rTmp2,(BLUE_VAL<<0)+(GREEN_VAL<<5)+(RED_VAL<<11)		//rgb value
	orr		\rTmp2,\rTmp2,\rTmp2,LSL #16
79:	str		\rTmp2,[\rTmp0],#4
	subs	\rTmp1,\rTmp1,#4
	bne		79b
#else
	BigMov	\rTmp2,(BLUE_VAL<<0)+(GREEN_VAL<<6)+(RED_VAL<<12)		//rgb value
	orr		\rTmp2,\rTmp2,\rTmp2,LSL #24

	mov		\rTmp3,\rTmp2,LSR #8
	orr		\rTmp3,\rTmp3,\rTmp3,LSL #24

	mov		\rTmp4,\rTmp3,LSR #8
	orr		\rTmp4,\rTmp4,\rTmp4,LSL #24
79:	stmia	\rTmp0!,{\rTmp2,\rTmp3,\rTmp4}
	subs	\rTmp1,\rTmp1,#12
	bne		79b
	add	\rTmp4,\rTmp0,#0x3fc			//round up to a nice boundary
	bic	\rTmp4,\rTmp4,#0x3fc
#endif

	InitLCD	\rTmp4,\rTmp0,\rTmp1	//\rTmp4 has descriptor address
	.endif
.endm
