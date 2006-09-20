#ifndef SM501_USB_INCLUDED
#define SM501_USB_INCLUDED
	
	#if defined(CONFIG_SM501)
		#define USB_GATE_MODE0	__REG(0xFE00040)
		#define USB_GATE_MODE1	__REG(0xFE00048)
		#define ENABLE_USBH	(1<<11)

		/*
		 * USB Host Controller
		 */
		#define USBH_BASE	0xFE40000
		#define UHCREV		__REG(0xFE40000)
		#define UHCHCON		__REG(0xFE40004)
		#define UHCCOMS		__REG(0xFE40008)
		#define UHCINTS		__REG(0xFE4000C)
		#define UHCINTE		__REG(0xFE40010)
		#define UHCINTD		__REG(0xFE40014)
		#define UHCHCCA		__REG(0xFE40018)
		#define UHCPCED		__REG(0xFE4001C)
		#define UHCCHED		__REG(0xFE40020)
		#define UHCCCED		__REG(0xFE40024)
		#define UHCBHED		__REG(0xFE40028)
		#define UHCBCED		__REG(0xFE4002C)
		#define UHCDHEAD	__REG(0xFE40030)
		#define UHCFMI		__REG(0xFE40034)
		#define UHCFMR		__REG(0xFE40038)
		#define UHCFMN		__REG(0xFE4003C)
		#define UHCPERS		__REG(0xFE40040)
		#define UHCLST		__REG(0xFE40044)
		#define UHCRHDA		__REG(0xFE40048)
		#define UHCRHDB		__REG(0xFE4004C)
		#define UHCRHS		__REG(0xFE40050)
		#define UHCRHPS1	__REG(0xFE40054)
		#define UHCRHPS2	__REG(0xFE40058)
		#define UHCRHPS3	__REG(0xFE4005C)
		#define UHCSTAT		__REG(0xFE40060)
		#define UHCHR		__REG(0xFE40064)
		#define UHCHIE		__REG(0xFE40068)
		#define UHCHIT		__REG(0xFE4006C)
		
		#define UHCHR_FSBIR	(1<<0)
		#define UHCHR_FHR	(1<<1)
		#define UHCHR_CGR	(1<<2)
		#define UHCHR_SSDC	(1<<3)
		#define UHCHR_UIT	(1<<4)
		#define UHCHR_SSE	(1<<5)
		#define UHCHR_PSPL	(1<<6)
		#define UHCHR_PCPL	(1<<7)
		#define UHCHR_SSEP0	(1<<9)
		#define UHCHR_SSEP1	(1<<10)
		#define UHCHR_SSEP2	(1<<11)
		
		#define UHCHIE_UPRIE	(1<<13)
		#define UHCHIE_UPS2IE	(1<<12)
		#define UHCHIE_UPS1IE	(1<<11)
		#define UHCHIE_TAIE	(1<<10)
		#define UHCHIE_HBAIE	(1<<8)
		#define UHCHIE_RWIE	(1<<7)
		
		#define UHCCOMS_HCR	1
		#define UHCRHS_LPS	1
		#define UHCHR_SSE	(1<<5)
		
	#endif
#endif
