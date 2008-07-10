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
	unsigned len ;
	int arg ;

	if( 3 > argc ){
		goto bail ;
        }

	addr = simple_strtoul(argv[1], &endptr, 16);
	if( !endptr || (0!=*endptr) ){
		goto bail ;
	}

	if( 3 == argc ){
		len = strlen(argv[2]) + 1;
	}
	else {
		len = simple_strtoul(argv[3],&endptr,10);
		if( !endptr || (0!=*endptr) ){
			printf( "invalid string <%s> %p %c\n", argv[3], endptr, endptr[0] );
			goto bail ;
		}
	}
	strncpy((void *)addr, argv[2], len);
	return 0 ;

bail:
	printf("%s: Usage:\n%s\n", __func__, cmdtp->usage);
	return 1;
}

U_BOOT_CMD(strcpy, 4, 1, do_strcpy,
	"strcpy - copy strings\n",
	"<addr> string [byte count]\n"
	"    - copy string to <addr>, length at most [byte count] or first NULL\n" );

#endif
