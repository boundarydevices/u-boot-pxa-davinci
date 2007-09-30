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

void addPanel(struct lcd_t *lcd)
{
   if( MAX_PANELS > getPanelCount() ){
      panels_[numPanels_++] = lcd ;
   }
}


/************************************************************************/
/*----------------------------------------------------------------------*/
static void console_scrollup( struct lcd_t *lcd )
{
	/* Copy up rows ignoring the first one */
   unsigned lineSize = lcd->info.xres ;
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
   	dest = (uchar *)lcd->fbAddr + (lcd->info.xres*VIDEO_FONT_HEIGHT*lcd->y) + lcd->x*VIDEO_FONT_WIDTH ;
   	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += lcd->info.xres ){
         memset(dest,lcd->bg,VIDEO_FONT_WIDTH);
      }
   }
}

static void lcd_drawchar8( struct lcd_t *lcd, char c )
{
	uchar *dest;
	ushort row;
   uchar *src = video_fontdata + c * VIDEO_FONT_HEIGHT ;

	dest = (uchar *)lcd->fbAddr + (lcd->info.xres*VIDEO_FONT_HEIGHT*lcd->y) + lcd->x*VIDEO_FONT_WIDTH ;
	for (row=0;  row < VIDEO_FONT_HEIGHT;  ++row, dest += lcd->info.xres, src++ ){
      uchar mask = '\x80' ;
      uchar bits = *src ;
      uchar *d = dest ;
      while( mask ){
         *d++ = (bits & mask) ? lcd->fg : lcd->bg ;
         mask >>= 1 ;
      }
	}
}

void lcd_putc (const char c)
{
   struct lcd_t *p = getPanel(getCurrentPanel());
   unsigned maxCols ;
	if (!p) {
		serial_putc(c);
		return;
	}

   maxCols = (p->info.xres/VIDEO_FONT_WIDTH);

	switch (c) {
	case '\r':	p->x = 0;
			break;

	case '\n':	console_newline(p);
			break;

	case '\t':	/* Tab (8 chars alignment) */
			p->x +=  8;
			p->x &= ~7;

			if (p->x >= maxCols) {
				console_newline(p);
			}
			break;

	case '\b':	
         console_back(p);
			break;

   case ' ' :
         
	default:	lcd_drawchar8(p, c);
			if (++p->x >= maxCols) {
				console_newline(p);
			}
			break;
	}
}

void lcd_puts (const char *s)
{
   if( s ){
      while( *s ){
         lcd_putc(*s++ );
      }
   }
}

int lcd_ClearScreen(void)
{
   struct lcd_t *p = getPanel(getCurrentPanel());
   if( p && p->fbAddr )
   memset( (char *)p->fbAddr, p->fbMemSize, p->bg );

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

static void addOne( struct lcd_panel_info_t const *panel )
{
	struct lcd_t *lcd ;
	print_panel_info( panel );
	lcd = newPanel(panel);
	if( lcd ){
		addPanel(lcd);
	}
}

int lcd_multi_init(void)
{
   unsigned i = 0 ;
   char *panelName = getenv( "panel" );
   if( panelName ){
      int found = 0 ;
      char *start = panelName = strdup(panelName);
      do {
         struct lcd_panel_info_t const *panel ;
         char *cur = panelName ;
         panelName = strchr(panelName,',');
         if( 0 != panelName )
            *panelName = '\0' ;
         panel = find_lcd_panel( cur );
         if( panel ) {
            addOne( panel );
	    found = 1 ;
            i++ ;
            setCurrentPanel(i);
         }
         if( panelName )
            *panelName++ = ',' ;
      } while( panelName );

      if( !found ){
	      struct lcd_panel_info_t panel_info ;
	      if( parse_panel_info( start, &panel_info ) ){
                      addOne( &panel_info );
		      if( getPanelCount() )
                         setCurrentPanel(getPanelCount()-1);
	      }
      }
      free(start);

      for( i = 0 ; i < getPanelCount(); i++ )
      {
         char cTemp[20];
         struct lcd_t *lcd = getPanel(i);
         setCurrentPanel(i);
         
         sprintf( cTemp, "panel %u\n  ", i );
         lcd_puts(cTemp);

         lcd_puts(lcd->info.name);
         sprintf( cTemp, "\n  %u x %u", lcd->info.xres, lcd->info.yres );
         lcd_puts(cTemp);
      }

   } // panel defined
   return 0 ;
}

#endif
