/*
 * Choose one of the following:
 *       
 *    hitachi_qvga
 *    sharp_qvga
 *    hitachi_hvga
 *    sharp_vga
 *    hitachi_wvga    -    7 or 9 inch
 */
#ifndef HITACHI_QVGA
#define HITACHI_QVGA	0
#define SHARP_QVGA		1
#define DA320X240		2
#define OKAYA_QVGA		3
#define OKAYA_480X272	4
#define DA640X240		5
#define DA800X480		6
#define DA640X480		7
#define DA240X320		8
#define lcd_svga		9
#define DA800X600		10
#define DA1024X768		11
#define HITACHI_92		12
#define tovis_w			13
#define DP480X320		14
#define DP320X240		15
#define DL122X32		16
#endif

#if DISPLAY_TYPE == HITACHI_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_qvga" "\0"
#elif DISPLAY_TYPE == SHARP_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_qvga" "\0"
#elif DISPLAY_TYPE == DA320X240
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_qvga" "\0"
#elif DISPLAY_TYPE == OKAYA_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=okaya_qvga" "\0"
#elif DISPLAY_TYPE == OKAYA_480X272
#define CONFIG_EXTRA_ENV_SETTINGS "panel=okaya_480X272" "\0"
#elif DISPLAY_TYPE == DA640X240
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_hvga" "\0"
#elif DISPLAY_TYPE == DA240X320
#define CONFIG_EXTRA_ENV_SETTINGS "panel=qvga_portrait" "\0"
#elif DISPLAY_TYPE == DA640X480
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_vga" "\0"
#elif DISPLAY_TYPE == DA800X480
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_wvga" "\0"
#elif DISPLAY_TYPE == lcd_svga
#define CONFIG_EXTRA_ENV_SETTINGS "panel=lcd_svga" "\0"
#elif DISPLAY_TYPE == DA1024X768
#define CONFIG_EXTRA_ENV_SETTINGS "panel=crt1024x768" "\0"
#elif DISPLAY_TYPE == HITACHI_92
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_92" "\0"
#elif DISPLAY_TYPE == tovis_w
#define CONFIG_EXTRA_ENV_SETTINGS "panel=tovis_w" "\0"
#else
#error No display selected
#endif
