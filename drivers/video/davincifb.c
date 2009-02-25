/*
 * Module davincifb.c
 *
 * This module defines the frame-buffer (LCD) interface
 * for the Davinci platform. It will set up an 8-bit color
 * palette, first based upon the ROM palette, allowing override
 * by the 'bmp' command.
 *
 * Change History :
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */
#include <common.h>
#include "lcd_multi.h"
#include "asm/arch/davinci_vpbe.h"
#include "exports.h"
#include "asm/arch/hardware.h"
#ifdef CONFIG_CMD_I2C
#include <i2c.h>
#endif

static unsigned long palette[] = {
	0x000000,0x800000,0x008000,0x808000,0x000080,0x800080,0x008080,0xC0C0C0,
	0xC0DCC0,0xA6CAF0,0xFBFBFB,0x080808,0x101010,0x181818,0x212121,0x292929,
	0x313131,0x4A4A4A,0x5A5A5A,0x737373,0x7B7B7B,0x949494,0xA5A5A5,0xBDBDBD,
	0x4A4242,0x7B6B6B,0x736363,0x635252,0x9C7B7B,0x846363,0x7B5A5A,0x7B5252,
	0x7B4A4A,0x9C5A5A,0x945252,0x944A4A,0xAD4A4A,0xC60800,0x943931,0x731008,
	0xBD1808,0x94524A,0x7B3931,0xB52918,0x521008,0x941808,0xAD1808,0x520800,
	0xB57B73,0xAD3929,0xCE3118,0x420800,0x8C2918,0xD62100,0xB56B5A,0x631808,
	0x632110,0xBD634A,0xC65231,0xDE3908,0xDE3100,0x522110,0x4A1808,0xDE4210,
	0xE74A10,0x635A4A,0x211800,0x5A4A18,0x524210,0x393110,0x8C7308,0x5A5229,
	0x524A21,0x8C7B29,0x847318,0x736310,0x312900,0x5A4A00,0xDEC642,0x423900,
	0x847B39,0xD6C65A,0x635A18,0xB5A529,0x948418,0xD6BD18,0xA59410,0xCEB510,
	0xD6BD08,0xDECE4A,0xCEBD31,0xD6C631,0xDECE31,0xCEBD21,0xB5A508,0x4A4200,
	0xE7DE5A,0xA59C31,0xD6C618,0xDECE10,0xD6C608,0xE7D608,0xF7E708,0xDED65A,
	0x8CC65A,0x94C66B,0x84C652,0x84C65A,0x73BD4A,0x63B542,0x5AB539,0x52AD31,
	0x52B531,0x42AD21,0x528C42,0x103108,0x31A518,0x299C10,0x427339,0x7BB573,
	0x31A521,0x219418,0x293129,0x184218,0x297329,0x39A539,0x319C31,0x216B21,
	0x298C29,0x082108,0x187318,0x082908,0x105A10,0x103918,0x848C8C,0x424A4A,
	0x293131,0x212929,0x182121,0x52636B,0x4AA5CE,0x4AB5E7,0x9CC6DE,0x299CDE,
	0x7BB5DE,0x6BA5CE,0x318CCE,0x218CD6,0x087BCE,0xADCEE7,0x7BA5C6,0x0873C6,
	0x8CB5D6,0x84B5DE,0x73A5CE,0x73ADDE,0x107BD6,0xB5C6D6,0x94A5B5,0x526373,
	0x314252,0x182939,0x0863B5,0x086BC6,0xE7EFF7,0xCEDEEF,0xBDD6EF,0xA5BDD6,
	0x94ADC6,0x212931,0x7B9CBD,0x5A738C,0x6B8CAD,0x84ADD6,0x739CC6,0x6B94BD,
	0x4A6B8C,0x527BA5,0x6BA5DE,0x1873CE,0x3973B5,0x6384AD,0x638CBD,0x527BAD,
	0x42638C,0x296BBD,0x9CB5D6,0x8CA5C6,0x7B94B5,0x6B84A5,0x314A6B,0x29528C,
	0x215AA5,0x7B8CA5,0x394A63,0x184A94,0x1052B5,0x083984,0x08429C,0xADBDD6,
	0x4A5A73,0x42526B,0x293952,0x314A73,0x3963A5,0xDEE7F7,0x7B8494,0x425A84,
	0x4A5A7B,0x212939,0x425273,0x394A6B,0x314263,0x082152,0x002163,0xADB5C6,
	0x8C94A5,0x737B8C,0x31394A,0x293142,0x102963,0x081842,0x5A637B,0x4A526B,
	0x4A527B,0xADADB5,0x9C9CA5,0x6B6B73,0x63636B,0x52525A,0x5A5A63,0x4A4A52,
	0x42424A,0x6B6B7B,0x636373,0x212129,0x4A4A63,0x181821,0x101018,0x4A424A,
	0x211821,0x846B7B,0x5A4A52,0x63525A,0x846B73,0x000000,0xFFFBF0,0xA0A0A4,
	0x808080,0xFF0000,0x00FF00,0xFFFF00,0x0000FF,0xFF00FF,0x00FFFF,0xFFFFFF
};

#define C0_0813  5328		/* Need to divide by 65536 */
#define C0_1140  7471
#define C0_1687 11056
#define C0_2990 19595
#define C0_3313 21712
#define C0_4187 27440
#define C0_5000 32768
#define C0_5870 38469


static void rgbToYuv( unsigned rgb, unsigned char *y, unsigned char *u, unsigned char *v )
{
	int R = (rgb&0xff0000)>>16 ;
	int G = (rgb&0xFF00)>>8 ;
	int B = rgb&0xFF ;

	int Y = (C0_2990*R + C0_5870*G + C0_1140*B)>>16;
	int Cb = ((-C0_1687*R - C0_3313*G + C0_5000*B)>>16) + 128;
	int Cr = (( C0_5000*R - C0_4187*G - C0_0813*B)>>16) + 128;
	*y = (signed char)Y ;
	*u = (signed char)Cb ;
	*v = (signed char)Cr ;
}

static void set_palette(unsigned long *colors, unsigned colorCount)
{
        unsigned i ;
	for( i = 0 ; i < colorCount ; i++ ){
		unsigned char y, u, v ;
		unsigned long regValue ;
		rgbToYuv(colors[i],&y,&u,&v);
                while( 0 != ( REGVALUE(OSD_MISCCT) & OSD_MISCCT_CPBSY) )
			;
		regValue = ((unsigned)y << OSD_CLUTRAMY_Y_SHIFT) | u ;
		REGVALUE(OSD_CLUTRAMYC) = regValue ;
                regValue = ((unsigned)v << OSD_CLUTRAM_CR_SHIFT) | i ;
		REGVALUE(OSD_CLUTRAMC)  = regValue ;
	}
	REGVALUE(OSD_OSDWIN0MD) |= OSD_OSDWIN0MD_CLUTS0 ;
}

static unsigned long get_palette_color(unsigned char idx)
{
	return palette[idx];
}

static void disable(void)
{
}

void disable_lcd_panel( void )
{
}

#define NUM_WINDOWS 4

static unsigned encPerPixel = 1 ;

static void setPixClock( unsigned long mhz )
{
	unsigned highError, lowError ;
	unsigned long divisor, high, low ;
	unsigned long pllIn = 27000000*(REGVALUE(PLL2_PLLM)+1);
	char *enc_spec = getenv( "encperpix" );
	if( enc_spec ){
		encPerPixel = simple_strtoul(enc_spec,0,0);
		if( 0 == encPerPixel ){
			printf( "Invalid enc_per_pixel" );
			encPerPixel = 1 ;
		}
	}
	divisor = pllIn/(encPerPixel*mhz);

	// choose nearest
	if( divisor < 16 ){
		high = pllIn/(encPerPixel*divisor);
		low = pllIn/(encPerPixel*(divisor+1));
		highError = high-mhz ;
		lowError = mhz-low ;
		if( lowError < highError )
			divisor++ ;
	}
	printf( "Pixel clock %lu/%u/%u == %lu\n", pllIn, divisor, encPerPixel, pllIn/(encPerPixel*divisor) );
	divisor-- ; // register value is divisor-1
	REGVALUE(PLL2_DIV1) = 0x8000 + divisor ;
	REGVALUE(PLL2_CMD) = 1 ;
}

#ifdef CONFIG_CMD_I2C
struct i2c_registers_t {
	unsigned char regno ;
	unsigned char value ;
};
static struct i2c_registers_t const i2c_static_regs[] = {
   { 0x1c, 0x00 }		// data path control
,  { 0x38, 0x87 }		// dtg on/mode VESA slave
,  { 0x4a, 0x89 }		// CSM clipping/scaling/mult factors
,  { 0x4b, 0x11 }
,  { 0x4c, 0x80 }
,  { 0x4d, 0x80 }
,  { 0x4e, 0x80 }
,  { 0x4f, 0xc0 }
};
static unsigned num_static_i2c = sizeof(i2c_static_regs)/sizeof(i2c_static_regs[0]);

#define THS8200_ADDR 0x21

static int i2c_field(unsigned char regno, unsigned value, unsigned startbit, unsigned numbits)
{
	unsigned mask = (1<<numbits)-1 ;
	unsigned char byte ;
	int rval ;
	value &= mask ;
	mask <<= startbit ;
	if( 0 == (rval = i2c_read(THS8200_ADDR,regno,1,&byte,1) ) ){
		byte &= ~mask ;
		byte |= (value<<startbit);
		if( 0 != (rval = i2c_write(THS8200_ADDR,regno,1,&byte,1) ) ){
			printf( "Error writing value of 0x%02x to THS8200 register 0x%02x\n", byte, regno );
		}
	}
	else
		printf( "Error reading THS8200 reg 0x%02x\n", regno );

	return rval ;
}

#endif

struct lcd_t *newPanel( struct lcd_panel_info_t const *info )
{
	unsigned i ;
	unsigned short val[4];
	int bit;
	int gbit;
	unsigned fbBytes = info->xres*info->yres ;
	struct lcd_t *lcd = (struct lcd_t *)malloc(sizeof(struct lcd_t));
	unsigned short totalh, totalv ;

        DECLARE_GLOBAL_DATA_PTR;
	memcpy(&lcd->info, info,sizeof(lcd->info));
	i = gd->bd->bi_dram[0].size;
	if (i > (128<<20)) i = (128<<20);
	lcd->fbAddr = (void *)( gd->bd->bi_dram[0].start + i - fbBytes );
        memset(lcd->fbAddr,0xff,fbBytes);

	lcd->fg = 0 ;
	lcd->bg = 255 ;
	lcd->x = lcd->y = 0 ;
	lcd->colorCount = 256 ;
	lcd->set_palette = set_palette ;
	lcd->get_palette_color = get_palette_color ;
	lcd->disable = disable ;

	REGVALUE(VPSS_CLKCTL) = 0x18 ;

	setPixClock(info->pixclock);

	REGVALUE(OSD_MODE) = 0 ;
	REGVALUE(OSD_OSDWIN0MD) = 0 ;

	REGVALUE(OSD_OSDWIN0ADR) = (unsigned long)lcd->fbAddr ;

	for( i = 0 ; i < NUM_WINDOWS ; i++ ){
                // first two windows are 2 bytes/pixel, second two are 1 byte
		unsigned bytesPerPixel = 2-(i/2);
		REGVALUE(OSD_WINOFST(i)) = info->xres/(32/bytesPerPixel);
                REGVALUE(OSD_WINXP(i)) = 0 ;
                REGVALUE(OSD_WINYP(i)) = 0 ;
		REGVALUE(OSD_WINXL(i)) = info->xres ;
		REGVALUE(OSD_WINYL(i)) = info->yres ;
	}

	REGVALUE(OSD_MODE) = (REGVALUE(OSD_MODE) & OSD_MODE_BCLUT) | 0 ;
	REGVALUE(OSD_MODE) = (REGVALUE(OSD_MODE) & OSD_MODE_CABG) | (0 << OSD_MODE_CABG_SHIFT);

	/* Field Inversion Workaround */
	REGVALUE(OSD_MODE) = 0x280 ;

	REGVALUE(OSD_BASEPX) = info->left_margin ;
	REGVALUE(OSD_BASEPY) = info->upper_margin ;

	/* Reset video encoder module */
	REGVALUE(VENC_VMOD) = 0 ;
	REGVALUE(VPSS_CLKCTL) = 0x09 ;	//disable DAC clock
	REGVALUE(VPBE_PCR) = 0 ;		//not divided by 2
	REGVALUE(VENC_VIDCTL) =((info->pclk_redg^1)<<14)|(1<<13);
	REGVALUE(VENC_SYNCCTL) = (info->vsyn_acth<<3)|(info->hsyn_acth<<2)^0x0f;
	REGVALUE(VENC_HSPLS) = info->hsync_len*encPerPixel;
	REGVALUE(VENC_VSPLS) = info->vsync_len ;
        totalh = info->xres+info->hsync_len+info->left_margin+info->right_margin ;
	REGVALUE(VENC_HINT) = (totalh-1)*encPerPixel;
	REGVALUE(VENC_HSTART) = info->left_margin*encPerPixel;
	REGVALUE(VENC_HVALID) = info->xres*encPerPixel;
        totalv = info->yres+info->vsync_len+info->upper_margin+info->lower_margin ;
	REGVALUE(VENC_VINT) = (totalv-1);
	REGVALUE(VENC_VSTART) = info->upper_margin ;
	REGVALUE(VENC_VVALID) = info->yres ;
	REGVALUE(VENC_HSDLY) = 0 ;
	REGVALUE(VENC_VSDLY) = 0 ;
	REGVALUE(VENC_RGBCTL) = 0 ;
	REGVALUE(PINMUX0) = (REGVALUE(PINMUX0) & ~(1<<22)) | (3<<23);	//rgb888 and output_enable
	REGVALUE(VENC_LCDOUT) = (info->oepol_actl<<1)|1 ;	//enable active high on gpio0

	val[0] = 0;
	val[1] = 0;
	val[2] = 0;
	val[3] = 0;
	gbit = (encPerPixel>>1);
	bit = (encPerPixel>>1);
	while (bit < 64) {
		val[bit>>4] |= (1<<(bit&0xf));
		bit++;
		gbit++;
		if (gbit>=encPerPixel) {
			gbit = (encPerPixel>>1);
			bit += gbit;
		}
	}
	bit -= gbit;
	REGVALUE(VENC_DCLKCTL) = ((encPerPixel==1)?(1<<11):0) | (bit-1);

	REGVALUE(VENC_DCLKPTN0) = val[0];
	REGVALUE(VENC_DCLKPTN1) = val[1];
	REGVALUE(VENC_DCLKPTN2) = val[2];
	REGVALUE(VENC_DCLKPTN3) = val[3];

	REGVALUE(VENC_DCLKHS) = 0 ;
	REGVALUE(VENC_DCLKHSA) = 0 ;
	REGVALUE(VENC_DCLKHR) = totalh ;
	REGVALUE(VENC_DCLKVS) = 0 ;
	REGVALUE(VENC_DCLKVR) = totalv ;

	val[0] = 0;
	bit = 0;
	while (bit < 16) {
		val[0] |= (1<<bit);
		bit+=encPerPixel;
	}
	if (bit>16) bit -= encPerPixel;
	REGVALUE(VENC_OSDCLK0) = bit-1 ;
	REGVALUE(VENC_OSDCLK1) = val[0];
	REGVALUE(VENC_OSDHAD) = 0 ;
	REGVALUE(VENC_VMOD) = (VENC_VMOD_VDMD_RGB666<<VENC_VMOD_VDMD_SHIFT)|VENC_VMOD_VMD|VENC_VMOD_VENC ;

	REGVALUE(OSD_OSDWIN0MD) = (3<<OSD_OSDWIN0MD_BMW0_SHIFT)
				| (7<<OSD_OSDWIN0MD_BLND0_SHIFT)
				| OSD_OSDWIN0MD_OACT0 ;

	printf( "%s: %ux%u @%p (%s)\n", __FUNCTION__, info->xres, info->yres, lcd->fbAddr, info->name );

#ifdef CONFIG_CMD_I2C

	if( i2c_probe(THS8200_ADDR) == 0){
		unsigned char byte ;
		unsigned short word ;


		printf( "Found THS8200 at address 0x%x\n", THS8200_ADDR );
		for( i = 0 ; i < num_static_i2c ; i++ ){
                        struct i2c_registers_t const *reg = i2c_static_regs+i;
			if (i2c_write(THS8200_ADDR, reg->regno, 1, (uchar *)&reg->value, 1) != 0)
				printf ("Error writing 0x%02x to address %02x\n", reg->value, reg->regno );
		}
		printf( "wrote %u static registers to THS8200\n", num_static_i2c );

		// set horizontal total length
		i2c_write(THS8200_ADDR, 0x35, 1, (uchar *)&totalh, 1 );
		i2c_field(0x34, totalh>>8, 0, 4);
		printf( "set horizontal total to 0x%x (regs 0x34/0x35)\n", totalh );

		// set vertical total length
		i2c_write(THS8200_ADDR, 0x3a, 1, (uchar *)&totalv, 1);
		i2c_field(0x39, (totalv>>8), 4, 3);
		printf( "set vertical total length to 0x%x (regs 0x39/0x3a)\n", totalv );

		totalv += 1 ;
		i2c_write(THS8200_ADDR, 0x3b, 1, (uchar *)&totalv, 1);
		i2c_field(0x39, (totalv>>8), 0, 3);
		printf( "set vertical field1 length to 0x%x (regs 0x39/0x3b)\n", totalv );

		// set horizontal pulse width and offset
		i2c_write(THS8200_ADDR, 0x70, 1, (uchar *)&info->hsync_len, 1 );
		byte = ((info->hsync_len>>8)&3)<<6 ;
		i2c_write(THS8200_ADDR, 0x72, 1, (uchar *)&info->left_margin, 1 );
		byte |= (info->left_margin>>8)&0x0f ;
		i2c_write(THS8200_ADDR, 0x71, 1, (uchar *)&byte, 1 );
		printf( "set hsync_len to %u (0x%x), offset %u (0x%x) in regs 0x70..0x72\n", info->hsync_len, info->hsync_len, info->left_margin, info->left_margin );

		byte = info->vsync_len+1 ;
		i2c_write(THS8200_ADDR, 0x73, 1, (uchar *)&byte, 1);
		printf( "wrote vsync len of %u(0x%x) to register 0x73\n", info->vsync_len,info->vsync_len);

		i2c_write(THS8200_ADDR,0x75,1,(uchar *)&info->yres,1);
		i2c_field(0x74,info->yres>>8,0,3);
		printf( "wrote vsync offset %u (0x%x) to registers 0x74..0x75\n", info->yres, info->yres );

		i2c_field(0x7b,0,0,3);
		byte = 0 ;
		i2c_write(THS8200_ADDR,0x7c,1,(uchar *)&byte,1);
		printf( "cleared input vertical delay fields\n" );

		i2c_write(THS8200_ADDR,0x72,1,(uchar *)&info->xres,1);
		i2c_field(0x71,info->xres>>8,0,5);
		printf( "save xres to registers 0x71..0x72\n" );

		word = info->left_margin - 2 ;
		i2c_write(THS8200_ADDR,0x7a,1,(uchar *)&word,1);
		i2c_field(0x79,word>>8,0,5);
		printf( "saved left margin and hsync_len to registers 0x79..0x7a\n" );

		byte = ((info->vsyn_acth<<0)|(info->hsyn_acth<<1)) ^ 0x40;
		byte |= (byte & 3) << 3;
		i2c_write(THS8200_ADDR, 0x82, 1, &byte, 1);
	}
#endif
	return lcd ;
}
