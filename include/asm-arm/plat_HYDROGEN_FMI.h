#ifndef __PLAT_H__
#define __PLAT_H__ 1
#define PLAT_SELECTED		1
#define PLAT_GAME		0
#define PLAT_PHYS_FLASH_BASE	0x0
#define PLAT_AX88796_BASE	0x04000000	//nCS1
//#define PLAT_LAN91C111_BASE	0x10000000
//#define PLAT_SM501_BASE	0x0c000000
#define PLAT_PXA_LCD		1
#define PLAT_HAS_BTUART		1
#define PLAT_HAS_STUART		1

#define PLAT_JTAG_RELEASE_RESET 0
#define PLAT_JTAG_TEST_DISABLE_OUTPUTS 0

#define PLAT_RAM_64_OR_128MB	0
#define PLAT_RAM_32BIT_WIDE	1
#define PLAT_ROM_WIDTH		16
#define PLAT_ROM_TYPE		2
#define PLAT_ROM_PLAITED	0
//access time 70ns, 25ns after CS data becomes valid
//					RRR		RDN		RDF		RBW	RTX
//					gap between	Address to	1st access	16-bit	non-burst(0),sram(1)
//			1-fast		chip selects	data valid	delay		bus	4cycle(2),8cycle(3)
//			0-slow		recovery	2nd burst access			VLIO(4)
#define PLAT_CS0_MSC  (1<<15)+	(((0+1)>>1)<<12)+	((3-1)<<8)+	(13<<4)+		(PLAT_ROM_TYPE)	//fast device
#define PLAT_CS1_MSC  (1<<15)+	(1<<12)+		(3<<8)+		((5-1)<<4)+	(1<<3)+  1		//AX88796B, SRAM interface
#define PLAT_CS2_MSC  (1<<15)+	(4<<12)+		((6-2)<<8)+	((4-1)<<4)+	(1<<3)+  4
#define PLAT_CS3_MSC  (1<<15)+	(1<<12)+		(2<<8)+		((4-1)<<4)+	(0<<3)+  4		//NC
#define PLAT_CS4_MSC  (1<<15)+	(3<<12)+		(2<<8)+		((5-1)<<4)+	(0<<3)+  4		//SMC chip

#define PLAT_BYTES_PER_PIXEL	2
#define PLAT_PXALCD_SCRAMBLED	0
#define PLAT_TEST_PC_IN_RAM	1
#define PLAT_HAS_PCMCIA		0
#define PLAT_SUPPORTS_MULTI_GAME	0
#define PLAT_SRVAL_IS_SRVALC	1
#define PLAT_IS_PXA27X		1
#define PLAT_IS_PXA2XX		1

#define PLAT_GP_IN_MBREQ		-1
#define PLAT_GP_IN_VIO_READY		18
#endif

#ifdef __DEFINE_GPIO_OVERRIDES
	SPEC_GP  10,IN,HIGH,0		//MMC card detect
	SPEC_GP  11,OUT,LOW,0		//RS485 tx enable, high active
	SPEC_GP  12,IN,HIGH,0		//AX88796B IRQ input
//	SPEC_GP  16,OUT,LOW,0		//LCD backlight brightness control (Argon/Hydrogen - Okaya panel want GP16 low)
	SPEC_GP  36,IN,HIGH,0		//(in alt 1:FF_DCD) (out alt 1:USB_P2_4) 4 output Vbus Enable
#endif
