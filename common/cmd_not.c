/*
 * (C) Copyright 2006
 * Eric Nelson, Boundary Devices
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

#include <common.h>
#include <config.h>
#include <command.h>

#if (CONFIG_COMMANDS & CFG_CMD_NOT)

int do_not (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   int rval = 1 ;
   if( 1 < argc )
   {
      cmd_tbl_t *cmd = find_cmd(argv[1]);
      if( cmd )
      {
         rval = ( 0 == cmd->cmd(cmd, flag, argc-1, argv+1) );
      }
      else
         printf( "command %s not found\n", argv[1] );
   }
   else
      printf( "Usage not command [..params]\n" );
   
   return rval ;
}

U_BOOT_CMD(
	not,	127,	0,	do_not,
	"not      - negate a command\n",
	NULL
);

#endif /* CONFIG_COMMANDS & CFG_CMD_NOT */
