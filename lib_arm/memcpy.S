.global memcpy
// **************************************************************************************************
// **************************************************************************************************
.macro memcpyMac bitsValid,bitsInvalid
		cmp		r2,#(32*1)
		ble		3f
		pld		[r1,#32*1]
		cmp		r2,#(32*2)
		ble		2f
		pld		[r1,#32*2]
		cmp		r2,#(32*3)
		ble		2f
1:
		pld		[r1,#32*3]	//all fill buffers started now
2:
		ldmia	r1!, {r4 - r9, ip,lr}
		sub		r2,r2,#32
		cmp		r2,#32*3
		orr		r3,r3,r4,LSL #\bitsValid
		mov		r4,r4,LSR #\bitsInvalid
		orr		r4,r4,r5,LSL #\bitsValid
		mov		r5,r5,LSR #\bitsInvalid
		orr		r5,r5,r6,LSL #\bitsValid
		mov		r6,r6,LSR #\bitsInvalid
		orr		r6,r6,r7,LSL #\bitsValid
		mov		r7,r7,LSR #\bitsInvalid
		orr		r7,r7,r8,LSL #\bitsValid
		mov		r8,r8,LSR #\bitsInvalid
		orr		r8,r8,r9,LSL #\bitsValid
		mov		r9,r9,LSR #\bitsInvalid
		orr		r9,r9,ip,LSL #\bitsValid
		mov		ip,ip,LSR #\bitsInvalid
		orr		ip,ip,lr,LSL #\bitsValid
		stmia	r0!, {r3 - r9, ip}
		mov		r3,lr,LSR #\bitsInvalid
		bgt		1b
		cmp		r2,#32
3:		bge		2b
		movs	r2,r2
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 32
//<32 bytes remain
		cmp		r2,#16
		ldmgeia	r1!, {r4 - r7}
		subge	r2, r2, #16
		orrge	r3,r3,r4,LSL #\bitsValid
		movge	r4,r4,LSR #\bitsInvalid
		orrge	r4,r4,r5,LSL #\bitsValid
		movge	r5,r5,LSR #\bitsInvalid
		orrge	r5,r5,r6,LSL #\bitsValid
		movge	r6,r6,LSR #\bitsInvalid
		orrge	r6,r6,r7,LSL #\bitsValid
		stmgeia	r0!, {r3 - r6}
		movge	r3,r7,LSR #\bitsInvalid
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 16
//<16 bytes
		cmp		r2,#8
		ldmgeia	r1!, {r4,r5}
		subge	r2, r2, #8
		orrge	r3,r3,r4,LSL #\bitsValid
		movge	r4,r4,LSR #\bitsInvalid
		orrge	r4,r4,r5,LSL #\bitsValid
		stmgeia	r0!, {r3,r4}
		movge	r3,r5,LSR #\bitsInvalid
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 8
//<8 bytes
		cmp		r2,#4
		ldrge	r4, [r1], #4
		subge	r2, r2, #4
		orrge	r3,r3,r4,LSL #\bitsValid
		strge	r3, [r0], #4
		movge	r3,r4,LSR #\bitsInvalid
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 4
//<4 bytes
		cmp		r2,#2
	.if (\bitsValid <= 8)
		ldrgeh	r4, [r1], #2
		orrge	r3,r3,r4,LSL #\bitsValid
	.endif
		strgeh	r3, [r0], #2

	.if ((\bitsValid-16) <> 0)
		movgt	r3,r3,LSR #16
	.else
		ldrgtb	r3, [r1]
	.endif

		strneb	r3, [r0]			//str if 1 or 3 bytes
		ldmia	sp!,{r0,r4-r9,pc}
.endm
// **************************************************************************************************

//void *memcpy(void *dest, const void *src, size_t n);
//In: r0 - dest, r1 - src, r2 - length
//Out: r0 - unchanged
memcpy:
		cmp		r2,#1
		ble		15f			//br if 0 or 1 byte
		pld		[r1,#0]
		stmdb	sp!,{r0,r4-r9,lr}

		tst		r0,#1
		ldrneb	r3,[r1],#1
		subne	r2,r2,#1
		strneb	r3,[r0],#1

		cmp		r2,#1
		ble		25f

		tst		r1,#1
		bne		memcpyUnalign1

// **************************************************************************************************

memcpyAlign2Join:
		tst		r0,#2
		ldrneh	r3,[r1],#2
		subne	r2,r2,#2
		strneh	r3,[r0],#2

		cmp		r2,#1
		ble		25f

		tst		r1,#2
		bne		memcpyUnalign2


// **************************************************************************************************

memcpyAlign4Join:
		cmp		r2,#32
		blt		4f
		and		r3,r1,#0x1f
		add		r3,r3,r2		//add skipped part of 1st cacheline
		cmp		r3,#(32*1)
		ble		2f
		pld		[r1,#32*1]
		cmp		r3,#(32*2)
		ble		2f
		pld		[r1,#32*2]
		cmp		r3,#(32*3)
		ble		2f
		pld		[r1,#32*3]	//all fill buffers started now
		tst		r1,#0x1f
		beq		2f
1:
		pld		[r1,#32*3+28]	//make sure last dword is started
2:
		ldmia	r1!, {r3 - r9, lr}
		sub		r2,r2,#32
		cmp		r2,#32*3+28
		stmia	r0!, {r3 - r9, lr}
		bgt		1b
3:		add		r3,r1,r2
		cmp		r2,#32
		pld		[r3,#-1]
		bge		2b
		cmp		r2,#0
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 32
//<32 bytes remain
4:		cmp		r2,#16
		ldmgeia	r1!, {r3 - r6}
		subge	r2, r2, #16
		stmgeia	r0!, {r3 - r6}
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 16
//<16 bytes
		cmp		r2,#8
		ldmgeia	r1!, {r3,r4}
		subge	r2, r2, #8
		stmgeia	r0!, {r3,r4}
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 8
//<8 bytes
		cmp		r2,#4
		ldrge	r3, [r1], #4
		subge	r2, r2, #4
		strge	r3, [r0], #4
		ldmeqia	sp!,{r0,r4-r9,pc}		//return if size was a multiple of 4
//<4 bytes
		cmp		r2,#2
		ldrgeh	r3, [r1], #2
		ldrneb	r4, [r1]			//load if 1 or 3 bytes
		strgeh	r3, [r0], #2
		strneb	r4, [r0]			//str if 1 or 3 bytes
		ldmia	sp!,{r0,r4-r9,pc}

// **************************************************************************************************

memcpyUnalign2:
		ldrh	r3,[r1],#2	//align r1
		memcpyMac 16,16

//0 or 1 bytes
15:
		ldreqb	r3,[r1]
		streqb	r3,[r0]
		mov		pc,lr

//z-1 means r2 = 1, z-0 means r2 = 0
25:		ldreqb	r3,[r1]
		streqb	r3,[r0]
		ldmia	sp!,{r0,r4-r9,pc}
// **************************************************************************************************
// r0 aligned on 2 byte boundary, r1 is odd
memcpyUnalign1:
		tst		r0,#2
		ldrneb	r4,[r1,#1]
		ldrneb	r3,[r1],#2
		orrne	r3,r3,r4,LSL #8
		subne	r2,r2,#2
		strneh	r3,[r0],#2
// r0 aligned on 4 byte boundary, r1 is odd
		cmp		r2,#1
		ble		25b

		ldrb	r3,[r1],#1	//align r1
		tst		r1,#2
		bne		memcpyUnalign3
		memcpyMac 8,24

memcpyUnalign3:
		ldrh	r4,[r1],#2	//align r1
		orr		r3,r3,r4,LSL #8
		memcpyMac 24,8

// **************************************************************************************************
//
//void memcpyAlign4(void * dest,void *src,int n);
//IN: R0,R1 both multiples of 4

memcpyAlign4:
		cmp		r2,#1
		ble		15b
		pld		[r1,#0]
		stmdb	sp!,{r0,r4-r9,lr}
		b		memcpyAlign4Join
//
//void memcpyAlign2(void * dest,void *src,int n);
//IN: R0,R1 both multiples of 2
//
memcpyAlign2:
		cmp		r2,#1
		ble		15b
		pld		[r1,#0]
		stmdb	sp!,{r0,r4-r9,lr}
		b		memcpyAlign2Join
// **************************************************************************************************
// **************************************************************************************************
