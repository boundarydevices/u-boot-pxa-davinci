#ifndef __LCD_PANELS_H__
#define __LCD_PANELS_H__ "$Id: lcd_panels.h,v 1.3 2005/06/02 04:03:37 ericn Exp $"

/*
 * lcd_panels.h
 *
 * This header file declares the lcd_panel_info_t 
 * data type and the num_lcd_panels and lcd_panels 
 * constants for use in initializing a variety of displays.
 *
 * Note that this is based on the pxafb_mach_info structure
 * in linux-2.4.19/drivers/video/pxafb.h (Nico's patches), 
 * but differs in a couple of respects:
 *
 *    Doesn't support the color-mapping stuff
 *    Includes an 'active' flag
 *
 * Doesn't include the lccr0 and lccr3 fields, since most of 
 * the fields in those registers are display controller options, 
 * not panel options, and the others (so far) can have reasonable
 * defaults. When (if) we need to support a non-standard display,
 * we can fill in the blanks with defaults in the previously 
 * supported displays and compute lccr0 and lccr3.
 *
 * Change History : 
 *
 * $Log: lcd_panels.h,v $
 * Revision 1.3  2005/06/02 04:03:37  ericn
 * -added rotation field
 *
 * Revision 1.2  2005/04/30 20:33:36  ericn
 * -added CRT support
 *
 * Revision 1.1  2005/04/09 17:49:18  ericn
 * -Initial import
 *
 *
 * Copyright Boundary Devices, Inc. 2005
 */

struct lcd_panel_info_t {
   char const       *name ;
	unsigned long		pixclock;

	unsigned short		xres;
	unsigned short		yres;

	unsigned     		act_high ;     // clock is active high
	unsigned     		hsync_len;
	unsigned     		left_margin;
	unsigned     		right_margin;
	unsigned     		vsync_len;
	unsigned     		upper_margin;
	unsigned     		lower_margin;
   unsigned          active ;       // active matrix (TFT) LCD
   unsigned          crt ;          // 1 == CRT, not LCD
   unsigned          rotation ;
};

#ifdef __cplusplus
extern "C" {
#endif 

extern unsigned const num_lcd_panels ;
extern struct lcd_panel_info_t const * const lcd_panels ;

extern struct lcd_panel_info_t const *find_lcd_panel( char const * name );


//
// Each platform needs to define this routine, and  
// set cur_lcd_panel within
//
void set_lcd_panel( struct lcd_panel_info_t const *panel );
extern struct lcd_panel_info_t const *cur_lcd_panel ;
void disable_lcd_panel( void );

#ifdef __CPLUSPLUS
};
#endif 

#endif

