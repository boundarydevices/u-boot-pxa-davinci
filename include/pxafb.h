#ifndef _PXAFB_H_
#define _PXAFB_H_

/*
 * Header file pxafb.h
 *
 * This module declares the initialization routine for the
 * PXA-2XX's internal graphics controller.
 *
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#ifdef CONFIG_LCD_MULTI
#include <lcd_multi.h>

void init_pxa_fb( struct lcd_t *lcd );

#endif

#endif
