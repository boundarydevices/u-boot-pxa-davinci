/*
 * cmd_strcpy.c - Copy string variable to memory location:
 *
 *    $ strcpy 0x1000000 "Hello"
 *
 * or more likely:
 *
 *    $ strcpy 0x1000000 $myvariable
 *
 * Three parameters turns command into strncpy for safety.
 * Note that you should test the existence of $myvariable first...
 *
 *    $strcpy 0x1000000 $myvariable 80
 *
 * Copyright (c) 2008 Boundary Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <common.h>
#include <command.h>

#ifdef CONFIG_CMD_STRCPY

int do_strcpy(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char *endptr ;
	unsigned long addr ;
	int arg ;
	
	if( 3 > argc ){
		goto bail ;
        }

	addr = simple_strtoul(argv[1], &endptr, 16);
	if( !endptr || (0!=*endptr) ){
		goto bail ;
	}

	for (arg = 2 ; arg < argc ; arg++) {
		char *src = argv[arg];
		char *dest = (char *)addr ;
		unsigned len ;
		if( '$' == *src ){
			src = getenv( src+1 );
			if( !src ){
				printf( "Invalid variable <%s>\n", argv[arg] );
				goto bail ;
			}
		}
                len = strlen(src);
		memcpy(dest, src, len);
		dest[len] = '\0' ;
		dest += strlen(dest); // in case it's shorter
		*dest++ = ' ' ;
		addr = (unsigned long)dest ;
        }
	*((char *)addr) = 0 ;
	return 0 ;

bail:
	printf("%s: Usage:\n%s\n", __func__, cmdtp->usage);
	return 1;
}

U_BOOT_CMD(strcpy, 100, 1, do_strcpy,
	"strcpy - copy strings\n",
	"<addr> string [byte count]\n"
	"    - copy string to <addr>, length at most [byte count] or first NULL\n" );

#endif
