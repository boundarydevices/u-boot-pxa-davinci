/*
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <configs/select.h>
#include <linux/byteorder/swab.h>
#include <asm/arch/pxa250Base.h>
#include <asm/arch/pxaHardware.h>
#include "lcd.h"

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips    */

/* Board support for 1 or 2 flash devices */
#if (PLATFORM_TYPE==HALOGEN)
#if (PLATFORM_REV==2)
#define FLASH_PORT_WIDTH16 1
#endif
#endif

#if (PLATFORM_TYPE==ARGON)
#define FLASH_PORT_WIDTH16 1
#endif

#if (PLATFORM_TYPE==OXYGEN)
#define FLASH_PORT_WIDTH16 1
#endif

#ifdef FLASH_PORT_WIDTH16
#define FLASH_PORT_WIDTH		ushort
#define FLASH_PORT_WIDTHV		vu_short
#define FLASH_CHIP_CNT 1
#define SWAP(x)               __swab16(x)
#else
#define FLASH_PORT_WIDTH		ulong
#define FLASH_PORT_WIDTHV		vu_long
#define FLASH_CHIP_CNT 2
#define SWAP(x)               __swab32(x)
#endif

#define SECTOR_SIZE_PER_CHIP 0x20000

#define FPW	   FLASH_PORT_WIDTH
#define FPWV   FLASH_PORT_WIDTHV

#define mb() __asm__ __volatile__ ("" : : : "memory")

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (volatile FPW *addr, flash_info_t *info);
static int write_data (flash_info_t *info, ulong dest, FPW data);
static void flash_get_offsets (ulong base, flash_info_t *info);
void inline spin_wheel_init(ulong addr, ulong cnt);
void inline spin_wheel_done( int worked );
void inline spin_wheel (ulong numleft);

/*-----------------------------------------------------------------------
 */
ulong bases[] = {PHYS_FLASH_1,PHYS_FLASH_2,1};
unsigned long flash_init (void)
{
	int i=0;
	int j=0;
	ulong size = 0;
	ulong base;

   while (i < CFG_MAX_FLASH_BANKS) {
		base = bases[j++];
		flash_info[i].start[0] = 0;
		if (base & 1) break;
		if (flash_get_size ((volatile FPW *) base, &flash_info[i])) {
			flash_get_offsets (base, &flash_info[i]);
			size += flash_info[i].size;
			i++;
		}
      else {
printf( "error reading flash size\n" );
      }
	}
	if (size>0) {
		base = flash_info[0].start[0];
		// Protect monitor and environment sectors
		flash_protect ( FLAG_PROTECT_SET,
			base,
			base + monitor_flash_len - 1,
			&flash_info[0] );

		flash_protect ( FLAG_PROTECT_SET,
			base+CFG_ENV_OFFSET,
			base+CFG_ENV_OFFSET + CFG_ENV_SIZE - 1, &flash_info[0] );
	}

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return;
	}

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * (SECTOR_SIZE_PER_CHIP*FLASH_CHIP_CNT));
			info->protect[i] = 0;
		}
	}
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:
		printf ("INTEL ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F128J3A:
		printf ("28F128J3A\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (volatile FPW *addr, flash_info_t *info)
{
	FPW manVal,devVal;
	volatile unsigned long *mc = (unsigned long *)MEMORY_CONTROL_BASE;
	unsigned long val = 1<<3;

	info->flash_id = FLASH_UNKNOWN;
	info->sector_count = 0;
	info->size = 0;

	if (((ulong)addr) > 0x14000000) return 0;
	val = mc[(MSC0>>2) +(((ulong)addr)>>27)];
	if (((ulong)addr) & 0x04000000) val = val>>16;
#ifndef FLASH_PORT_WIDTH16
	if ( val & (1<<3)) return 0;	//if 16 bit bus then return
#else
	if (!( val & (1<<3))) return 0;	//if 32 bit bus then return
#endif
	
	/* Write auto select command: read Manufacturer ID */
	addr[0x5555] = (FPW) 0x00AA00AA;
	addr[0x2AAA] = (FPW) 0x00550055;
	addr[0x5555] = (FPW) 0x00900090;
	udelay(1);
	manVal = addr[0];
	devVal = addr[1];			/* device ID        */
	addr[0] = (FPW) 0x00FF00FF;		/* restore read mode */

	switch (manVal) {
	case (FPW) 0:
	case (FPW) STM_MANUFACT:
#ifndef FLASH_PORT_WIDTH16
	case (FPW) INTEL_MANUFACT & 0xFF0000 :
	case (FPW) INTEL_MANUFACT & 0x0000FF :
#endif
	case (FPW) INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		printf( "Invalid flash manufacturer %x, %x\n", manVal,devVal );
		return (0);			/* no or unknown flash  */
	}


	switch (devVal) {

	case (FPW) 0:
#ifndef FLASH_PORT_WIDTH16
	case (FPW) INTEL_ID_28F128J3A & 0xFF0000 :
	case (FPW) INTEL_ID_28F128J3A & 0x0000FF :
#endif
	case (FPW) INTEL_ID_28F128J3A:
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = info->sector_count*(SECTOR_SIZE_PER_CHIP*FLASH_CHIP_CNT);
		break;				/* => 16 MB x 2  */
   case (FPW) INTEL_ID_28F320J3A:
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = info->sector_count*(SECTOR_SIZE_PER_CHIP*FLASH_CHIP_CNT);
		break;				/* => 4 MB x 2 */
   case (FPW) INTEL_ID_28F640J3A:
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = info->sector_count*(SECTOR_SIZE_PER_CHIP*FLASH_CHIP_CNT);
		break;				/* => 8 MB  */

	default:
		printf( "Unknown flash device %x,%x\n", manVal,devVal );
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (info->sector_count > CFG_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CFG_MAX_FLASH_SECT);
		info->sector_count = CFG_MAX_FLASH_SECT;
	}


	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong type, start, last;
	int rcode = 0;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);
	if ((type != FLASH_MAN_INTEL)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);
	last = start;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
         char temp[80];
			FPWV *addr = (FPWV *) (info->start[sect]);
			FPW status;

			sprintf (temp, "Erasing sector %2d ... \r", sect);
         lcd_puts( temp );

			/* arm simple, non interrupt dependent timer */
			reset_timer_masked ();

			*addr = (FPW) 0x00500050;	/* clear status register */
			*addr = (FPW) 0x00200020;	/* erase setup */
			*addr = (FPW) 0x00D000D0;	/* erase confirm */

			while (((status = *addr) & (FPW) 0x00800080) != (FPW) 0x00800080) {
				if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = (FPW) 0x00B000B0;	/* suspend erase     */
					*addr = (FPW) 0x00FF00FF;	/* reset to read mode */
					rcode = 1;
					break;
				}
			}

			*addr = (FPW)0x00500050;	/* clear status register cmd.   */
			*addr = (FPW)0x00FF00FF;	/* resest to read mode          */
		}
	}
   lcd_puts( "\r\n" );
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	FPW data;
	int count, i, l, rc, port_width;

   rc = 0 ;
	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}
/* get lower word aligned address */
#ifdef FLASH_PORT_WIDTH16
	wp = (addr & ~1);
	port_width = 2;
#else
	wp = (addr & ~3);
	port_width = 4;
#endif

   spin_wheel_init(addr,cnt);

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}
		for (; i < port_width && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < port_width; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		if ((rc = write_data (info, wp, SWAP (data))) != 0) {
         goto out;
		}
		wp += port_width;
	}

	/*
	 * handle word aligned part
	 */
	count = 0;
	while (cnt >= port_width) {
		data = 0;
		for (i = 0; i < port_width; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_data (info, wp, SWAP (data))) != 0) {
         goto out;
		}
		wp += port_width;
		cnt -= port_width;
		if (count++ > 0x800) {
			spin_wheel (cnt);
			count = 0;
		}
	}

	if (cnt) {
   	/*
   	 * handle unaligned tail bytes
   	 */
   	data = 0;
   	for (i = 0, cp = wp; i < port_width && cnt > 0; ++i, ++cp) {
   		data = (data << 8) | *src++;
   		--cnt;
   	}
   	for (; i < port_width; ++i, ++cp) {
   		data = (data << 8) | (*(uchar *) cp);
   	}
   
   	rc = write_data (info, wp, SWAP (data));
	}

out:
   spin_wheel_done(0 == rc);
   return rc ;
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, ulong dest, FPW data)
{
	FPWV *addr = (FPWV *) dest;
   FPW old = *addr ;
	ulong status;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((old & data) != data) {
		printf ("not erased at %08lx (%lx)\n", (ulong) addr, old);
		return (2);
	}

   if( old != data )
   {
   	/* Disable interrupts which might cause a timeout here */
   	flag = disable_interrupts ();

   	*addr = (FPW) 0x00400040;	/* write setup */
   	*addr = data;
   
   	/* arm simple, non interrupt dependent timer */
   	reset_timer_masked ();
   
   	/* wait while polling the status register */
   	while (((status = *addr) & (FPW) 0x00800080) != (FPW) 0x00800080) {
   		if (get_timer_masked () > CFG_FLASH_WRITE_TOUT) {
   			*addr = (FPW) 0x00FF00FF;	/* restore read mode */
   			return (1);
   		}
   	}
   	*addr = (FPW) 0x00FF00FF;	/* restore read mode */
   } /* need to program? */


	return (0);
}

void inline spin_wheel_init(ulong addr, ulong cnt)
{
   char temp[80];
   sprintf( temp, 
            "\nprogramming flash\n"
            "%08lx->%08lx\n"
            " ", addr, cnt );
   lcd_puts( temp );
}

void inline spin_wheel_done( int worked )
{
   if( worked )
      spin_wheel(0);
   lcd_puts( worked ? "\ncompleted.\n" : "\nfailed!\n" );
}

void inline spin_wheel( ulong numleft )
{
   char temp[40];
   sprintf( temp, "\r          %08lx", numleft );
   lcd_puts( temp );
}
