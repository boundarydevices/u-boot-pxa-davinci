#include "platformTypes.h"
#define WINCE	1
#define LINUX	2
#define GAME	3

#define PXAFB_BPP 16

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

#if (PLATFORM_TYPE==MERCURY)
#define BYTES_PER_PIXEL 1	//1, 2, or 3 for mercury
#else
#if (PLATFORM_TYPE==HALOGEN)
#define BYTES_PER_PIXEL 3
#else
#define BYTES_PER_PIXEL 2
#endif
#endif

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
//              xres,xsyncWidth,xbegin,xend,  yres,ysyncWidth,ybegin,yend,  fPolarity,lPolarity,pPolarity,dPolarity, enable,unscramble,rotate,active,bpp,vSyncHz,type
#define HITACHI_QVGA_P 320, 64, 1, 16,		240, 20, 8, 3,		0,0,1,0,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0
#define SHARP_QVGA_P   320, 20, 1, 30,		240, 4, 17, 3,		0,0,1,0,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0
#define OKAYA_QVGA_P   320, 30,20, 38,		240, 3, 5, 15,		1,1,1,1,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0
//#define DA320X240_P  320, 64, 34, 1,		240, 20, 8, 3,		0,0,1,0,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0
#define DA320X240_P  320, 64, 34, 11,		240, 5, 8, 3,		0,0,1,0,	1,0,0,1,PXAFB_BPP,62,LCD_PANEL //0

#define DA640X240_P  640, 64, 34, 1,		240, 20, 8, 3,		0,0,1,0,	1,1,0,1,PXAFB_BPP,62,LCD_PANEL //1
#define DA800X480_P  800, 64, 34, 1,		480, 20, 8, 3,		0,0,1,0,	1,1,0,1,PXAFB_BPP,62,LCD_PANEL //2
#define DA640X480_P  640, 64, 34,105,		480, 20, 8,14,		0,0,1,0,	1,1,0,1,PXAFB_BPP,62,LCD_PANEL //3
#define DA240X320_P  240, 64, 34, 1,		320, 20, 8, 3,		0,0,1,0,	1,0,1,1,PXAFB_BPP,62,LCD_PANEL //4
#define DA800X600_P  800,0x9b,0x31,0x69,	600,0x04,0x01,0x17,	0,0,1,0,	1,1,0,1,PXAFB_BPP,62,CRT       //5
//#define DA1024X768_P 1024,0xc8,0x55,0xb4,	768,0x06,0x0b,0x1d,	0,0,1,0,	1,1,0,1,PXAFB_BPP,62,CRT		//6
//#define DA1024X768_P 1024,0xe4,0x3c,0x70,	768,0x0c,0x0b,0x20,	0,0,1,0,	1,1,0,1,PXAFB_BPP,62,CRT		//6
#define DA1024X768_P 1024,0xe4,0x3c,0x70,	768,0x0c,0x0b,0x20,	0,0,1,0,	1,1,0,1,PXAFB_BPP,62,LCD_PANEL //6
#define DP480X320_P  480, 64, 34, 1,		320,20,8,3,			0,0,1,0,	1,0,0,0,PXAFB_BPP,62,LCD_PANEL //7
#define DP320X240_P  320, 64, 34, 1,		240,20,8,3,			0,0,1,0,	1,0,0,0,PXAFB_BPP,62,LCD_PANEL //8
#define DL122X32_P   320, 64, 34, 1,		240,20,8,3,			0,0,1,0,	0,0,0,0,PXAFB_BPP,62,0         //9

// ********************************************************************************
#if (PLATFORM_TYPE==BOUNDARY_OLD_BOARD)
#define MOTHERBOARD_SCRAMBLED
#endif

//#define CONFIG_UNSCRAMBLE_LCD
#ifdef CONFIG_UNSCRAMBLE_LCD
#if 1  //FL_ACTIVE(DEF_P)					//passive cannot swap pin order
#ifdef MOTHERBOARD_SCRAMBLED

#ifndef DAUGHTERBOARD_UNSCRAMBLE
#define LCD_REORDER_BLUE  15,14, 8, 7, 6
#define LCD_REORDER_GREEN 13,12,11, 5, 4, 3
#define LCD_REORDER_RED   10, 9, 2, 1, 0
#endif

#else
//motherboard is NOT scrambled
#ifdef DAUGHTERBOARD_UNSCRAMBLE
#define LCD_REORDER_BLUE  15,14,13,10, 9
#define LCD_REORDER_GREEN  8, 4, 3, 2, 12, 11
#define LCD_REORDER_RED    7, 6, 5, 1, 0
#endif
#endif	//#ifdef MOTHERBOARD_SCRAMBLED
#endif	//#if FL_ACTIVE(DEF_P)
#endif	//#ifdef CONFIG_UNSCRAMBLE_LCD

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
#endif
#endif
#endif
