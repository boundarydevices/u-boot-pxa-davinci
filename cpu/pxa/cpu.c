/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/arch/pxa-regs.h>
#include <asm/arch/mmc.h>

#ifdef CONFIG_USE_IRQ
DECLARE_GLOBAL_DATA_PTR;
#endif

#define GP_INDEX(gp) ((gp<96)? (gp>>5) : 0x40)
#define GP_BITMASK(gp)  (1 << ((gp) & 0x1f) )
#define GPLRx(gp) __REG((0x40e00000+(GP_INDEX(gp)<<2)))
#define GPSRx(gp) __REG((0x40e00018+(GP_INDEX(gp)<<2)))
#define GPCRx(gp) __REG((0x40e00024+(GP_INDEX(gp)<<2)))

#define GPIO_SET(val,gp)     if (val) GPSRx(gp) = GP_BITMASK(gp); else GPCRx(gp) = GP_BITMASK(gp)

#if (PLATFORM_TYPE==NEON270)
#define GPIO_SMSC_RESET 23
#else
#define GPIO_SMSC_RESET -1
#endif

int cpu_init (void)
{
	/*
	 * setup up stacks if necessary
	 */
#ifdef CONFIG_USE_IRQ
	IRQ_STACK_START = _armboot_start - CFG_MALLOC_LEN - CFG_GBL_DATA_SIZE - 4;
	FIQ_STACK_START = IRQ_STACK_START - CONFIG_STACKSIZE_IRQ;
#endif

	if (GPIO_SMSC_RESET >=0) {
		GPIO_SET(0,GPIO_SMSC_RESET);
		udelay(10);
		GPIO_SET(1,GPIO_SMSC_RESET);
	}
	return 0;
}

int cleanup_before_linux (void)
{
	/*
	 * this function is called just before we call linux
	 * it prepares the processor for linux
	 *
	 * just disable everything that can disturb booting linux
	 */

	unsigned long i;

   MMC_STRPCL = MMC_STRPCL_STOP_CLK;
	
	disable_interrupts ();

   dcache_disable();
	
	/* turn off I-cache */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));
	i &= ~0x1000;
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush I-cache */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));

	return (0);
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	printf ("resetting ...\n");

	udelay (50000);				/* wait 50 ms */
	disable_interrupts ();
	reset_cpu (0);

	/*NOTREACHED*/
	return (0);
}

/* taken from blob */
void icache_enable (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* set i-cache */
	i |= 0x1000;

	/* write back to control register */
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));
}

void icache_disable (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* clear i-cache */
	i &= ~0x1000;

	/* write back to control register */
	asm ("mcr p15, 0, %0, c1, c0, 0": :"r" (i));

	/* flush i-cache */
	asm ("mcr p15, 0, %0, c7, c5, 0": :"r" (i));
}

int icache_status (void)
{
	register u32 i;

	/* read control register */
	asm ("mrc p15, 0, %0, c1, c0, 0":"=r" (i));

	/* return bit */
	return (i & 0x1000);
}


#ifndef CONFIG_CPU_MONAHANS
void set_GPIO_mode(int gpio_mode)
{
	int gpio = gpio_mode & GPIO_MD_MASK_NR;
	int fn = (gpio_mode & GPIO_MD_MASK_FN) >> 8;
	int gafr = (GAFR(gpio) & ~(0x3 << (((gpio) & 0xf)*2)) ) |
			 (fn  << (((gpio) & 0xf)*2));

	if ( (gpio & 0x7f) < 96 ) {
		if (gpio_mode & GPIO_MD_MASK_DIR) {
			_GPDR(gpio) |= GPIO_bit(gpio);
		} else {
			_GPDR(gpio) &= ~GPIO_bit(gpio);
		}
		_GAFR(gpio) = gafr;
	} else {
#ifdef GPDR3
		if (gpio_mode & GPIO_MD_MASK_DIR) {
			GPDR3 |= GPIO_bit(gpio);
		} else {
			GPDR3 &= ~GPIO_bit(gpio);
		}
		if ( (gpio & 0x7f) < 112) {
			GAFR3_L = gafr;
		} else {
			GAFR3_U = gafr;
		}
#endif
	}
}
#endif /* CONFIG_CPU_MONAHANS */

/*
 * Note that only speeds where CLKCFG[B] = 1 are supported
 * Position in this table indicates the value of CCCR[2N].
 *
 */
static unsigned const cpuSpeeds[] = {
	0   // 0
,	0   // 1
,	208 // 2
,	312 // 3
,	416 // 4
,	520 // 5
,	624 // 6
};

static unsigned const numCpuSpeeds = sizeof(cpuSpeeds)/sizeof(cpuSpeeds[0]);

static void set_clkcfg( unsigned value )
{
	unsigned oldval ;
	do {
		asm volatile("mrc p14, 0, %0, c6,c0, 0" : : "r" (value));
		asm volatile("mcr p14, 0, %0, c6,c0, 0" : "=r" (oldval));
		printf( "%s: %08x..%08x\n", __FUNCTION__, value, oldval );
	} while( oldval != value );
}

int do_cpuclk (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rval = 1 ;
	if( 2 == argc ){
		unsigned const speed = simple_strtoul(argv[1], 0, 0);
		if( speed ){
			unsigned idx = 0 ;
			for( idx = 0 ; idx < numCpuSpeeds; idx++ ){
				if( speed == cpuSpeeds[idx] )
					break ;
			}
			if( idx < numCpuSpeeds ){
				unsigned clkcfg ;
				printf( "cpu speed %u, idx %u\n", speed, idx );
				CCCR = ( CCCR & ~CCCR_N_MASK ) | (idx << 7);
				asm volatile("mcr p14, 0, %0, c6,c0, 0" : "=r" (clkcfg));
				udelay(50000);
				clkcfg |= 2 ;
				clkcfg &= ~5 ; // no turbo durinc clock change
				set_clkcfg(clkcfg);
				clkcfg |= 1 ; // now turbo mode
				set_clkcfg(clkcfg);
				rval = 0 ;
			}
		} // don't change speed to zero
	} else {
		int i = 0 ;
		unsigned idx = ( CCCR & CCCR_N_MASK ) >> 7 ;
		for( i = 0 ; i < numCpuSpeeds ; i++ ){
			if( cpuSpeeds[i] ){
				printf( "%c %u MHz\n", i==idx ? '*' : ' ', cpuSpeeds[i] );
			}
		}
	}
	return rval ;
}

U_BOOT_CMD(
	cpuclk,	  2,	0,	do_cpuclk,
	"cpuclk  - set CPU clock\n",
	NULL
);


