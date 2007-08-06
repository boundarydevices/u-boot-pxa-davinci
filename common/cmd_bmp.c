/*
 * (C) Copyright 2002
 * Detlev Zundel, DENX Software Engineering, dzu@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * BMP handling routines
 */

#include <common.h>
#include <bmp_layout.h>
#include <command.h>
#include <asm/byteorder.h>
#include <malloc.h>

#ifdef CONFIG_LCD_MULTI
#include <lcd_multi.h>
#endif

#include <lcd.h>
static int bmp_info (ulong addr);
static int bmp_display (ulong addr, int x, int y);

int gunzip(void *, int, unsigned char *, unsigned long *);

/*
 * Allocate and decompress a BMP image using gunzip().
 *
 * Returns a pointer to the decompressed image data. Must be freed by
 * the caller after use.
 *
 * Returns NULL if decompression failed, or if the decompressed data
 * didn't contain a valid BMP signature.
 */
#ifdef CONFIG_VIDEO_BMP_GZIP
static bmp_image_t *gunzip_bmp(unsigned long addr, unsigned long *lenp)
{
	void *dst;
	unsigned long len;
	bmp_image_t *bmp;

	/*
	 * Decompress bmp image
	 */
	len = CFG_VIDEO_LOGO_MAX_SIZE;
	dst = malloc(CFG_VIDEO_LOGO_MAX_SIZE);
	if (dst == NULL) {
		puts("Error: malloc in gunzip failed!\n");
		return NULL;
	}
	if (gunzip(dst, CFG_VIDEO_LOGO_MAX_SIZE, (uchar *)addr, &len) != 0) {
		free(dst);
		return NULL;
	}
	if (len == CFG_VIDEO_LOGO_MAX_SIZE)
		puts("Image could be truncated"
				" (increase CFG_VIDEO_LOGO_MAX_SIZE)!\n");

	bmp = dst;

	/*
	 * Check for bmp mark 'BM'
	 */
	if (!((bmp->header.signature[0] == 'B') &&
	      (bmp->header.signature[1] == 'M'))) {
		free(dst);
		return NULL;
	}

	puts("Gzipped BMP image detected!\n");

	return bmp;
}
#else
static bmp_image_t *gunzip_bmp(unsigned long addr, unsigned long *lenp)
{
	return NULL;
}
#endif


/*
 * Subroutine:  do_bmp
 *
 * Description: Handler for 'bmp' command..
 *
 * Inputs:	argv[1] contains the subcommand
 *
 * Return:      None
 *
 */
int do_bmp(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr;
	int x = 0, y = 0;

	switch (argc) {
	case 2:		/* use load_addr as default address */
		addr = load_addr;
		break;
	case 3:		/* use argument */
		addr = simple_strtoul(argv[2], NULL, 16);
		break;
	case 5:
		addr = simple_strtoul(argv[2], NULL, 16);
	        x = simple_strtoul(argv[3], NULL, 10);
	        y = simple_strtoul(argv[4], NULL, 10);
	        break;
	default:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/* Allow for short names
	 * Adjust length if more sub-commands get added
	 */
	if (strncmp(argv[1],"info",1) == 0) {
		return (bmp_info(addr));
	} else if (strncmp(argv[1],"display",1) == 0) {
	    return (bmp_display(addr, x, y));
	} else {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
}

U_BOOT_CMD(
	bmp,	5,	1,	do_bmp,
	"bmp     - manipulate BMP image data\n",
	"info <imageAddr>          - display image info\n"
	"bmp display <imageAddr> [x y] - display image at x,y\n"
);

/*
 * Subroutine:  bmp_info
 *
 * Description: Show information about bmp file in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int bmp_info(ulong addr)
{
	bmp_image_t *bmp=(bmp_image_t *)addr;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len);

	if (bmp == NULL) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

	printf("Image size    : %d x %d\n", le32_to_cpu(bmp->header.width),
	       le32_to_cpu(bmp->header.height));
	printf("Bits per pixel: %d\n", le16_to_cpu(bmp->header.bit_count));
	printf("Compression   : %d\n", le32_to_cpu(bmp->header.compression));

	if ((unsigned long)bmp != addr)
		free(bmp);

	return(0);
}

/*
 * Subroutine:  bmp_display
 *
 * Description: Display bmp file located in memory
 *
 * Inputs:	addr		address of the bmp file
 *
 * Return:      None
 *
 */
static int bmp_display(ulong addr, int x, int y)
{
	int ret;
	bmp_image_t *bmp = (bmp_image_t *)addr;
	unsigned long len;

	if (!((bmp->header.signature[0]=='B') &&
	      (bmp->header.signature[1]=='M')))
		bmp = gunzip_bmp(addr, &len);

	if (!bmp) {
		printf("There is no valid bmp file at the given address\n");
		return 1;
	}

#if defined(CONFIG_LCD)
	extern int lcd_display_bitmap (ulong, int, int);

	ret = lcd_display_bitmap ((unsigned long)bmp, x, y);
#elif defined(CONFIG_VIDEO)
	extern int video_display_bitmap (ulong, int, int);
	ret = video_display_bitmap ((unsigned long)bmp, x, y);
#elif defined(CONFIG_LCD_MULTI)
	ushort i, j;
	uchar *fb;
	bmp_image_t *bmp=(bmp_image_t *)addr ;
	uchar *bmap;
	ushort padded_line;
	unsigned long width, height;
	unsigned colors,bpix;
	unsigned long compression;
	int     maxLum = 0 ;
	int     bgCol = 0 ;
	int     minLum = 0xFFFF ;
	int     fgCol = 0 ;

	if (!((bmp->header.signature[0]=='B') &&
		(bmp->header.signature[1]=='M'))) {
		printf ("Error: no valid bmp image at %lx\n", addr);
		return 1;
	}

	width = le32_to_cpu (bmp->header.width);
	height = le32_to_cpu (bmp->header.height);
	colors = 1<<le16_to_cpu (bmp->header.bit_count);
	compression = le32_to_cpu (bmp->header.compression);

	bpix = 8 ;

	if ((bpix != 1) && (bpix != 8)) {
		printf ("Error: %d bit/pixel mode not supported by U-Boot\n",
			bpix);
		return 1;
	}

	if (bpix != le16_to_cpu(bmp->header.bit_count)) {
		printf ("Error: %d bit/pixel mode, but BMP has %d bit/pixel\n",
			bpix,
			le16_to_cpu(bmp->header.bit_count));
		return 2;
	}

	if (bpix==8) {
		unsigned long palette[256];
		unsigned long* cmap = palette;
      struct lcd_t *lcd = getPanel(getCurrentPanel());
      if( ( 0 == lcd ) || ( 0 == lcd->set_palette ) ){
         printf( "No current panel\n" );
         return 3 ;
      }
printf( "current LCD: %ux%u at %p\n", lcd->info.xres, lcd->info.yres, lcd->fbAddr );

		/* Set color map */
		for (i=0; i<colors; ++i) {
			int lum ;
			bmp_color_table_entry_t cte = bmp->color_table[i];
			*cmap++ = cte.blue | (cte.green << 8) | (cte.red << 16) | 0xFF000000;
			lum = luminance( cte.red, cte.green, cte.blue );
			if( lum > maxLum ) {
				maxLum = lum ;
				bgCol = i ;
			}
			if( lum < minLum ) {
				minLum = lum ;
				fgCol = i ;
			}
		}
      lcd->set_palette( palette, colors);
      lcd->colorCount = colors ;
		printf( "bgcolor %u, fg %u\n", bgCol, fgCol );
      lcd->bg = bgCol ;
      lcd->fg = fgCol ;

	padded_line = (width+3)&~0x3;
	if ((x + width)>lcd->info.xres)
		width = lcd->info.xres - x;
	if ((y + height)>lcd->info.yres)
		height = lcd->info.yres - y;

	bmap = (uchar *)bmp + le32_to_cpu (bmp->header.data_offset);
	fb   = ((uchar *)lcd->fbAddr) +((y + height - 1) * lcd->info.xres) + x;
   //	printf("fb:0x%p,lcd_base:0x%p,x:%i,y:%i,width:%i,height:%i,colums:%i\n",
   //		fb,lcd_base,x,y,width,height,panel_info.vl_lcd_line_length);
	for (i = 0; i < height; ++i) {
		for (j = 0; j < width ; j++)
			*fb++=*bmap++;
		bmap += (padded_line-width);
		fb   -= (width + lcd->info.xres);
	}
	}
   else
      printf( "Unsupported bit depth %u\n", bpix );
#else
# error bmp_display() requires CONFIG_LCD, CONFIG_LCD_MULTI, or CONFIG_VIDEO
#endif

	if ((unsigned long)bmp != addr)
		free(bmp);

	return ret;
}
