/*
 * Module cmd_lcdpanel.cpp
 *
 * This module defines the "lcdpanel" command for use in 
 * defining the set of displays (LCD panels) attached to a 
 * board.
 *
 * Change History : 
 *
 * $Log: cmd_lcdpanel.c,v $
 * Revision 1.12  2007/01/15 19:51:10  ericn
 * -rename pclk_acth to pclk_redg
 *
 * Revision 1.11  2007/01/15 00:11:47  ericn
 * -separate vertical/horizontal sync from pixel clock polarity
 *
 * Revision 1.10  2007/01/14 14:27:45  ericn
 * -support direct lcdp name:value
 *
 * Revision 1.9  2006/12/12 00:23:26  ericn
 * -make print_panel_info public
 *
 * Revision 1.8  2005/09/19 13:15:59  ericn
 * -allow zeros in most fields
 *
 * Revision 1.7  2005/08/22 16:30:32  ericn
 * -update panel env var w/lcdp command
 *
 * Revision 1.6  2005/07/18 03:05:53  ericn
 * -allow cmdline config of CRT
 *
 * Revision 1.5  2005/07/06 05:26:54  ericn
 * -make lcdinfo command conditional on PXALCD
 *
 * Revision 1.4  2005/07/04 18:49:01  ericn
 * -added lcdi command
 *
 * Revision 1.3  2005/06/02 04:01:30  ericn
 * -allow zero value of pixclock (meaning slow one)
 *
 * Revision 1.2  2005/04/30 20:32:44  ericn
 * -added disable cmd
 *
 * Revision 1.1  2005/04/09 17:49:15  ericn
 * -Initial import
 *
 *
 * Copyright Boundary Devices, Inc. 2005
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <command.h>

#if defined( CONFIG_LCDPANEL ) && (defined(CONFIG_LCD)||defined(CONFIG_LCD_MULTI))

#include <malloc.h>
#include <lcd_panels.h>

#ifdef CONFIG_LCD_MULTI
#include <lcd_multi.h>
#endif

extern char console_buffer[];		/* console I/O buffer	*/

void print_panel_info( struct lcd_panel_info_t const *panel )
{
   printf( "------------------------------------\n"
           "name           : %s\n", panel->name );
   printf( "pixclock       : %u\n", panel->pixclock );
   printf( "xres           : %u\n", panel->xres );
   printf( "yres           : %u\n", panel->yres );
   printf( "pclk_redg      : %u\n", panel->pclk_redg );
   printf( "hsyn_acth      : %u\n", panel->hsyn_acth );
   printf( "vsyn_acth      : %u\n", panel->vsyn_acth );
   printf( "hsync_len      : %u\n", panel->hsync_len );
   printf( "left_margin    : %u\n", panel->left_margin );
   printf( "right_margin   : %u\n", panel->right_margin );
   printf( "vsync_len      : %u\n", panel->vsync_len );
   printf( "upper_margin   : %u\n", panel->upper_margin );
   printf( "lower_margin   : %u\n", panel->lower_margin );
   printf( "active         : %u\n", panel->active );
   printf( "CRT            ? %s\n", panel->crt ? "Yes" : "No" );
}

static void short_panel_info( struct lcd_panel_info_t const *panel )
{
   printf( "%4u\t%4u\t %c \t%s\n", 
           panel->xres, panel->yres,
           panel->crt ? 'Y' : 'N',
           panel->name );
}

static struct lcd_panel_info_t const *prompt_for_panel( void )
{
   struct lcd_panel_info_t *panel = (struct lcd_panel_info_t *)malloc(sizeof(struct lcd_panel_info_t));
   int bytesRead ;

   memset( panel, 0, sizeof(*panel));

   bytesRead = readline( "name: " );
   if( 0 < bytesRead )
   {
      panel->name = strdup( console_buffer );
      bytesRead = readline( "pixclock: " );
      if( 0 < bytesRead )
      {
         char *endp;
         ulong value = simple_strtoul( console_buffer, &endp, 0 );
         if( endp > console_buffer )
         {
            panel->pixclock = value ;
            bytesRead = readline( "xres: " );
            if( ( 0 < bytesRead ) 
                && ( 0 != ( value = simple_strtoul( console_buffer, &endp, 0 ) ) ) 
                && ( endp > console_buffer ) )
            {
               panel->xres = value ;
               bytesRead = readline( "yres: " );
               if( ( 0 < bytesRead ) 
                   && ( 0 != ( value = simple_strtoul( console_buffer, &endp, 0 ) ) )
                   && ( endp > console_buffer ) )
               {
                  panel->yres = value ;
                  bytesRead = readline( "pclk_redg: " );
                  if( ( 0 < bytesRead ) 
                      && ( 1 >= ( value = simple_strtoul( console_buffer, &endp, 0 ) ) )
                      && ( endp > console_buffer ) )
                  {
                     panel->pclk_redg = value ;
                     bytesRead = readline( "hsyn_acth: " );
                     if( ( 0 < bytesRead ) 
                         && ( 1 >= ( value = simple_strtoul( console_buffer, &endp, 0 ) ) )
                         && ( endp > console_buffer ) )
                     {
                        panel->hsyn_acth = value ;
                        bytesRead = readline( "vsyn_acth: " );
                        if( ( 0 < bytesRead ) 
                            && ( 1 >= ( value = simple_strtoul( console_buffer, &endp, 0 ) ) )
                            && ( endp > console_buffer ) )
                        {
                           panel->vsyn_acth = value ;
		                   bytesRead = readline( "hsync_len: " );
        		           if( ( 0 < bytesRead ) 
	                           && ( 0 != ( value = simple_strtoul( console_buffer, &endp, 0 ) ) )
      		                   && ( endp > console_buffer ) )
            		       {
                    		  panel->hsync_len = value ;
                        	  bytesRead = readline( "left_margin: " );
	                          value = simple_strtoul( console_buffer, &endp, 0 );
        		              if( ( 0 < bytesRead ) 
                	              && ( endp > console_buffer ) )
                      		  {
		                         panel->left_margin = value ;
        		                 bytesRead = readline( "right_margin: " );
                	             value = simple_strtoul( console_buffer, &endp, 0 );
                      		     if( ( 0 < bytesRead ) 
                            		 && ( endp > console_buffer ) )
		                         {
        		                    panel->right_margin = value ;
		                            bytesRead = readline( "vsync_len: " );
		                            value = simple_strtoul( console_buffer, &endp, 0 );
		                            if( ( 0 < bytesRead ) 
		                                && ( endp > console_buffer ) )
		                            {
		                               panel->vsync_len = value ;
		                               bytesRead = readline( "upper_margin: " );
		                               value = simple_strtoul( console_buffer, &endp, 0 );
		                               if( ( 0 < bytesRead ) 
		                                   && ( endp > console_buffer ) )
		                               {
		                                  panel->upper_margin = value ;
		                                  bytesRead = readline( "lower_margin: " );
		                                  value = simple_strtoul( console_buffer, &endp, 0 );
		                                  if( ( 0 < bytesRead ) 
		                                      && ( endp > console_buffer ) )
		                                  {
		                                     panel->lower_margin = value ;
		                                     bytesRead = readline( "active (0|1) : " );
		                       		         value = simple_strtoul( console_buffer, &endp, 0 );
		                                     if( ( 0 < bytesRead ) && ( endp > console_buffer ) )
		                                     {
		                                        panel->active = value ;
		                                        bytesRead = readline( "crt (0|1) : " );
    		                                    value = simple_strtoul( console_buffer, &endp, 0 );
		                                        if( ( 0 < bytesRead ) 
		                                            && 
		                                            ( endp > console_buffer ) )
		                                        {
		                                           panel->crt = value ;
		                                           print_panel_info( panel );
		                                           return panel ;
		                                        }
		                                     }
                                          }
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }

   free( panel );

   return 0 ;
}

static char *build_panel_name(struct lcd_panel_info_t const *panel)
{
   char tempBuf[512];
   sprintf( tempBuf, "%s:%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u"
           , panel->name
           , panel->pixclock
           , panel->xres
           , panel->yres
           , panel->pclk_redg
           , panel->hsyn_acth
           , panel->vsyn_acth
           , panel->hsync_len
           , panel->left_margin
           , panel->right_margin
           , panel->vsync_len
           , panel->upper_margin
           , panel->lower_margin
           , panel->active
           , panel->crt );
   return strdup(tempBuf);
}

#ifdef CONFIG_LCD
static int lcdpanel(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if ( 1 == argc ) {
      if( cur_lcd_panel )
         print_panel_info( cur_lcd_panel );
      else
         printf( "no panel defined\n" );
	}
   else {
      struct lcd_panel_info_t const *panel = find_lcd_panel( argv[1] );
      if( panel ) {
         printf( "found panel %s\n", panel->name );
         set_lcd_panel( panel );
         setenv( "panel", (char*)panel->name );
      }
      else if( '+' == *argv[1] ) {
         panel = prompt_for_panel();
         if( panel )
         {
            print_panel_info( panel );
            set_lcd_panel( panel );
            setenv( "panel", build_panel_name(panel) );
         }
      }
      else if( '?' == *argv[1] )
      {
         int i ; 
         for( i = 0 ; i < num_lcd_panels ; i++ )
            print_panel_info( lcd_panels+i );
      }
      else if( '*' == *argv[1] )
      {
         int i ; 
         printf( "xres\tyres\tCRT\tname\n" );
      
         for( i = 0 ; i < num_lcd_panels ; i++ )
            short_panel_info( lcd_panels+i );
      }
      else if( '-' == *argv[1] )
      {
         disable_lcd_panel();
         printf( "panel disabled\n" );
      }
      else if( strchr( argv[1], ':' ) ){
         printf( "parse LCD panel <%s> here\n", argv[1] );
         struct lcd_panel_info_t *const newPanel = (struct lcd_panel_info_t *)malloc( sizeof(struct lcd_panel_info_t) );
         if( parse_panel_info( argv[1], newPanel ) ){
            print_panel_info( newPanel );
            set_lcd_panel( newPanel );
            setenv( "panel", build_panel_name(panel));
         }
         else {
            printf( "Error parsing panel\n" );
            free(newPanel);
         }
      }
      else {
         char const *rv = strchr( argv[1], ':' );
         printf( "panel <%s> not found (%p)\n", argv[1], rv );
      }
   }
   
	return 0;
}
#elif defined(CONFIG_LCD_MULTI)

static int lcdpanel(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   printf( "handle Multi-panel lcdpanel command here\n" );
	if ( 1 == argc ) {
      unsigned count = 0 ;
      unsigned i ;
      for( i = 0 ; i < getPanelCount(); i++ ){
         struct lcd_t *lcd = getPanel(i);
         if(lcd){
            print_panel_info(&lcd->info);
            count++ ;
         }
      }
      printf( "%u panel(s) defined\n", count );
	}
   else {
      unsigned matched = 0 ;
      char *next = argv[1];
      disablePanels();

      do {
         struct lcd_panel_info_t const *panel ;
         char *cur = next ;
         next = strchr(next,',');
         if( 0 != next )
            *next = '\0' ;
         panel = find_lcd_panel( cur );
         if( panel ) {
            struct lcd_t *lcd ;
            print_panel_info( panel );
            lcd = newPanel(panel);
            if( lcd ){
               addPanel(lcd);
               matched++ ;
            }
         }
         else
            printf( "panel %s not found\n", cur );
         if( next )
            *next++ = ',' ;
      } while( next );

      if( 0 == matched ){
         if( '+' == *argv[1] ) {
            struct lcd_panel_info_t const *panel = prompt_for_panel();
            if( panel )
            {
               struct lcd_t *lcd ;
               print_panel_info( panel );
               lcd = newPanel(panel);
               if( lcd ){
                  addPanel(lcd);
                  setenv( "panel", build_panel_name(panel) );
               }
            }
         }
         else if( '?' == *argv[1] )
         {
            int i ; 
            for( i = 0 ; i < num_lcd_panels ; i++ )
               print_panel_info( lcd_panels+i );
         }
         else if( '*' == *argv[1] )
         {
            int i ; 
            printf( "xres\tyres\tCRT\tname\n" );
         
            for( i = 0 ; i < num_lcd_panels ; i++ )
               short_panel_info( lcd_panels+i );
         }
         else if( strchr( argv[1], ':' ) ){
            printf( "parse LCD panel <%s> here\n", argv[1] );
            struct lcd_panel_info_t *const panel = (struct lcd_panel_info_t *)malloc( sizeof(struct lcd_panel_info_t) );
            if( parse_panel_info( argv[1], panel ) ){
               struct lcd_t *lcd ;
               print_panel_info( panel );
               lcd = newPanel(panel);
               if( lcd ){
                  char *panelName ; 
                  addPanel(lcd);
                  setCurrentPanel(0);
                  panelName = build_panel_name(panel);
                  setenv( "panel", panelName );
               }
               else
                  printf( "error from newPanel()\n" );
            }
            else {
               printf( "Error parsing panel\n" );
               free(newPanel);
            }
         }
         else {
            char const *rv = strchr( argv[1], ':' );
            printf( "panel <%s> not found (%p)\n", argv[1], rv );
         }
      } // nothing matched
      else {
         setenv( "panel", argv[1] );
      }
   }
   
	return 0;
}

#endif

U_BOOT_CMD(
	lcdpanel,	10,	0,	lcdpanel,
	"lcdpanel [panelName|?|+|-]\n"
   "     init lcd panel with panel name\n"
   "     ? will display the supported panels\n"
   "     + will prompt for panel details\n"
#ifdef CONFIG_LCD
   "     - will disable the panel\n"
#endif
   , NULL
);

#endif	/* CONFIG_LCDPANEL */




