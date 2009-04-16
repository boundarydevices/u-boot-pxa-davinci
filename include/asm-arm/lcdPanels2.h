#include "lcdPanels.h"
#define hitachi_qvga	0
#define sharp_qvga		1
#define okaya_qvga		2
#define okaya_480x272		3
#define urt_480x272		4
#define hitachi_hvga		5
#define hitachi_wvga		6
#define sharp_vga		7
#define qvga_portrait		8
#define lcd_svga		9
#define crt800x600		10
#define crt1024x768		11
#define hitachi_92		12
#define tovis_w			13
#define DP480X320		14
#define DP320X240		15
#define DL122X32		16
#define UNKNOWN		0xcc

#ifdef __ARMASM
	GBLS DEF_P
#endif

#if (DISPLAY_TYPE==hitachi_qvga)			//5.7 inch display
#define DEF_P	hitachi_qvga_P
#define __DEF_DISPLAY_INDEX	hitachi_qvga
#elif (DISPLAY_TYPE==sharp_qvga)			//5.7 inch display
#define DEF_P	sharp_qvga_P
#define __DEF_DISPLAY_INDEX	sharp_qvga
#elif (DISPLAY_TYPE==okaya_qvga)			//3.5 inch display
#define DEF_P	okaya_qvga_P
#define __DEF_DISPLAY_INDEX	okaya_qvga
#elif (DISPLAY_TYPE==okaya_480x272)
#define DEF_P	okaya_480x272_P
#define __DEF_DISPLAY_INDEX	okaya_480x272
#elif (DISPLAY_TYPE==urt_480x272)
#define DEF_P	urt_480x272_P
#define __DEF_DISPLAY_INDEX	urt_480x272
#elif (DISPLAY_TYPE==hitachi_hvga)		//6.2 inch display
#define DEF_P	hitachi_hvga_P
#define __DEF_DISPLAY_INDEX	hitachi_hvga
#elif (DISPLAY_TYPE==hitachi_wvga)		//7 or 9 inch display
#define DEF_P	hitachi_wvga_P
#define __DEF_DISPLAY_INDEX	hitachi_wvga
#elif (DISPLAY_TYPE==sharp_vga)		//10.4 inch display
#define DEF_P	sharp_vga_P
#define __DEF_DISPLAY_INDEX	sharp_vga
#elif (DISPLAY_TYPE==qvga_portrait)		//3.5 inch display
#define DEF_P	qvga_portrait_P
#define __DEF_DISPLAY_INDEX	qvga_portrait
#elif (DISPLAY_TYPE==lcd_svga)
#define DEF_P	lcd_svga_P
#define __DEF_DISPLAY_INDEX	lcd_svga
#elif (DISPLAY_TYPE==crt800x600)
#define DEF_P	crt800x600_P
#define __DEF_DISPLAY_INDEX	crt800x600
#elif (DISPLAY_TYPE==crt1024x768)
#define DEF_P	crt1024x768_P
#define __DEF_DISPLAY_INDEX	crt1024x768
#elif (DISPLAY_TYPE==hitachi_92)
#define DEF_P	hitachi_92_P
#define __DEF_DISPLAY_INDEX	hitachi_92
#elif (DISPLAY_TYPE==tovis_w)
#define DEF_P	tovis_w_P
#define __DEF_DISPLAY_INDEX	tovis_w
#elif (DISPLAY_TYPE==DP480X320)		//5.7 inch display
#define DEF_P	DP480X320_P
#define __DEF_DISPLAY_INDEX	DP480X320
#elif (DISPLAY_TYPE==DP320X240)	//5.7 inch display
#define DEF_P	DP320X240_P
#define __DEF_DISPLAY_INDEX	DP320X240
#elif (DISPLAY_TYPE==DL122X32)
#define DEF_P	DL122X32_P
#define __DEF_DISPLAY_INDEX	DL122X32
#elif (DISPLAY_TYPE==UNKNOWN)
#define DEF_P	sharp_qvga_P
#define __DEF_DISPLAY_INDEX	UNKNOWN
#endif

#ifndef __DEF_DISPLAY_INDEX
#define DEF_P	hitachi_qvga_P
#define __DEF_DISPLAY_INDEX	0
#warning "No display selected, defaulting to hitachi_qvga"
#endif

#define DEF_DISPLAY_INDEX __DEF_DISPLAY_INDEX

#ifndef __ARMASM
#define GRX_RES(xres,xsyncWidth,xbegin,xend, yres,ysyncWidth,ybegin,yend, fPolarity,lPolarity,pPolarity,dPolarity, enable,unscramble,rotate,active,bpp,vSyncHz,type) xres
#define GRY_RES(xres,xsyncWidth,xbegin,xend, yres,ysyncWidth,ybegin,yend, fPolarity,lPolarity,pPolarity,dPolarity, enable,unscramble,rotate,active,bpp,vSyncHz,type) yres
#define CALL(a,b) a(b)
#define SCREEN_WIDTH CALL(GRX_RES,DEF_P)
#define SCREEN_HEIGHT CALL(GRY_RES,DEF_P)
#endif
