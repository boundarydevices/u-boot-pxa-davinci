#include "lcdPanels.h"
#define HITACHI_QVGA	0
#define SHARP_QVGA		1
#define DA320X240		2
#define OKAYA_QVGA		3
#define OKAYA_480X272	4
#define DA640X240		5
#define DA800X480		6
#define DA640X480		7
#define DA240X320		8
#define DA800X600		9
#define DA1024X768		10
#define HITACHI_92		11
#define DP480X320		12
#define DP320X240		13
#define DL122X32		14
#define UNKNOWN		0xcc

#ifdef __ARMASM
	GBLS DEF_P
#endif

//don't use #elif because converting to ARMASM format
//there is no corresponding directive
#if (DISPLAY_TYPE==HITACHI_QVGA)			//5.7 inch display
#define DEF_P	HITACHI_QVGA_P
#define DEF_DISPLAY_INDEX	HITACHI_QVGA
#else	
#if (DISPLAY_TYPE==SHARP_QVGA)			//5.7 inch display
#define DEF_P	SHARP_QVGA_P
#define DEF_DISPLAY_INDEX	SHARP_QVGA
#else	
#if (DISPLAY_TYPE==DA320X240)			//5.7 inch display
#define DEF_P	DA320X240_P
#define DEF_DISPLAY_INDEX	DA320X240
#else	
#if (DISPLAY_TYPE==OKAYA_QVGA)			//3.5 inch display
#define DEF_P	OKAYA_QVGA_P
#define DEF_DISPLAY_INDEX	OKAYA_QVGA
#else	
#if (DISPLAY_TYPE==OKAYA_480X272)
#define DEF_P	OKAYA_480X272_P
#define DEF_DISPLAY_INDEX	OKAYA_480X272
#else
#if (DISPLAY_TYPE==DA640X240)		//6.2 inch display
#define DEF_P	DA640X240_P
#define DEF_DISPLAY_INDEX	DA640X240
#else	
#if (DISPLAY_TYPE==DA800X480)		//7 or 9 inch display
#define DEF_P	DA800X480_P
#define DEF_DISPLAY_INDEX	DA800X480
#else	
#if (DISPLAY_TYPE==DA640X480)		//10.4 inch display
#define DEF_P	DA640X480_P
#define DEF_DISPLAY_INDEX	DA640X480
#else	
#if (DISPLAY_TYPE==DA240X320)		//3.5 inch display
#define DEF_P	DA240X320_P
#define DEF_DISPLAY_INDEX	DA240X320
#else	
#if (DISPLAY_TYPE==DA800X600)
#define DEF_P	DA800X600_P
#define DEF_DISPLAY_INDEX	DA800X600
#else	
#if (DISPLAY_TYPE==DA1024X768)
#define DEF_P	DA1024X768_P
#define DEF_DISPLAY_INDEX	DA1024X768
#else	
#if (DISPLAY_TYPE==DP480X320)		//5.7 inch display
#define DEF_P	DP480X320_P
#define DEF_DISPLAY_INDEX	DP480X320
#else	
#if (DISPLAY_TYPE==DP320X240)	//5.7 inch display
#define DEF_P	DP320X240_P
#define DEF_DISPLAY_INDEX	DP320X240
#else	
#if (DISPLAY_TYPE==DL122X32)
#define DEF_P	DL122X32_P
#define DEF_DISPLAY_INDEX	DL122X32
#else
#if (DISPLAY_TYPE==UNKNOWN)
#define DEF_P	DA320X240_P
#define DEF_DISPLAY_INDEX	0xcc

#else	
#define DEF_P	HITACHI_QVGA_P
#define DEF_DISPLAY_INDEX	0
#warning "No display selected, defaulting to HITACHI_QVGA"

#endif		//0xcc
#endif		//13
#endif		//12
#endif		//11
#endif		//10
#endif		//9
#endif		//8
#endif		//7
#endif		//6
#endif		//5
#endif		//4
#endif		//3
#endif		//2
#endif		//1
#endif		//0


#ifndef __ARMASM
#define GRX_RES(xres,xsyncWidth,xbegin,xend, yres,ysyncWidth,ybegin,yend, fPolarity,lPolarity,pPolarity,dPolarity, enable,unscramble,rotate,active,bpp,vSyncHz,type) xres
#define GRY_RES(xres,xsyncWidth,xbegin,xend, yres,ysyncWidth,ybegin,yend, fPolarity,lPolarity,pPolarity,dPolarity, enable,unscramble,rotate,active,bpp,vSyncHz,type) yres
#define CALL(a,b) a(b)
#define SCREEN_WIDTH CALL(GRX_RES,DEF_P)
#define SCREEN_HEIGHT CALL(GRY_RES,DEF_P)
#endif
