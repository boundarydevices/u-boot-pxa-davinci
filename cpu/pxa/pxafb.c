/*
 * PXA LCD Controller
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
// #define DEBUG

#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <lcd.h>
#include <asm/arch/pxa-regs.h>

#ifdef CONFIG_LCDPANEL
#include <lcd_panels.h>
#endif
#include "pxafb.h"

#if defined( CONFIG_LCD ) || defined( CONFIG_LCD_MULTI )

/*----------------------------------------------------------------------*/
/*
 * Define panel bpp, LCCR0, LCCR3 and panel_info video struct for
 * your display.
 */

#ifdef CONFIG_PXA_VGA
/* LCD outputs connected to a video DAC  */
# define LCD_BPP	LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x003008f8
# define REG_LCCR3	0x0300FF01

/* 640x480x16 @ 61 Hz */
vidinfo_t panel_info = {
	vl_col:		640,
	vl_row:		480,
	vl_width:	640,
	vl_height:	480,
	vl_clkp:	CFG_HIGH,
	vl_oep:		CFG_HIGH,
	vl_hsp:		CFG_HIGH,
	vl_vsp:		CFG_HIGH,
	vl_dp:		CFG_HIGH,
	vl_bpix:	LCD_BPP,
	vl_lbw:		0,
	vl_splt:	0,
	vl_clor:	0,
	vl_tft:		1,
	vl_hpw:		40,
	vl_blw:		56,
	vl_elw:		56,
	vl_vpw:		20,
	vl_bfw:		8,
	vl_efw:		8,
};
#endif /* CONFIG_PXA_VIDEO */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_SHARP_LM8V31

# define LCD_BPP	LCD_COLOR8
# define LCD_INVERT_COLORS	/* Needed for colors to be correct, but why?	*/

/* you have to set lccr0 and lccr3 (including pcd) */
# define REG_LCCR0	0x0030087C
# define REG_LCCR3	0x0340FF08

vidinfo_t panel_info = {
	vl_col:		640,
	vl_row:		480,
	vl_width:	157,
	vl_height:	118,
	vl_clkp:	CFG_HIGH,
	vl_oep:		CFG_HIGH,
	vl_hsp:		CFG_HIGH,
	vl_vsp:		CFG_HIGH,
	vl_dp:		CFG_HIGH,
	vl_bpix:	LCD_BPP,
	vl_lbw:		0,
	vl_splt:	1,
	vl_clor:	1,
	vl_tft:		0,
	vl_hpw:		1,
	vl_blw:		3,
	vl_elw:		3,
	vl_vpw:		1,
	vl_bfw:		0,
	vl_efw:		0,
};
#endif /* CONFIG_SHARP_LM8V31 */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_HITACHI_SX14
/* Hitachi SX14Q004-ZZA color STN LCD */
#define LCD_BPP		LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
#define REG_LCCR0	0x00301079
#define REG_LCCR3	0x0340FF20

vidinfo_t panel_info = {
	vl_col:		320,
	vl_row:		240,
	vl_width:	167,
	vl_height:	109,
	vl_clkp:	CFG_HIGH,
	vl_oep:		CFG_HIGH,
	vl_hsp:		CFG_HIGH,
	vl_vsp:		CFG_HIGH,
	vl_dp:		CFG_HIGH,
	vl_bpix:	LCD_BPP,
	vl_lbw:		1,
	vl_splt:	0,
	vl_clor:	1,
	vl_tft:		0,
	vl_hpw:		1,
	vl_blw:		1,
	vl_elw:		1,
	vl_vpw:		7,
	vl_bfw:		0,
	vl_efw:		0,
};
#endif /* CONFIG_HITACHI_SX14 */

/*----------------------------------------------------------------------*/
#ifdef CONFIG_SHARP_QVGA
/* Sharp 1/4 VGA LCD */
#define LCD_BPP		LCD_COLOR8

/* you have to set lccr0 and lccr3 (including pcd) */
#define REG_LCCR0	0x003008F8
#define REG_LCCR3	(0x0040FF0C|(LCD_BPP<<24))

vidinfo_t panel_info = {
	vl_col:		320,
	vl_row:		240,
	vl_width:	167,
	vl_height:	109,
	vl_clkp:	CFG_HIGH,
	vl_oep:		CFG_HIGH,
	vl_hsp:		CFG_HIGH,
	vl_vsp:		CFG_HIGH,
	vl_dp:		CFG_HIGH,
	vl_bpix:	LCD_BPP,
	vl_lbw:		1,
	vl_splt:	0,
	vl_clor:	1,
	vl_tft:		1,
	vl_hpw:		64,
	vl_blw:		34,
	vl_elw:		1,
	vl_vpw:		20,
	vl_bfw:		8,
	vl_efw:		3,
};
#endif /* CONFIG_SHARP_QVGA */


#if LCD_BPP == LCD_COLOR8
void lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue);
#endif
#if LCD_BPP == LCD_MONOCHROME
void lcd_initcolregs (void);
#endif

#ifdef NOT_USED_SO_FAR
void lcd_disable (void);
void lcd_getcolreg (ushort regno, ushort *red, ushort *green, ushort *blue);
#endif /* NOT_USED_SO_FAR */

void lcd_ctrl_init	(void *lcdbase);
void lcd_enable	(void);

static unsigned long default_palette[] = { 
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


static int pxafb_init_mem (void *lcdbase, vidinfo_t *vid);
static void pxafb_setup_gpio (vidinfo_t *vid);
static void pxafb_enable_controller (vidinfo_t *vid);
static int pxafb_init (vidinfo_t *vid);
/************************************************************************/

/************************************************************************/
/* ---------------  PXA chipset specific functions  ------------------- */
/************************************************************************/
#if defined(CONFIG_LCD)

int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;		/* Start of console buffer	*/

short console_col;
short console_row;

void lcd_ctrl_init (void *lcdbase)
{
#ifdef CONFIG_LCDPANEL
   char const *panelName = getenv( "panel" );
   if( panelName )
   {
      struct lcd_panel_info_t const *panel ;
      panel = find_lcd_panel( panelName );
      if( panel )
      {
         printf( "panel %s found: %u x %u\n", panelName, panel->xres, panel->yres );
         panel_info.pxa.screen = (u_long)lcdbase;
         set_lcd_panel( panel ); 
      }
      else
         printf( "panel %s not found\n", panelName );
   }
#else   
	pxafb_init_mem(lcdbase, &panel_info);
	pxafb_init(&panel_info);
	pxafb_setup_gpio(&panel_info);
	pxafb_enable_controller(&panel_info);
#endif
}

/*----------------------------------------------------------------------*/
#if LCD_BPP == LCD_COLOR8
void lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
#if defined( CONFIG_PXA250 )
	struct pxafb_info *fbi = &panel_info.pxa;
	unsigned short *palette = (unsigned short *)fbi->palette;
	u_int val;

	if (regno < fbi->palette_size) {
		val = ((red << 8) & 0xf800);
		val |= ((green << 4) & 0x07e0);
		val |= (blue & 0x001f);

#ifdef LCD_INVERT_COLORS
		palette[regno] = ~val;
#else
		palette[regno] = val;
#endif
	}
#elif defined( CONFIG_PXA270 )
	struct pxafb_info *fbi = &panel_info.pxa;
	u32 *palette = (u32 *)fbi->palette;
	u32 val;

	if (regno < fbi->palette_size) {
		val = 0xFF000000 ; // transparency
		val |= (red << 16);
		val |= (green << 8);
		val |= blue ;

		palette[regno] = val;
	}
#else
#error no processor defined
#endif
	debug ("setcolreg: reg %2d @ %p: R=%02X G=%02X B=%02X => %04X\n",
		regno, &palette[regno],
		red, green, blue,
		palette[regno]);
}

void lcd_enable (void)
{
}

#endif

#ifdef CONFIG_PXA270
//each entry is in ARGB format, alpha high byte, blue low byte
void lcd_SetPalette(ulong* palette,int colorCnt)
{
	PALETTEVAL_TYPE *cmap = (PALETTEVAL_TYPE *)panel_info.pxa.palette;
	while (colorCnt--) {
		*cmap++ = *palette++;
	}
}
#else
//each entry is in ARGB format, alpha high byte, blue low byte
void lcd_SetPalette(ulong* palette,int colorCnt)
{
	PALETTEVAL_TYPE *cmap = (PALETTEVAL_TYPE *)panel_info.pxa.palette;
	while (colorCnt--) {
		ulong tmp = *palette++;	//5,6,5 format
		*cmap++ =	( (tmp>>8) & 0xf800) |
					( (tmp>>5) & 0x07e0) |
					( (tmp>>3) & 0x001f) ;
	}
}
#endif
#endif /* LCD_COLOR8 */

/*----------------------------------------------------------------------*/
#if LCD_BPP == LCD_MONOCHROME
void lcd_initcolregs (void)
{
	struct pxafb_info *fbi = &panel_info.pxa;
	cmap = (ushort *)fbi->palette;
	ushort regno;

	for (regno = 0; regno < 16; regno++) {
		cmap[regno * 2] = 0;
		cmap[(regno * 2) + 1] = regno & 0x0f;
	}
}
#endif /* LCD_MONOCHROME */

/************************************************************************/
/* ** PXA255 specific routines						*/
/************************************************************************/

static int pxafb_init_mem (void *lcdbase, vidinfo_t *vid)
{
	u_long palette_mem_size;
	struct pxafb_info *fbi = &vid->pxa;
	int fb_size = vid->vl_row * (vid->vl_col * NBITS (vid->vl_bpix)) / 8;

	fbi->screen = (u_long)lcdbase;

	fbi->palette_size = NBITS(vid->vl_bpix) == 8 ? 256 : 16;
	palette_mem_size = fbi->palette_size * sizeof(PALETTEVAL_TYPE);

	debug("palette_mem_size = 0x%08lx\n", (u_long) palette_mem_size);
	/* locate palette and descs at end of page following fb */
	fbi->palette = (u_long)lcdbase + fb_size + PAGE_SIZE - palette_mem_size;

	memcpy((void *)fbi->palette, default_palette,sizeof(default_palette));
	memset((void *)fbi->screen, 0xff, fb_size );
	return 0;
}

static void pxafb_setup_gpio (vidinfo_t *vid)
{
	u_long lccr0;

	/*
	 * setup is based on type of panel supported
	 */

	lccr0 = vid->pxa.reg_lccr0;

	/* 4 bit interface */
	if ((lccr0 & LCCR0_CMS) && (lccr0 & LCCR0_SDS) && !(lccr0 & LCCR0_DPD))
	{
		debug("Setting GPIO for 4 bit data\n");
		/* bits 58-61 */
		GPDR1 |= (0xf << 26);
		GAFR1_U = (GAFR1_U & ~(0xff << 20)) | (0xaa << 20);

		/* bits 74-77 */
		GPDR2 |= (0xf << 10);
		GAFR2_L = (GAFR2_L & ~(0xff << 20)) | (0xaa << 20);
	}

	/* 8 bit interface */
	else if (((lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_DPD))) ||
		(!(lccr0 & LCCR0_CMS) && !(lccr0 & LCCR0_PAS) && !(lccr0 & LCCR0_SDS)))
	{
		debug("Setting GPIO for 8 bit data\n");
		/* bits 58-65 */
		GPDR1 |= (0x3f << 26);
		GPDR2 |= (0x3);

		GAFR1_U = (GAFR1_U & ~(0xfff << 20)) | (0xaaa << 20);
		GAFR2_L = (GAFR2_L & ~0xf) | (0xa);

		/* bits 74-77 */
		GPDR2 |= (0xf << 10);
		GAFR2_L = (GAFR2_L & ~(0xff << 20)) | (0xaa << 20);
	}

	/* 16 bit interface */
	else if (!(lccr0 & LCCR0_CMS) && ((lccr0 & LCCR0_SDS) || (lccr0 & LCCR0_PAS)))
	{
		debug("Setting GPIO for 16 bit data\n");
		/* bits 58-77 */
		GPDR1 |= (0x3f << 26);
		GPDR2 |= 0x00003fff;

		GAFR1_U = (GAFR1_U & ~(0xfff << 20)) | (0xaaa << 20);
		GAFR2_L = (GAFR2_L & 0xf0000000) | 0x0aaaaaaa;
	}
	else
	{
		printf("pxafb_setup_gpio: unable to determine bits per pixel\n");
	}
}

static void pxafb_enable_controller (vidinfo_t *vid)
{
	debug("Enabling LCD controller\n");

	/* Sequence from 11.7.10 */
	LCCR3  = vid->pxa.reg_lccr3;
	LCCR2  = vid->pxa.reg_lccr2;
	LCCR1  = vid->pxa.reg_lccr1;
	LCCR0  = vid->pxa.reg_lccr0 & ~LCCR0_ENB;
	FDADR0 = vid->pxa.fdadr0;
	FDADR1 = vid->pxa.fdadr1;
	LCCR0 |= LCCR0_ENB;

	CKEN |= CKEN16_LCD;

	debug("FDADR0 = 0x%08x\n", (unsigned int)FDADR0);
	debug("FDADR1 = 0x%08x\n", (unsigned int)FDADR1);
	debug("LCCR0 = 0x%08x\n", (unsigned int)LCCR0);
	debug("LCCR1 = 0x%08x\n", (unsigned int)LCCR1);
	debug("LCCR2 = 0x%08x\n", (unsigned int)LCCR2);
	debug("LCCR3 = 0x%08x\n", (unsigned int)LCCR3);
}

static int pxafb_init (vidinfo_t *vid)
{
	struct pxafb_info *fbi = &vid->pxa;
	unsigned long const reg_lccr3 = 0x0000FF0C|(LCD_BPP<<24);

	debug("Configuring PXA LCD\n");

#if defined( CONFIG_PXA270 )

#if (PLATFORM_TYPE==HALOGEN)
#if (PLATFORM_REV==1)||(PLATFORM_REV==2)
#define PALETTE_SELECT	0x00010000 ;        // 18-bits to panel
#endif
#endif

#if (PLATFORM_TYPE==NEON270)
#define PALETTE_SELECT	0x00010000 ;        // 18-bits to panel
#endif

#ifndef PALETTE_SELECT
#define PALETTE_SELECT	0x00008000 ;        // 16-bits to panel, default
#endif

	LCCR4 = PALETTE_SELECT;
#endif

	fbi->reg_lccr0 = 0x003008F8;
	fbi->reg_lccr3 = reg_lccr3 ;

	debug("vid: vl_col=%d hslen=%d lm=%d rm=%d\n",
		vid->vl_col, vid->vl_hpw,
		vid->vl_blw, vid->vl_elw);
	debug("vid: vl_row=%d vslen=%d um=%d bm=%d\n",
		vid->vl_row, vid->vl_vpw,
		vid->vl_bfw, vid->vl_efw);

	fbi->reg_lccr1 =
		LCCR1_DisWdth(vid->vl_col) +
		LCCR1_HorSnchWdth(vid->vl_hpw) +
		LCCR1_BegLnDel(vid->vl_blw) +
		LCCR1_EndLnDel(vid->vl_elw);

	fbi->reg_lccr2 =
		LCCR2_DisHght(vid->vl_row) +
		LCCR2_VrtSnchWdth(vid->vl_vpw) +
		LCCR2_BegFrmDel(vid->vl_bfw) +
		LCCR2_EndFrmDel(vid->vl_efw);

	fbi->reg_lccr3 = (reg_lccr3 & ~(LCCR3_HSP | LCCR3_VSP))|
		(vid->vl_hsp ? 0 : LCCR3_HSP) |
		(vid->vl_vsp ? 0 : LCCR3_VSP) |
		(vid->vl_clkp ? LCCR3_PCP : 0);


	/* setup dma descriptors */
	fbi->dmadesc_fblow = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 3*16);
	fbi->dmadesc_fbhigh = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 2*16);
	fbi->dmadesc_palette = (struct pxafb_dma_descriptor *)((unsigned int)fbi->palette - 1*16);

	#define BYTES_PER_PANEL	((fbi->reg_lccr0 & LCCR0_SDS) ? \
		(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8 / 2) : \
		(vid->vl_col * vid->vl_row * NBITS(vid->vl_bpix) / 8))

	/* populate descriptors */
	fbi->dmadesc_fblow->fdadr = (u_long)fbi->dmadesc_fblow;
	fbi->dmadesc_fblow->fsadr = fbi->screen + BYTES_PER_PANEL;
	fbi->dmadesc_fblow->fidr  = 0;
	fbi->dmadesc_fblow->ldcmd = BYTES_PER_PANEL;

	fbi->fdadr1 = (u_long)fbi->dmadesc_fblow; /* only used in dual-panel mode */

	fbi->dmadesc_fbhigh->fsadr = fbi->screen;
	fbi->dmadesc_fbhigh->fidr = 0;
	fbi->dmadesc_fbhigh->ldcmd = BYTES_PER_PANEL;

	fbi->dmadesc_palette->fsadr = fbi->palette;
	fbi->dmadesc_palette->fidr  = 0;
	fbi->dmadesc_palette->ldcmd = (fbi->palette_size * sizeof(PALETTEVAL_TYPE)) | LDCMD_PAL;

	if( NBITS(vid->vl_bpix) < 12)
	{
		/* assume any mode with <12 bpp is palette driven */
		fbi->dmadesc_palette->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_palette;
		/* flips back and forth between pal and fbhigh */
		fbi->fdadr0 = (u_long)fbi->dmadesc_palette;
	}
	else
	{
		/* palette shouldn't be loaded in true-color mode */
		fbi->dmadesc_fbhigh->fdadr = (u_long)fbi->dmadesc_fbhigh;
		fbi->fdadr0 = (u_long)fbi->dmadesc_fbhigh; /* no pal just fbhigh */
	}

	debug("fbi->dmadesc_fblow = 0x%lx\n", (u_long)fbi->dmadesc_fblow);
	debug("fbi->dmadesc_fbhigh = 0x%lx\n", (u_long)fbi->dmadesc_fbhigh);
	debug("fbi->dmadesc_palette = 0x%lx\n", (u_long)fbi->dmadesc_palette);

	debug("fbi->dmadesc_fblow->fdadr = 0x%lx\n", fbi->dmadesc_fblow->fdadr);
	debug("fbi->dmadesc_fbhigh->fdadr = 0x%lx\n", fbi->dmadesc_fbhigh->fdadr);
	debug("fbi->dmadesc_palette->fdadr = 0x%lx\n", fbi->dmadesc_palette->fdadr);

	debug("fbi->dmadesc_fblow->fsadr = 0x%lx\n", fbi->dmadesc_fblow->fsadr);
	debug("fbi->dmadesc_fbhigh->fsadr = 0x%lx\n", fbi->dmadesc_fbhigh->fsadr);
	debug("fbi->dmadesc_palette->fsadr = 0x%lx\n", fbi->dmadesc_palette->fsadr);

	debug("fbi->dmadesc_fblow->ldcmd = 0x%lx\n", fbi->dmadesc_fblow->ldcmd);
	debug("fbi->dmadesc_fbhigh->ldcmd = 0x%lx\n", fbi->dmadesc_fbhigh->ldcmd);
	debug("fbi->dmadesc_palette->ldcmd = 0x%lx\n", fbi->dmadesc_palette->ldcmd);

	return 0;
}

/************************************************************************/
/************************************************************************/

#endif /* CONFIG_LCD */

#ifdef CONFIG_LCDPANEL

unsigned int get_lclk(void)
{
//    pfreq == LCLK/(2*(PCD+1))
//    pfreq*(2*(PCD+1)) == LCLK
//    2*(PCD+1) == LCLK/pfreq
//    (PCD+1) == LCLK/(pfreq*2)
//    PCD == (LCLK/(pfreq*2)) - 1 ;
//
	unsigned l = CCCR & 0x1F ;
	unsigned lclk;
	if (l<2) l = 2;
	lclk = 13000000*l;
	if (l>=8) lclk >>= (l<=16)? 1 : 2;
	return lclk;
}

static inline unsigned int get_pcd(unsigned long pixclock)
{
   unsigned lclk = get_lclk();
   unsigned long pcd = (lclk/(2*pixclock)) - 1;
   return pcd & 0xFF ;
}

#if defined(CONFIG_LCD_MULTI)
static void *lcd_palette = 0 ;
static void *frame_buffer = 0 ;

static void set_palette(unsigned long *colors, unsigned colorCount)
{
	memcpy( lcd_palette, colors, colorCount*sizeof(*colors) );
}

static unsigned long get_palette_color(unsigned char idx)
{
	return ((unsigned long *)lcd_palette)[idx];
}

static void disable(void)
{
}
#endif

#ifdef CONFIG_LCD
void set_lcd_panel( struct lcd_panel_info_t const *panel )
#elif defined(CONFIG_LCD_MULTI)
void init_pxa_fb( struct lcd_t *lcd )
#endif
{
#if defined(CONFIG_LCD_MULTI)
   struct lcd_panel_info_t const *panel = &lcd->info ;
   vidinfo_t panel_info ;
#endif
   int pixClock = panel->pixclock;

   panel_info.vl_col = panel->xres ;
   panel_info.vl_row = panel->yres ;
   panel_info.vl_clkp = panel->pclk_redg ;
   panel_info.vl_oep  = panel->pclk_redg ;
   panel_info.vl_hsp  = panel->hsyn_acth ;
   panel_info.vl_vsp  = panel->vsyn_acth ;
   panel_info.vl_dp   = panel->pclk_redg ;
   panel_info.vl_bpix = LCD_BPP ;
   panel_info.vl_lcd_line_length = (panel_info.vl_col * NBITS (panel_info.vl_bpix)) >> 3;
   panel_info.vl_lbw  = 1 ;
   panel_info.vl_splt = 0 ;
   panel_info.vl_clor = 1 ;
   panel_info.vl_tft = panel->active ;
   panel_info.vl_hpw = panel->hsync_len ;
   panel_info.vl_blw = panel->left_margin ;
   panel_info.vl_elw = panel->right_margin ;
   panel_info.vl_vpw = panel->vsync_len ;
   panel_info.vl_bfw = panel->upper_margin ;
   panel_info.vl_efw = panel->lower_margin ;

   panel_info.pxa.screen = CFG_DRAM_BASE+CFG_DRAM_SIZE-(panel->xres*panel->yres+1024+4096);
   pxafb_init_mem( (void *)panel_info.pxa.screen, &panel_info);
   pxafb_init(&panel_info);
   
   if ( pixClock < 10) {
   	  pixClock = (panel_info.vl_col+panel_info.vl_hpw+panel_info.vl_blw+panel_info.vl_elw)*
   	  			 (panel_info.vl_row+panel_info.vl_vpw+panel_info.vl_bfw+panel_info.vl_efw)*55;
   }
   panel_info.pxa.reg_lccr3 &= ~0xFF ;
   panel_info.pxa.reg_lccr3 |= get_pcd( pixClock );

   debug( "set panel to %s\n"
          "pcd == 0x%08lx\n"
          "lclk == %i\n"
          "palette at %p\n"
          "frame buffer at %p\n"
          , panel->name ? panel->name : "<Unnamed>"
          , get_pcd( pixClock )
          , get_lclk()
          , panel_info.pxa.palette
          , panel_info.pxa.screen
   );
   pxafb_setup_gpio(&panel_info);
   pxafb_enable_controller(&panel_info);

#if defined(CONFIG_LCD_MULTI)
   lcd_palette = (void *)panel_info.pxa.palette ;
   lcd->fbAddr = frame_buffer = (void *)panel_info.pxa.screen ;
   lcd->set_palette = set_palette ;
   lcd->get_palette_color = get_palette_color ;
   lcd->disable = disable ;
   lcd->bg = 0xff ;
   lcd->fg = 0 ;
#endif

#ifdef CONFIG_LCD
   cur_lcd_panel = panel ;
#endif
}

#endif // dynamic LCD panel support


