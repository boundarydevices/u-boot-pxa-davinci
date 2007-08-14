/*
 * Common LCD routines for supported CPUs
 *
 * (C) Copyright 2001-2002
 * Wolfgang Denk, DENX Software Engineering -- wd@denx.de
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/************************************************************************/
/* ** HEADER FILES							*/
/************************************************************************/

/* #define DEBUG */

#include <config.h>
#include <common.h>
#include <command.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#if defined(CONFIG_POST)
#include <post.h>
#endif
#include <lcd.h>
#include <watchdog.h>

#if defined(CONFIG_PXA250) || defined(CONFIG_PXA270)
#include <asm/byteorder.h>
#endif

#if defined(CONFIG_MPC823)
#include <lcdvideo.h>
#endif

int luminance( int red, int green, int blue )
{
   // 
   // I've seen a couple of different algorithms here:
   // (max+min)/2
/*

   int max = MAX( red, MAX( green, blue ) );
   int min = MIN( red, MIN( green, blue ) );
   return (max+min)/2 ;
*/

   // A more mathematically-correct version
// return (int)(c.R*0.3 + c.G*0.59+ c.B*0.11);

   // Just return 'green'
   // return green;

   // and one that uses shifts and adds to come close to the above
   //
   //    red   = 5/16= 0.3125    == 1/4 + 1/16
   //    green = 9/16= 0.5625    == 1/2 + 1/16
   //    blue  = 1/8 = 0.125
   //
   if( 0 < red )
      red = (red>>2) + (red>>4);
   else
      red = 0 ;
   if( 0 < green )
      green = (green>>1) + (green>>4) ;
   else
      green = 0 ;
   if( 0 < blue )
      blue = blue >> 3 ;
   else
      blue = 0 ;
   return red+green+blue ;
}

#ifdef CONFIG_LCD

/************************************************************************/
/* ** FONT DATA								*/
/************************************************************************/
#include <video_font.h>		/* Get font data, width and height	*/

/************************************************************************/
/* ** LOGO DATA								*/
/************************************************************************/
#ifdef CONFIG_LCD_LOGO
# include <bmp_logo.h>		/* Get logo data, width and height	*/
# if (CONSOLE_COLOR_WHITE >= BMP_LOGO_OFFSET)
#  error Default Color Map overlaps with Logo Color Map
# endif
#endif

DECLARE_GLOBAL_DATA_PTR;

ulong lcd_setmem (ulong addr);

static void lcd_drawchars (ushort x, ushort y, uchar *str, int count);
static inline void lcd_puts_xy (ushort x, ushort y, uchar *s);
static inline void lcd_putc_xy (ushort x, ushort y, uchar  c);

static int lcd_init (void *lcdbase);

static int lcd_clear (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);
extern void lcd_ctrl_init (void *lcdbase);
extern void lcd_enable (void);
static void *lcd_logo (void);


#if LCD_BPP == LCD_COLOR8
extern void lcd_setcolreg (ushort regno,
				ushort red, ushort green, ushort blue);
#endif
#if LCD_BPP == LCD_MONOCHROME
extern void lcd_initcolregs (void);
#endif

static int lcd_getbgcolor (void);
static void lcd_setfgcolor (int color);
static void lcd_setbgcolor (int color);


char lcd_is_enabled = 0;

extern vidinfo_t panel_info;

/************************************************************************/
/*----------------------------------------------------------------------*/

static void console_scrollup (void)
{
#if 1
	/* Copy up rows ignoring the first one */
	memcpy (CONSOLE_ROW_FIRST, CONSOLE_ROW_SECOND, CONSOLE_SCROLL_SIZE);

	/* Clear the last one */
	memset (CONSOLE_ROW_LAST, COLOR_MASK(lcd_color_bg), CONSOLE_ROW_SIZE);
#else
	/*
	 * Poor attempt to optimize speed by moving "long"s.
	 * But the code is ugly, and not a bit faster :-(
	 */
	ulong *t = (ulong *)CONSOLE_ROW_FIRST;
	ulong *s = (ulong *)CONSOLE_ROW_SECOND;
	ulong    l = CONSOLE_SCROLL_SIZE / sizeof(ulong);
	uchar  c = lcd_color_bg & 0xFF;
	ulong val= (c<<24) | (c<<16) | (c<<8) | c;

	while (l--)
		*t++ = *s++;

	t = (ulong *)CONSOLE_ROW_LAST;
	l = CONSOLE_ROW_SIZE / sizeof(ulong);

	while (l-- > 0)
		*t++ = val;
#endif
}

/*----------------------------------------------------------------------*/

static inline void console_back (void)
{
	if (--console_col < 0) {
		console_col = CONSOLE_COLS-1 ;
		if (--console_row < 0) {
			console_row = 0;
		}
	}

	lcd_putc_xy (console_col * VIDEO_FONT_WIDTH,
		     console_row * VIDEO_FONT_HEIGHT,
		     ' ');
}

/*----------------------------------------------------------------------*/

static inline void console_newline (void)
{
	++console_row;
	console_col = 0;

	/* Check if we need to scroll the terminal */
	if (console_row >= CONSOLE_ROWS) {
		/* Scroll everything up */
		console_scrollup () ;
		--console_row;
	}
}

/*----------------------------------------------------------------------*/

void lcd_putc (const char c)
{
	if (!lcd_is_enabled) {
		serial_putc(c);
		return;
	}

	switch (c) {
	case '\r':	console_col = 0;
			return;

	case '\n':	console_newline();
			return;

	case '\t':	/* Tab (8 chars alignment) */
			console_col |=  8;
			console_col &= ~7;

			if (console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;

	case '\b':	console_back();
			return;

	default:	lcd_putc_xy (console_col * VIDEO_FONT_WIDTH,
				     console_row * VIDEO_FONT_HEIGHT,
				     c);
			if (++console_col >= CONSOLE_COLS) {
				console_newline();
			}
			return;
	}
	/* NOTREACHED */
}

/*----------------------------------------------------------------------*/

void lcd_puts (const char *s)
{
	if (!lcd_is_enabled) {
		serial_puts (s);
		return;
	}

	while (*s) {
		lcd_putc (*s++);
	}
}

/************************************************************************/
/* ** Low-Level Graphics Routines					*/
/************************************************************************/

static void lcd_drawchars (ushort x, ushort y, uchar *str, int count)
{
	uchar *dest;
	ushort off, row;

	dest = (uchar *)(lcd_base + y * panel_info.vl_lcd_line_length + x * (1 << LCD_BPP) / 8);
	off  = x * (1 << LCD_BPP) % 8;

	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += panel_info.vl_lcd_line_length)  {
		uchar *s = str;
		uchar *d = dest;
		int i;

#if LCD_BPP == LCD_MONOCHROME
		uchar rest = *d & -(1 << (8-off));
		uchar sym;
#endif
		for (i=0; i<count; ++i) {
			uchar c, bits;

			c = *s++;
			bits = video_fontdata[c * VIDEO_FONT_HEIGHT + row];

#if LCD_BPP == LCD_MONOCHROME
			sym  = (COLOR_MASK(lcd_color_fg) & bits) |
			       (COLOR_MASK(lcd_color_bg) & ~bits);

			*d++ = rest | (sym >> off);
			rest = sym << (8-off);
#elif LCD_BPP == LCD_COLOR8
			for (c=0; c<8; ++c) {
				*d++ = (bits & 0x80) ?
						lcd_color_fg : lcd_color_bg;
				bits <<= 1;
			}
#elif LCD_BPP == LCD_COLOR16
			for (c=0; c<16; ++c) {
				*d++ = (bits & 0x80) ?
						lcd_color_fg : lcd_color_bg;
				bits <<= 1;
			}
#endif
		}
#if LCD_BPP == LCD_MONOCHROME
		*d  = rest | (*d & ((1 << (8-off)) - 1));
#endif
	}
}

/*----------------------------------------------------------------------*/

static inline void lcd_puts_xy (ushort x, ushort y, uchar *s)
{
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	lcd_drawchars (x, y+BMP_LOGO_HEIGHT, s, strlen ((char *)s));
#else
	lcd_drawchars (x, y, s, strlen ((char *)s));
#endif
}

/*----------------------------------------------------------------------*/

static inline void lcd_putc_xy (ushort x, ushort y, uchar c)
{
#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	lcd_drawchars (x, y+BMP_LOGO_HEIGHT, &c, 1);
#else
	lcd_drawchars (x, y, &c, 1);
#endif
}

/************************************************************************/
/**  Small utility to check that you got the colours right		*/
/************************************************************************/
#ifdef LCD_TEST_PATTERN

#define	N_BLK_VERT	2
#define	N_BLK_HOR	3

static int test_colors[N_BLK_HOR*N_BLK_VERT] = {
	CONSOLE_COLOR_RED,	CONSOLE_COLOR_GREEN,	CONSOLE_COLOR_YELLOW,
	CONSOLE_COLOR_BLUE,	CONSOLE_COLOR_MAGENTA,	CONSOLE_COLOR_CYAN,
};

static void test_pattern (void)
{
	ushort v_max  = panel_info.vl_row;
	ushort h_max  = panel_info.vl_col;
	ushort v_step = (v_max + N_BLK_VERT - 1) / N_BLK_VERT;
	ushort h_step = (h_max + N_BLK_HOR  - 1) / N_BLK_HOR;
	ushort v, h;
	uchar *pix = (uchar *)lcd_base;

	printf ("[LCD] Test Pattern: %d x %d [%d x %d]\n",
		h_max, v_max, h_step, v_step);

	/* WARNING: Code silently assumes 8bit/pixel */
	for (v=0; v<v_max; ++v) {
		uchar iy = v / v_step;
		for (h=0; h<h_max; ++h) {
			uchar ix = N_BLK_HOR * iy + (h/h_step);
			*pix++ = test_colors[ix];
		}
	}
}
#endif /* LCD_TEST_PATTERN */


/************************************************************************/
/* ** GENERIC Initialization Routines					*/
/************************************************************************/

int drv_lcd_init (void)
{
	device_t lcddev;
	int rc;

	lcd_base = (void *)(gd->fb_base);

	lcd_init (lcd_base);		/* LCD initialization */

	/* Device initialization */
	memset (&lcddev, 0, sizeof (lcddev));

	strcpy (lcddev.name, "lcd");
	lcddev.ext   = 0;			/* No extensions */
	lcddev.flags = DEV_FLAGS_OUTPUT;	/* Output only */
	lcddev.putc  = lcd_putc;		/* 'putc' function */
	lcddev.puts  = lcd_puts;		/* 'puts' function */

	rc = device_register (&lcddev);

	return (rc == 0) ? 1 : rc;
}

/*----------------------------------------------------------------------*/
int lcd_ClearScreen(void)
{
#if LCD_BPP == LCD_MONOCHROME
	/* Setting the palette */
	lcd_initcolregs();

#elif LCD_BPP == LCD_COLOR8
	/* Setting the palette */
	lcd_setcolreg  (CONSOLE_COLOR_BLACK,       0,    0,    0);
	lcd_setcolreg  (CONSOLE_COLOR_RED,	0xFF,    0,    0);
	lcd_setcolreg  (CONSOLE_COLOR_GREEN,       0, 0xFF,    0);
	lcd_setcolreg  (CONSOLE_COLOR_YELLOW,	0xFF, 0xFF,    0);
	lcd_setcolreg  (CONSOLE_COLOR_BLUE,        0,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_MAGENTA,	0xFF,    0, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_CYAN,	   0, 0xFF, 0xFF);
	lcd_setcolreg  (CONSOLE_COLOR_GREY,	0xAA, 0xAA, 0xAA);
	lcd_setcolreg  (CONSOLE_COLOR_WHITE,	0xFF, 0xFF, 0xFF);
#endif

#ifndef CFG_WHITE_ON_BLACK
	lcd_setfgcolor (CONSOLE_COLOR_BLACK);
	lcd_setbgcolor (CONSOLE_COLOR_WHITE);
#else
	lcd_setfgcolor (CONSOLE_COLOR_WHITE);
	lcd_setbgcolor (CONSOLE_COLOR_BLACK);
#endif	/* CFG_WHITE_ON_BLACK */

#ifdef	LCD_TEST_PATTERN
	test_pattern();
#else
	/* set framebuffer to background color */
	memset ((char *)lcd_base,
		COLOR_MASK(lcd_getbgcolor()),
		panel_info.vl_lcd_line_length*panel_info.vl_row);
#endif
	/* Paint the logo and retrieve LCD base address */
	debug ("[LCD] Drawing the logo...\n");
	lcd_console_address = lcd_logo ();

	console_col = 0;
	console_row = 0;

	return (0);
}
static int lcd_clear (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	return lcd_ClearScreen();
}

U_BOOT_CMD(
	cls,	1,	1,	lcd_clear,
	"cls     - clear screen\n",
	NULL
);

/*----------------------------------------------------------------------*/

static int lcd_init (void *lcdbase)
{
	/* Initialize the lcd controller */
	debug ("[LCD] Initializing LCD frambuffer at %p\n", lcdbase);

	lcd_ctrl_init (lcdbase);
	lcd_clear (NULL, 1, 1, NULL);	/* dummy args */
	lcd_enable ();

	/* Initialize the console */
	console_col = 0;
#ifdef CONFIG_LCD_INFO_BELOW_LOGO
	console_row = 7 + BMP_LOGO_HEIGHT / VIDEO_FONT_HEIGHT;
#else
	console_row = 1;	/* leave 1 blank line below logo */
#endif
	lcd_is_enabled = 1;

	return 0;
}


/************************************************************************/
/* ** ROM capable initialization part - needed to reserve FB memory	*/
/************************************************************************/
/*
 * This is called early in the system initialization to grab memory
 * for the LCD controller.
 * Returns new address for monitor, after reserving LCD buffer memory
 *
 * Note that this is running from ROM, so no write access to global data.
 */
ulong lcd_setmem (ulong addr)
{
	ulong size;
	int line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) / 8;

	debug ("LCD panel info: %d x %d, %d bit/pix\n",
		panel_info.vl_col, panel_info.vl_row, NBITS (panel_info.vl_bpix) );

	size = line_length * panel_info.vl_row;

	/* Round up to nearest full page */
	size = (size + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1);

	/* Allocate pages for the frame buffer. */
	addr -= size;

	debug ("Reserving %ldk for LCD Framebuffer at: %08lx\n", size>>10, addr);

	return (addr);
}

/*----------------------------------------------------------------------*/

static void lcd_setfgcolor (int color)
{
	lcd_color_fg = color & 0x0F;
}

/*----------------------------------------------------------------------*/

static void lcd_setbgcolor (int color)
{
	lcd_color_bg = color & 0x0F;
}

/*----------------------------------------------------------------------*/

#ifdef	NOT_USED_SO_FAR
static int lcd_getfgcolor (void)
{
	return lcd_color_fg;
}
#endif	/* NOT_USED_SO_FAR */

/*----------------------------------------------------------------------*/

static int lcd_getbgcolor (void)
{
	return lcd_color_bg;
}

/*----------------------------------------------------------------------*/

/************************************************************************/
/* ** Chipset depending Bitmap / Logo stuff...                          */
/************************************************************************/
#ifdef CONFIG_LCD_LOGO
void bitmap_plot (int x, int y)
{
	PALETTEVAL_TYPE *cmap;
	ushort i, j;
	uchar *bmap;
	uchar *fb;
	ushort *fb16;

#if defined(PXALCD)
	struct pxafb_info *fbi = &panel_info.pxa;
#elif defined(CONFIG_MPC823)
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
#endif

	debug ("Logo: width %d  height %d  colors %d  cmap %d\n",
		BMP_LOGO_WIDTH, BMP_LOGO_HEIGHT, BMP_LOGO_COLORS,
		sizeof(bmp_logo_palette)/(sizeof(PALETTEVAL_TYPE)));

	bmap = &bmp_logo_bitmap[0];
	fb   = (uchar *)(lcd_base + y * panel_info.vl_lcd_line_length + x);

	if (NBITS(panel_info.vl_bpix) < 12) {
		/* Leave room for default color map */
#if defined(PXALCD)
		cmap = (PALETTEVAL_TYPE *)fbi->palette;
#elif defined(CONFIG_MPC823)
		cmap = (ushort *)&(cp->lcd_cmap[BMP_LOGO_OFFSET*sizeof(ushort)]);
#endif

		WATCHDOG_RESET();

		/* Set color map */
		for (i=0; i<(sizeof(bmp_logo_palette)/(sizeof(ushort))); ++i) {
			ushort colreg = bmp_logo_palette[i];
#if defined(CONFIG_SM501)
#else
   #ifdef  CFG_INVERT_COLORS
   			*cmap++ = 0xffff - colreg;
   #else
			*cmap++ = colreg;
   #endif
#endif
		}

		WATCHDOG_RESET();

		for (i=0; i<BMP_LOGO_HEIGHT; ++i) {
			memcpy (fb, bmap, BMP_LOGO_WIDTH);
			bmap += BMP_LOGO_WIDTH;
			fb   += panel_info.vl_col;
		}
	}
	else { /* true color mode */
		fb16 = (ushort *)(lcd_base + y * panel_info.vl_lcd_line_length + x);
		for (i=0; i<BMP_LOGO_HEIGHT; ++i) {
			for (j=0; j<BMP_LOGO_WIDTH; j++) {
				fb16[j] = bmp_logo_palette[(bmap[j])];
				}
			bmap += BMP_LOGO_WIDTH;
			fb16 += panel_info.vl_col;
		}
	}

	WATCHDOG_RESET();
}
#endif /* CONFIG_LCD_LOGO */

/*----------------------------------------------------------------------*/
#if (CONFIG_COMMANDS & CFG_CMD_BMP) || defined(CONFIG_SPLASH_SCREEN)

#if defined(CONFIG_MPC823)
//each entry is in ARGB format, alpha high byte, blue low byte
void lcd_SetPalette(ulong* palette,int colorCnt)
{
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile cpm8xx_t *cp = &(immr->im_cpm);
	PALETTEVAL_TYPE *cmap = (ushort *)&(cp->lcd_cmap[255*sizeof(ushort)]);
	while (colorCnt--) {
		ulong tmp = *palette++;	//5,6,5 format
		ushort val =( (tmp>>8) & 0xf800) |
					( (tmp>>5) & 0x07e0) |
					( (tmp>>3) & 0x001f) ;
#ifdef CFG_INVERT_COLORS
		val = ~val;
#endif
		*cmap-- = val;
	}
}
#else
//other machines have this function defined in their lcd driver
#endif

void lcd_SetPalette(unsigned long* palette,unsigned colors);
/*
 * Display the BMP file located at address bmp_image.
 * Only uncompressed.
 */
int lcd_display_bitmap(ulong bmp_image, int x, int y)
{
	ushort i, j;
	uchar *fb;
	bmp_image_t *bmp=(bmp_image_t *)bmp_image;
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height;
	unsigned colors,bpix;
	unsigned long compression;
	int     maxLum = 0 ;
	int     bgCol = 0 ;
	int     minLum = 0xFFFF ;
	int     fgCol = 0 ;

	if (!((bmp->header.signature[0]=='B') &&
		(bmp->header.signature[1]=='M'))) {
		printf ("Error: no valid bmp image at %lx\n", bmp_image);
		return 1;
	}

	width = le32_to_cpu (bmp->header.width);
	height = le32_to_cpu (bmp->header.height);
	colors = 1<<le16_to_cpu (bmp->header.bit_count);
	compression = le32_to_cpu (bmp->header.compression);

	bpix = NBITS(panel_info.vl_bpix);

	if ((bpix != 1) && (bpix != 8)) {
		printf ("Error: %d bit/pixel mode not supported by U-Boot\n",
			bpix);
		return 1;
	}

	if (bpix != le16_to_cpu(bmp->header.bit_count)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix,
			le16_to_cpu(bmp->header.bit_count));
		return 1;
	}

	debug ("Display-bmp: %d x %d  with %d colors\n",
		(int)width, (int)height, (int)colors);

	if (bpix==8) {
		unsigned long palette[256];
		unsigned long* cmap = palette;

		/* Set color map */
		for (i=0; i<colors; ++i) {
			int lum ;
			bmp_color_table_entry_t cte = bmp->color_table[i];
			*cmap++ = cte.blue | (cte.green << 8) | (cte.red << 16) | 0xFF000000;
			lum = luminance( cte.red, cte.green, cte.blue );
			if( lum > maxLum ) {
				maxLum = lum ;
				bgCol = i ;
			}
			if( lum < minLum ) {
				minLum = lum ;
				fgCol = i ;
			}
		}
		lcd_SetPalette(palette,colors);
		printf( "bgcolor %u, fg %u\n", bgCol, fgCol );
		lcd_color_fg = fgCol ;
		lcd_color_bg = bgCol ;
	}

	padded_line = (width+3)&~0x3;
	if ((x + width)>panel_info.vl_col)
		width = panel_info.vl_col - x;
	if ((y + height)>panel_info.vl_row)
		height = panel_info.vl_row - y;

	bmap = (uchar *)bmp + le32_to_cpu (bmp->header.data_offset);
	fb   = (uchar *) (lcd_base +
		(y + height - 1) * panel_info.vl_lcd_line_length + x);
//	printf("fb:0x%p,lcd_base:0x%p,x:%i,y:%i,width:%i,height:%i,colums:%i\n",
//		fb,lcd_base,x,y,width,height,panel_info.vl_lcd_line_length);
	for (i = 0; i < height; ++i) {
		WATCHDOG_RESET();
		for (j = 0; j < width ; j++)
#if defined(CONFIG_MPC823)
			*fb++=255-*bmap++;
#else
			*fb++=*bmap++;
#endif
		bmap += (padded_line-width);
		fb   -= (width + panel_info.vl_lcd_line_length);
	}

	return (0);
}
#endif /* (CONFIG_COMMANDS & CFG_CMD_BMP) || CONFIG_SPLASH_SCREEN */


static void *lcd_logo (void)
{
#ifdef CONFIG_LCD_INFO
	char info[80];
	char temp[32];
#endif /* CONFIG_LCD_INFO */

#ifdef CONFIG_SPLASH_SCREEN
	char *s;
	ulong addr;
	static int do_splash = 1;

	if (do_splash && (s = getenv("splashimage")) != NULL) {
		addr = simple_strtoul(s, NULL, 16);
		do_splash = 0;

		if (lcd_display_bitmap (addr, 0, 0) == 0) {
			return ((void *)lcd_base);
		}
	}
#endif /* CONFIG_SPLASH_SCREEN */

#ifdef CONFIG_LCD_LOGO
	bitmap_plot (0, 0);
#endif /* CONFIG_LCD_LOGO */

#ifdef CONFIG_MPC823
# ifdef CONFIG_LCD_INFO
	sprintf (info, "%s (%s - %s) ", U_BOOT_VERSION, __DATE__, __TIME__);
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y, (uchar *)info, strlen(info));

	sprintf (info, "(C) 2004 DENX Software Engineering");
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT,
					(uchar *)info, strlen(info));

	sprintf (info, "    Wolfgang DENK, wd@denx.de");
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 2,
					(uchar *)info, strlen(info));
#  ifdef CONFIG_LCD_INFO_BELOW_LOGO
	sprintf (info, "MPC823 CPU at %s MHz",
		strmhz(temp, gd->cpu_clk));
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 3,
					info, strlen(info));
	sprintf (info, "  %ld MB RAM, %ld MB Flash",
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 4,
					info, strlen(info));
#  else
	/* leave one blank line */

	sprintf (info, "MPC823 CPU at %s MHz, %ld MB RAM, %ld MB Flash",
		strmhz(temp, gd->cpu_clk),
		gd->ram_size >> 20,
		gd->bd->bi_flashsize >> 20 );
	lcd_drawchars (LCD_INFO_X, LCD_INFO_Y + VIDEO_FONT_HEIGHT * 4,
					(uchar *)info, strlen(info));

#  endif /* CONFIG_LCD_INFO_BELOW_LOGO */
# endif /* CONFIG_LCD_INFO */
#endif /* CONFIG_MPC823 */

#if defined(CONFIG_LCD_LOGO) && !defined(CONFIG_LCD_INFO_BELOW_LOGO)
	return ((void *)((ulong)lcd_base + BMP_LOGO_HEIGHT * panel_info.vl_lcd_line_length));
#else
	return ((void *)lcd_base);
#endif /* CONFIG_LCD_LOGO && !CONFIG_LCD_INFO_BELOW_LOGO */
}

/************************************************************************/
/************************************************************************/

#endif /* CONFIG_LCD */
