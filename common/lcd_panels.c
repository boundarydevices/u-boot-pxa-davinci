/*
 * Module lcd_panels.cpp
 *
 * This module defines the num_lcd_panels and lcd_panels
 * constants as declared in lcd_panels.h
 *
 * Change History :
 *
 * $Log: lcd_panels.c,v $
 * Revision 1.20  2007/01/15 20:39:48  ericn
 * -bump clock rate for Okaya
 *
 * Revision 1.19  2007/01/15 19:50:42  ericn
 * -rename pclk_acth to pclk_redg
 *
 * Revision 1.18  2007/01/15 00:11:55  ericn
 * -separate vertical/horizontal sync from pixel clock polarity
 *
 * Revision 1.17  2006/12/12 14:58:40  ericn
 * -add panel sxga60hz
 *
 * Revision 1.16  2006/12/12 00:23:44  ericn
 * -added parse_panel_info() routine
 *
 * Revision 1.15  2006/11/04 20:25:20  ericn
 * -put back hitachi_154
 *
 * Revision 1.14  2006/09/23 22:13:50  ericn
 * -add vga_crt
 *
 * Revision 1.13  2006/09/16 19:10:33  ericn
 * -add crt_800x600 for Hexachain panels
 *
 * Revision 1.12  2006/09/07 17:47:40  tkisky
 * -add OKAYA
 *
 * Revision 1.11  2006/09/01 01:03:16  ericn
 * -add olevia display
 *
 * Revision 1.10  2006/08/03 22:27:00  ericn
 * -add hitachi_154
 *
 * Revision 1.9  2006/05/23 18:46:39  ericn
 * -added hitachi_wxga panel
 *
 * Revision 1.8  2005/09/19 13:15:43  ericn
 * -updated sharp_qvga timing
 *
 * Revision 1.7  2005/07/18 03:05:35  ericn
 * -update crt1024x768 timings
 *
 * Revision 1.6  2005/07/07 03:42:22  tkisky
 * -make my CRT 1024x768 display work
 *
 * Revision 1.5  2005/06/02 04:02:39  ericn
 * -added qvga_portrait
 *
 * Revision 1.4  2005/05/15 18:59:00  tkisky
 * -change polarity of hitachi_wvga
 *
 * Revision 1.3  2005/05/03 15:32:11  ericn
 * -fast pixclock for hvga, remove redundant 1024x768
 *
 * Revision 1.2  2005/04/30 20:33:22  ericn
 * -added CRT support
 *
 * Revision 1.1  2005/04/09 17:49:17  ericn
 * -Initial import
 *
 *
 * Copyright Boundary Devices, Inc. 2005
 */


#include "lcd_panels.h"
#include <common.h>
#include <asm/string.h>
#include <malloc.h>
#ifdef CONFIG_CMD_I2C
#include <i2c.h>
#include "edid.h"
#endif
/*
Settings for Hitachi 5.7
		PANEL_HORIZONTAL_TOTAL, 01c00160);    // should be 34+320+1+64-1= 418 = 0x1A2 (Hex)
                                               + 0x13f+16
		PANEL_HORIZONTAL_SYNC, 00400161);            hsync_len == 64    lmargin=0x161-0x13f=34
		PANEL_VERTICAL_TOTAL, 0x010800f0);
		PANEL_VERTICAL_SYNC, 0x00020104);     vsync=2,  upper_margin=0x0104-0xf0-1= 19

In bdlogo.bmp - offset 436 is pixel data

Sharp 5.7 active

   STUFFREG( hTotalReg,      0x01800140 );  // should be 16+320+1+8-1 == 0x158
                                             + 0x13f (width-1)
   STUFFREG( hSyncReg,       0x0008014f );         hsync_len == 8    lmargin=0x14f-0x13f=16
   STUFFREG( vTotalReg,      0x010700F0 );
   STUFFREG( vSyncReg,       0x00020100 );     vsync=2,  upper_margin=0x0100-0xf0+1= 17

static unsigned const hTotalReg      = 0x00080024 ;  //  015F0140
static unsigned const hSyncReg       = 0x00080028 ;  //  0008014f
static unsigned const vTotalReg      = 0x0008002c ;  //  010700F0
static unsigned const vSyncReg       = 0x00080030 ;  //  000200FE

const unsigned int sm501_list2[]={0x0fe80000,
dispctrl	  0x0F013104,             // 0f0d0105
pan        0x00000000,
colorkey   0x00000000,
fbaddr     0x00000000,
offsetww   ((LCD_XRES)<<16)+(LCD_XRES),
fbwidth    (LCD_XRES<<16),
fbheight   (LCD_YRES<<16),
tllocate   0x00000000,
brlocate   ((LCD_YRES-1)<<16)+(LCD_XRES-1),
htotal     ((LCD_BEGIN_OF_LINE_WAIT_COUNT+
             LCD_XRES+
             LCD_END_OF_LINE_WAIT_COUNT+
             LCD_HORIZONTAL_SYNC_PULSE_WIDTH-1)<<16)
           +(LCD_XRES-1),
hsync      (LCD_HORIZONTAL_SYNC_PULSE_WIDTH<<16)
          +(LCD_XRES+LCD_BEGIN_OF_LINE_WAIT_COUNT-1),
vtotal     ((LCD_BEGIN_FRAME_WAIT_COUNT+
             LCD_YRES+
             LCD_END_OF_FRAME_WAIT_COUNT+
             LCD_VERTICAL_SYNC_PULSE_WIDTH-1)<<16)
           +(LCD_YRES-1),
vsync   	  (LCD_VERTICAL_SYNC_PULSE_WIDTH<<16)
           +(LCD_YRES+LCD_BEGIN_FRAME_WAIT_COUNT-1)};
*/

static struct lcd_panel_info_t const lcd_panels_[] = {
{
	name: "hitachi_qvga",
	pixclock: 0,
	xres: 320,
	yres: 240,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin:  1,
	right_margin: 16,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "sharp_qvga",
	pixclock: 0,
	xres: 320,
	yres: 240,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 20,		/* 8    */
	left_margin: 1,		/* 16   */
	right_margin: 30,	/* 1    */
	vsync_len: 4,		/* 20   */
	upper_margin: 17,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
//#define okaya_qvga_P   320, 30,37, 38,	240, 3, 16, 15,	1,1,0,0,0,	1,0,0,1,120,LCD_PANEL
	name: "okaya_qvga",
	pixclock: 13974000,	//(320+30+37+38)*(240+3+16+15)*120 Hz = 425*274*120
	xres: 320,
	yres: 240,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 30,
	left_margin: 37,
	right_margin: 38,
	vsync_len: 3,
	upper_margin: 16,
	lower_margin: 15,
	active: 1,
	crt: 0
}, {
#define okaya_480x272_P 480, 41, 1, 2,	272, 10, 3, 2,	1,1,1,0,0,	1,0,0,1,62,LCD_PANEL
	name: "okaya_480x272",
	pixclock: 10226384,	//(480+41+1+2)*(272+10+3+2)*68 Hz = 568*295*68 = 10226384
	xres: 480,
	yres: 272,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 41,
	left_margin: 1,
	right_margin: 2,
	vsync_len: 10,
	upper_margin: 3,
	lower_margin: 2,
	active: 1,
	crt:0
}, {
	name: "urt_480x272",
	pixclock: 12000000,
	xres: 480,
	yres: 272,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 0,
	oepol_actl: 1,
	hsync_len: 41,
	left_margin: 42,
	right_margin: 42,
	vsync_len: 10,
	upper_margin: 1,
	lower_margin: 6,
	active: 1,
	crt:0
}, {
	name: "qvga_portrait",
	pixclock: 0,
	xres: 240,
	yres: 320,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 34,
	right_margin: 1,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0,
	rotation: 90
}, {
	name: "hitachi_hvga",
	pixclock: (640+64+34+1)*(240+20+8+3)*68,
	xres: 640,
	yres: 240,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 34,
	right_margin: 1,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "sharp_vga",
	pixclock: 1,
	xres: 640,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 60,		//3
	right_margin: 60,
	vsync_len: 20,		//34
	upper_margin: 34,		//24
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "vga_crt",
	pixclock: 1,		//24000000
	xres: 640,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 60,		//3
	right_margin: 60,
	vsync_len: 20,		//34
	upper_margin: 34,		//24
	lower_margin: 3,
	active: 1,
	crt: 1
}, {
	name: "hitachi_wvga",
	pixclock: 1,
	xres: 800,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 1,
	right_margin: 39,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "crt_800x600",
	pixclock: 56000000,
	xres: 800,
	yres: 600,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 32,
	right_margin: 152,
	vsync_len: 3,
	upper_margin: 1,
	lower_margin: 27,
	active: 1,
	crt: 1
}, {
	name: "gvision_10.4",
	pixclock: 56000000,
	xres: 800,
	yres: 600,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 32,
	right_margin: 16,
	vsync_len: 8,
	upper_margin: 3,
	lower_margin: 2,
	active: 1,
	crt: 1
}, {
	name: "lcd_svga",
	pixclock: 56000000,
	xres: 800,
	yres: 600,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 32,
	right_margin: 152,
	vsync_len: 3,
	upper_margin:1,
	lower_margin: 27,
	active: 1,
	crt: 0
}, {
// Note that you can use the nifty tool at the
// following location to generate these values:
//    http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
	name: "crt1024x768",
	pixclock: 65000000,
	xres: 1024,
	yres: 768,
	pclk_redg: 0,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 136,
	left_margin: 24,
	right_margin: 160,
	vsync_len: 6,
	upper_margin: 3,
	lower_margin: 29,
	active: 0,
	crt: 1
}, {
	name: "hitachi_wxga",
	pixclock: 1,
	xres: 1024,
	yres: 768,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 1,
	right_margin: 39,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
#define hitachi_154_P 1280,64, 24,16,	800, 20, 4, 3,	1,1,0,0,0,	1,0,0,1,62,LCD_PANEL
	name: "hitachi_154",
	pixclock: 72000000,
	xres: 1280,
	yres: 800,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 24,
	right_margin: 16,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "olevia",
	pixclock: 65000000,
	xres: 1024,
	yres: 768,
	pclk_redg: 0,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 200,
	left_margin: 37,
	right_margin: 228,
	vsync_len: 6,
	upper_margin: 3,
	lower_margin: 29,
	active: 0,
	crt: 1
}, {
	name: "crt_sxga60hz",
	pixclock: 108000000,
	xres: 1280,
	yres: 1024,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 120,
	left_margin: 64,
	right_margin: 264,
	vsync_len: 4,
	upper_margin: 2,
	lower_margin: 44,
	active: 1,
	crt: 1
}, {
	name: "lcd_sxga60hz",
	pixclock: 108000000,
	xres: 1280,
	yres: 1024,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 120,
	left_margin: 64,
	right_margin: 264,
	vsync_len: 4,
	upper_margin: 2,
	lower_margin: 44,
	active: 1,
	crt: 0
}, {
	name: "hitachi_92",
	pixclock: 20000000,
	xres: 960,
	yres: 160,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 15,
	left_margin: 220,
	right_margin: 1,
	vsync_len: 200,
	upper_margin: 148,
	lower_margin: 3,
	active: 1,
	crt: 0
}, {
	name: "sharp_480x272",
	pixclock: 9000000,
	xres: 480,
	yres: 272,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 41,
	left_margin: 2,
	right_margin: 2,
	vsync_len: 10,
	upper_margin: 2,
	lower_margin: 2,
	active: 1,
	crt: 0
}, {
#if 1
#define tovis_w_P	1024,104,56,160,	200,3,201,11,	0,0,1,0,0,	1,0,0,1,75,CRT
	name: "tovis_w",
	pixclock: (1024+104+56+160)*(200+3+201+11)*75,
	xres: 1024,
	yres: 200,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 104,
	left_margin: 56,
	right_margin: 160,
	vsync_len: 3,
	upper_margin: 201,
	lower_margin: 11,
	active: 1,
	crt: 1
#else
#define tovis_w_P	1024,64,1,39,		204,20,8,3,		1,1,1,0,0,	1,0,0,1,75,LCD_PANEL
	name: "tovis_w",
	pixclock: (1024+64+1+39)*(204+20+8+3)*75,
	xres: 1024,
	yres: 204,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 1,
	right_margin: 39,
	vsync_len: 20,
	upper_margin: 8,
	lower_margin: 3,
	active: 1,
	crt: 0
#endif
}, {
	name: "samsung1600x1050",
	pixclock: 142000000,
	xres: 1600,
	yres: 1050,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 104,
	left_margin: 128,
	right_margin: 264,
	vsync_len: 4,
	upper_margin: 2,
	lower_margin: 44,
	active: 1,
	crt: 0
}, {
	name: "adap216x24",
	pixclock: 25125000,
	xres: 640,
	yres: 480,
	pclk_redg: 0,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 95,
	left_margin: 16,
	right_margin: 49,
	vsync_len: 1,
	upper_margin: 2,
	lower_margin: 42,
	active: 1,
	crt: 0
}, {
	name: "lg1360x398",  //             LG:81000000,1360,398,1,0,1,0,50,268,420,1,214,21,1,0
	pixclock: 81000000,
	xres: 1360,
	yres: 398,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 50,
	left_margin: 268,
	right_margin: 420,
	vsync_len: 1,
	upper_margin: 214,
	lower_margin: 21,
	active: 1,
	crt: 1
}, {
	name: "lg1360x480",  //             LG:84750000,1360,480,1,0,1,208,72,136,5,291,22,1,1
	pixclock: 81000000,
	xres: 1360,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 50,
	left_margin: 268,
	right_margin: 420,
	vsync_len: 1,
	upper_margin: 124,
	lower_margin: 21,
	active: 1,
	crt: 1
}
};

/*
. e
typedef enum _polarity_t
{
	POSITIVE,
	NEGATIVE,
}
polarity_t;

typedef struct _mode_table_t
{
	// Horizontal timing.
	int horizontal_total;
	int horizontal_display_end;
	int horizontal_sync_start;
	int horizontal_sync_width;
	polarity_t horizontal_sync_polarity;

	// Vertical timing.
	int vertical_total;
	int vertical_display_end;
	int vertical_sync_start;
	int vertical_sync_height;
	polarity_t vertical_sync_polarity;

	// Refresh timing.
	long pixel_clock;
	long horizontal_frequency;
	long vertical_frequency;
}
mode_table_t;

	// 1024 x 768
 htotal dend hsstrt hsw  hpolar	vtot vdend vdstrt vsh vpolar	pixclk	hfreq vfreq
{ 1344, 1024, 1048, 136, NEGATIVE, 806, 768,	771,  6, NEGATIVE, 65000000, 48363, 60 },
{ 1328, 1024, 1048, 136, NEGATIVE, 806, 768,	771,  6, NEGATIVE, 75000000, 56476, 70 },
{ 1312, 1024, 1040,  96, POSITIVE, 800, 768,	769,  3, POSITIVE, 78750000, 60023, 75 },
{ 1376, 1024, 1072,  96, POSITIVE, 808, 768,	769,  3, POSITIVE, 94500000, 68677, 85 },

0FE80200/00010000 +		   CRT regs
0FE80204/00180000 +
0FE80208/08000800 +
0FE8020C/05D003FF +
0FE80210/00C80424 +
0FE80214/032502FF +
0FE80218/00060302 +
0FE8021C/00000000 +
0FE80220/00000000 +
0FE80224/00400200 +
0FE80228/00000000 +
0FE8022C/00000000 +
0FE80230/00000800 +
0FE80234/00000000 +
0FE80238/08000000 +
0FE8023C/00000400 +
*/

struct lcd_panel_info_t const * const lcd_panels = lcd_panels_ ;
unsigned const num_lcd_panels = sizeof(lcd_panels_)/sizeof(lcd_panels_[0]);

struct lcd_panel_info_t const *find_lcd_panel( char const * name )
{
   unsigned i ;
   for( i = 0 ; i < num_lcd_panels ; i++ )
   {
	  if( 0 == strcmp( lcd_panels_[i].name, name ) )
		 return lcd_panels_+i ;
   }
   return 0 ;
}

#define FB_CVT_CELLSIZE			   8
#define FB_CVT_GTF_C				 40
#define FB_CVT_GTF_J				 20
#define FB_CVT_GTF_K				128
#define FB_CVT_GTF_M				600
#define FB_CVT_MIN_VSYNC_BP		 550
#define FB_CVT_MIN_VPORCH			 3
#define FB_CVT_MIN_BPORCH			 6

#define FB_CVT_RB_MIN_VBLANK		460
#define FB_CVT_RB_HBLANK			160
#define FB_CVT_RB_V_FPORCH			3

#define FB_CVT_FLAG_REDUCED_BLANK 1
#define FB_CVT_FLAG_MARGINS	   2
#define FB_CVT_FLAG_INTERLACED	4

#define FB_VMODE_NONINTERLACED  0	/* non interlaced */
#define FB_VMODE_INTERLACED	1	/* interlaced	*/
#define FB_VMODE_DOUBLE		2	/* double scan */
#define FB_VMODE_ODD_FLD_FIRST	4	/* interlaced: top line first */
#define FB_VMODE_MASK		255

#define FB_VMODE_YWRAP		256	/* ywrap instead of panning	 */
#define FB_VMODE_SMOOTH_XPAN	512	/* smooth xpan possible (internally used) */
#define FB_VMODE_CONUPDATE	512	/* don't update x/yoffset	*/

#define FB_SYNC_HOR_HIGH_ACT	1	/* horizontal sync high active	*/
#define FB_SYNC_VERT_HIGH_ACT	2	/* vertical sync high active	*/

#define KHZ2PICOS(a) (1000000000UL/(a))

struct fb_videomode {
	const char *name;	/* optional */
	u32 refresh;		/* optional */
	u32 xres;
	u32 yres;
	u32 pixclock;
	u32 left_margin;
	u32 right_margin;
	u32 upper_margin;
	u32 lower_margin;
	u32 hsync_len;
	u32 vsync_len;
	u32 sync;
	u32 vmode;
	u32 flag;
};

struct fb_cvt_data {
	u32 xres;
	u32 yres;
	u32 refresh;
	u32 f_refresh;
	u32 pixclock;
	u32 hperiod;
	u32 hblank;
	u32 hfreq;
	u32 htotal;
	u32 vtotal;
	u32 vsync;
	u32 hsync;
	u32 h_front_porch;
	u32 h_back_porch;
	u32 v_front_porch;
	u32 v_back_porch;
	u32 h_margin;
	u32 v_margin;
	u32 interlace;
	u32 aspect_ratio;
	u32 active_pixels;
	u32 flags;
	u32 status;
};

static const unsigned char fb_cvt_vbi_tab[] = {
	4,		/* 4:3	  */
	5,		/* 16:9	 */
	6,		/* 16:10	*/
	7,		/* 5:4	  */
	7,		/* 15:9	 */
	8,		/* reserved */
	9,		/* reserved */
	10		/* custom   */
};

static u32 fb_cvt_aspect_ratio(struct fb_cvt_data *cvt)
{
	u32 xres = cvt->xres;
	u32 yres = cvt->yres;
	u32 aspect = -1;

	if (xres == (yres * 4)/3 && !((yres * 4) % 3))
		aspect = 0;
	else if (xres == (yres * 16)/9 && !((yres * 16) % 9))
		aspect = 1;
	else if (xres == (yres * 16)/10 && !((yres * 16) % 10))
		aspect = 2;
	else if (xres == (yres * 5)/4 && !((yres * 5) % 4))
		aspect = 3;
	else if (xres == (yres * 15)/9 && !((yres * 15) % 9))
		aspect = 4;
	else {
		printf( "fbcvt: Aspect ratio not CVT "
			   "standard\n");
		aspect = 7;
		cvt->status = 1;
	}

	return aspect;
}

/* returns hperiod * 1000 */
static u32 fb_cvt_hperiod(struct fb_cvt_data *cvt)
{
	u32 num = 1000000000/cvt->f_refresh;
	u32 den;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK) {
		num -= FB_CVT_RB_MIN_VBLANK * 1000;
		den = 2 * (cvt->yres/cvt->interlace + 2 * cvt->v_margin);
	} else {
		num -= FB_CVT_MIN_VSYNC_BP * 1000;
		den = 2 * (cvt->yres/cvt->interlace + cvt->v_margin * 2
			   + FB_CVT_MIN_VPORCH + cvt->interlace/2);
	}

	return 2 * (num/den);
}

/* returns ideal duty cycle * 1000 */
static u32 fb_cvt_ideal_duty_cycle(struct fb_cvt_data *cvt)
{
	u32 c_prime = (FB_CVT_GTF_C - FB_CVT_GTF_J) *
		(FB_CVT_GTF_K) + 256 * FB_CVT_GTF_J;
	u32 m_prime = (FB_CVT_GTF_K * FB_CVT_GTF_M);
	u32 h_period_est = cvt->hperiod;

	return (1000 * c_prime  - ((m_prime * h_period_est)/1000))/256;
}

static u32 fb_cvt_hblank(struct fb_cvt_data *cvt)
{
	u32 hblank = 0;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK)
		hblank = FB_CVT_RB_HBLANK;
	else {
		u32 ideal_duty_cycle = fb_cvt_ideal_duty_cycle(cvt);
		u32 active_pixels = cvt->active_pixels;

		if (ideal_duty_cycle < 20000)
			hblank = (active_pixels * 20000)/
				(100000 - 20000);
		else {
			hblank = (active_pixels * ideal_duty_cycle)/
				(100000 - ideal_duty_cycle);
		}
	}

	hblank &= ~((2 * FB_CVT_CELLSIZE) - 1);

	return hblank;
}

static u32 fb_cvt_hsync(struct fb_cvt_data *cvt)
{
	u32 hsync;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK)
		hsync = 32;
	else
		hsync = (FB_CVT_CELLSIZE * cvt->htotal)/100;

	hsync &= ~(FB_CVT_CELLSIZE - 1);
	return hsync;
}

static u32 fb_cvt_vbi_lines(struct fb_cvt_data *cvt)
{
	u32 vbi_lines, min_vbi_lines, act_vbi_lines;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK) {
		vbi_lines = (1000 * FB_CVT_RB_MIN_VBLANK)/cvt->hperiod + 1;
		min_vbi_lines =  FB_CVT_RB_V_FPORCH + cvt->vsync +
			FB_CVT_MIN_BPORCH;

	} else {
		vbi_lines = (FB_CVT_MIN_VSYNC_BP * 1000)/cvt->hperiod + 1 +
			 FB_CVT_MIN_VPORCH;
		min_vbi_lines = cvt->vsync + FB_CVT_MIN_BPORCH +
			FB_CVT_MIN_VPORCH;
	}

	if (vbi_lines < min_vbi_lines)
		act_vbi_lines = min_vbi_lines;
	else
		act_vbi_lines = vbi_lines;

	return act_vbi_lines;
}

static u32 fb_cvt_vtotal(struct fb_cvt_data *cvt)
{
	u32 vtotal = cvt->yres/cvt->interlace;

	vtotal += 2 * cvt->v_margin + cvt->interlace/2 + fb_cvt_vbi_lines(cvt);
	vtotal |= cvt->interlace/2;

	return vtotal;
}

static u32 fb_cvt_pixclock(struct fb_cvt_data *cvt)
{
	u32 pixclock;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK)
		pixclock = (cvt->f_refresh * cvt->vtotal * cvt->htotal)/1000;
	else
		pixclock = (cvt->htotal * 1000000)/cvt->hperiod;

	printf( "pixclock == %lu (refresh %u, vtot %u, htot %u\n", pixclock, cvt->f_refresh, cvt->vtotal, cvt->htotal );

	pixclock /= 250;
	pixclock *= 250;
	pixclock *= 1000;

	printf( " == %lu\n", pixclock );
	return pixclock;
}

static void fb_cvt_convert_to_mode(struct fb_cvt_data *cvt,
				   struct fb_videomode *mode)
{
	mode->refresh = cvt->f_refresh;
	mode->pixclock = cvt->pixclock ;
	mode->left_margin = cvt->h_back_porch;
	mode->right_margin = cvt->h_front_porch;
	mode->hsync_len = cvt->hsync;
	mode->upper_margin = cvt->v_back_porch;
	mode->lower_margin = cvt->v_front_porch;
	mode->vsync_len = cvt->vsync;

	mode->sync &= ~(FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT);

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK)
		mode->sync |= FB_SYNC_HOR_HIGH_ACT;
	else
		mode->sync |= FB_SYNC_VERT_HIGH_ACT;
}

/*
 * fb_find_mode_cvt - calculate mode using VESA(TM) CVT
 * @mode: pointer to fb_videomode; xres, yres, refresh and vmode must be
 *		pre-filled with the desired values
 * @margins: add margin to calculation (1.8% of xres and yres)
 * @rb: compute with reduced blanking (for flatpanels)
 *
 * RETURNS:
 * 0 for success
 * @mode is filled with computed values.  If interlaced, the refresh field
 * will be filled with the field rate (2x the frame rate)
 *
 * DESCRIPTION:
 * Computes video timings using VESA(TM) Coordinated Video Timings
 */
int fb_find_mode_cvt(struct fb_videomode *mode, int margins, int rb)
{
	struct fb_cvt_data cvt;

	memset(&cvt, 0, sizeof(cvt));

	if (margins)
		cvt.flags |= FB_CVT_FLAG_MARGINS;

	if (rb)
		cvt.flags |= FB_CVT_FLAG_REDUCED_BLANK;

	if (mode->vmode & FB_VMODE_INTERLACED)
		cvt.flags |= FB_CVT_FLAG_INTERLACED;

	cvt.xres = mode->xres;
	cvt.yres = mode->yres;
	cvt.refresh = mode->refresh;
	cvt.f_refresh = cvt.refresh;
	cvt.interlace = 1;

	if (!cvt.xres || !cvt.yres || !cvt.refresh) {
		printf( "fbcvt: Invalid input parameters\n");
		return 1;
	}

	if (!(cvt.refresh == 50 || cvt.refresh == 60 || cvt.refresh == 70 ||
		  cvt.refresh == 85)) {
		printf( "fbcvt: Refresh rate not CVT "
			   "standard\n");
		cvt.status = 1;
	}

	cvt.xres &= ~(FB_CVT_CELLSIZE - 1);

	if (cvt.flags & FB_CVT_FLAG_INTERLACED) {
		cvt.interlace = 2;
		cvt.f_refresh *= 2;
	}

	if (cvt.flags & FB_CVT_FLAG_REDUCED_BLANK) {
		if (cvt.refresh != 60) {
			printf( "fbcvt: 60Hz refresh rate "
				   "advised for reduced blanking\n");
			cvt.status = 1;
		}
	}

	if (cvt.flags & FB_CVT_FLAG_MARGINS) {
		cvt.h_margin = (cvt.xres * 18)/1000;
		cvt.h_margin &= ~(FB_CVT_CELLSIZE - 1);
		cvt.v_margin = ((cvt.yres/cvt.interlace)* 18)/1000;
	}

	cvt.aspect_ratio = fb_cvt_aspect_ratio(&cvt);
	cvt.active_pixels = cvt.xres + 2 * cvt.h_margin;
	cvt.hperiod = fb_cvt_hperiod(&cvt);
	cvt.vsync = fb_cvt_vbi_tab[cvt.aspect_ratio];
	cvt.vtotal = fb_cvt_vtotal(&cvt);
	cvt.hblank = fb_cvt_hblank(&cvt);
	cvt.htotal = cvt.active_pixels + cvt.hblank;
	cvt.hsync = fb_cvt_hsync(&cvt);
	cvt.pixclock = fb_cvt_pixclock(&cvt);
	cvt.hfreq = cvt.pixclock/cvt.htotal;
	cvt.h_back_porch = cvt.hblank/2 + cvt.h_margin;
	cvt.h_front_porch = cvt.hblank - cvt.hsync - cvt.h_back_porch +
		2 * cvt.h_margin;
	cvt.v_front_porch = 3 + cvt.v_margin;
	cvt.v_back_porch = cvt.vtotal - cvt.yres/cvt.interlace -
		cvt.v_front_porch - cvt.vsync;
	fb_cvt_convert_to_mode(&cvt, mode);

	return 0;
}
#ifdef CONFIG_CMD_I2C
static int read_edid_block(unsigned char * buf)
{
#if 1
	int ret;
	int failures = 0;
	do {
		ret = i2c_read(I2C_MONITOR_EDID, 0, 1, buf, 128);
		if (ret == 0) {
			/* validate checksum */
			unsigned char sum = 0;
			unsigned char* p = buf;
			int count;
			for (count=0; count<128; count++)
				sum += *p++;
			if (sum==0) {
				if (failures)
					printf("%s: success after %i failures\n", __func__, failures);
				return 0;
			}
		} else {
			/* read from invalid address expecting NACK, to cleanup bus */
			unsigned i;
			for (i=0; i<8; i++)
				i2c_read(0x42, 0, 0, buf, 1);
		}
		failures++;
	} while (failures <= 10);
	printf("%s: failed\n", __func__);
	return -1;
#else
//1680x1050
	unsigned char edt1680[] = {0x21, 0x39,
		0x90, 0x30, 0x62, 0x1a, 0x27, 0x40, 0x68, 0xb0,
		0x36, 0x00, 0xda, 0x28, 0x11, 0x00, 0x00, 0x1c};
	memcpy(&buf[EDID_DETAILED_TIMING_DESCRIPTIONS_START], edt1680,
		sizeof(struct edid_detailed_timings));
	buf[2] = 2;
	return 0;
#endif
}

static int fb_find_edid(struct lcd_panel_info_t *panel)
{
	unsigned char buf[128];
	struct edid_detailed_timings* edt;
	int ret = read_edid_block(buf);
	if (ret)
		return ret;
	if ((buf[EDID_FEATURE_REG] & 2) == 0)
		return -1;
	edt = (struct edid_detailed_timings *)
		(&buf[EDID_DETAILED_TIMING_DESCRIPTIONS_START]);
	panel->xres = edt_xres(edt);
	panel->yres = edt_yres(edt);
	panel->pixclock = edt_pixel_clock(edt);
	panel->left_margin = edt_leftmargin(edt);
	panel->right_margin = edt_rightmargin(edt);
	panel->hsync_len = edt_hsync_width(edt);
	panel->upper_margin = edt_uppermargin(edt);
	panel->lower_margin = edt_lowermargin(edt);
	panel->vsync_len = edt_vsync_width(edt);
	panel->hsyn_acth = (edt->flags & EDT_FLAGS_HSYNC_POLARITY) ? 1 : 0;
	panel->vsyn_acth = (edt->flags & EDT_FLAGS_VSYNC_POLARITY) ? 1 : 0;
	if (0) {
		unsigned int refresh = panel->pixclock /
			((panel->left_margin + panel->xres + panel->right_margin + panel->hsync_len) *
			(panel->upper_margin + panel->yres + panel->lower_margin + panel->vsync_len));
		printf("edid: %ux%u at %u Hz\n", panel->xres, panel->yres, refresh);
		printf("left=%u, right=%u, hsync=%u hacth=%u upper=%u lower=%u vsync=%u vacth=%u",
			panel->left_margin, panel->right_margin, panel->hsync_len, panel->hsyn_acth,
			panel->upper_margin, panel->lower_margin, panel->vsync_len, panel->vsyn_acth);
	}
	return 0;
}
#else
#define fb_find_edid(a) 1
#endif


#define UPCASE(c) ((c)&~0x20)
int parse_panel_info( char const *panelInfo, // input
		struct lcd_panel_info_t *panel )	// output
{
   memset( panel, 0, sizeof(*panel));
   char const *nameEnd=strchr(panelInfo,':');
   if (nameEnd == panelInfo+4) {
      panel->pclk_redg = 1;
      panel->oepol_actl = 0;
      panel->active = 1 ;
      panel->rotation = 0 ;
      if ('V' == UPCASE(*panelInfo)) {
	  // Use VESA GTF
	  char temp[40];
	  char *term ;
	  struct fb_videomode mode ;
	  memset( &mode, 0, sizeof(mode) );
	  strcpy(temp,nameEnd+1);
	  char *nextIn ;
	  nameEnd++ ;
	  term = strchr(temp, 'x');
	  if( !term )
		 goto bail ;
	  *term++ = 0 ;
	  mode.xres = simple_strtoul(temp,0,0);
	  nextIn = term ;
	  term = strchr(term,'@');
	  if( !term )
		 goto bail ;
	  *term++ = 0 ;
	  mode.yres = simple_strtoul(nextIn,0,0);
	  printf( "hz string == %s\n", term );
	  mode.refresh = simple_strtoul(term,&term,0);

	  printf( "VESA: %ux%u at %u Hz\n", mode.xres, mode.yres, mode.refresh );

	  if( 0 != fb_find_mode_cvt(&mode, 0, 0) ){
		 printf( "Error finding mode\n" );
		 goto bail ;
	  }
	  panel->name = "vesafb";	// don't call it VESA!
	  panel->xres = mode.xres ;
	  panel->yres = mode.yres ;
	  panel->pixclock = mode.pixclock ;
	  panel->left_margin = mode.left_margin ;
	  panel->right_margin = mode.right_margin ;
	  panel->upper_margin = mode.upper_margin ;
	  panel->lower_margin = mode.lower_margin ;
	  panel->hsync_len = mode.hsync_len ;
	  panel->vsync_len = mode.vsync_len ;
	  panel->hsyn_acth = (mode.sync & FB_SYNC_HOR_HIGH_ACT) ? 1 : 0;
	  panel->vsyn_acth = (mode.sync & FB_SYNC_VERT_HIGH_ACT) ? 1 : 0;
	  panel->crt = (term && (UPCASE(*term) == 'C')) ? 1 : 0;
	  return 1;
      } else if ('E' == UPCASE(*panelInfo)) {
	  nameEnd++;
	  panel->name = "edidfb";
	  panel->crt = (UPCASE(*nameEnd) == 'C') ? 1 : 0;
	  if (fb_find_edid(panel)) {
		 printf( "Error finding edid\n" );
		 goto bail;
	  }
	  return 1;
      }
   }
   if( nameEnd && *nameEnd ){
	  char const *nextIn = nameEnd+1 ;
	  unsigned const numValues = 15 ;
	  unsigned long values[numValues];
	  unsigned i ;
	  for( i = 0 ; i < numValues ; i++ ){
		 char *endptr ;
		 values[i] = simple_strtoul(nextIn, &endptr, 0 );
		 if((endptr != nextIn) && ( *endptr || (i == numValues-1))){
			nextIn = endptr+1 ;
		 }
		 else
			break ;
	  }
	  if( numValues == i ){
		 unsigned const nameLen = nameEnd-panelInfo;
		 panel->name = (char const *)malloc(nameLen+1);
		 memcpy( (char *)panel->name, panelInfo, nameLen );
		 ((char *)panel->name)[nameLen] = 0 ;
		 panel->pixclock = values[0];
		 panel->xres = values[1];
		 panel->yres = values[2];
		 panel->pclk_redg= values[3];
		 panel->hsyn_acth= values[4];
		 panel->vsyn_acth= values[5];
		 panel->oepol_actl= values[6];
		 panel->hsync_len = values[7];
		 panel->left_margin = values[8];
		 panel->right_margin = values[9];
		 panel->vsync_len = values[10];
		 panel->upper_margin = values[11];
		 panel->lower_margin = values[12];
		 panel->active = values[13];
		 panel->crt = values[14];
		 panel->rotation = 0 ;
		 return 1 ;
	  }
   }

bail:
//   0	  1	2 3 4 5 6 7 8 9 10 11 12 13 14
   printf( "Usage: myPanel:pixclock,xres,yres,...\n"
		   " | vesa:WxH@Hz[C]\n"
		   );
   return 0 ;
}

#ifdef CONFIG_LCD
struct lcd_panel_info_t const *cur_lcd_panel = 0 ;
#endif
