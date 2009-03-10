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
   printf( "oepol_actl     : %u\n", panel->oepol_actl );
   printf( "hsync_len      : %u\n", panel->hsync_len );
   printf( "left_margin    : %u\n", panel->left_margin );
   printf( "right_margin   : %u\n", panel->right_margin );
   printf( "vsync_len      : %u\n", panel->vsync_len );
   printf( "upper_margin   : %u\n", panel->upper_margin );
   printf( "lower_margin   : %u\n", panel->lower_margin );
   printf( "active         : %u\n", panel->active );
   printf( "CRT            ? %u\n", panel->crt);
}

static void short_panel_info( struct lcd_panel_info_t const *panel )
{
   printf( "%4u\t%4u\t %c \t%s\n",
           panel->xres, panel->yres,
           panel->crt ? 'Y' : 'N',
           panel->name );
}
struct FieldData
{
	char* name;
	unsigned offset;
#define TYPE_ULONG 0
#define TYPE_USHORT_NOT_ZERO 1
#define TYPE_USHORT 2
#define TYPE_FLAG 3
#define TYPE_STRING 4
	unsigned char type;
};
#define OFFSETOF(a) ((unsigned)(&((struct lcd_panel_info_t*)NULL)->a))

struct FieldData fields[] = {
	{"name",	OFFSETOF(name),		TYPE_STRING},
	{"pixclock",	OFFSETOF(pixclock),	TYPE_ULONG},
	{"xres",	OFFSETOF(xres),		TYPE_USHORT_NOT_ZERO},
	{"yres",	OFFSETOF(yres),		TYPE_USHORT_NOT_ZERO},
	{"pclk_redg",	OFFSETOF(pclk_redg),	TYPE_FLAG},
	{"hsyn_acth",	OFFSETOF(hsyn_acth),	TYPE_FLAG},
	{"vsyn_acth",	OFFSETOF(vsyn_acth),	TYPE_FLAG},
	{"oepol_actl",	OFFSETOF(oepol_actl),	TYPE_FLAG},
	{"hsync_len",	OFFSETOF(hsync_len),	TYPE_USHORT_NOT_ZERO},
	{"left_margin",	OFFSETOF(left_margin),	TYPE_USHORT},
	{"right_margin",OFFSETOF(right_margin),	TYPE_USHORT},
	{"vsync_len",	OFFSETOF(vsync_len),	TYPE_USHORT},
	{"upper_margin",OFFSETOF(upper_margin),	TYPE_USHORT},
	{"lower_margin",OFFSETOF(lower_margin),	TYPE_USHORT},
	{"active (0|1)",OFFSETOF(active),	TYPE_FLAG},
	{"crt (0|1)",	OFFSETOF(crt),		TYPE_FLAG},
	{NULL, 0, 0},
};

static struct lcd_panel_info_t const *prompt_for_panel( void )
{
	struct lcd_panel_info_t *panel = (struct lcd_panel_info_t *)malloc(sizeof(struct lcd_panel_info_t));
	unsigned char * panel_byte = (unsigned char *)panel;
	int bytesRead ;
	struct FieldData* pF = fields;
	memset( panel, 0, sizeof(*panel));

	while (pF->name) {
		printf("%s",pF->name);
		bytesRead = readline( ": " );
		if (0 < bytesRead) {
			if (pF->type==TYPE_STRING) {
				*((const char**)(panel_byte+pF->offset)) = strdup(console_buffer);
			} else {
				char *endp;
				ulong value = simple_strtoul( console_buffer, &endp, 0 );
				if (endp <= console_buffer) {
					printf("%s is not an integer\n",pF->name);
					continue;
				}
				if (pF->type==TYPE_USHORT_NOT_ZERO){
					if (value==0) {
						printf("%s cannot be 0\n",pF->name);
						continue;
					}
				}
				if ((pF->type==TYPE_USHORT_NOT_ZERO)||(pF->type==TYPE_USHORT)) {
					if (value>=0x10000) {
						printf("%s value %i is too big\n",pF->name,value);
						continue;
					}
					*((unsigned short*)(panel_byte+pF->offset)) = value ;
				} else if (pF->type==TYPE_ULONG) {
					*((unsigned long*)(panel_byte+pF->offset)) = value ;
				} else if (pF->type==TYPE_FLAG) {
					if (value > 1) {
						printf("%s can only be 0 or 1\n",pF->name);
						continue;
					}
					*((unsigned char*)(panel_byte+pF->offset)) = value;
				}
			}
			pF++;
		} else {
			free(panel);
			return 0 ;
		}
	}
	return panel ;
}

void build_panel_name(char* buffer, struct lcd_panel_info_t const *panel)
{
   sprintf(buffer, "%s:%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u"
           , panel->name
           , panel->pixclock
           , panel->xres
           , panel->yres
           , panel->pclk_redg
           , panel->hsyn_acth
           , panel->vsyn_acth
           , panel->oepol_actl
           , panel->hsync_len
           , panel->left_margin
           , panel->right_margin
           , panel->vsync_len
           , panel->upper_margin
           , panel->lower_margin
           , panel->active
           , panel->crt );
}

#if defined(CONFIG_LCD) || defined(CONFIG_LCD_MULTI)

int set_p(struct lcd_panel_info_t const *panel)
{
	char panelname[512];
	build_panel_name(panelname, panel);
#if defined(CONFIG_LCD_MULTI)
	if (addPanel(panel, panelname))
		return 1;
	printf( "error from addPanel()\n" );
	return 0;
#else
	print_panel_info( panel );
	set_lcd_panel(panel);
	lcd_puts(panelname);
	return 1;
#endif
}

char* find_set_panel(char* next, int* pmatched)
{
	int matched = 0;
	struct lcd_panel_info_t const *panel ;
	char *cur = next ;
	next = strchr(next,'&');
	if (next)
		*next = '\0' ;
	panel = find_lcd_panel( cur );
	if (panel) {
		printf( "panel %s found: %u x %u\n", panel->name, panel->xres, panel->yres );
		if (set_p(panel))
			matched++;
	} else if (strchr( cur, ':') ) {
		struct lcd_panel_info_t *newP;
//		printf( "parse LCD panel <%s> here\n", cur );
		newP = (struct lcd_panel_info_t *)malloc( sizeof(struct lcd_panel_info_t) );
		if (parse_panel_info( cur, newP) ){
			if (set_p(newP)){
				matched++;
			}
		} else {
			printf( "Error parsing panel\n" );
			free(newP);
		}
	} else {
		char const *rv = strchr( cur, ':' );
		printf( "panel <%s> not found (%p)\n", cur, rv );
	}
	if (next)
		*next++ = '&' ;
	if (pmatched) *pmatched = matched;
	return next;
}

void disable_lcd_panel(void);


static int lcdpanel(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_LCD_MULTI)
	printf( "handle Multi-panel lcdpanel command here\n" );
#endif
	if ( 1 == argc ) {
#if defined(CONFIG_LCD_MULTI)
		unsigned count = 0 ;
		unsigned i ;
		for( i = 0 ; i < getPanelCount(); i++ ) {
			struct lcd_t *lcd = getPanel(i);
			if(lcd){
				print_panel_info(&lcd->info);
				count++ ;
			}
		}
		printf( "%u panel(s) defined\n", count );
#else
		if( cur_lcd_panel )
			print_panel_info( cur_lcd_panel );
		else
			printf( "no panel defined\n" );
#endif
	} else if( '+' == *argv[1] ) {
		struct lcd_panel_info_t const *panel = prompt_for_panel();
		if( panel ) {
			char panelname[512];
			build_panel_name(panelname, panel);
#if defined(CONFIG_LCD_MULTI)
			if (addPanel(panel, panelname))
				setenv( "panel", panelname );
#else
			print_panel_info( panel );
			set_lcd_panel( panel );
			setenv( "panel", panelname );
			lcd_puts(panelname);
#endif
		}
	} else if( '?' == *argv[1] ) {
		int i ;
		for( i = 0 ; i < num_lcd_panels ; i++ )
			print_panel_info( lcd_panels+i );
	} else if( '*' == *argv[1] ) {
		int i ;
		printf( "xres\tyres\tCRT\tname\n" );
		for( i = 0 ; i < num_lcd_panels ; i++ )
			short_panel_info( lcd_panels+i );
	} else if( '-' == *argv[1] ) {
		disable_lcd_panel();
		printf( "panel disabled\n" );
	} else {
		int matched = 0 ;
		char *next = argv[1];
#if defined(CONFIG_LCD_MULTI)
		disablePanels();
		do {
			next = find_set_panel(next,&matched);
		} while (next);
#else
		next = find_set_panel(next,&matched);
#endif
		if (matched){
#if defined(CONFIG_LCD_MULTI)
			setCurrentPanel(0);
#endif
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

