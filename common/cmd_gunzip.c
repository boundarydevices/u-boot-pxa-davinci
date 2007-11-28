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

#include <common.h>
#include <command.h>

#if defined(CONFIG_CMD_GUNZIP)

#include <asm/byteorder.h>
#include <malloc.h>
#include <gunzip.h>

#define CONFIG_GUNZIP_ADLER
#ifdef CONFIG_GUNZIP_ADLER
#include <zlib.h>
#endif 

/*
 * Subroutine:  do_gunzip
 *
 * Description: Handler for 'gunzip' command..
 *
 * Inputs:	argv[1]    input address
 *              argv[2]    input length
 *              argv[3]    output address
 *              argv[4]    output length
 *
 * Return:      zero if successful
 *
 */
int do_gunzip(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if( 5 != argc ){
		printf ("Usage:\n%s\n", cmdtp->help);
		return 1;
	}
	unsigned long inaddr = simple_strtoul(argv[1],0,16);
	unsigned long inlen = simple_strtoul(argv[2],0,16);
	unsigned long outaddr = simple_strtoul(argv[3],0,16);
	unsigned long outlen = simple_strtoul(argv[4],0,16);
	if( ( 0 == inlen ) || ( 0 == outlen ) ){
		printf ("Usage:\n%s\n", cmdtp->help);
		return 1;
	}

	int rval = gunzip( (void *)outaddr, outlen, (unsigned char *)inaddr, &inlen );
	if( rval ){
		printf( "gunzip error %d\n", rval );
		return rval ;
	}
	printf( "%lu bytes decompressed\n", inlen );

#ifdef CONFIG_GUNZIP_ADLER
	printf( "adler == 0x%08lx\n", adler32(0, (Bytef *)outaddr, inlen ) );
#endif

	return 0 ;
}

U_BOOT_CMD(
	gunzip,	5,	1,	do_gunzip,
	"gunzip     - unzip compressed data\n",
	"gunzip srcaddr srclen destaddr destlen\n"
);
#endif /* defined(CONFIG_CMD_BMP) */
