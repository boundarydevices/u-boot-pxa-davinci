/*
 * Choose one of the following:
 *       
 *    hitachi_qvga
 *    sharp_qvga
 *    hitachi_hvga
 *    sharp_vga
 *    hitachi_wvga    -    7 or 9 inch
 */
#define str(s) #s
#define xstr(s) str(s)

#ifdef DISPLAY_TYPE
#define CONFIG_EXTRA_ENV_SETTINGS "panel=" xstr(DISPLAY_TYPE) "\0"
#else
#error No display selected
#endif

