/*
 * Allow inversion of HSYNC, VSYNC, PIXCLK polarities through the
 * environment variable 'vclkinv' (short for video clock invert)
 *
 * (C) Copyright 2009
 * Boundary Devices
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "lcd_panels.h"
#include <common.h>
#include <command.h>
#include "exports.h"

#define UPCASE(c) ((c)&~0x20)

struct lcd_panel_info_t *fixup_lcd_panel(unsigned panel_idx, struct lcd_panel_info_t const *panel)
{
	struct lcd_panel_info_t *rval = (struct lcd_panel_info_t *)panel ;
	char *clkinv_env = getenv("vclkinv");
	if( clkinv_env ){
		unsigned idx ;
		for( idx = 0 ; clkinv_env && (idx < panel_idx) ; idx++ ){
			clkinv_env = strchr(clkinv_env, ',');
		}
		if( clkinv_env ){
			rval = (struct lcd_panel_info_t *)malloc(sizeof(struct lcd_panel_info_t));
			memcpy(rval,panel,sizeof(*rval));
			do {
				char c = UPCASE(*clkinv_env);
				if( 'P' == c ){
					rval->pclk_redg ^= 1 ;
				} else if( 'H' == c ){
					rval->hsyn_acth ^= 1 ;
				} else if( 'V' == c ){
					rval->vsyn_acth ^= 1 ;
				} else if( (',' == c ) || ('\0' == c) )
					break;
				clkinv_env++ ;
			} while( 1 );
		}
	}
	return rval ;
}
