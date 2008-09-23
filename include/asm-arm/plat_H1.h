#define PLAT_SELECTED		1
#define PLAT_GAME		0
#define PLAT_PHYS_FLASH_BASE	0x0
//#define PLAT_AX88796_BASE	0x04000000
#define PLAT_LAN91C111_BASE	0x10000000
//#define PLAT_SM501_BASE	0x0c000000
#define PLAT_PXA_LCD		1
#define PLAT_HAS_BTUART		1
#define PLAT_HAS_STUART		1

#define PLAT_JTAG_RELEASE_RESET 0
#define PLAT_JTAG_TEST_DISABLE_OUTPUTS 0

#define PLAT_RAM_64_OR_128MB	0
#define PLAT_RAM_32BIT_WIDE	1
#define PLAT_ROM_WIDTH		32
#define PLAT_ROM_TYPE		2
#define PLAT_ROM_PLAITED	0
//access time 70ns, 25ns after CS data becomes valid
//					RRR		RDN		RDF		RBW	RTX
//					gap between	Address to	1st access	16-bit	non-burst(0),sram(1)
//			1-fast		chip selects	data valid	delay		bus	4cycle(2),8cycle(3)
//			0-slow		recovery	2nd burst access			VLIO(4)
#define PLAT_CS0_MSC  (1<<15)+  (((0+1)>>1)<<12)+	((3-1)<<8)+	(13<<4)+		(PLAT_ROM_TYPE)	//fast device
#define PLAT_CS1_MSC  (1<<15)+	(3<<12)+		(2<<8)+		((3-1)<<4)+	(1<<3)+    4		//SMC chip
#define PLAT_CS3_MSC  (1<<15)+	(1<<12)+		(2<<8)+		((4-1)<<4)+	(0<<3)+    4		//NC
#define PLAT_CS4_MSC  (1<<15)+	(3<<12)+		(2<<8)+		((5-1)<<4)+	(0<<3)+    4		//SMC chip

#define PLAT_BYTES_PER_PIXEL	3
#define PLAT_PXALCD_SCRAMBLED	0
#define PLAT_TEST_PC_IN_RAM	1
#define PLAT_HAS_PCMCIA		0
#define PLAT_SUPPORTS_MULTI_GAME	0
#define PLAT_SRVAL_IS_SRVALC	1
#define PLAT_IS_PXA27X		1
#define PLAT_IS_PXA2XX		1

#define PLAT_GP_IN_USB_CLIENT_STATUS	-1
#define PLAT_GP_USB_CLIENT_READY	-1
#define PLAT_GP_OUT_I2C_POWER		-1
#define PLAT_GP_IN_IRQ_UCB1400		-1
#define PLAT_GP_IN_IRQP_UCB1400		-1
#define PLAT_GP_OUT_J12_PIN1		-1
#define PLAT_GP_OUT_MBGNT		-1
#define PLAT_GP_IN_MBREQ		-1
#define PLAT_GP_OUT_nCS1		-1
#define PLAT_GP_OUT_BACKLIGHT_PWM	16
#define PLAT_GP_OUT_BACKLIGHT_PWM_LEVEL	1
#define PLAT_GP_OUT_BACKLIGHT_ENABLE	17
#define PLAT_GP_IN_VIO_READY		18
#define PLAT_GP_IN_DREQ1		-1
#define PLAT_GP_IN_DREQ0		-1
#define PLAT_GP_IN_IRQ_SMSC		-1
#define PLAT_GP_IN_J12_PIN2		-1
#define PLAT_GP_IN_J12_PIN3		-1

#define PLAT_GP_IN_LOW_BATTERY		-1
#define PLAT_GP_IN_MMC_WP		-1
#define PLAT_GP_IN_IRQ_AX88796		-1
#define PLAT_GP_IN_MAGSTRIPE1		-1
#define PLAT_GP_IN_MAGSTRIPE2		-1
#define PLAT_GP_IN_MAGSTRIPE3		-1
#define PLAT_GP_IN_MAGSTRIPE4		-1
#define PLAT_GP_IN_IRQ_SM501		-1
#define PLAT_GP_OUT_SMSC_RESET		-1
#define PLAT_GP_IN_FF_DCD		-1
#define PLAT_GP_OUT_FF_DTR		40
#define PLAT_GP_IN_USB_OVER_CURRENT	-1
#define PLAT_GP_IN_USB_OVER_CURRENT105	105
#define PLAT_GP_OUT_USB_POWER_ENABLE	-1
