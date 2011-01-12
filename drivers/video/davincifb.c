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
#include <div64.h>
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
#define OSC_RATE 27000000

#if 1
#define DDR2_MIN	(140000000 * 2)	/* after div2 */
#define	DDR2_MAX	(200000000 * 2)
#else
#define DDR2_MIN	(161000000 * 2)	/* after div2 */
#define	DDR2_MAX	(163000000 * 2)
#endif

//#define MAX_VPBE	75018754	//75 Mhz Max clock input to back end (13.33ns/clock)
#define MAX_VPBE	112000000	//112 Mhz Max clock input to back end

static int check_ddr2(unsigned mrate)
{
	unsigned rate;
	unsigned div = ((mrate - 1) / DDR2_MAX) + 1;
	if (div > 16)
		return 0;
	rate = mrate /div;
//	printf("%s: rate:%u div:%u\n", __func__, rate, div);
	return (rate < DDR2_MIN) ? 0 : div;
}

struct clk_factors {
	u32 mult;
	u32 div;
	u32 enc_mult;
	u32 enc_div;
	u32 error;
};

int best_ddr2_speed(struct clk_factors *a, struct clk_factors *b)
{
	unsigned a_rate = OSC_RATE * a->mult;
	unsigned a_div = check_ddr2(a_rate);
	unsigned b_rate = OSC_RATE * b->mult;
	unsigned b_div = check_ddr2(b_rate);
	unsigned a_ddr, b_ddr;
	if (!b_div)
		return 0;
	if (!a_div)
		return 1;
	a_ddr = a_rate / a_div;
	b_ddr = b_rate / a_div;
	return (a_ddr >= b_ddr) ? 0 : 1;
}

static int relatively_prime(u32 a, u32 b)
{
	u32 c;
	if (a < b) {
		c = a;
		a = b;
		b = c;
	}
	do {
		if (b == 1)
			return 1;
		c = a % b;
		if (c == 0)
			break;
		a = b;
		b = c;
	} while (1);
	return 0;
}

int new_best(struct clk_factors *best, struct clk_factors *test)
{
	if (best->error > test->error) {
		if (best->enc_div != 1)
			return 1;
		if (test->enc_div == 1)
			return 1;
		if (((best->error * 3)/4) > test->error)
			return 1;
		/*
		 * Best error is larger, but not more than 33% larger
		 * and Best has advantage of a normal pixel clock
		 */
		return 0;
	}
	if (best->error < test->error) {
		if (test->enc_div != 1)
			return 0;
		if (best->enc_div == 1)
			return 0;
		if (((test->error * 3)/4) > best->error)
			return 0;
		/*
		 * Test error is larger, but not more than 33% larger
		 * and Test has advantage of a normal pixel clock
		 */
		return 1;
	}
	if (best->enc_div != test->enc_div) {
		if (best->enc_div == 1)
			return 0;
		if (test->enc_div == 1)
			return 1;
		/* This is not needed */
		if (0) if ((test->enc_div > best->div) &&
				relatively_prime(test->enc_div,test->enc_mult)) {
			return 1;
		}
	}
	return best_ddr2_speed(best, test);
}
//VENC_DCLKCTL encodes 1-64 clks, but VENC_OSDCLK0 encodes only 1-16 clks,
#define MAX_ENC_DIV 16

#define MAX_ENC_MULT 1	//was MAX_ENC_DIV - 1, but doesn't look good on CRTS
static unsigned query_clk_settings(u32 mhz, struct clk_factors *pbest)
{
	struct clk_factors best;
	struct clk_factors cur;
	u32 mrate;
	u32 enc_mult_start = 1;
	u32 enc_mult_end = MAX_ENC_MULT;
	u32 enc_div_start = 1;
	u32 enc_div_end = MAX_ENC_DIV;
	best.error = ~0;
	best.mult = 0;
	best.div = 0;
	best.enc_mult = 0;
	best.enc_div = 0;
	char *enc_spec = getenv( "encperpix" );
	unsigned force27 = getenv( "force27" ) ? 1 : 0;
	unsigned max_div = 16;
	if (enc_spec){
		u32 m = 1;
		u32 d;
		char *endptr ;
		d = simple_strtoul(enc_spec, &endptr, 0);
		if ((endptr != enc_spec) &&  *endptr) {
			enc_spec = endptr+1 ;
			m = simple_strtoul(enc_spec, &endptr, 0);
			if (m > d) {
				u32 tmp = m;
				m = d;
				d = tmp;
			}
		}
		if (d) {
			if (d > 64) {
				printf("Invalid encperpix(%u:%u)\n", m, d);
			} else {
				printf("forcing encperpix=%u:%u\n", m, d);
				enc_mult_start = m;
				enc_mult_end = m;
				enc_div_start = d;
				enc_div_end = d;
			}
		}
	}
	cur.mult = (DDR2_MIN + OSC_RATE -1) / OSC_RATE;
	mrate = cur.mult * OSC_RATE;
	for (; cur.mult <= 32; cur.mult++, mrate += OSC_RATE) {
		if (!check_ddr2(mrate))
			continue;
		cur.div = (mrate + MAX_VPBE - 1)/MAX_VPBE;
		if (force27) {
			cur.div = cur.mult;
			max_div = (cur.mult < 16) ? cur.mult : 16;
		}
		for (; cur.div <= max_div; cur.div++) {
			u32 vpbe = mrate / cur.div;
			cur.enc_mult = enc_mult_start;
//			printf("vpbe=%u, mhz=%u\n", vpbe, mhz);
			for (; cur.enc_mult <= enc_mult_end; cur.enc_mult++) {
				u32 rate;
				u64 vm = vpbe;
				vm *= cur.enc_mult;
				cur.enc_div = do_divq(vm, mhz);
				if (cur.enc_div > enc_div_end)
					cur.enc_div = enc_div_end;
				if (cur.enc_div < enc_div_start)
					cur.enc_div = enc_div_start;
				if (cur.enc_div <= cur.enc_mult) {
					cur.enc_div = cur.enc_mult;
					if (cur.enc_mult != enc_mult_start) {
						cur.enc_div++;
						if (cur.enc_div > enc_div_end)
							break;
					}
				}
				do {
					rate = do_divq(vm, cur.enc_div);
					cur.error = (rate > mhz) ? (rate - mhz) : (mhz - rate);
					if (0) printf("mult:%u div:%u enc:%u:%u error:%u\n",
						cur.mult, cur.div, cur.enc_mult, cur.enc_div, cur.error);
					if (new_best(&best, &cur)) {
						if (0) printf("%s: mult:%u div:%u enc:%u:%u error:%u\n", __func__,
							cur.mult, cur.div, cur.enc_mult, cur.enc_div, cur.error);
						best = cur;
					}
					cur.enc_div++;
				} while ((rate > mhz) && (cur.enc_div <= enc_div_end));
				if (vpbe <= mhz)
					break;
			}
			if (vpbe <= mhz)
				break;
		}
	}
	*pbest = best;
	mrate = 0;
	if (best.mult) {
		u64 m = OSC_RATE;
		m *= (best.mult * best.enc_mult);
		mrate = do_divq(m, (best.div * best.enc_div));
	}
	return mrate;
}

u32 query_pixel_Clock(u32 mhz)
{
	struct clk_factors clk;
	return query_clk_settings(mhz, &clk);
}
void reset_pll2(u32 mult, u32 div, u32 ddr2_div);

static int setPixClock(u32 mhz, u32 *penc_mult, u32 *penc_div)
{
	struct clk_factors clk;
	u32 pixel_clk = query_clk_settings(mhz, &clk);
	if (pixel_clk) {
		u32 mrate = OSC_RATE * clk.mult;
		u32 ddr2_div = check_ddr2(mrate);
		u32 ddr2_clk =  (mrate / ddr2_div) >> 1;
		printf( "Pixel clock %u, mult = %u, div = %u, enc = %u:%u, DDR2 clock = %u, div = %u\n",
				pixel_clk, clk.mult, clk.div, clk.enc_mult, clk.enc_div, ddr2_clk, ddr2_div);

#define GOSET	1
#define GOSTAT	1
		udelay(1);	/* cache routine */
		clk.mult--;
		clk.div--;
		ddr2_div--;

		if (REGVALUE(PLL2_PLLM) != clk.mult) {
			reset_pll2(clk.mult, clk.div, ddr2_div);
		} else if (REGVALUE(PLL2_DIV1) != (clk.div | 0x8000)) {
			/*
			 * Changing only the pixel clock divisor
			 */
			while (REGVALUE(PLL2_STAT) & GOSTAT);
			REGVALUE(PLL2_DIV1) = clk.div | 0x8000;

			REGVALUE(PLL2_CMD) = GOSET;
			while (REGVALUE(PLL2_STAT) & GOSTAT);
		}
		*penc_mult = clk.enc_mult;
		*penc_div = clk.enc_div;
	} else {
		printf( "%s: no clock found\n", __func__);
	}
	return pixel_clk;
}

#ifdef CONFIG_CMD_I2C
struct i2c_registers_t {
	unsigned char regno ;
	unsigned char value ;
};
static struct i2c_registers_t const i2c_static_regs[] = {
   { 0x1c, 0x38 }		// data path control: no ifir filters
,  { 0x38, 0x87 }		// dtg on/mode VESA slave
,  { 0x4a, 0x09 }		// mult off, CSM clipping/scaling/mult factors (0x8a, for u-boot shadow)
,  { 0x4b, 0x11 }		// multiplication constants: (0.7V/1.3V)*1024 == 0x2e... brightened til U-Boot shadow disappears (0x22)
,  { 0x4c, 0x80 }
,  { 0x4d, 0x80 }
,  { 0x4e, 0x80 }
,  { 0x4f, 0x00 }		// mult off
};
static unsigned num_static_i2c = sizeof(i2c_static_regs)/sizeof(i2c_static_regs[0]);

#define THS8200_ADDR 0x21
static int ths_write(u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	int retval = i2c_write(THS8200_ADDR, addr, 1, buf, len);
	if (retval)
		printf ("%s: Error setting reg %02x = \n", __func__, addr, buf[0]);
	return retval;
}

#endif

#define TO_ENC(pixels, enc_mult, enc_div) (((pixels) * (enc_div)) / (enc_mult))
struct lcd_t *newPanel( struct lcd_panel_info_t const *info )
{
	unsigned i ;
	unsigned short val[4];
	unsigned dclkctl;
	unsigned stride = ((info->xres + 0x1f) & ~0x1f);
	unsigned fbBytes =  stride * info->yres ;
	struct lcd_t *lcd = (struct lcd_t *)malloc(sizeof(struct lcd_t));
	unsigned short totalh, totalv ;
	unsigned hstart, vstart;
	u32 enc_mult = 0;
	u32 enc_div = 0;

        DECLARE_GLOBAL_DATA_PTR;
	memcpy(&lcd->info, info,sizeof(lcd->info));
	i = gd->bd->bi_dram[0].size;
	if (i > (128<<20)) i = (128<<20);
	lcd->fbAddr = (void *)( gd->bd->bi_dram[0].start + i - fbBytes );
	lcd->fbMemSize = fbBytes;
        memset(lcd->fbAddr,0xff,fbBytes);

	lcd->fg = 0 ;
	lcd->bg = 255 ;
	lcd->x = lcd->y = 0 ;
	lcd->colorCount = 256 ;
	lcd->set_palette = set_palette ;
	lcd->get_palette_color = get_palette_color ;
	lcd->disable = disable ;
	lcd->stride = stride;

	REGVALUE(VPSS_CLKCTL) = 0x18 ;

	/*
	 * turn off video unit, DDR2 may be disabled while
	 * clock is changing
	 */
	REGVALUE(VENC_VMOD) = 0;
	setPixClock(info->pixclock, &enc_mult, &enc_div);

	REGVALUE(OSD_MODE) = 0 ;
	REGVALUE(OSD_OSDWIN0MD) = 0 ;

	REGVALUE(OSD_OSDWIN0ADR) = (unsigned long)lcd->fbAddr ;

	for( i = 0 ; i < NUM_WINDOWS ; i++ ){
                // first two windows are 2 bytes/pixel, second two are 1 byte
		unsigned bytesPerPixel = 2-(i/2);
		REGVALUE(OSD_WINOFST(i)) = stride /(32/bytesPerPixel);
                REGVALUE(OSD_WINXP(i)) = 0 ;
                REGVALUE(OSD_WINYP(i)) = 0 ;
		REGVALUE(OSD_WINXL(i)) = info->xres ;
		REGVALUE(OSD_WINYL(i)) = info->yres ;
	}

	REGVALUE(OSD_MODE) = (REGVALUE(OSD_MODE) & OSD_MODE_BCLUT) | 0 ;
	REGVALUE(OSD_MODE) = (REGVALUE(OSD_MODE) & OSD_MODE_CABG) | (0 << OSD_MODE_CABG_SHIFT);

	/* Field Inversion Workaround */
	REGVALUE(OSD_MODE) = 0x280 ;

	hstart = info->hsync_len + info->left_margin;
	vstart = info->vsync_len + info->upper_margin;
	REGVALUE(OSD_BASEPX) = hstart;
//	REGVALUE(OSD_BASEPX) = TO_ENC(hstart, enc_mult, enc_div);
	REGVALUE(OSD_BASEPY) = vstart;

	/* Reset video encoder module */
	REGVALUE(VPSS_CLKCTL) = getenv( "force27" ) ? 0x08 : 0x09 ;	//disable DAC clock
	REGVALUE(VPBE_PCR) = 0 ;		//not divided by 2
	REGVALUE(VENC_VIDCTL) =((info->pclk_redg^1)<<14)|(1<<13);
	REGVALUE(VENC_SYNCCTL) = ((info->vsyn_acth <<3 ) | (info->hsyn_acth << 2)) ^ 0x0f;
	REGVALUE(VENC_HSPLS) = TO_ENC(info->hsync_len, enc_mult, enc_div);
	REGVALUE(VENC_VSPLS) = info->vsync_len ;
        totalh = info->xres+info->hsync_len+info->left_margin+info->right_margin ;
	REGVALUE(VENC_HINT) = TO_ENC((totalh-1), enc_mult, enc_div);
	REGVALUE(VENC_HSTART) = TO_ENC(hstart, enc_mult, enc_div);
	REGVALUE(VENC_HVALID) = TO_ENC(info->xres, enc_mult, enc_div);
        totalv = info->yres+info->vsync_len+info->upper_margin+info->lower_margin ;
	REGVALUE(VENC_VINT) = (totalv-1);

	REGVALUE(VENC_VSTART) = info->upper_margin + info->vsync_len;
	REGVALUE(VENC_VVALID) = info->yres;
	REGVALUE(VENC_HSDLY) = 0 ;
	REGVALUE(VENC_VSDLY) = 0 ;
	REGVALUE(VENC_RGBCTL) = 0 ;
	REGVALUE(PINMUX0) = (REGVALUE(PINMUX0) & ~(1<<22)) | (3<<23);	//rgb888 and output_enable
	REGVALUE(VENC_LCDOUT) = (info->oepol_actl<<1)|1 ;	//enable active high on gpio0

	val[0] = 0;
	val[1] = 0;
	val[2] = 0;
	val[3] = 0;
	dclkctl = ((64 / enc_div) * enc_div) - 1;
	if ((enc_mult * 2) <= enc_div) {
/*
 * Example mul = 5, div = 14, 2.8 width cell, actual waveform
 * |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  | 10  | 11  | 12  | 13  |
 * |     |     |11111|11111|22222|22222|33333|33333|44444|44444|55555|55555|66666|66666|
 * |01234|56789|01234|56789|01234|56789|01234|56789|01234|56789|01234|56789|01234|56789|
 * |    _|__1__|_    |    _|_1___|     |  ___|1___ |     | ___1|___  |     |___1_|__   |
 * |  0  |  1  |  0  |  0  |  1  |  0  |  1  |  1  |  0  |  1  |  1  |  0  |  1  |  0  |
 *
 * Example mul = 2, div = 7
 * |0 |1 |2 |3 |4 |5 |6 |
 * |  |  |  |  |  |11|11|
 * |01|23|45|67|89|01|23|
 * |  |_1|_ |  | _|1_|  |
 * |0 |1 |0 |0 |0 |1 |0 |
 *
 * Example mul = 2, div = 6
 * |0 |1 |2 |3 |4 |5 |
 * |  |  |  |  |  |11|
 * |01|23|45|67|89|01|
 * |  |1 |  |  |1 |  |
 * |0 |1 |0 |0 |1 |0 |
 *
 * Example mul = 1, div = 3
 * |0 |1 |2 |
 * |  |  |  |
 * |0 |1 |2 |
 * |  |1 |  |
 * |0 |1 |0 |
 */
		u32 setbits = enc_div >> 1;
		u32 min = (enc_mult >> 1) + 1;
		u32 bit = 0;
		u32 gbit = setbits - (setbits >> 1);	//Start in middle of 1st bit
		while (1) {
			u32 bits_set_in_cell;
			while (gbit >= enc_mult) {
				gbit -= enc_mult;
				bit++;
			}
			if (bit >= 64)
				break;
			bits_set_in_cell = (enc_mult - gbit);
			if (bits_set_in_cell >= min)
				val[bit>>4] |= (1<<(bit&0xf));
			{
				u32 tbit = bit+1;
				u32 width = setbits - bits_set_in_cell;
				while (width >= enc_mult) {
					if (tbit >= 64)
						break;
					val[tbit>>4] |= (1<<(tbit&0xf));
					width -= enc_mult;
					tbit++;
				}
				if (tbit >= 64)
					break;
				if (width >= min)
					val[tbit>>4] |= (1<<(tbit&0xf));
			}
			gbit += enc_div;
		}
	} else {
/*
 *
 *
 * Example mul = 7, div = 8, 1 means pixel clock here
 * |   0   |   1   |   2   |   3   |   4   |   5   |   6   |   7   |
 * |       |   1111|1111112|2222222|2233333|3333344|4444444|4555555|
 * |0123456|7890123|4567890|1234567|8901234|5678901|2345678|9012345|
 * |    1  |     1 |      1|       |1      | 1     |  1    |   1   |
 * |   1   |   1   |   1   |   0   |   1   |   1   |   1   |   1   |
 *
 */
		u32 bit = 0;
		u32 gbit = enc_div >> 1;	//Start in middle of 1st bit
		while (1) {
			while (gbit >= enc_mult) {
				gbit -= enc_mult;
				bit++;
			}
			if (bit >= 64)
				break;
			val[bit>>4] |= (1<<(bit&0xf));
			gbit += enc_div;
		}
		dclkctl |= (1 << 11);
	}
	REGVALUE(VENC_DCLKCTL) = dclkctl;

	REGVALUE(VENC_DCLKPTN0) = val[0];
	REGVALUE(VENC_DCLKPTN1) = val[1];
	REGVALUE(VENC_DCLKPTN2) = val[2];
	REGVALUE(VENC_DCLKPTN3) = val[3];

	REGVALUE(VENC_DCLKHS) = 0 ;
	REGVALUE(VENC_DCLKHSA) = 0 ;
	REGVALUE(VENC_DCLKHR) = totalh ;
	REGVALUE(VENC_DCLKVS) = 0 ;
	REGVALUE(VENC_DCLKVR) = totalv ;

	if ((dclkctl & (1 << 11)) == 0) {
		u32 bit = 0;
		u32 gbit = 0;
		val[0] = 0;
		while (1) {
			while (gbit >= enc_mult) {
				gbit -= enc_mult;
				bit++;
			}
			if (bit >= 16)
				break;
			val[0] |= (1 << bit);
			gbit += enc_div;
		}
	}
	REGVALUE(VENC_OSDCLK0) = (enc_div <= 16) ? ((16 / enc_div) * enc_div) - 1 : 15;
	REGVALUE(VENC_OSDCLK1) = val[0];
	REGVALUE(VENC_OSDHAD) = 0 ;

#if PLAT_HAS_THS == 1
	if( i2c_probe(THS8200_ADDR) == 0){
		unsigned char byte ;
		unsigned vsync_length ;
		unsigned hs_in_dly, vs_in_dly;
		unsigned hdly, vdly;
		unsigned field_size;
		uchar buf[13];	/* 0x70-0x7c */

		hs_in_dly = info->hsync_len + info->left_margin;
		vs_in_dly = info->vsync_len + info->upper_margin;
		hdly = info->xres + info->right_margin;
		/*
		 * hdly += 1 if mult used (if 0x4a bit 7, 0x4f bit 7,6 == 1)
		 * hdly += 8 if CSC used (if 0x19 bit 1 == 0)
		 * hdly += 36 (if ifir12 not bypassed)(if 0x1c bit 5 == 0)
		 * hdly += 18 if 2x interpolation used (if ifir35 not bypassed)(if 0x1c bit 4 == 0)
		 */
		vdly = info->yres + info->lower_margin;

		printf( "Found THS8200 at address 0x%x\n", THS8200_ADDR );
		/* put in reset state */
		byte = 0;
		ths_write(0x03, 1, (uchar *)&byte, 1);
		for( i = 0 ; i < num_static_i2c ; i++ ){
                        struct i2c_registers_t const *reg = i2c_static_regs+i;
			ths_write(reg->regno, 1, (uchar *)&reg->value, 1);
		}

		// set horizontal total length
		buf[0x34 - 0x34] = (uchar)((totalh & 0x1f00) >> 8);
		buf[0x35 - 0x34] = (uchar)totalh;
		ths_write(0x34, 1, buf, 2);

		// set vertical total length
		field_size = totalv + 1;
		buf[0x39 - 0x39] = (uchar)(((totalv & 0x700) >> 4) |
				((field_size & 0x700) >> 8));
		buf[0x3a - 0x39] = (uchar)totalv;
		buf[0x3b - 0x39] = (uchar)field_size;
		ths_write(0x39, 1, buf, 3);

		// set horizontal pulse width and offset
/* hlength 7:0 */
		buf[0x70 - 0x70] = (uchar)info->hsync_len;
		buf[0x71 - 0x70] = (uchar)
/* hlength 9:8 */
			(((info->hsync_len & 0x300) >> 2) |
/* hdly 12:8 */
			((hdly & 0x1f00) >> 8));
/* hdly 7:0 */
		buf[0x72 - 0x70] = (uchar)hdly;
		vsync_length = (info->vsync_len) + 1;	 /* +1 is needed for some reason, bad docs */
/* vlength1 7:0 */
		buf[0x73 - 0x70] = (uchar)vsync_length;
		buf[0x74 - 0x70] = (uchar)
/* vlength1 9:8 */
			(((vsync_length & 0x300) >> 2) |
/* vdly1 10:8 */
			((vdly & 0x700) >> 8));
/* vdly1 7:0 */
		buf[0x75 - 0x70] = (uchar)vdly;
/* vlength2 7:0 */
		buf[0x76 - 0x70] = (uchar)0;
		buf[0x77 - 0x70] = (uchar)
/* vlength2 9:8 */
			(((0 & 0x300) >> 2) |
/* vdly2 10:8*/
			((0x7ff & 0x700) >> 8));
/* vdly2 7:0 */
		buf[0x78 - 0x70] = (uchar)0x7ff;
/* hs_in_dly 12:8 */
		buf[0x79 - 0x70] = (uchar)((hs_in_dly & 0x1f00) >> 8);
/* hs_in_dly 7:0 */
		buf[0x7a - 0x70] = (uchar)hs_in_dly;
/* vs_in_dly 10:8 */
		buf[0x7b - 0x70] = (uchar)((vs_in_dly & 0x1f00) >> 8);
/* vs_in_dly 7:0 */
		buf[0x7c - 0x70] = (uchar)vs_in_dly;
		ths_write(0x70, 1, buf, 13);

		byte = ((0!=info->vsyn_acth)<<1)|((0!=info->hsyn_acth)<<0);
		byte |= (byte & 3) << 3;
		byte |= 0x44;
		ths_write(0x82, 1, &byte, 1);
		/* release from reset state */
		byte = 1;
		ths_write(0x03, 1, (uchar *)&byte, 1);
	}
#endif
	REGVALUE(VENC_VMOD) = (VENC_VMOD_VDMD_RGB666<<VENC_VMOD_VDMD_SHIFT)|VENC_VMOD_VMD|VENC_VMOD_VENC ;

	REGVALUE(OSD_OSDWIN0MD) = (3<<OSD_OSDWIN0MD_BMW0_SHIFT)
				| (7<<OSD_OSDWIN0MD_BLND0_SHIFT)
				| OSD_OSDWIN0MD_OACT0 ;

	printf( "%s: %ux%u @%p (%s)\n", __FUNCTION__, info->xres, info->yres, lcd->fbAddr, info->name );

	return lcd ;
}
