#include "platformTypes.h"
#define WINCE	1
#define LINUX	2
#define GAME	3

#define LCD_PANEL 0
#define CRT 1

#ifdef __ARMASM
#define SET_BYTES_PER_PIXEL 1
#else

#ifndef BYTES_PER_PIXEL
#define SET_BYTES_PER_PIXEL 1
#else
#define SET_BYTES_PER_PIXEL 0
#endif
#endif

//pixel clock frequency = LCLK / (2*(PCD+1))
#if SET_BYTES_PER_PIXEL
#define BYTES_PER_PIXEL PLAT_BYTES_PER_PIXEL
#endif


#if (BYTES_PER_PIXEL==3)
#define __BPP 18
#define BPP_FORMAT 6
#define PDFOR 3
#endif

#if (BYTES_PER_PIXEL==2)
#define __BPP 16
#define BPP_FORMAT 4
#define PDFOR 0
#endif

#if (BYTES_PER_PIXEL==1)
#define __BPP 8
//#define BPP_FORMAT 4
//#define PDFOR 0
#endif

// ****************************************************************************
// This is where different display settings reside
//vSyncHz is the minimum refresh rate, it may be higher
//vsPol - (FCLK) vertical sync polarity (1 - low active)
//hsPol - (LCLK) Horizontal sync polarity (1 - low active)
//pPol - (PCLK) Pixel clock polarity (1 - falling edge sample)
//oePol - (LBIAS) output enable polarity (1 - low active)
//dPol - imx31 can invert the data lines if needed
//              xres,xsyncWidth,xbegin,xend, yres,ysyncWidth,ybegin,yend,
//							vsPol,hsPol,pPol,oePol,dPol,
//								 enable,unscramble,rotate,active,vSyncHz,type
#define hitachi_qvga_P 320, 64, 1, 16,	240, 20, 8, 3,	0,0,1,0,0,	1,0,0,1,62,LCD_PANEL
#define sharp_qvga_P   320, 20, 1, 30,	240, 4, 17, 3,	0,0,1,0,0,	1,0,0,1,62,LCD_PANEL
//#define sharp_qvga_P  320, 64, 34, 11,	240, 5, 8, 3,	0,0,1,0,0,	1,0,0,1,62,LCD_PANEL
//#define okaya_qvga_P   320, 30,20, 38,	240, 5, 8, 14,	1,1,0,0,0,	1,0,0,1,62,LCD_PANEL
#define okaya_qvga_P   320, 30,37, 38,	240, 3, 16, 15,	1,1,0,0,0,	1,0,0,1,120,LCD_PANEL
#define okaya_480x272_P 480, 41, 1, 2,	272, 10, 3, 2,	1,1,1,0,0,	1,0,0,1,62,LCD_PANEL
#define urt_480x272_P 480, 41, 42, 42,  272, 10, 1, 6,  0,1,0,0,0,      1,0,0,1,62,LCD_PANEL

#define hitachi_hvga_P  640, 64, 34, 1,	240, 20, 8, 3,	0,0,1,0,0,	1,1,0,1,62,LCD_PANEL
#define hitachi_wvga_P  800, 64, 34, 1,	480, 20, 8, 3,	0,0,1,0,0,	1,1,0,1,62,LCD_PANEL
#define sharp_vga_P  640, 64, 34,105,	480, 20, 8,14,	0,0,1,0,0,	1,1,0,1,62,LCD_PANEL
#define qvga_portrait_P  240, 64, 34, 1,	320, 20, 8, 3,	0,0,1,0,0,	1,0,1,1,62,LCD_PANEL

#define lcd_svga_P  800, 64, 32,152,	600,  3, 1,27,	1,1,0,0,0,	1,1,0,1,62,LCD_PANEL
#define crt800x600_P  800, 64, 32,152,	600,  3, 1,27,	1,1,0,0,0,	1,1,0,1,62,CRT
//#define crt800X600_P  800,0x9b,0x31,0x69,600,0x04,0x01,0x17,	0,0,1,0,0,	1,1,0,1,62,CRT
//#define crt1024x768_P 1024,0xe4,0x3c,0x70,	768,0x0c,0x0b,0x20,	0,0,1,0,0,	1,1,0,1,62,LCD_PANEL
//#define crt1024x768_P 1024, 104,56,160,	768,3,1,23,	0,0,1,0,0,	1,0,0,1,62,CRT	//from GTF spreadsheet
#define crt1024x768_P 1024,0xe4,0x3c,0x70,	768,0x0c,0x0b,0x20,	0,0,1,0,0,	1,1,0,1,62,CRT
//#define crt1024x768_P 1024,0xc8,0x55,0xb4,	768,0x06,0x0b,0x1d,	0,0,1,0,0,	1,1,0,1,62,CRT
#define hitachi_92_P 960,15,220,1,    160,200,148,3,	0,0,1,0,0,	1,0,0,1,62,LCD_PANEL
#define hitachi_wxga_P	1024,64,1,39,	768,20,8,3,	1,1,1,0,0,	1,0,0,1,75,LCD_PANEL
#define tovis_w_P	1024,104,56,160,	200,3,201,11,	1,1,0,0,0,	1,0,0,1,75,CRT
//#define tovis_w_P	1024,64,1,39,		204,20,8,3,		1,1,1,0,0,	1,0,0,1,75,LCD_PANEL

#define DP480X320_P  480, 64, 34, 1,	320,20,8,3,		0,0,1,0,0,	1,0,0,0,62,LCD_PANEL
#define DP320X240_P  320, 64, 34, 1,	240,20,8,3,		0,0,1,0,0,	1,0,0,0,62,LCD_PANEL
#define DL122X32_P   320, 64, 34, 1,	240,20,8,3,		0,0,1,0,0,	0,0,0,0,62,0
///////////////////////////////
// ********************************************************************************
#ifdef __ARMASM
#define __SKIP_LCD_REORDER 1
#endif

#if (PLAT_PXALCD_SCRAMBLED)
#define __MOTHERBOARD_SCRAMBLED 1
#endif

#ifdef __CONFIG_UNSCRAMBLE_LCD
#if 1  //FL_ACTIVE(DEF_P)					//passive cannot swap pin order
#ifdef __MOTHERBOARD_SCRAMBLED

#ifndef __DAUGHTERBOARD_UNSCRAMBLE
#define LCD_REORDER_BLUE  15,14, 8, 7, 6
#define LCD_REORDER_GREEN 13,12,11, 5, 4, 3
#define LCD_REORDER_RED   10, 9, 2, 1, 0
#endif

#else
//motherboard is NOT scrambled
#ifdef __DAUGHTERBOARD_UNSCRAMBLE
#define LCD_REORDER_BLUE  15,14,13,10, 9
#define LCD_REORDER_GREEN  8, 4, 3, 2, 12, 11
#define LCD_REORDER_RED    7, 6, 5, 1, 0
#endif
#endif	//#ifdef __MOTHERBOARD_SCRAMBLED
#endif	//#if 1 //FL_ACTIVE(DEF_P)
#endif	//#ifdef __CONFIG_UNSCRAMBLE_LCD

#ifndef __SKIP_LCD_REORDER
#ifndef LCD_REORDER_BLUE
#if (BYTES_PER_PIXEL==1)
#define LCD_REORDER_BLUE  0,1
#define LCD_REORDER_GREEN 2,3,4
#define LCD_REORDER_RED   5,6,7
#else
#if (BYTES_PER_PIXEL==2)
#define LCD_REORDER_BLUE  0,1,2,3,4
#define LCD_REORDER_GREEN 5,6,7,8,9,10
#define LCD_REORDER_RED   11,12,13,14,15
#else
#define LCD_REORDER_BLUE  0,1,2,3,4,5
#define LCD_REORDER_GREEN 6,7,8,9,10,11
#define LCD_REORDER_RED   12,13,14,15,16,17
#endif	//else 2
#endif	//else 1
#endif	//#ifndef LCD_REORDER_BLUE
#endif	//#ifndef __SKIP_LCD_REORDER
