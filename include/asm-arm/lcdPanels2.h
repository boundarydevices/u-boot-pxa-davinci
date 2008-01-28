#include "lcdPanels.h"
#define hitachi_qvga	0
#define sharp_qvga		1
#define okaya_qvga		2
#define okaya_480x272	3
#define hitachi_hvga	4
#define hitachi_wvga	5
#define sharp_vga		6
#define qvga_portrait	7
#define lcd_svga		8
#define crt800x600		9
#define crt1024x768		10
#define hitachi_92		11
#define tovis_w			12
#define DP480X320		13
#define DP320X240		14
#define DL122X32		15
#define UNKNOWN		0xcc

#ifdef __ARMASM
	GBLS DEF_P
#endif

//don't use #elif because converting to ARMASM format
//there is no corresponding directive
#if (DISPLAY_TYPE==hitachi_qvga)			//5.7 inch display
#define DEF_P	hitachi_qvga_P
#define DEF_DISPLAY_INDEX	hitachi_qvga
#else	
#if (DISPLAY_TYPE==sharp_qvga)			//5.7 inch display
#define DEF_P	sharp_qvga_P
#define DEF_DISPLAY_INDEX	sharp_qvga
#else	
#if (DISPLAY_TYPE==okaya_qvga)			//3.5 inch display
#define DEF_P	okaya_qvga_P
#define DEF_DISPLAY_INDEX	okaya_qvga
#else	
#if (DISPLAY_TYPE==okaya_480x272)
#define DEF_P	okaya_480x272_P
#define DEF_DISPLAY_INDEX	okaya_480x272
#else
#if (DISPLAY_TYPE==hitachi_hvga)		//6.2 inch display
#define DEF_P	hitachi_hvga_P
#define DEF_DISPLAY_INDEX	hitachi_hvga
#else	
#if (DISPLAY_TYPE==hitachi_wvga)		//7 or 9 inch display
#define DEF_P	hitachi_wvga_P
#define DEF_DISPLAY_INDEX	hitachi_wvga
#else	
#if (DISPLAY_TYPE==sharp_vga)		//10.4 inch display
#define DEF_P	sharp_vga_P
#define DEF_DISPLAY_INDEX	sharp_vga
#else	
#if (DISPLAY_TYPE==qvga_portrait)		//3.5 inch display
#define DEF_P	qvga_portrait_P
#define DEF_DISPLAY_INDEX	qvga_portrait
#else	
#if (DISPLAY_TYPE==lcd_svga)
#define DEF_P	lcd_svga_P
#define DEF_DISPLAY_INDEX	lcd_svga
#else	
#if (DISPLAY_TYPE==crt800x600)
#define DEF_P	crt800x600_P
#define DEF_DISPLAY_INDEX	crt800x600
#else	
#if (DISPLAY_TYPE==crt1024x768)
#define DEF_P	crt1024x768_P
#define DEF_DISPLAY_INDEX	crt1024x768
#else	
#if (DISPLAY_TYPE==hitachi_92)
#define DEF_P	hitachi_92_P
#define DEF_DISPLAY_INDEX	hitachi_92
#else	
#if (DISPLAY_TYPE==tovis_w)
#define DEF_P	tovis_w_P
#define DEF_DISPLAY_INDEX	tovis_w
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
#define DEF_P	sharp_qvga_P
#define DEF_DISPLAY_INDEX	UNKNOWN
#else	
#define DEF_P	hitachi_qvga_P
#define DEF_DISPLAY_INDEX	0
#warning "No display selected, defaulting to hitachi_qvga"

#endif		//0xcc
#endif		//15
#endif		//14
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
