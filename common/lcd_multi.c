/*
 * Module lcd_multi.c
 *
 * This module defines the multi-panel interface
 * routines declared in lcd_multi.h
 *
 *
 * Change History :
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include <config.h>
#ifdef CONFIG_LCD_MULTI

#include "lcd_multi.h"
#include <common.h>
#include <exports.h>
#include <command.h>
#include <video_font.h>		/* Get font data, width and height	*/

#define MAX_PANELS 3

static unsigned curPanel_ = 0 ;
static unsigned numPanels_ = 0 ;
static struct lcd_t *panels_[MAX_PANELS] = {
   0
};

unsigned getPanelCount( void )
{
   return numPanels_ ;
}

struct lcd_t *getPanel( unsigned idx )
{
   if( idx < getPanelCount() )
      return panels_[idx];
   else
      return 0 ;
}

unsigned getCurrentPanel( void )
{
   return curPanel_ ;
}

void setCurrentPanel( unsigned idx )
{
   if( ( idx < getPanelCount() )
       &&
       ( 0 != panels_[idx] ) ){
      curPanel_ = idx ;
   }
}

void disablePanels( void )
{
   unsigned i ;
   for( i = 0 ; i < getPanelCount(); i++ ){
      struct lcd_t *p = panels_[i];
      if( p && p->disable )
         p->disable();
      panels_[i] = 0 ;
      free(p);
   }
   numPanels_ = curPanel_ = 0 ;
}

/************************************************************************/
/*----------------------------------------------------------------------*/
static void console_scrollup( struct lcd_t *lcd )
{
	/* Copy up rows ignoring the first one */
   unsigned lineSize = lcd->stride * VIDEO_FONT_HEIGHT;
	memcpy( lcd->fbAddr, (char *)lcd->fbAddr + lineSize, lcd->fbMemSize-lineSize);

	/* Clear the last one */
   memset( (char *)lcd->fbAddr + lcd->fbMemSize-lineSize, lcd->bg, lineSize );
}

/*----------------------------------------------------------------------*/
static inline void console_newline( struct lcd_t *lcd )
{
   unsigned maxRows = lcd->info.yres / VIDEO_FONT_HEIGHT ;
	lcd->x = 0;
	++lcd->y ;

	/* Check if we need to scroll the terminal */
	if (lcd->y >= maxRows) {
		/* Scroll everything up */
		console_scrollup(lcd) ;
		--lcd->y ;
	}
}

/*----------------------------------------------------------------------*/
static inline void console_back( struct lcd_t *lcd )
{
   if( lcd->x ){
      uchar *dest ;
	ushort row;

      lcd->x-- ;
	dest = (uchar *)lcd->fbAddr + (lcd->stride*VIDEO_FONT_HEIGHT*lcd->y) + lcd->x*VIDEO_FONT_WIDTH ;
	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += lcd->stride ){
         memset(dest,lcd->bg,VIDEO_FONT_WIDTH);
      }
   }
}

static int transparent_back = 0 ;
static int checked_transp = 0 ;

static void lcd_drawchar8( struct lcd_t *lcd, char c )
{
	uchar *dest;
	ushort row;
	uchar *src = video_fontdata + c * VIDEO_FONT_HEIGHT ;
	dest = (uchar *)lcd->fbAddr + (lcd->stride*VIDEO_FONT_HEIGHT*lcd->y) + lcd->x*VIDEO_FONT_WIDTH ;

        if( 0 == checked_transp ){
           transparent_back = (0 != getenv("transp_lcdtext") );
           checked_transp = 1 ;
        }
	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += lcd->stride, src++ ){
		uchar mask = '\x80' ;
		uchar bits = *src ;
		uchar *d = dest ;
		while( mask ){
			if(bits & mask)
				*d = lcd->fg ;
			else if(!transparent_back)
				*d = lcd->bg ;
			d++ ;
			mask >>= 1 ;
		}
	}
}
static void lcd_putc_panel(struct lcd_t *lcd, const char c)
{
   unsigned maxCols ;
	if (!lcd) {
		serial_putc(c);
		return;
	}

   maxCols = (lcd->info.xres/VIDEO_FONT_WIDTH);

	switch (c) {
	case '\r':	lcd->x = 0;
			break;

	case '\n':	console_newline(lcd);
			break;

	case '\t':	/* Tab (8 chars alignment) */
			lcd->x +=  8;
			lcd->x &= ~7;

			if (lcd->x >= maxCols) {
				console_newline(lcd);
			}
			break;

	case '\b':
         console_back(lcd);
			break;

   case ' ' :

	default:	lcd_drawchar8(lcd, c);
			if (++lcd->x >= maxCols) {
				console_newline(lcd);
			}
			break;
	}
}

static void lcd_puts_panel(struct lcd_t *lcd, const char *s)
{
	if( s ){
		while( *s ){
			lcd_putc_panel(lcd, *s++);
		}
	}
}

void lcd_putc (const char c)
{
	struct lcd_t *lcd = getPanel(getCurrentPanel());
	lcd_putc_panel(lcd, c);
}

void lcd_puts(const char *s)
{
	struct lcd_t *lcd = getPanel(getCurrentPanel());
	lcd_puts_panel(lcd, s);
}

int lcd_ClearScreen(void)
{
	struct lcd_t *lcd = getPanel(getCurrentPanel());
	if (lcd && lcd->fbAddr ) {
		memset( (char *)lcd->fbAddr, lcd->bg, lcd->fbMemSize);
		lcd->x = 0;
		lcd->y = 0;
	}
	return 0 ;
}

static int lcd_clear (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	return lcd_ClearScreen();
}

U_BOOT_CMD(
	cls,	1,	1,	lcd_clear,
	"cls     - clear screen\n",
	NULL
);

int do_curpanel(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if ( 1 == argc ) {
      unsigned i ;
      for( i = 0 ; i < getPanelCount(); i++ ){
         struct lcd_t *lcd = getPanel(i);
         if(lcd){
            char const c = ( i == getCurrentPanel() ) ? '*' : ' ' ;
            printf( "%c%u\t%4u\t%4u\t%s\n", c, i, lcd->info.xres, lcd->info.yres, lcd->info.name );
         }
      }
      return 0 ;
	} else if( 2 == argc ){
      unsigned long val = simple_strtoul( argv[1], 0, 0 );
      if( val < getPanelCount() ){
         setCurrentPanel(val);
         return do_curpanel(cmdtp,flag,1,argv);
      } else {
         printf( "invalid panel index\n" );
      }
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
	}
   return 1;
}

U_BOOT_CMD(
	curpanel,	2,	0,	do_curpanel,
	"curpanel - select current panel\n",
	"curpanel - show current panel\n"
	"curpanel N - set current panel to n\n"
);

struct lcd_t * addPanel(struct lcd_panel_info_t const *panel, const char *msg)
{
	struct lcd_t *lcd = NULL;
	int panel_num = getPanelCount();
	if (panel_num < MAX_PANELS) {
		lcd = newPanel(panel);
		if (lcd) {
			char buffer[32];
			panels_[panel_num] = lcd ;
			numPanels_++;

			sprintf(buffer, "panel %u\n", panel_num);
			lcd_puts_panel(lcd, buffer);
			if (msg) {
				lcd_puts_panel(lcd, msg);
				console_newline(lcd);
			}
			print_panel_info(panel);
		}
	}
	return lcd;
}

int lcd_multi_init(void)
{
   unsigned i = 0 ;
   char *panelName = getenv( "panel" );
   printf("panel env variable : %s\n\n", panelName);
   if( panelName ){
      do {
	 struct lcd_panel_info_t const *panel;
	 struct lcd_panel_info_t panel_info;
	 char *cur = panelName;
	 char panel_str[512];
         panelName = strchr(panelName,'&');
         if( panelName )
            *panelName = '\0';
         panel = find_lcd_panel( cur );
         if (!panel)
            if (strchr( cur, ':' ))
	       if (parse_panel_info(cur, &panel_info))
	          panel = &panel_info;
         if (panel) {
            build_panel_name(panel_str, panel);
            if (addPanel(panel, panel_str))
	       printf( "ADDED PANEL : %s\n", panel->name );
         } else {
            char const *rv = strchr( cur, ':' );
            printf( "panel <%s> not found (%p)\n", cur, rv );
         }
         printf( "\n" );
         if ( panelName )
             *panelName++ = '&';
      } while( panelName );
   } // panel defined
   return 0 ;
}

#endif
