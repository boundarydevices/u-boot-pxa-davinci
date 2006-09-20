/*
 * Module lcd_panels.cpp
 *
 * This module defines the num_lcd_panels and lcd_panels
 * constants as declared in lcd_panels.h
 *
 * Change History : 
 *
 * $Log: lcd_panels.c,v $
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
   /* unsigned char     act_high     */    , 1
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
   /* unsigned char     act_high     */    , 1          /* , 1    */
   /* unsigned char     hsync_len    */    , 20         /* , 8    */
   /* unsigned char     left_margin  */    , 1          /* , 16   */
   /* unsigned char     right_margin */    , 30         /* , 1    */
   /* unsigned char     vsync_len    */    , 4          /* , 20   */
   /* unsigned char     upper_margin */    , 17         /* , 17   */
   /* unsigned char     lower_margin */    , 3          /* , 3    */
   /* unsigned char     active       */    , 1          /* , 1    */
   /* unsigned char     crt          */    , 0 }

   /* char const       *name         */ , { "okaya_qvga"
   /* unsigned long     pixclock     */    , 6400000          
   /* unsigned short    xres         */    , 320
   /* unsigned short    yres         */    , 240
   /* unsigned char     act_high     */    , 1  
   /* unsigned char     hsync_len    */    , 30 
   /* unsigned char     left_margin  */    , 20  
   /* unsigned char     right_margin */    , 38 
   /* unsigned char     vsync_len    */    , 3
   /* unsigned char     upper_margin */    , 5 
   /* unsigned char     lower_margin */    , 15  
   /* unsigned char     active       */    , 1  
   /* unsigned char     crt          */    , 0 }

   /* char const       *name         */ , { "qvga_portrait"
   /* unsigned long     pixclock     */    , 0
   /* unsigned short    xres         */    , 240
   /* unsigned short    yres         */    , 320
   /* unsigned char     act_high     */    , 1
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
   /* unsigned long     pixclock     */    , 1      
   /* unsigned short    xres         */    , 640
   /* unsigned short    yres         */    , 240
   /* unsigned char     act_high     */    , 1
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
   /* unsigned char     act_high     */    , 1
   /* unsigned char     hsync_len    */    , 64       
   /* unsigned char     left_margin  */    , 60       
   /* unsigned char     right_margin */    , 60       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 34       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }
   
   /* char const       *name         */ , { "hitachi_wvga"
   /* unsigned long     pixclock     */    , 1      
   /* unsigned short    xres         */    , 800
   /* unsigned short    yres         */    , 480
   /* unsigned char     act_high     */    , 1 
   /* unsigned char     hsync_len    */    , 64
   /* unsigned char     left_margin  */    , 1       
   /* unsigned char     right_margin */    , 39       
   /* unsigned char     vsync_len    */    , 20       
   /* unsigned char     upper_margin */    , 8       
   /* unsigned char     lower_margin */    , 3       
   /* unsigned char     active       */    , 1
   /* unsigned char     crt          */    , 0 }
// Note that you can use the nifty tool at the 
// following location to generate these values:
//    http://www.tkk.fi/Misc/Electronics/faq/vga2rgb/calc.html
, {
    name: "crt1024x768",
    pixclock: 65000000,
    xres: 1024,
    yres: 768,
    act_high : 0,
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
    name: "hitachi_154",
    pixclock: 72000000,
    xres: 1280,
    yres: 800,
    act_high : 1,
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
    name: "lcd_sxga",
    pixclock: 65000000,
    xres: 1024,
    yres: 768,
    act_high : 1,
    hsync_len: 64,
    left_margin: 1,
    right_margin: 39,
    vsync_len: 20,
    upper_margin: 8,
    lower_margin: 3,
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

struct lcd_panel_info_t const *cur_lcd_panel = 0 ;
