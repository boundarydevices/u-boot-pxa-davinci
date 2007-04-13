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

   /* char const       *name         */ , { "okaya_qvga"
   /* unsigned long     pixclock     */    , 7400000	//(320+30+20+38)*(240+3+5+15)*68 Hz = 408*263*60 = 7296672
   /* unsigned short    xres         */    , 320
   /* unsigned short    yres         */    , 240
   /* unsigned char     pclk_redg    */    , 0
   /* unsigned char     hsyn_acth    */    , 0
   /* unsigned char     vsyn_acth    */    , 0
   /* unsigned char     hsync_len    */    , 30 
   /* unsigned char     left_margin  */    , 20  
   /* unsigned char     right_margin */    , 38 
   /* unsigned char     vsync_len    */    , 3
   /* unsigned char     upper_margin */    , 5 
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
    name: "hitachi_154",
    pixclock: 72000000,
    xres: 1280,
    yres: 800,
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
    name: "sxga60hz",
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

int parse_panel_info( char const              *panelInfo, // input
                      struct lcd_panel_info_t *panel )    // output
{
   memset( panel, 0, sizeof(*panel));
   char const *nameEnd=strchr(panelInfo,':');
   if( nameEnd && *nameEnd ){
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
   
                        //   0      1    2     3         4         5         6           7           8           9         10           11        12    13 
   printf( "Usage: myPanel:freqHz,xres,yres,pclk_redg,hsyn_acth,vsyn_acth,hsync_len,left_margin,right_margin,vsync_len,top_margin,bottom_margin,active,crt\n" );
   return 0 ;
}

struct lcd_panel_info_t const *cur_lcd_panel = 0 ;
