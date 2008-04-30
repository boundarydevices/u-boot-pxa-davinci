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

   /* char const       *name         */   { "hitachi_qvga"
   /* unsigned long     pixclock     */    , 0
   /* unsigned short    xres         */    , 320       
   /* unsigned short    yres         */    , 240
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    ,  1
   /* unsigned char     right_margin */    , 16       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 8       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }

   /* char const       *name         */ , { "sharp_qvga"
   /* unsigned long     pixclock     */    , 0          
   /* unsigned short    xres         */    , 320        /* , 320  */
   /* unsigned short    yres         */    , 240        /* , 240  */
   /* unsigned char     pclk_redg    */    , 1          /* , 1    */
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 20         /* , 8    */
   /* unsigned char     left_margin  */    , 1          /* , 16   */
   /* unsigned char     right_margin */    , 30         /* , 1    */
   /* unsigned char     vsync_len    */    , 4          /* , 20   */
   /* unsigned char     upper_margin */    , 17         /* , 17   */
   /* unsigned char     lower_margin */    , 3          /* , 3    */
   /* unsigned char     active       */    , 1          /* , 1    */
   /* unsigned char     crt          */    , 0 }

//#define OKAYA_QVGA_P   320, 30,37, 38,	240, 3, 16, 15,	1,1,0,0,0,	1,0,0,1,120,LCD_PANEL
   /* char const       *name         */ , { "okaya_qvga"
   /* unsigned long     pixclock     */    , 13974000	//(320+30+37+38)*(240+3+16+15)*120 Hz = 425*274*120
   /* unsigned short    xres         */    , 320
   /* unsigned short    yres         */    , 240
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 0
   /* unsigned char     vsyn_acth    */    , 0
   /* unsigned char     hsync_len    */    , 30 
   /* unsigned char     left_margin  */    , 37
   /* unsigned char     right_margin */    , 38 
   /* unsigned char     vsync_len    */    , 3
   /* unsigned char     upper_margin */    , 16 
   /* unsigned char     lower_margin */    , 15  
   /* unsigned char     active       */    , 1  
   /* unsigned char     crt          */    , 0 }

#define OKAYA_480X272_P 480, 3,20, 38,		272, 3, 5, 15,		1,1,0,0,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0
//name,pixclock,
	, { "okaya_480X272", 11394080	//(480+30+20+38)*(272+3+5+15)*68 Hz = 568*295*68 = 11,394,080
	, 480, 272, 1, 0, 0			//xres,yres,pclk_redg,hsyn_acth,vsyn_acth,
	,  30, 20, 38				//hsync_len,left_margin,right_margin,
	, 3, 5, 15					//vsync_len,upper_margin,lower_margin,
    , 1, 0  					//active,crt
	 }

   /* char const       *name         */ , { "qvga_portrait"
   /* unsigned long     pixclock     */    , 0
   /* unsigned short    xres         */    , 240
   /* unsigned short    yres         */    , 320
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    , 34
   /* unsigned char     right_margin */    , 1       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 8       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 
   /* unsigned          rotation     */    , 90 }

   /* char const       *name         */ , { "hitachi_hvga"
   /* unsigned long     pixclock     */    , (640+64+34+1)*(240+20+8+3)*68
   /* unsigned short    xres         */    , 640
   /* unsigned short    yres         */    , 240
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    , 34       
   /* unsigned char     right_margin */    , 1       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 8       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }

   /* char const       *name         */ , { "sharp_vga"
   /* unsigned long     pixclock     */    , 1      
   /* unsigned short    xres         */    , 640
   /* unsigned short    yres         */    , 480
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    , 60		//3       
   /* unsigned char     right_margin */    , 60       
   /* unsigned char     vsync_len    */    , 20		//34       
   /* unsigned char     upper_margin */    , 34		//24       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }
   
   /* char const       *name         */ , { "vga_crt"
   /* unsigned long     pixclock     */    , 1		//24000000      
   /* unsigned short    xres         */    , 640
   /* unsigned short    yres         */    , 480
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    , 60		//3       
   /* unsigned char     right_margin */    , 60       
   /* unsigned char     vsync_len    */    , 20		//34       
   /* unsigned char     upper_margin */    , 34		//24       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 1 }
   
   /* char const       *name         */ , { "hitachi_wvga"
   /* unsigned long     pixclock     */    , 1      
   /* unsigned short    xres         */    , 800
   /* unsigned short    yres         */    , 480
   /* unsigned char     pclk_redg    */    , 1
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64
   /* unsigned char     left_margin  */    , 1       
   /* unsigned char     right_margin */    , 39       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 8       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }

   /* char const       *name         */ , { "crt_800x600"
   /* unsigned long     pixclock     */    , 56000000
   /* unsigned short    xres         */    , 800
   /* unsigned short    yres         */    , 600
   /* unsigned char     pclk_redg    */    , 1 
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64
   /* unsigned char     left_margin  */    , 32       
   /* unsigned char     right_margin */    , 152       
   /* unsigned char     vsync_len    */    , 3       
   /* unsigned char     upper_margin */    , 1       
   /* unsigned char     lower_margin */    , 27       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 1 }

   /* char const       *name         */ , { "gvision_10.4"
   /* unsigned long     pixclock     */    , 56000000
   /* unsigned short    xres         */    , 800
   /* unsigned short    yres         */    , 600
   /* unsigned char     pclk_redg    */    , 1 
   /* unsigned char     hsyn_acth    */    , 1
   /* unsigned char     vsyn_acth    */    , 1
   /* unsigned char     hsync_len    */    , 64
   /* unsigned char     left_margin  */    , 32       
   /* unsigned char     right_margin */    , 16       
   /* unsigned char     vsync_len    */    , 8
   /* unsigned char     upper_margin */    , 3       
   /* unsigned char     lower_margin */    , 2       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 1 }
, { "lcd_svga", 56000000, 800, 600, 1 , 1, 1,
		64, 32, 152,
		3, 1, 27,
		1, 0 }
   
// Note that you can use the nifty tool at the 
// following location to generate these values:
//    http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
, {
    name: "crt1024x768",
    pixclock: 65000000,
    xres: 1024,
    yres: 768,
    pclk_redg: 0,
    hsyn_acth: 0,
    vsyn_acth: 0,
    hsync_len: 136,
    left_margin: 24,
    right_margin: 160,
    vsync_len: 6,
    upper_margin: 3,
    lower_margin: 29,
    active : 0,
    crt : 1
}
, {
    name: "hitachi_wxga",
    pixclock: 1,
    xres: 1024,
    yres: 768,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 64,
    left_margin: 1,
    right_margin: 39,
    vsync_len: 20,
    upper_margin: 8,
    lower_margin: 3,
    active : 1,
    crt : 0
}
, {
#define hitachi_154_P 1280,64, 24,16,	800, 20, 4, 3,	1,1,0,0,0,	1,0,0,1,62,LCD_PANEL
    name: "hitachi_154",
    pixclock: 72000000,
    xres: 1280,
    yres: 800,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 64,
    left_margin: 24,
    right_margin: 16,
    vsync_len: 20,
    upper_margin: 8,
    lower_margin: 3,
    active : 1,
    crt : 0
}
, {
    name: "olevia",
    pixclock: 65000000,
    xres: 1024,
    yres: 768,
    pclk_redg: 0,
    hsyn_acth: 0,
    vsyn_acth: 0,
    hsync_len: 200,
    left_margin: 37,
    right_margin: 228,
    vsync_len: 6,
    upper_margin: 3,
    lower_margin: 29,
    active : 0,
    crt : 1
}
, {
    name: "crt_sxga60hz",
    pixclock: 108000000,
    xres: 1280,
    yres: 1024,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 120,
    left_margin: 64,
    right_margin: 264,
    vsync_len: 4,
    upper_margin: 2,
    lower_margin: 44,
    active : 1,
    crt : 1
}
, {
    name: "lcd_sxga60hz",
    pixclock: 108000000,
    xres: 1280,
    yres: 1024,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 120,
    left_margin: 64,
    right_margin: 264,
    vsync_len: 4,
    upper_margin: 2,
    lower_margin: 44,
    active : 1,
    crt : 0
}
, {
    name: "hitachi_92",
    pixclock: 20000000,
    xres: 960,
    yres: 160,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 15,
    left_margin: 220,
    right_margin: 1,
    vsync_len: 200,
    upper_margin: 148,
    lower_margin: 3,
    active : 1,
    crt : 0
}
, { name: "sharp_480x272", 
    pixclock: 9000000,
    xres: 480,
    yres: 272,
    pclk_redg: 1,
    hsyn_acth: 0,
    vsyn_acth: 0,
    hsync_len: 41,
    left_margin: 2,
    right_margin: 2,
    vsync_len: 10,
    upper_margin: 2,
    lower_margin: 2,
    active : 1,
    crt : 0 }
#if 1
#define tovis_w_P	1024,104,56,160,	200,3,201,11,	0,0,1,0,0,	1,0,0,1,75,CRT
, {
    name: "tovis_w",
    pixclock: (1024+104+56+160)*(200+3+201+11)*75,
    xres: 1024,
    yres: 200,
    pclk_redg: 1,
    hsyn_acth: 0,
    vsyn_acth: 0,
    hsync_len: 104,
    left_margin: 56,
    right_margin: 160,
    vsync_len: 3,
    upper_margin: 201,
    lower_margin: 11,
    active : 1,
    crt : 1
}
#else
#define tovis_w_P	1024,64,1,39,		204,20,8,3,		1,1,1,0,0,	1,0,0,1,75,LCD_PANEL
, {
    name: "tovis_w",
    pixclock: (1024+64+1+39)*(204+20+8+3)*75,
    xres: 1024,
    yres: 204,
    pclk_redg: 1,
    hsyn_acth: 1,
    vsyn_acth: 1,
    hsync_len: 64,
    left_margin: 1,
    right_margin: 39,
    vsync_len: 20,
    upper_margin: 8,
    lower_margin: 3,
    active : 1,
    crt : 0
}
#endif
, {
    name: "samsung1600x1050",
    pixclock: 142000000,
    xres: 1600,
    yres: 1050,
    pclk_redg: 1,
    hsyn_acth: 0,
    vsyn_acth: 1,
    hsync_len: 104,
    left_margin: 128,
    right_margin: 264,
    vsync_len: 4,
    upper_margin: 2,
    lower_margin: 44,
    active : 1,
    crt : 0
}
, {
    name: "adap216x24",
    pixclock: 25125000,
    xres: 640,
    yres: 480,
    pclk_redg: 0,
    hsyn_acth: 0,
    vsyn_acth: 0,
    hsync_len: 95,
    left_margin: 16,
    right_margin: 49,
    vsync_len: 1,
    upper_margin: 2,
    lower_margin: 42,
    active : 1,
    crt : 0
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
 htotal dend hsstrt hsw  hpolar    vtot vdend vdstrt vsh vpolar    pixclk    hfreq vfreq
{ 1344, 1024, 1048, 136, NEGATIVE, 806, 768,    771,  6, NEGATIVE, 65000000, 48363, 60 },
{ 1328, 1024, 1048, 136, NEGATIVE, 806, 768,    771,  6, NEGATIVE, 75000000, 56476, 70 },
{ 1312, 1024, 1040,  96, POSITIVE, 800, 768,    769,  3, POSITIVE, 78750000, 60023, 75 },
{ 1376, 1024, 1072,  96, POSITIVE, 808, 768,    769,  3, POSITIVE, 94500000, 68677, 85 },

0FE80200/00010000 +           CRT regs
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

#define MARGIN_PERCENT    1.8   /* % of active vertical image                */
#define CELL_GRAN         8.0   /* assumed character cell granularity        */
#define MIN_PORCH         1     /* minimum front porch                       */
#define V_SYNC_RQD        3     /* width of vsync in lines                   */
#define H_SYNC_PERCENT    8.0   /* width of hsync as % of total line         */
#define MIN_VSYNC_PLUS_BP 550.0 /* min time of vsync + back porch (microsec) */
#define M                 600.0 /* blanking formula gradient                 */
#define C                 40.0  /* blanking formula offset                   */
#define K                 128.0 /* blanking formula scaling factor           */
#define J                 20.0  /* blanking formula scaling factor           */

/* C' and M' are part of the Blanking Duty Cycle computation */

#define C_PRIME           (((C - J) * K/256.0) + J)
#define M_PRIME           (K/256.0 * M)

typedef struct __mode
{
    int hr, hss, hse, hfl;
    int vr, vss, vse, vfl;
    float pclk, h_freq;
    int v_freq;
} mode;

#define rint(__f) ((int)(__f))
inline void print_value(int n, char *name, float val){}

/*
 * vert_refresh() - as defined by the GTF Timing Standard, compute the
 * Stage 1 Parameters using the vertical refresh frequency.  In other
 * words: input a desired resolution and desired refresh rate, and
 * output the GTF mode timings.
 *
 * XXX All the code is in place to compute interlaced modes, but I don't
 * feel like testing it right now.
 *
 * XXX margin computations are implemented but not tested (nor used by
 * XFree86 of fbset mode descriptions, from what I can tell).
 */

static void vert_refresh( mode *m )
{
    float h_pixels_rnd;
    float v_lines_rnd;
    float v_field_rate_rqd;
    float top_margin;
    float bottom_margin;
    float interlace;
    float h_period_est;
    float vsync_plus_bp;
    float v_back_porch;
    float total_v_lines;
    float v_field_rate_est;
    float h_period;
    float v_field_rate;
    float v_frame_rate;
    float left_margin;
    float right_margin;
    float total_active_pixels;
    float ideal_duty_cycle;
    float h_blank;
    float total_pixels;
    float pixel_freq;
    float h_freq;

    float h_sync;
    float h_front_porch;
    float v_odd_front_porch_lines;
   
    /*  1. In order to give correct results, the number of horizontal
     *  pixels requested is first processed to ensure that it is divisible
     *  by the character size, by rounding it to the nearest character
     *  cell boundary:
     *
     *  [H PIXELS RND] = ((ROUND([H PIXELS]/[CELL GRAN RND],0))*[CELLGRAN RND])
     */
    
    h_pixels_rnd = rint((float) m->hr / CELL_GRAN) * CELL_GRAN;
    
    print_value(1, "[H PIXELS RND]", h_pixels_rnd);

    
    /*  2. If interlace is requested, the number of vertical lines assumed
     *  by the calculation must be halved, as the computation calculates
     *  the number of vertical lines per field. In either case, the
     *  number of lines is rounded to the nearest integer.
     *   
     *  [V LINES RND] = IF([INT RQD?]="y", ROUND([V LINES]/2,0),
     *                                     ROUND([V LINES],0))
     */

    v_lines_rnd = rint((float) m->vr);
    
    print_value(2, "[V LINES RND]", v_lines_rnd);
    
    
    /*  3. Find the frame rate required:
     *
     *  [V FIELD RATE RQD] = IF([INT RQD?]="y", [I/P FREQ RQD]*2,
     *                                          [I/P FREQ RQD])
     */

    v_field_rate_rqd = ((float)m->v_freq);

    print_value(3, "[V FIELD RATE RQD]", v_field_rate_rqd);
    

    /*  4. Find number of lines in Top margin:
     *
     *  [TOP MARGIN (LINES)] = IF([MARGINS RQD?]="Y",
     *          ROUND(([MARGIN%]/100*[V LINES RND]),0),
     *          0)
     */

    top_margin = (0.0);

    print_value(4, "[TOP MARGIN (LINES)]", top_margin);
    

    /*  5. Find number of lines in Bottom margin:
     *
     *  [BOT MARGIN (LINES)] = IF([MARGINS RQD?]="Y",
     *          ROUND(([MARGIN%]/100*[V LINES RND]),0),
     *          0)
     */

    bottom_margin = (0.0);

    print_value(5, "[BOT MARGIN (LINES)]", bottom_margin);

    
    /*  6. If interlace is required, then set variable [INTERLACE]=0.5:
     *   
     *  [INTERLACE]=(IF([INT RQD?]="y",0.5,0))
     */

    interlace = 0.0;

    print_value(6, "[INTERLACE]", interlace);
    

    /*  7. Estimate the Horizontal period
     *
     *  [H PERIOD EST] = ((1/[V FIELD RATE RQD]) - [MIN VSYNC+BP]/1000000) /
     *                    ([V LINES RND] + (2*[TOP MARGIN (LINES)]) +
     *                     [MIN PORCH RND]+[INTERLACE]) * 1000000
     */

    h_period_est = (((1.0/v_field_rate_rqd) - (MIN_VSYNC_PLUS_BP/1000000.0))
                    / (v_lines_rnd + (2*top_margin) + MIN_PORCH + interlace)
                    * 1000000.0);

    print_value(7, "[H PERIOD EST]", h_period_est);
    

    /*  8. Find the number of lines in V sync + back porch:
     *
     *  [V SYNC+BP] = ROUND(([MIN VSYNC+BP]/[H PERIOD EST]),0)
     */

    vsync_plus_bp = rint(MIN_VSYNC_PLUS_BP/h_period_est);

    print_value(8, "[V SYNC+BP]", vsync_plus_bp);
    
    
    /*  9. Find the number of lines in V back porch alone:
     *
     *  [V BACK PORCH] = [V SYNC+BP] - [V SYNC RND]
     *
     *  XXX is "[V SYNC RND]" a typo? should be [V SYNC RQD]?
     */
    
    v_back_porch = vsync_plus_bp - V_SYNC_RQD;
    
    print_value(9, "[V BACK PORCH]", v_back_porch);
    

    /*  10. Find the total number of lines in Vertical field period:
     *
     *  [TOTAL V LINES] = [V LINES RND] + [TOP MARGIN (LINES)] +
     *                    [BOT MARGIN (LINES)] + [V SYNC+BP] + [INTERLACE] +
     *                    [MIN PORCH RND]
     */

    total_v_lines = v_lines_rnd + top_margin + bottom_margin + vsync_plus_bp +
        interlace + MIN_PORCH;
    
    print_value(10, "[TOTAL V LINES]", total_v_lines);
    

    /*  11. Estimate the Vertical field frequency:
     *
     *  [V FIELD RATE EST] = 1 / [H PERIOD EST] / [TOTAL V LINES] * 1000000
     */

    v_field_rate_est = 1.0 / h_period_est / total_v_lines * 1000000.0;
    
    print_value(11, "[V FIELD RATE EST]", v_field_rate_est);
    

    /*  12. Find the actual horizontal period:
     *
     *  [H PERIOD] = [H PERIOD EST] / ([V FIELD RATE RQD] / [V FIELD RATE EST])
     */

    h_period = h_period_est / (v_field_rate_rqd / v_field_rate_est);
    
    print_value(12, "[H PERIOD]", h_period);
    

    /*  13. Find the actual Vertical field frequency:
     *
     *  [V FIELD RATE] = 1 / [H PERIOD] / [TOTAL V LINES] * 1000000
     */

    v_field_rate = 1.0 / h_period / total_v_lines * 1000000.0;

    print_value(13, "[V FIELD RATE]", v_field_rate);
    

    /*  14. Find the Vertical frame frequency:
     *
     *  [V FRAME RATE] = (IF([INT RQD?]="y", [V FIELD RATE]/2, [V FIELD RATE]))
     */

    v_frame_rate = v_field_rate;

    print_value(14, "[V FRAME RATE]", v_frame_rate);
    

    /*  15. Find number of pixels in left margin:
     *
     *  [LEFT MARGIN (PIXELS)] = (IF( [MARGINS RQD?]="Y",
     *          (ROUND( ([H PIXELS RND] * [MARGIN%] / 100 /
     *                   [CELL GRAN RND]),0)) * [CELL GRAN RND],
     *          0))
     */

    left_margin = 0.0;
    
    print_value(15, "[LEFT MARGIN (PIXELS)]", left_margin);
    

    /*  16. Find number of pixels in right margin:
     *
     *  [RIGHT MARGIN (PIXELS)] = (IF( [MARGINS RQD?]="Y",
     *          (ROUND( ([H PIXELS RND] * [MARGIN%] / 100 /
     *                   [CELL GRAN RND]),0)) * [CELL GRAN RND],
     *          0))
     */
    
    right_margin = 0.0;
    
    print_value(16, "[RIGHT MARGIN (PIXELS)]", right_margin);
    

    /*  17. Find total number of active pixels in image and left and right
     *  margins:
     *
     *  [TOTAL ACTIVE PIXELS] = [H PIXELS RND] + [LEFT MARGIN (PIXELS)] +
     *                          [RIGHT MARGIN (PIXELS)]
     */

    total_active_pixels = h_pixels_rnd + left_margin + right_margin;
    
    print_value(17, "[TOTAL ACTIVE PIXELS]", total_active_pixels);
    
    
    /*  18. Find the ideal blanking duty cycle from the blanking duty cycle
     *  equation:
     *
     *  [IDEAL DUTY CYCLE] = [C'] - ([M']*[H PERIOD]/1000)
     */

    ideal_duty_cycle = C_PRIME - (M_PRIME * h_period / 1000.0);
    
    print_value(18, "[IDEAL DUTY CYCLE]", ideal_duty_cycle);
    

    /*  19. Find the number of pixels in the blanking time to the nearest
     *  double character cell:
     *
     *  [H BLANK (PIXELS)] = (ROUND(([TOTAL ACTIVE PIXELS] *
     *                               [IDEAL DUTY CYCLE] /
     *                               (100-[IDEAL DUTY CYCLE]) /
     *                               (2*[CELL GRAN RND])), 0))
     *                       * (2*[CELL GRAN RND])
     */

    h_blank = rint(total_active_pixels *
                   ideal_duty_cycle /
                   (100.0 - ideal_duty_cycle) /
                   (2.0 * CELL_GRAN)) * (2.0 * CELL_GRAN);
    
    print_value(19, "[H BLANK (PIXELS)]", h_blank);
    

    /*  20. Find total number of pixels:
     *
     *  [TOTAL PIXELS] = [TOTAL ACTIVE PIXELS] + [H BLANK (PIXELS)]
     */

    total_pixels = total_active_pixels + h_blank;
    
    print_value(20, "[TOTAL PIXELS]", total_pixels);
    

    /*  21. Find pixel clock frequency:
     *
     *  [PIXEL FREQ] = [TOTAL PIXELS] / [H PERIOD]
     */
    
    pixel_freq = total_pixels / h_period;
    
    print_value(21, "[PIXEL FREQ]", pixel_freq);
    

    /*  22. Find horizontal frequency:
     *
     *  [H FREQ] = 1000 / [H PERIOD]
     */

    h_freq = 1000.0 / h_period;
    
    print_value(22, "[H FREQ]", h_freq);
    


    /* Stage 1 computations are now complete; I should really pass
       the results to another function and do the Stage 2
       computations, but I only need a few more values so I'll just
       append the computations here for now */

    

    /*  17. Find the number of pixels in the horizontal sync period:
     *
     *  [H SYNC (PIXELS)] =(ROUND(([H SYNC%] / 100 * [TOTAL PIXELS] /
     *                             [CELL GRAN RND]),0))*[CELL GRAN RND]
     */

    h_sync = rint(H_SYNC_PERCENT/100.0 * total_pixels / CELL_GRAN) * CELL_GRAN;

    print_value(17, "[H SYNC (PIXELS)]", h_sync);
    

    /*  18. Find the number of pixels in the horizontal front porch period:
     *
     *  [H FRONT PORCH (PIXELS)] = ([H BLANK (PIXELS)]/2)-[H SYNC (PIXELS)]
     */

    h_front_porch = (h_blank / 2.0) - h_sync;

    print_value(18, "[H FRONT PORCH (PIXELS)]", h_front_porch);
    
    
    /*  36. Find the number of lines in the odd front porch period:
     *
     *  [V ODD FRONT PORCH(LINES)]=([MIN PORCH RND]+[INTERLACE])
     */
    
    v_odd_front_porch_lines = MIN_PORCH + interlace;
    
    print_value(36, "[V ODD FRONT PORCH(LINES)]", v_odd_front_porch_lines);
    

    /* finally, pack the results in the mode struct */
    
    m->hss = (int) (h_pixels_rnd + h_front_porch);
    m->hse = (int) (h_pixels_rnd + h_front_porch + h_sync);
    m->hfl = (int) (total_pixels);

    m->vss = (int) (v_lines_rnd + v_odd_front_porch_lines);
    m->vse = (int) (int) (v_lines_rnd + v_odd_front_porch_lines + V_SYNC_RQD);
    m->vfl = (int) (total_v_lines);

    m->pclk   = pixel_freq;
    m->h_freq = h_freq;

} // vert_refresh()

#define UPCASE(c) ((c)&~0x20)

int parse_panel_info( char const              *panelInfo, // input
                      struct lcd_panel_info_t *panel )    // output
{
   memset( panel, 0, sizeof(*panel));
   char const *nameEnd=strchr(panelInfo,':');
   if( ('V' == UPCASE(*panelInfo))
       &&
       (nameEnd == panelInfo+4) ){
      // Use VESA GTF
      char temp[40];
      char *term ;
      strcpy(temp,nameEnd+1);
      char *nextIn ;
      mode m ;
      nameEnd++ ;
      term = strchr(temp, 'x');
      if( !term )
         goto bail ;
      *term++ = 0 ;
      m.hr = simple_strtoul(temp,0,0);
      nextIn = term ;
      term = strchr(term,'@');
      if( !term )
         goto bail ;
      *term++ = 0 ;
      m.vr = simple_strtoul(nextIn,0,0);
      printf( "hz string == %s\n", term );
      m.v_freq = simple_strtoul(term,&term,0);
      
      printf( "VESA: %ux%u at %u Hz\n", m.hr, m.vr, m.v_freq );
      vert_refresh(&m);
      
      panel->name = "vesafb" ; // don't call it VESA!
      panel->pixclock = (unsigned)(m.pclk*1000000);
      panel->xres = m.hr ;
      panel->yres = m.vr ;
      panel->pclk_redg= 1;
      panel->hsyn_acth= 0;
      panel->vsyn_acth= 1;
      panel->hsync_len = m.hse - m.hss;
      panel->left_margin = m.hss - m.hr;
      panel->right_margin = m.hfl - m.hse ;
      panel->vsync_len = m.vse - m.vss;
      panel->upper_margin = m.vss - m.vr;
      panel->lower_margin = m.vfl - m.vse;
      panel->active = 1 ;
      panel->crt = term && ('C' == UPCASE(*term));
      panel->rotation = 0 ;
      return 1 ;
   } else if( nameEnd && *nameEnd ){
      char const *nextIn = nameEnd+1 ;
      unsigned const numValues = 14 ;
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
         panel->hsync_len = values[6];
         panel->left_margin = values[7];
         panel->right_margin = values[8];
         panel->vsync_len = values[9];
         panel->upper_margin = values[10];
         panel->lower_margin = values[11];
         panel->active = values[12];
         panel->crt = values[13];
         panel->rotation = 0 ;
         return 1 ;
      }
   } 
bail:
   
                        //   0      1    2     3         4         5         6           7           8           9         10           11        12    13 
   printf( "Usage: myPanel:pixclock,xres,yres,...\n" 
           " | vesa:WxH@Hz[C]\n" 
           );
   return 0 ;
}

#ifdef CONFIG_LCD
struct lcd_panel_info_t const *cur_lcd_panel = 0 ;
#endif
