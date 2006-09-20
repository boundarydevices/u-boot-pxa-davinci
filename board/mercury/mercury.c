/*
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

DECLARE_GLOBAL_DATA_PTR;

void flash__init (void);
void ether__init (void);

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

/*
 * Miscellaneous platform dependent initialisations
 */
//void serial_waitTxComplete(void);

#define CLOCK_BASE 0x53f80000
#define CLK_RCSR 0x000c
#define CLK_COSR 0x001c
#define CLK_CGR0 0x0020
#define CLK_CGR1 0x0024
#define CLK_CGR2 0x0028

#define M(mode,bi) (mode<<(bi<<1))

int board_init (void)
{
	volatile uint * ccm = (volatile uint *)CLOCK_BASE;
//	serial_puts("board_init\n");
//	serial_waitTxComplete();
	ccm[CLK_RCSR>>2] = (ccm[CLK_RCSR>>2]&~(3<<30)) | ((CFG_NAND_16BIT<<31)|(CFG_NAND_2K_PAGE<<30));
	

	
#if 0
	ccm[CLK_CGR0>>2] = M(3,0)|M(3,3)|M(3,10);		//turn on clocks for sdhc1, epit1(Periodic timer), uart1
	ccm[CLK_CGR1>>2] = 0;	//M(3,4);				//real-time clock
	ccm[CLK_CGR2>>2] = M(3,7)|M(3,4);		//weird reserved and emi
#endif

	/* arch number of Integrator Board */
	gd->bd->bi_arch_number = MACH_TYPE_IMX31_MERCURY;

	/* adress of boot parameters */
	gd->bd->bi_boot_params = PHYSICAL_SDRAM+0x0100;

	gd->flags = 0;

	icache_enable ();

	flash__init ();
	ether__init ();
	return 0;
}


int misc_init_r (void)
{
	setenv("verify", "n");
	return (0);
}

/******************************
 Routine:
 Description:
******************************/
void flash__init (void)
{
}
/*************************************************************
 Routine:ether__init
 Description: take the Ethernet controller out of reset and wait
	      for the EEPROM load to complete.
*************************************************************/
void ether__init (void)
{
}

/******************************
 Routine:
 Description:
******************************/
int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size	 = PHYS_SDRAM_1_SIZE;
	return 0;
}

#if BOARD_LATE_INIT
int board_late_init(void)
{
	setenv("stdout", "serial");
	setenv("stderr", "serial");
	return 0;
}
#endif
