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
#include <div64.h>
#ifdef CONFIG_CMD_I2C
#include <i2c.h>
#include "edid.h"
#endif
/*
Settings for Hitachi 5.7
		PANEL_HORIZONTAL_TOTAL, 01c00160);    // should be 34+320+1+64-1= 418 = 0x1A2 (Hex)
                                               + 0x13f+16
		PANEL_HORIZONTAL_SYNC, 00400161);            hsync_len == 64    right_margin=0x161-0x13f=34
		PANEL_VERTICAL_TOTAL, 0x010800f0);
		PANEL_VERTICAL_SYNC, 0x00020104);     vsync=2,  lower_margin=0x0104-0xf0-1= 19

In bdlogo.bmp - offset 436 is pixel data

Sharp 5.7 active

   STUFFREG( hTotalReg,      0x01800140 );  // should be 16+320+1+8-1 == 0x158
                                             + 0x13f (width-1)
   STUFFREG( hSyncReg,       0x0008014f );         hsync_len == 8    right_margin=0x14f-0x13f=16
   STUFFREG( vTotalReg,      0x010700F0 );
   STUFFREG( vSyncReg,       0x00020100 );     vsync=2,  lower_margin=0x0100-0xf0+1= 17

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
	left_margin:  2,
	right_margin: 15,
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
	left_margin: 2,		/* 16   */
	right_margin: 29,	/* 1    */
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
	left_margin: 38,
	right_margin: 37,
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
	left_margin: 2,
	right_margin: 1,
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
	left_margin: 43,
	right_margin: 41,
	vsync_len: 10,
	upper_margin: 1,
	lower_margin: 6,
	active: 1,
	crt:0
}, {
	name: "urt_800x480",
	pixclock: 24000000,
	xres: 800,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 72,
	left_margin: 24,
	right_margin: 96,
	vsync_len: 7,
	upper_margin: 3,
	lower_margin: 10,
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
	pixclock: 14000000,
	xres: 640,		//739=640+64+34+1
	yres: 240,		//271=240+20+8+3
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
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
	name: "dmt640x350_85",
	pixclock: 31500000,
	xres: 640,		//832 = 640+64+96+32
	yres: 350,		//445 = 350+3+60+32
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 96,
	right_margin: 32,
	vsync_len: 3,
	upper_margin: 60,
	lower_margin: 32,
	active: 1,
	crt: 1
}, {
	name: "dmt640x400_85",
	pixclock: 31500000,
	xres: 640,		//832 = 640+64+96+32
	yres: 400,		//445 = 400+3+41+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 96,
	right_margin: 32,
	vsync_len: 3,
	upper_margin: 41,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt720x400_85",
	pixclock: 35500000,
	xres: 720,		//936 = 720+72+108+36
	yres: 400,		//446 = 400+3+42+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 72,
	left_margin: 108,
	right_margin: 36,
	vsync_len: 3,
	upper_margin: 42,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt640x480_60",
	pixclock: 25175000,
	xres: 640,		//800 = 640+96+48+16
	yres: 480,		//525 = 480+2+33+10
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 96,
	left_margin: 48,
	right_margin: 16,
	vsync_len: 2,
	upper_margin: 33,
	lower_margin: 10,
	active: 1,
	crt: 1
}, {
	name: "dmt640x480_72",
	pixclock: 31500000,
	xres: 640,		//832 = 640+40+128+24
	yres: 480,		//520 = 480+3+28+9
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 40,
	left_margin: 128,
	right_margin: 24,
	vsync_len: 3,
	upper_margin: 28,
	lower_margin: 9,
	active: 1,
	crt: 1
}, {
	name: "dmt640x480_75",
	pixclock: 31500000,
	xres: 640,		//840 = 640+64+120+16
	yres: 480,		//500 = 480+3+16+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 120,
	right_margin: 16,
	vsync_len: 3,
	upper_margin: 16,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt640x480_85",
	pixclock: 36000000,
	xres: 640,		//832 = 640+56+80+56
	yres: 480,		//509 = 480+3+25+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 56,
	left_margin: 80,
	right_margin: 56,
	vsync_len: 3,
	upper_margin: 25,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt800x600_56",
	pixclock: 36000000,
	xres: 800,		//1024 = 800+72+128+24
	yres: 600,		//625 = 600+2+22+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 72,
	left_margin: 128,
	right_margin: 24,
	vsync_len: 2,
	upper_margin: 22,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt800x600_60",
	pixclock: 40000000,
	xres: 800,		//1056 = 800+128+88+40
	yres: 600,		//628 = 600+4+23+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 128,
	left_margin: 88,
	right_margin: 40,
	vsync_len: 4,
	upper_margin: 23,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt800x600_72",
	pixclock: 50000000,
	xres: 800,		//1040 = 800+120+64+56
	yres: 600,		//666 = 600+6+23+37
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 120,
	left_margin: 64,
	right_margin: 56,
	vsync_len: 6,
	upper_margin: 23,
	lower_margin: 37,
	active: 1,
	crt: 1
}, {
	name: "dmt800x600_75",
	pixclock: 49500000,
	xres: 800,		//1056 = 800+80+160+16
	yres: 600,		//625 = 600+3+21+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 80,
	left_margin: 160,
	right_margin: 16,
	vsync_len: 3,
	upper_margin: 21,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt800x600_85",
	pixclock: 56250000,
	xres: 800,		//1048 = 800+64+152+32
	yres: 600,		//631 = 600+3+27+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 64,
	left_margin: 152,
	right_margin: 32,
	vsync_len: 3,
	upper_margin: 27,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt848x480_60",
	pixclock: 33750000,
	xres: 848,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 112,
	left_margin: 112,
	right_margin: 16,
	vsync_len: 8,
	upper_margin: 23,
	lower_margin: 6,
	active: 1,
	crt: 1
}, {
	name: "dmt1024x768_60",
	pixclock: 65000000,
	xres: 1024,		//1344 = 1024+136+160+24
	yres: 768,		//806 = 768+6+29+3
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 136,
	left_margin: 160,
	right_margin: 24,
	vsync_len: 6,
	upper_margin: 29,
	lower_margin: 3,
	active: 1,
	crt: 1
}, {
	name: "dmt1024x768_70",
	pixclock: 75000000,
	xres: 1024,		//1328 = 1024+136+144+24
	yres: 768,		//806 = 768+6+29+3
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 136,
	left_margin: 144,
	right_margin: 24,
	vsync_len: 6,
	upper_margin: 29,
	lower_margin: 3,
	active: 1,
	crt: 1
}, {
	name: "dmt1024x768_75",
	pixclock: 78750000,
	xres: 1024,		//1312 = 1024+96+176+16
	yres: 768,		//800 = 768+3+28+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 96,
	left_margin: 176,
	right_margin: 16,
	vsync_len: 3,
	upper_margin: 28,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1024x768_85",
	pixclock: 94500000,
	xres: 1024,		//1376 = 1024+96+208+48
	yres: 768,		//808 = 768+3+36+1
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 96,
	left_margin: 208,
	right_margin: 48,
	vsync_len: 3,
	upper_margin: 36,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1152x864_75",
	pixclock: 108000000,
	xres: 1152,
	yres: 864,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 128,
	left_margin: 256,
	right_margin: 64,
	vsync_len: 3,
	upper_margin: 32,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
//#define INCLUDE_GTF_SETTINGS
#ifdef INCLUDE_GTF_SETTINGS
//This is a GTF formula
	name: "gtf1152x864_60",
	pixclock: 1520*895*60,	//81624000
	xres: 1152,		//1520=1152+120+184+64
	yres: 864,		//895=864+3+27+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 120,
	left_margin: 184,
	right_margin: 64,
	vsync_len: 3,
	upper_margin: 27,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
//This is a GTF formula
	name: "gtf1176x664_60",
	pixclock: 1528*688*60,	//63075840
	xres: 1176,	//1528=1176+120+176+56
	yres: 664,	//688=664+3+20+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 120,
	left_margin: 176,
	right_margin: 56,
	vsync_len: 3,
	upper_margin: 20,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
#endif
//The should be same as CVT reduced
	name: "dmt1280x768_60r",
	pixclock: 68250000, //ROUND_QM(1440*790*60)
	xres: 1280,		//1440 = 1280+32+80+48
	yres: 768,		//790 = 768+7+12+3
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 0,
	oepol_actl: 0,
	hsync_len: 32,
	left_margin: 80,
	right_margin: 48,
	vsync_len: 7,
	upper_margin: 12,
	lower_margin: 3,
	active: 1,
	crt: 1
//cvt compliant 1280x768@60 //1664=1280+128+192+64 //798=768+7+20+3
//lcdp "vesa:1280x768@60" or
//lcdp "v:79672320,1280,768,1,0,1,0,128,192,64,7,20,3,1,1"
}, {
#ifdef INCLUDE_GTF_SETTINGS
//This is a GTF formula
	name: "gtf1280x720_60",
	pixclock: 1664*746*60,	//74480640
	xres: 1280,		//1664 = 1280+136+192+56
	yres: 720,		//746 = 720+3+22+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 136,
	left_margin: 192,
	right_margin: 56,
	vsync_len: 3,
	upper_margin: 22,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
#endif
	name: "dmt1280x960_60",
	pixclock: 108000000,
	xres: 1280,
	yres: 960,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 112,
	left_margin: 312,
	right_margin: 96,
	vsync_len: 3,
	upper_margin: 36,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1280x960_85",
	pixclock: 148500000,
	xres: 1280,
	yres: 960,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 160,
	left_margin: 224,
	right_margin: 64,
	vsync_len: 3,
	upper_margin: 47,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1280x1024_60",
	pixclock: 108000000,
	xres: 1280,
	yres: 1024,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 112,
	left_margin: 248,
	right_margin: 48,
	vsync_len: 3,
	upper_margin: 38,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1280x1024_75",
	pixclock: 135000000,
	xres: 1280,
	yres: 1024,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 144,
	left_margin: 248,
	right_margin: 16,
	vsync_len: 3,
	upper_margin: 38,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1280x1024_85",
	pixclock: 157500000,
	xres: 1280,
	yres: 1024,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 160,
	left_margin: 224,
	right_margin: 64,
	vsync_len: 3,
	upper_margin: 44,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
	name: "dmt1360x768_60",
	pixclock: 85500000,
	xres: 1360,
	yres: 768,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 112,
	left_margin: 256,
	right_margin: 64,
	vsync_len: 6,
	upper_margin: 18,
	lower_margin: 3,
	active: 1,
	crt: 1
}, {
#ifdef INCLUDE_GTF_SETTINGS
//This is a GTF formula
	name: "gtf1600x1024_60",
	pixclock: 2144*1060*60,	//136358400
	xres: 1600,		//2144=1600+168+272+104
	yres: 1024,		//1060=1024+3+32+1
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 168,
	left_margin: 272,
	right_margin: 104,
	vsync_len: 3,
	upper_margin: 32,
	lower_margin: 1,
	active: 1,
	crt: 1
}, {
#endif
	name: "dmt1600x1200_60",
	pixclock: 162000000,
	xres: 1600,
	yres: 1200,
	pclk_redg: 1,
	hsyn_acth: 1,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 192,
	left_margin: 304,
	right_margin: 64,
	vsync_len: 3,
	upper_margin: 46,
	lower_margin: 1,
	active: 1,
	crt: 1
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
	left_margin: 39,
	right_margin: 1,
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
	left_margin: 152,
	right_margin: 32,
	vsync_len: 3,
	upper_margin: 27,
	lower_margin: 1,
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
	left_margin: 16,
	right_margin: 32,
	vsync_len: 8,
	upper_margin: 2,
	lower_margin: 3,
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
	left_margin: 152,
	right_margin: 32,
	vsync_len: 3,
	upper_margin: 27,
	lower_margin: 1,
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
	left_margin: 160,
	right_margin: 24,
	vsync_len: 6,
	upper_margin: 29,
	lower_margin: 3,
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
	left_margin: 39,
	right_margin: 1,
	vsync_len: 20,
	upper_margin: 3,
	lower_margin: 8,
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
	left_margin: 16,
	right_margin: 24,
	vsync_len: 20,
	upper_margin: 3,
	lower_margin: 8,
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
	left_margin: 228,
	right_margin: 37,
	vsync_len: 6,
	upper_margin: 29,
	lower_margin: 3,
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
	left_margin: 264,
	right_margin: 64,
	vsync_len: 4,
	upper_margin: 44,
	lower_margin: 2,
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
	left_margin: 264,
	right_margin: 64,
	vsync_len: 4,
	upper_margin: 44,
	lower_margin: 2,
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
	left_margin: 160,
	right_margin: 56,
	vsync_len: 3,
	upper_margin: 11,
	lower_margin: 201,
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
	left_margin: 39,
	right_margin: 1,
	vsync_len: 20,
	upper_margin: 3,
	lower_margin: 8,
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
	left_margin: 264,
	right_margin: 128,
	vsync_len: 4,
	upper_margin: 44,
	lower_margin: 2,
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
	left_margin: 49,
	right_margin: 16,
	vsync_len: 1,
	upper_margin: 42,
	lower_margin: 2,
	active: 1,
	crt: 0
}, {
	name: "lg1360x398",  //             LG:81000000,1360,398,1,0,1,0,50,420,268,1,21,214,1,1
	pixclock: 81000000,
	xres: 1360,
	yres: 398,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 50,
	left_margin: 420,
	right_margin: 268,
	vsync_len: 1,
	upper_margin: 21,
	lower_margin: 214,
	active: 1,
	crt: 1
}, {
	name: "lg1360x480",  //             LG:81000000,1360,480,1,0,1,0,50,420,268,1,21,124,1,1
	pixclock: 81000000,
	xres: 1360,
	yres: 480,
	pclk_redg: 1,
	hsyn_acth: 0,
	vsyn_acth: 1,
	oepol_actl: 0,
	hsync_len: 50,
	left_margin: 420,
	right_margin: 268,
	vsync_len: 1,
	upper_margin: 21,
	lower_margin: 124,
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
static u32 fb_cvt_ideal_duty_cycle(u32 h_period_est)
{
	u32 c_prime = (FB_CVT_GTF_C - FB_CVT_GTF_J) *
		(FB_CVT_GTF_K) + 256 * FB_CVT_GTF_J;
	u32 m_prime = (FB_CVT_GTF_K * FB_CVT_GTF_M);

	return (1000 * c_prime  - ((m_prime * h_period_est)/1000))/256;
}

static u32 fb_cvt_hblank(struct fb_cvt_data *cvt)
{
	u32 hblank = 0;

	if (cvt->flags & FB_CVT_FLAG_REDUCED_BLANK)
		hblank = FB_CVT_RB_HBLANK;
	else {
		u32 ideal_duty_cycle = fb_cvt_ideal_duty_cycle(cvt->hperiod);
		u32 active_pixels = cvt->active_pixels;

		if (ideal_duty_cycle < 20000)
			ideal_duty_cycle = 20000;
		hblank = (active_pixels * ideal_duty_cycle)/
			(100000 - ideal_duty_cycle);
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

#ifdef CONFIG_GP_HSYNC
#define HMASK (1 << (CONFIG_GP_HSYNC & 0x1f))
#define VMASK (1 << (CONFIG_GP_VSYNC & 0x1f))
volatile u32 *gpio_get_in_ptr(u32 gp);
volatile u32 *gpio_get_stat_ptr(u32 gp, int edge);

struct v_aspect_ratio {
	unsigned char vsync_len;
	unsigned char num;
	unsigned char den;
	unsigned char yres_div256;
};
static const struct v_aspect_ratio cvt_vsync_len_to_aspect_ratio[] = {
	{4, 4, 3, 0},		/* 1.333 : 1 */
	{5, 16, 9, 0},		/* 1.777 : 1 */
	{6, 8, 5, 0},		/* 1.6   : 1 */
	{7, 5, 4, 1024/256},	/* 1.25  : 1 */
	{7, 5, 3, 0},		/* 1.666 : 1, 768/256 */
	{0, 0, 0, 0}
};

int get_xres_guess(int yres)
{
	int xres;
	if ((yres % 83) == 0)
		xres = (yres * 147) / 83;
	else if ((yres % 27) == 0)
		xres = (yres * 4) / 3;
	else if ((yres % 9) == 0)
		xres = (yres * 16) / 9;
	else if ((yres % 5) == 0)
		xres = (yres * 8) / 5;
	else if ((yres % 128) == 0)
		xres = (yres * 25) / 16;	//still a multiple of 8
	else
		xres = (yres * 4) / 3;
	return (xres + 4) & ~7;	//round to multiple of 8
}
int get_xres(int yres, int vsync_len)
{
	int xres = -1;
	const struct v_aspect_ratio *p = cvt_vsync_len_to_aspect_ratio;
	while (p->vsync_len) {
		if (vsync_len == p->vsync_len) {
			if ((p->yres_div256 == 0) ||
					((yres >> 8) == p->yres_div256)) {
				xres = (yres * p->num + (p->den << 2)) / (p->den << 3);
				return xres << 3;
			}
		}
		p++;
	}
	return get_xres_guess(yres);
}

int count_hsync_while_vsync_val(int hsync_active, int vsync, ulong ticks,
		volatile u32 *phstat,
		volatile u32 *phsync,
		volatile u32 *pvsync)
{
	int hcnt = 0;
	ulong elapsed;
	u32 check = 0x10000;
	do {
		if (*phstat & HMASK) {
			if ((*phsync ^ hsync_active) & HMASK) {
				/* hsync is inactive now*/
				*phstat = HMASK;
				hcnt++;
			}
		}
		if ((*pvsync ^ vsync) & VMASK)
			break;
		if (check==0) {
			elapsed = get_timer(ticks);
			if (elapsed > CFG_HZ)
				return -1;
			check = 0x10000;
		}
		check--;
	} while (1);

	/* if HSYNC about to count */
	if (*phstat & HMASK) {
		do {
			if ((*phsync ^ hsync_active) & HMASK) {
				/* hsync is inactive now*/
				*phstat = HMASK;
				hcnt++;
				break;
			}
			if (check==0) {
				elapsed = get_timer(ticks);
				if (elapsed > CFG_HZ)
					return -1;
				check = 0x10000;
			}
			check--;
		} while (1);
	}
	return hcnt;
}

//#define MEASURE_HSYNC_DUTY_CYCLE	//no need to do this, use 8% rounded down
#ifdef MEASURE_HSYNC_DUTY_CYCLE
int count_while_high(volatile u32 *phsync, ulong ticks)
{
	u32 check = 0x10000;
	u32 high = 0;
	do {
		if (check==0) {
			ulong elapsed = get_timer(ticks);
			if (elapsed > (CFG_HZ >> 1))
				return -1;
			check = 0x10000;
		}
		check--;
		if (!(*phsync & HMASK))
			break;
		high++;
		if (!(*phsync & HMASK))
			break;
		high++;
		if (!(*phsync & HMASK))
			break;
		high++;
		if (!(*phsync & HMASK))
			break;
		high++;
	} while (1);
	return high;
}
int count_while_low(volatile u32 *phsync, ulong ticks)
{
	u32 check = 0x10000;
	u32 low = 0;
	do {
		if (check==0) {
			ulong elapsed = get_timer(ticks);
			if (elapsed > (CFG_HZ >> 1))
				return -1;
			check = 0x10000;
		}
		check--;
		if (*phsync & HMASK)
			break;
		low++;
		if (*phsync & HMASK)
			break;
		low++;
		if (*phsync & HMASK)
			break;
		low++;
		if (*phsync & HMASK)
			break;
		low++;
	} while (1);
	return low;
}
u32 measure_hsync_duty_cycle(volatile u32 *phsync, u32 hsync_active)
{
	u32 frac;
	u32 hsync_low_cnt = 1;
	u32 hsync_high_cnt = 1;
	u32 check = 0x10000;
	ulong ticks = get_timer(0);
	if (hsync_active) {
		count_while_low(phsync, ticks);		/* returns when high */
		count_while_high(phsync, ticks);	/* returns when low */
	} else {
		count_while_high(phsync, ticks);	/* returns when low */
		count_while_low(phsync, ticks);		/* returns when high */
	}
	/* hsync just went inactive */

	/* count for 1/4 second */
	do {
		if (check==0) {
			ulong elapsed = get_timer(ticks);
			check = 0x10000;
			if (elapsed > (CFG_HZ >> 2))	/* sample for 1/4 second */
				break;
		}
		check--;
		if (*phsync & HMASK)
			hsync_high_cnt++;
		else
			hsync_low_cnt++;
		if (*phsync & HMASK)
			hsync_high_cnt++;
		else
			hsync_low_cnt++;
		if (*phsync & HMASK)
			hsync_high_cnt++;
		else
			hsync_low_cnt++;
		if (*phsync & HMASK)
			hsync_high_cnt++;
		else
			hsync_low_cnt++;
	} while (1);

	/* count till hsync inactive edge */
	if (hsync_active) {
		hsync_low_cnt += count_while_low(phsync, ticks);	/* returns when high */
		hsync_high_cnt += count_while_high(phsync, ticks);	/* returns when low */
	} else {
		hsync_high_cnt += count_while_high(phsync, ticks);	/* returns when low */
		hsync_low_cnt += count_while_low(phsync, ticks);	/* returns when high */
	}
	{
		u32 duty;
		u32 duty_int;
		u64 t = (hsync_active) ? hsync_high_cnt : hsync_low_cnt;
		int power2 = 0;
		u32 total = hsync_high_cnt + hsync_low_cnt;
		while ((t & (1ULL << 63)) == 0) {
			power2++;
			t <<= 1;
		}
		t = t / total;
		if (power2 > 32) {
			t >>= (power2 - 32);
			power2 = 32;
		}
		frac = (u32)t;
		t *= 100;
		duty_int = (u32)(t >> power2);
		t &= ((1ULL << power2) - 1);
		t *= 10000;
		duty = (u32)(t >> power2);
		printf("hsync_high_cnt=%i hsync_low_cnt=%i duty=%i.%04i%% frac=%08x\n",
				hsync_high_cnt, hsync_low_cnt, duty_int, duty, frac);
	}
	return frac;
}
#endif

int get_hsync_polarity(volatile u32 *phsync, volatile u32 *pvsync)
{
	u32 vsync;
	u32 check = 0x10000;
	u32 hsync_low_cnt = 0;
	u32 hsync_high_cnt = 0;
	u32 vsync_cnt = 0;

	/* calc HSYNC duty cycle */
	ulong ticks = get_timer(0);

	/* count hsync until vsync changes twice */
	vsync = *pvsync & VMASK;
	do {
		if (check==0) {
			ulong elapsed = get_timer(ticks);
			if (elapsed > (CFG_HZ >> 2))
				break;
			check = 0x10000;
		}
		check--;
		if (*phsync & HMASK)
			hsync_high_cnt++;
		else
			hsync_low_cnt++;
		if ((*pvsync ^ vsync) & VMASK) {
			vsync ^= VMASK;
			vsync_cnt++;
			if (vsync_cnt >= 2)
				break;
		}
	} while (1);
	if (1) printf("hsync_high_cnt=%i hsync_low_cnt=%i\n",
			hsync_high_cnt, hsync_low_cnt);
	if ((hsync_high_cnt == 0) || (hsync_low_cnt == 0))
		return -1;
	return (hsync_high_cnt < hsync_low_cnt) ? 1 : 0;
}

unsigned query_pixel_Clock(unsigned long mhz);
static inline u32 cvt_len(u32 len, u32 real_clk, u32 ideal_clk)
{
#define FRAC_BITS 1
	u64 tmp = len << FRAC_BITS;
	tmp *= real_clk;
	tmp /= ideal_clk;
	tmp += 1 << (FRAC_BITS-1);
	tmp >>= FRAC_BITS;
	return (u32)tmp;
}

static u32 calc_pixclock(u32 htotal, u32 hperiod)
{
/*
 *  ((htotal pixels/line) / (hperiod ns/line)) * (1000000000 ns/sec)  = pixels/sec
 */
	u64 tmp;
	tmp = htotal;
	tmp *= 1000000000;
	tmp += hperiod >> 1;	/* round */
	return do_divq(tmp, hperiod);
}

int calc_gtf_settings(struct lcd_panel_info_t *panel, u32 v_total, u32 hperiod)
{
	u64 tmp;
	u32 ideal_duty_cycle;	//div 10**8 = frac
	u32 hsync;
	u32 hblank;
	u32 htotal;
//	u32 v_period = hperiod * v_total;
	panel->lower_margin = 1;
#define MIN_VSYNC_PLUS_BP 550000	/* min time of vsync + back porch (nanosec) */

/* C' and M' are part of the Blanking Duty Cycle computation */

#define C_PRIME           30
#define M_PRIME           300
	panel->name = "calc_gtf";
	panel->upper_margin = ((MIN_VSYNC_PLUS_BP + (hperiod >> 1)) / hperiod)
		- panel->vsync_len;
	panel->yres = v_total - panel->lower_margin - panel->upper_margin - panel->vsync_len;

// divided by 10**8 gives fraction
	ideal_duty_cycle = (C_PRIME * 1000000) - ((M_PRIME * hperiod));
	panel->xres = get_xres_guess(panel->yres);

	tmp = ideal_duty_cycle;
	tmp *= panel->xres;
	tmp += (100000000 - ideal_duty_cycle) << 3;	/* round */
	hblank = do_divq(tmp, (100000000 - ideal_duty_cycle) << 4);
	hblank <<= 4;

	htotal =  panel->xres + hblank;
	hsync = (htotal * 2 + (25*4)) / (25*8);	//8%
	hsync <<= 3;

	panel->left_margin = hblank >> 1;
	panel->right_margin = (hblank >> 1) - hsync;
	panel->hsync_len = hsync;
	panel->pixclock = calc_pixclock(htotal, hperiod);
	return 0;
}

static int scan_for_dmt_entry(struct lcd_panel_info_t *panel, u32 v_total, u32 hperiod)
{
	const struct lcd_panel_info_t *p = lcd_panels_;
	const struct lcd_panel_info_t *best = NULL;
	u32 best_error = ~0;
	int i;
	u32 htotal;
	for (i = 0; i < num_lcd_panels; i++, p++ ) {
		u32 htot, vtot;
		u32 cur_hperiod;
		u32 error;
		u64 tmp;
		if (p->name[0] != 'd') continue;
		if (p->name[1] != 'm') continue;
		if (p->name[2] != 't') continue;
		if (p->hsyn_acth != panel->hsyn_acth) continue;
		if (p->vsyn_acth != panel->vsyn_acth) continue;
		if (p->vsync_len != panel->vsync_len) continue;
		vtot = p->upper_margin + p->yres + p->lower_margin + p->vsync_len;
		if (vtot != v_total) continue;
		htot = p->left_margin + p->xres + p->right_margin + p->hsync_len;
		tmp = htot;
		tmp *= 1000000000;
		cur_hperiod = do_divq(tmp, p->pixclock);		//nanoseconds/line
		error = (cur_hperiod >= hperiod) ? (cur_hperiod - hperiod) :
				(hperiod - cur_hperiod);
		if (best_error > error) {
			best = p;
			best_error = error;
		}
	}
	if (!best) {
		return -1;
	}
	panel->name = "calc_dmt";
	panel->xres = best->xres;
	panel->hsync_len = best->hsync_len;
	panel->left_margin = best->left_margin;
	panel->right_margin = best->right_margin;

	panel->yres = best->yres;
	panel->upper_margin = best->upper_margin;
	panel->lower_margin = best->lower_margin;
	htotal = panel->xres + panel->hsync_len +
			panel->left_margin + panel->right_margin;
	panel->pixclock = calc_pixclock(htotal, hperiod);
	printf("found %s, error=%i ns, hperiod=%i ns\n", best->name, best_error, hperiod);
	return 0;
}

void calc_cvt_settings(struct lcd_panel_info_t *panel, u32 v_total, u32 hperiod)
{
	u32 v_period = hperiod * v_total;
	u32 reduced = 0;
	panel->lower_margin = 3;
	if (panel->hsyn_acth && !panel->vsyn_acth) {
		reduced = 1;
		v_period -= FB_CVT_RB_MIN_VBLANK * 1000;
		panel->yres = v_period / hperiod;
		panel->name = "calc_cvtr";
	} else {
		v_period -= FB_CVT_MIN_VSYNC_BP * 1000;
		panel->yres = v_period / hperiod;
		panel->yres -= FB_CVT_MIN_VPORCH;
		panel->name = "calc_cvt";
	}

	panel->upper_margin = v_total - panel->vsync_len - panel->lower_margin - panel->yres;
	panel->xres = get_xres(panel->yres, panel->vsync_len);
	{
		u32 htotal;
		u32 hblank  = FB_CVT_RB_HBLANK;
		panel->hsync_len = 32;
		if (!reduced) {
			u32 ideal_duty_cycle = fb_cvt_ideal_duty_cycle(hperiod);
			u32 active_pixels = panel->xres;

			if (ideal_duty_cycle < 20000)
				ideal_duty_cycle = 20000;
			hblank = (active_pixels * ideal_duty_cycle)/
				(100000 - ideal_duty_cycle);
			hblank &= ~((2 * FB_CVT_CELLSIZE) - 1);
		}
		htotal =  panel->xres + hblank;
		panel->pixclock = calc_pixclock(htotal, hperiod);
		panel->left_margin = hblank >> 1;
		if (!reduced) {
			u32 t;
			u64 tmp = hperiod;
			tmp *= 2;	/* (2/25) 8% width */
			tmp = do_divq(tmp, 25);	/* nanosecond width of sync pulse */
			tmp *= panel->pixclock;
			t = do_divq(tmp, 1000000000);	/* # of clocks */
			t &= ~(FB_CVT_CELLSIZE - 1);
			panel->hsync_len = t;
		}
		panel->right_margin = hblank - panel->left_margin - panel->hsync_len;

//panel now has CVT settings
	}
#define ROUND_QM(a) (((a)+125000)/250000)*250000
	panel->pixclock = ROUND_QM(panel->pixclock);
}

static int calc_settings_from_hsync_vsync(struct lcd_panel_info_t *panel)
{
	u32 shift;
	u32 ideal_clk;
	u32 real_clk;
	u32 v_total;
	u32 vsync;
	int hsync_cnt1, hsync_cnt2;
	ulong ticks;
	ulong elapsed;
	u32 hperiod;
	u32 hsync_active;
#ifdef MEASURE_HSYNC_DUTY_CYCLE
	u32 frac;
#endif

	volatile u32 *phsync = gpio_get_in_ptr(CONFIG_GP_HSYNC);
	volatile u32 *pvsync = gpio_get_in_ptr(CONFIG_GP_VSYNC);
	/*
	 * this enables/disables rising/falling edge interrupts
	 * and returns a pointer to the interrupt status register
	 */
	volatile u32 *phstat = gpio_get_stat_ptr(CONFIG_GP_HSYNC, 3);
	int ret = get_hsync_polarity(phsync, pvsync);
	if (ret < 0)
		return ret;
	hsync_active =	(ret) ? HMASK : 0;

#ifdef MEASURE_HSYNC_DUTY_CYCLE
	frac = measure_hsync_duty_cycle(phsync, hsync_active);
#endif
	ticks = get_timer(0);
	vsync = *pvsync & VMASK;
	hsync_cnt1 = count_hsync_while_vsync_val(hsync_active, vsync, ticks, phstat, phsync, pvsync);
	ticks = get_timer(0);
	if (hsync_cnt1 < 0)
		return hsync_cnt1;

	hsync_cnt1 = count_hsync_while_vsync_val(hsync_active, vsync ^ VMASK, ticks, phstat, phsync, pvsync);
	if (hsync_cnt1 < 0)
		return hsync_cnt1;

	hsync_cnt2 = count_hsync_while_vsync_val(hsync_active, vsync, ticks, phstat, phsync, pvsync);
	elapsed = get_timer(ticks);
	gpio_get_stat_ptr(CONFIG_GP_HSYNC, 0);
	if (hsync_cnt2 < 0)
		return hsync_cnt2;
	if (hsync_cnt1 > hsync_cnt2) {
		int tmp = hsync_cnt1;
		hsync_cnt1 = hsync_cnt2;
		hsync_cnt2 = tmp;
		vsync ^= VMASK;
	}
	if (0) printf("hsync_cnt1=%i hsync_cnt2=%i\n",
			hsync_cnt1, hsync_cnt2);
	v_total = hsync_cnt1 + hsync_cnt2;
	panel->active = 1;
	panel->crt = 1;
	panel->rotation = 0 ;
	panel->hsyn_acth = (hsync_active) ? 1 : 0;
	panel->vsyn_acth = (vsync) ? 0 : 1;
	panel->vsync_len =  hsync_cnt1;

	{
		/*
		 * elasped is # of jiffies/vertical sync period
		 * convert from jiffies to ns
		 */
		u32 v_period;
		u64 v = elapsed;
		v *= 1000000000;
		v += CFG_HZ >> 1;	/* round */
		v_period = do_divq(v, CFG_HZ);
		hperiod = (v_period + (v_total >> 1)) / v_total;
	}

	printf("looking for hsync=%u, vsync=%u, vsync_len=%u, v_total=%u, hperiod=%u ns\n",
		panel->hsyn_acth, panel->vsyn_acth, panel->vsync_len, v_total, hperiod);
//Check if DMT timings should be used
	do {
		if (((panel->hsyn_acth ^ panel->vsyn_acth) == 0) ||
				(panel->vsync_len < 4)) {
			if (scan_for_dmt_entry(panel, v_total, hperiod)==0)
				break;
			if ((panel->hsyn_acth == 0) && (panel->vsyn_acth == 1) && (panel->vsync_len == 3))
				if (calc_gtf_settings(panel, v_total, hperiod)==0)
					break;
			printf("*** warning!!! **** weird panel, hsync=%u, vsync=%u, vsync_len=%u, v_total=%u, hperiod=%u ns\n",
				panel->hsyn_acth, panel->vsyn_acth, panel->vsync_len, v_total, hperiod);
		}
		calc_cvt_settings(panel, v_total, hperiod);
	} while (0);

	ideal_clk = panel->pixclock;
	shift = 0;
	while (1) {
		real_clk = query_pixel_Clock(ideal_clk);
		if (real_clk == 0)
			return -1;
		printf("ideal_clk=%u, real_clk=%u\n", ideal_clk, real_clk);
		if ((ideal_clk > 112000000) &&
				(ideal_clk > ((real_clk * 9) >> 3))) {
			/* ideal_clk is 12.5% more than real clock, scale down */
			ideal_clk >>= 1;
			shift++;
			continue;
		}
		break;
	}

	panel->pixclock = ideal_clk;
	if (shift) {
		/*
		 * all settings start as multiples of 8,
		 * so just zeros are shifted out
		 */
		panel->xres >>= shift;
		panel->hsync_len >>= shift;
		panel->left_margin >>= shift;
		panel->right_margin >>= shift;
	}
#if 0
	{
		u32 new_htotal = cvt_len(htotal, real_clk, ideal_clk);
		u32 new_xres = cvt_len(panel->xres, real_clk, ideal_clk);
		panel->xres = new_xres & ~7;	/* Keep multiple of 8*/
		panel->hsync_len = cvt_len(panel->hsync_len, real_clk, ideal_clk);
		panel->left_margin = cvt_len(panel->left_margin, real_clk, ideal_clk);
		panel->left_margin += (new_xres & 7) >> 1;
		panel->right_margin = new_htotal -
			(panel->xres + panel->hsync_len + panel->left_margin);
		panel->pixclock = calc_pixclock(new_htotal, hperiod);
	}
#endif

#ifdef MEASURE_HSYNC_DUTY_CYCLE
	{
		u32 t;
		u64 tmp = hperiod;
		tmp *= frac;
		tmp >>= 32;	/* nanosecond width of sync pulse */
		tmp *= panel->pixclock;
		tmp /= 500000000;	/* # of clocks */
		t = (u32)((tmp + 1) >> 1);
//		t += FB_CVT_CELLSIZE >> 1;
//		t &= ~(FB_CVT_CELLSIZE - 1);
		printf("measured val=%i, using =%i\n", t, panel->hsync_len);
	}
#endif
	return 0;
}
#else
#define calc_settings_from_hsync_vsync(a) 1
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
		 return 0;
	  }
	  return 1;
      } else if ('C' == UPCASE(*panelInfo)) {
	  if (calc_settings_from_hsync_vsync(panel)) {
#ifdef CONFIG_GP_HSYNC
		 printf( "Error calc from hsync(gp%u) vsync(gp%u)\n",
				 CONFIG_GP_HSYNC, CONFIG_GP_VSYNC);
#else
		 printf( "Error CONFIG_GP_HSYNC not defined\n");
#endif
		 return 0;
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
