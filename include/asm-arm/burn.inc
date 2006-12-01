	.equiv	FLASH_GPIO_STATUS, 0				//1 means use gp to tell when flash is ready

#if (PLATFORM_TYPE==NEONB)
	.equiv	FLASH_BASE_ADDRESS, 0x04000000
#else
	.equiv	FLASH_BASE_ADDRESS, 0x0
#endif


	.equiv	FLASH_ID_CMD,		((0x0090<<16) + 0x0090)			//read Identifier Codes
	.equiv	FLASH_STATUS_CMD,	((0x0070<<16) + 0x0070)
	.equiv	FLASH_STATUS_CLEAR_CMD,	((0x0050<<16) + 0x0050)
	.equiv	FLASH_READ_CMD,		((0x00FF<<16) + 0x00FF)			//Read Array/Reset
	.equiv	FLASH_ERASE_CMD,	((0x0020<<16) + 0x0020)			//Block Erase
	.equiv	FLASH_CONFIRM_CMD,	((0x00D0<<16) + 0x00D0)			//Block Erase and Program Resume
//	.equiv	FLASH_WRITE_CMD,	((0x0040<<16) + 0x0040)			//Program word
	.equiv	FLASH_WRITE_TO_BUFFER_CMD,	((0x00E8<<16) + 0x00E8)			//Program word

	.equiv	FLASH_SUCCESS_RSP,	((0x0080<<16) + 0x0080)

	.equiv	stmicro_manCode,	((0x0020<<16) + 0x0020)
	.equiv	intel_manCode,		((0x0089<<16) + 0x0089)
	.equiv	SECT_SIZE,			(1<<18)		//256k

	.equiv	f320j3a_devCode,	((0x0016<<16) + 0x0016)
	.equiv	f320j3a_NUM_SECTORS, 32		//for 8M flash
	.equiv	f320j3a_SIZE,		(f320j3a_NUM_SECTORS*SECT_SIZE)

	.equiv	f640j3a_devCode,	((0x0017<<16) + 0x0017)
	.equiv	f640j3a_NUM_SECTORS, 64		//for 16M flash (8 meg on 16bit boards)
	.equiv	f640j3a_SIZE,		(f640j3a_NUM_SECTORS*SECT_SIZE)

	.equiv	f128j3a_devCode,	((0x0018<<16) + 0x0018)
	.equiv	f128j3a_NUM_SECTORS, 128		//for 32M flash
	.equiv	f128j3a_SIZE,		(f128j3a_NUM_SECTORS*SECT_SIZE)
// *****************************
	.equiv	MANCODE, 0
	.equiv	DEVCODE, 4



#define rRet		r6
#define rRamSectorEnd	r7
#define rRamSector	rNum1
#define rRamEnd		rNum2
#define rFlashBase	sl	//r10
#define rFlashSector	fp	//r11

	.equiv	CMD_BURN, 0
	.equiv	CMD_VERIFY, 1
	.equiv	CMD_BURNALL, 2

.macro delayCnt rTemp,cnt
	mov	\rTemp,#\cnt
90:	subs	\rTemp,\rTemp,#1
	bne	90b
.endm
// *********************************
//default delay is for write recovery before read, allow 400 MHZ cpu
.macro delay
	delayCnt r4,((40/10)*COUNT_MULT)
.endm



.macro FLASH_GPIO_WAIT_FOR_IDLE
	.if  FLASH_GPIO_STATUS
	bl FlashGpioWaitForIdle
	.endif
.endm

.macro DEFINE_FLASH_GPIO_WAIT_FOR_IDLE
	.if  FLASH_GPIO_STATUS
FlashGpioWaitForIdle:
//500 ns delay for STS going low
	delayCnt r4,((500/10)*COUNT_MULT)	//500ns /10ns (for 100MHZ cpu cycle time) * 4 (100MHZ cpu)
	mov	r5,#0x300000
	BigMov	r4,GPIO_BASE
91:	ldr	r2,[r4,#GPLR0]
	and	r2,r2,#3
	cmp	r2,#3
	subnes	r5,r5,#1
	bne	91b
	mov	pc,lr			//return
	.endif
.endm

//out: z-1 timeout
.macro	waitForReady istr,ildr,mask,inc
	BigMov	r2,FLASH_STATUS_CMD&\mask
	\istr	r2,[r1,#-\inc]

	FLASH_GPIO_WAIT_FOR_IDLE
	BigMov	r3,FLASH_SUCCESS_RSP&\mask

	mov	r5,#0x300000*COUNT_MULT
	.if  FLASH_GPIO_STATUS
	b	92f
	.endif

71:
	delayCnt r4,((500/10)*COUNT_MULT)
92:	\ildr	r4,[r1,#-\inc]	//!!! read status
	and	r4,r4,r3
	cmp	r4,r3
	subnes	r5,r5,#1
	bne	71b

	BigMov	r2,FLASH_READ_CMD&\mask
	\istr	r2,[r1,#-\inc]
	delay
	teq	r5,#0
.endm



//out: z-1 timeout
.macro	WaitForEraseDone istr,ildr,mask
	FLASH_GPIO_WAIT_FOR_IDLE
	BigMov	r3,FLASH_SUCCESS_RSP&\mask
	mov	r5,#0x10000
92:
	delayCnt r4,0x3000
	\ildr	r4,[rFlashSector]
	and	r4,r4,r3
	cmp	r4,r3
	subnes	r5,r5,#1
	bne	92b

	BigMov	r2,FLASH_READ_CMD&\mask
	\istr	r2,[rFlashSector,#0]
	delay
	teq	r5,#0
.endm
// ***********************************
//rFlashBase(sl,r10) - flash base
//Out: r0 - size if z-1
//		if z-0, flash is of unknown type
.macro	GetFlashSizeRtn istr,ildr,mask,shift,sizeShift,inc,plait
	BigMov	r0,FLASH_STATUS_CLEAR_CMD&\mask
	\istr	r0,[rFlashBase,#0]
	delay
	BigMov	r0,FLASH_ID_CMD&\mask
	\istr	r0,[rFlashBase,#0]
	delay
	BigMov	r4,intel_manCode&\mask
	BigMov	r1,stmicro_manCode&\mask
	BigMov	r5,f320j3a_devCode&\mask
	\ildr	r2,[rFlashBase,#MANCODE>>\shift]
	\ildr	r3,[rFlashBase,#DEVCODE>>\shift]

	BigMov	r0,FLASH_READ_CMD&\mask
	\istr	r0,[rFlashBase,#0]
	delayCnt r0,((40/10)*COUNT_MULT)

	mov	r0,#f320j3a_SIZE>>\sizeShift
	cmp	r3,r5

	BigMovNe r5,f128j3a_devCode&\mask
	movne	r0,#f128j3a_SIZE>>\sizeShift
	cmpne	r3,r5

	BigMovNe r5,f640j3a_devCode&\mask
	movne	r0,#f640j3a_SIZE>>\sizeShift
	cmpne	r3,r5

//	movne	r0,#0		//if no device codes match, then unknown size
	bne		99f
	cmp		r2,r4		//is it intel
	cmpne	r2,r1		//is it stmicro
99:
.endm

//r5 - command - CMD_BURN or CMD_BURNALL
//rRet(r6) succesful burn return address
//rRamSectorEnd(r7) - but not an input, just how it is used
//rRamSector(r8) - start
//rRamEnd(r9) - end
//rFlashBase(sl,r10) - flash base
//rFlashSector(fp,r11) - starting flash sector
//rDBG(sp,r13)
//lr - return address on failure

.macro	BurnRtn istr,ildr,mask,shift,sizeShift,inc,plait
	str	r5,[rDBG,#DBG_TEMP]
	GetFlashSizeRtn \istr,\ildr,\mask,\shift,\sizeShift,\inc,\plait
	movne	r0,#MANCODE>>2
	movne	r1,#DEVCODE>>2
	movne	pc,lr			//return if unrecognized chip

	sub	r1,rRamEnd,rRamSector
	sub	r2,rFlashSector,rFlashBase
	add	r1,r1,r2
	cmp	r1,r0
	movhi	pc,lr			//return if trying to write too much

	ldrb	r5,[rDBG,#DBG_TEMP]
	cmp	r5,#CMD_BURNALL
	streq	r0,[rDBG,#DBG_TEMP]	//r0 is size of flash

	FLASH_GPIO_WAIT_FOR_IDLE
1:
	bl	PrintSector

//Now see if block needs erased
	add	rRamSectorEnd,rRamSector,#SECT_SIZE>>\shift
	cmp	rRamSectorEnd,rRamEnd
	movhi	rRamSectorEnd,rRamEnd
	b	3f
2:
	ldr	r2,[r0],#4	//read ram
	ldr	r3,[r1],#4	//read flash
	and	r4,r3,r2
	cmp	r4,r2
	bne	10f
3:	cmp	r0,rRamSectorEnd
	blo	2b


	ldr	r2,[rDBG,#DBG_TEMP]
	movs	r2,r2
	beq	20f
	add	r0,rFlashSector,#SECT_SIZE>>\shift
	b	62f
61:
	ldr	r2,[r1],#4	//read flash
	adds	r2,r2,#1
	bne	10f
62:	cmp	r1,r0
	blo	61b
	b	19f		//erase not needed

//erase
10:
	BigMov	r3,FLASH_READ_CMD&\mask
	\istr	r3,[rFlashBase,#0]
	delay
	ldr	r3,[r1,#-4]	//read flash
	and	r4,r3,r2
	cmp	r4,r2
	beq	3b		//br if (we signal) glitch caused a spurious status register read

	BigMov	r0,FLASH_ERASE_CMD&\mask
	\istr	r0,[rFlashSector]
	BigMov	r0,FLASH_CONFIRM_CMD&\mask
	\istr	r0,[rFlashSector]
	bl	PrintErasing

	WaitForEraseDone \istr,\ildr,\mask
	beq	Timeout		//br if erase timed out

	.if (\plait)
	BigMov	r0,FLASH_ERASE_CMD&\mask
	\istr	r0,[rFlashSector,#2]
	BigMov	r0,FLASH_CONFIRM_CMD&\mask
	\istr	r0,[rFlashSector,#2]
	bl	PrintErasing

	WaitForEraseDone \istr,\ildr,\mask
	beq	Timeout		//br if erase timed out
	.endif

19:	cmp	rRamSector,rRamSectorEnd
	bhs	42f

//programming
20:
	bl	PrintProgramming
21:
	ldr	r2,[r0],#4	//read ram
	ldr	r3,[r1],#4	//read flash
	cmp	r2,r3
	bne	22f
28:
	cmp	r0,rRamSectorEnd
	blo	21b
	b	30f		//goto verify

22:
	sub	r1,r1,#4
	sub	r0,r0,#4
	and	r2,r1,#0x3f>>\shift			//align to a 64 byte boundary (32 per device)
	sub	r1,r1,r2
	sub	r0,r0,r2

	BigMov	r2,FLASH_WRITE_TO_BUFFER_CMD&\mask
	\istr	r2,[r1]		//!!! write_to_buffer

	sub	r3,rRamSectorEnd,r0
	mov	r3,r3,LSR #2-\shift
	sub	r3,r3,#1
	cmp	r3,#0x0f
	movhi	r3,#0x0f
	orr	r2,r3,r3,LSL #16
	\istr	r2,[r1]		//!!! word cnt -1
25:	\ildr	r2,[r0],#\inc
	\istr	r2,[r1],#\inc	//!!! words
	subs	r3,r3,#1
	bpl	25b
	BigMov	r2,FLASH_CONFIRM_CMD&\mask
	\istr	r2,[r1,#-\inc]	//!!! confirm

	waitForReady \istr,\ildr,\mask,\inc

	.if (\plait)
	beq	88f
	sub	r1,r1,#\inc
	sub	r0,r0,#\inc
	and	r2,r1,#0x3f>>\shift			//align to a 64 byte boundary (32 per device)
	sub	r1,r1,r2
	sub	r0,r0,r2

	BigMov	r2,FLASH_WRITE_TO_BUFFER_CMD&\mask
	\istr	r2,[r1,#2]!		//!!! write_to_buffer
	sub	r3,rRamSectorEnd,r0
	add	r0,r0,#2
	mov	r3,r3,LSR #2-\shift
	sub	r3,r3,#1
	cmp	r3,#0x0f
	movhi	r3,#0x0f
	orr	r2,r3,r3,LSL #16
	\istr	r2,[r1]		//!!! word cnt -1
85:	\ildr	r2,[r0],#\inc
	\istr	r2,[r1],#\inc	//!!! words
	subs	r3,r3,#1
	bpl	85b
	BigMov	r2,FLASH_CONFIRM_CMD&\mask
	\istr	r2,[r1,#-\inc]	//!!! confirm

	waitForReady \istr,\ildr,\mask,\inc
	sub	r0,r0,#2
	sub	r1,r1,#2

	bne	28b
88:

	.else
	bne	28b
	.endif
	sub	rRamSector,r0,#0x40>>\shift	//whoops, timeout
	sub	rFlashSector,r1,#0x40>>\shift
	b	Timeout



//verify
30:
	bl	PrintVerifying
31:
	ldr	r2,[r0],#4	//read ram
	ldr	r3,[r1],#4	//read flash
	cmp	r2,r3
	bne	33f
32:	cmp	r0,rRamSectorEnd
	blo	31b
42:
	add	rRamSector,rRamSector,#SECT_SIZE>>\shift
	add	rFlashSector,rFlashSector,#SECT_SIZE>>\shift
	cmp	rRamSector,rRamEnd
	blo	1b		//goto next sector

	ldr	r2,[rDBG,#DBG_TEMP]
	add	r2,r2,rFlashBase
	cmp	rFlashSector,r2
	blo	1b

	b	PrintSuccess
33:
	BigMov	r3,FLASH_READ_CMD&\mask
	\istr	r3,[rFlashBase,#0]
	delay
	ldr	r3,[r1,#-4]	//read flash
	cmp	r2,r3
	beq	32b		//br if (we signal) glitch caused a spurious status register read
	b	ReturnError
.endm
