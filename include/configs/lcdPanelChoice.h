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
#define HITACHI_QVGA	 0
#define SHARP_QVGA		 1
#define OKAYA_QVGA		 2
#define DA320X240		 3
#define DA640X240		 4
#define DA800X480		 5
#define DA640X480		 6
#define DA240X320		 7
#define DA800X600		 8
#define DA1024X768		 9
#define DP480X320		10
#define DP320X240		11
#define DL122X32		12
#endif

#if DISPLAY_TYPE == HITACHI_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_qvga" "\0"
#elif DISPLAY_TYPE == SHARP_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_qvga" "\0"
#elif DISPLAY_TYPE == OKAYA_QVGA
#define CONFIG_EXTRA_ENV_SETTINGS "panel=okaya_qvga" "\0"
#elif DISPLAY_TYPE == DA320X240
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_qvga" "\0"
#elif DISPLAY_TYPE == DA640X240
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_hvga" "\0"
#elif DISPLAY_TYPE == DA240X320
#define CONFIG_EXTRA_ENV_SETTINGS "panel=qvga_portrait" "\0"
#elif DISPLAY_TYPE == DA8000X480
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_wvga" "\0"
#elif DISPLAY_TYPE == DA640X480
#define CONFIG_EXTRA_ENV_SETTINGS "panel=sharp_vga" "\0"
#elif DISPLAY_TYPE == DA800X480
#define CONFIG_EXTRA_ENV_SETTINGS "panel=hitachi_wvga" "\0"
#elif DISPLAY_TYPE == DA1024X768
#define CONFIG_EXTRA_ENV_SETTINGS "panel=crt1024x768" "\0"
#else
#error No display selected
#endif
