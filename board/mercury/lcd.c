#include <common.h>
#include <config.h>
#include <common.h>
#include <version.h>
#include <stdarg.h>
#include <linux/types.h>
#include <devices.h>
#include <lcd.h>
#include <configs/select.h>
#include <asm/lcdPanels2.h>

int lcd_color_fg;
int lcd_color_bg;

void *lcd_base;			/* Start of framebuffer memory	*/
void *lcd_console_address;		/* Start of console buffer	*/

short console_col;
short console_row;

vidinfo_t panel_info = {
	vl_col:		SCREEN_WIDTH,
	vl_row:		SCREEN_HEIGHT,
	vl_bpix:    LCD_BPP,
	vl_lcd_line_length: (SCREEN_WIDTH * NBITS(LCD_BPP) ) >> 3
};

#define IPU_BASE 0x53fc0000
#define IPU_IMA_ADDR 0x20	//indirect internal memory access address
#define IPU_IMA_DATA 0x24

void lcd_setcolreg (ushort regno, ushort red, ushort green, ushort blue)
{
	unsigned long *const ipuBase = (unsigned long *)IPU_BASE ;
	unsigned long const rgb = (((unsigned long)red ) << 24)
		| (((unsigned long)green ) << 16)
		| (((unsigned long)blue) <<8) ;
//	printf( "palette set 0x%02x = 0x%8x\n",regno,rgb);
	ipuBase[IPU_IMA_ADDR>>2] = 0x20000+(regno<<3) ;
	ipuBase[IPU_IMA_DATA>>2] = rgb ;
}
//each entry is in ARGB format, alpha high byte, blue low byte
void lcd_SetPalette(ulong* palette,int colorCnt)
{
	unsigned long *const ipuBase = (unsigned long *)IPU_BASE ;
	ipuBase[IPU_IMA_ADDR>>2] = 0x20000;
	while (colorCnt--) {
		ulong tmp = *palette++;
		ipuBase[IPU_IMA_DATA>>2] = (tmp<<8)|(tmp>>24);	//put into RGBA format
	}
}

void lcd_enable	(void)
{
}

void lcd_disable(void)
{
}
void lcd_panel_disable(void)
{
}
void _lcd_ctrl_init(void *lcdbase);

void lcd_ctrl_init(void *lcdbase)
{
	printf( "lcd_ctrl_init, framebuffer:%p %ix%ix%i\n",lcdbase,panel_info.vl_col,panel_info.vl_row,NBITS(LCD_BPP));
	lcd_base = lcdbase;
	_lcd_ctrl_init(lcdbase);
//	printf( "lcd init done\n");
}