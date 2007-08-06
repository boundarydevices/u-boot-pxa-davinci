#ifndef __SM501_BD_H__
#define __SM501_BD_H__
/*
 * Header file sm501_bd.h
 *
 * This module defines the initialization routines
 * for the SM501's LCD and CRT channels.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#ifdef CONFIG_LCD_MULTI
#include <lcd_multi.h>

/* Called by board_init to disable on boot */
void disable_sm501( void );

void init_sm501_lcd( struct lcd_t *lcd );
void init_sm501_crt( struct lcd_t *info );

#endif

#endif
