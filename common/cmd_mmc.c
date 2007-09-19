/*
 * (C) Copyright 2003
 * Kyle Harris, kharris@nexus-tech.net
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

#if defined(CONFIG_CMD_MMC)

#include <mmc.h>
#include <part.h>

int do_mmc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (mmc_init (1 < argc) != 0) {
		printf ("No MMC card found\n");
		return 1;
	}
	return 0;
}

U_BOOT_CMD(
	mmcinit,	2,	0,	do_mmc,
	"mmcinit - init mmc card (-v for verbose)\n",
	NULL
);

int do_mmc_cmd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   if( 5 == argc )
   {
      unsigned long args[4];
      unsigned i ;
      for( i = 1 ; i < 5 ; i++ )
      {
         char *endp ;
         args[i-1] = simple_strtoul(argv[i], &endp, 16 );
         if( 0 != *endp )
         {
            printf( "arg[%u] is not a valid hex number\n", i );
            break;
         }
      }
   
      if( 5 == i )
      {
         uchar *resp = mmc_cmd( (ushort)args[0],(args[1]<<16)|args[2],
                                (ushort)args[3] );
         ushort numWords = 0 ;
         switch( args[3] )
         {
            case MMC_CMDAT_R1:
            case MMC_CMDAT_R3:
               numWords = 3;
               break;
   
            case MMC_CMDAT_R2:
               numWords = 8;
               break;
   
            default:
               printf( "Invalid response type %lu, options are [1,2,3]\n", args[3] );
               break;
         }
   
         if( resp )
         {
            for( i = 0 ; i < numWords*2 ; i++ )
            {
               printf( "%02X ", resp[i] );
            }
            printf( "\n" );
         }
         else
            printf( "no response\n" );
      }
   }
   else
      printf ("Usage:\n%s\n", cmdtp->usage);
   
   return 0;
}

U_BOOT_CMD(
	mmccmd,	5,	0,	do_mmc_cmd,
	"mmccmd - issue mmc command\n",
	"mmccmd cmd# argh(hex) argl(hex) rsptype\n"
);

int do_mmc_bread(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
   if( 3 == argc ){
      char *endp ;
      void *addr ;
      unsigned long blocknum = simple_strtoul(argv[1], &endp, 16 );
      if( 0 != *endp )
      {
         printf( "Invalid blockNum %s (use hex)\n", argv[1] );
         return -1;
      }
      addr = (void *)simple_strtoul(argv[2], &endp, 16 );
      if( 0 != *endp )
      {
         printf( "Invalid address %s (use hex)\n", argv[2] );
         return -1;
      }
      printf( "read block number 0x%x into address %p\n", blocknum, addr );
      
      block_dev_desc_t *dev = mmc_get_dev(0);
      if( dev ){
         unsigned long rval = dev->block_read(0, blocknum, 1, addr );
         if( 1 == rval ){
            printf( "read block %lu (0x%lx)\n", blocknum, blocknum );
         }
         else
            printf( "error reading block %lu (0x%lx)\n", blocknum, blocknum );
      }
   }
   else
      printf( "Usage: mmcread blockNum(hex) address(hex)\n" );
   
   return 0 ;
}

U_BOOT_CMD(
	mmcread,	3,	1,	do_mmc_bread,
	"mmcread - read mmc block\n",
	"mmcread blockNum(hex) address(hex)\\n"
);

#endif
