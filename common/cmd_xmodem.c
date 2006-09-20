/*
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
#ifdef CMD_XMODEM
#include <command.h>
ulong load_xmodem(ulong loadAddress,ulong* pEndAddress);

int xmodem_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong loadAddress = 0;
	ulong endAddress = 0;
	int rcode = 0;
	char *s;

	if (argc < 2) return -1;
	loadAddress = simple_strtoul(argv[1], NULL, 16);

	printf ("## Ready for binary (xmodem) download to 0x%08lX\n",loadAddress);
	rcode = load_xmodem(loadAddress,&endAddress);
	if (rcode==0) {
		printf("\nOK %08lX-%08lX\n",loadAddress,endAddress);
	} else {
		printf("\nError\n");
	}
	return rcode;
}

U_BOOT_CMD(
	xmodem, 3, 0,	xmodem_bin,
	"xmodem address - load binary file over serial line (xmodem protocol)\n",
	"'address' is ram location of dowload\n"
);

#endif
