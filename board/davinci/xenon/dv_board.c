/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts are shamelessly stolen from various TI sources, original copyright
 * follows:
 * -----------------------------------------------------------------
 *
 * Copyright (C) 2004 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/emac_defs.h>

#define MACH_TYPE_DAVINCI_XENON	1485

extern void	i2c_init(int speed, int slaveaddr);
extern void	timer_init(void);
extern int	eth_hw_init(void);
extern phy_t	phy;

#define MAC_VARIABLE "ethaddr"
#define GP_BINTEN 0x08
#define GP_BANK0_OFFSET	0x10
#define GP_BANK_LENGTH	0x28
#define GP_DIR			0x00
#define GP_OUT			0x04
#define GP_SET			0x08
#define GP_CLR			0x0C
#define GP_IN			0x10
#define GP_SET_RISING_EDGE	0x14
#define GP_CLR_RISING_EDGE	0x18
#define GP_SET_FALLING_EDGE	0x1c
#define GP_CLR_FALLING_EDGE	0x20
#define GP_INT_STAT		0x24
#define GPIO_DISPLAY_SELECT 45
#define GPIO_THS_ENABLE 42

void gpio_set_val(u32 gp, int val)
{
	u32 mask = (1 << (gp & 0x1f));
	u32 bank = (gp >> 5);
	volatile u32 *p = (u32 *)(DAVINCI_GPIO_BASE+GP_BANK0_OFFSET +
			(bank * GP_BANK_LENGTH));
	if (val)
		p[GP_SET >> 2] = mask;
	else
		p[GP_CLR >> 2] = mask;
	p[GP_DIR >> 2] &= ~mask;
}

#define DIR_IN 1
#define DIR_OUT 0
void gpio_set_dir(u32 gp, int in)
{
	u32 mask = (1 << (gp & 0x1f));
	u32 bank = (gp >> 5);
	volatile u32 *p = (u32 *)(DAVINCI_GPIO_BASE + GP_BANK0_OFFSET +
			GP_DIR + (bank * GP_BANK_LENGTH));
	if (in)
		*p |= mask;
	else
		*p &= ~mask;
}

volatile u32 *gpio_get_in_ptr(u32 gp)
{
	u32 bank = (gp >> 5);
	volatile u32 *p = (u32 *)(DAVINCI_GPIO_BASE + GP_BANK0_OFFSET + GP_IN +
			(bank * GP_BANK_LENGTH));
//	gpio_set_dir(gp, DIR_IN);
	return p;
}
u32 gpio_get_val(u32 gp)
{
	volatile u32 *p = gpio_get_in_ptr(gp);
	return (*p >> (gp & 0x1f)) & 1;
}


volatile u32 *gpio_get_stat_ptr(unsigned gp, int edge)
{
	u32 mask = (1 << (gp & 0x1f));
	u32 bank = (gp >> 5);
	volatile u32 *p = (u32 *)(DAVINCI_GPIO_BASE + GP_BINTEN);
	*p |= 1 << (bank >> 1);	/* enable proper bank */

	p = (u32 *)(DAVINCI_GPIO_BASE + GP_BANK0_OFFSET +
		(bank * GP_BANK_LENGTH));
	if (edge & 1)
		p[GP_SET_RISING_EDGE >> 2] = mask;
	else
		p[GP_CLR_RISING_EDGE >> 2] = mask;
	if (edge & 2)
		p[GP_SET_FALLING_EDGE >> 2] = mask;
	else
		p[GP_CLR_FALLING_EDGE >> 2] = mask;
	return &p[GP_INT_STAT >> 2];
}

/* Works on Always On power domain only (no PD argument) */
void lpsc_on(unsigned int id)
{
	dv_reg_p	mdstat, mdctl;

	if (id >= DAVINCI_LPSC_GEM)
		return;			/* Don't work on DSP Power Domain */

	mdstat = REG_P(PSC_MDSTAT_BASE + (id * 4));
	mdctl = REG_P(PSC_MDCTL_BASE + (id * 4));

	while (REG(PSC_PTSTAT) & 0x01) {;}

	if ((*mdstat & 0x1f) == 0x03)
		return;			/* Already on and enabled */

	*mdctl |= 0x03;

	/* Special treatment for some modules as for sprue14 p.7.4.2 */
	if (	(id == DAVINCI_LPSC_VPSSSLV) ||
		(id == DAVINCI_LPSC_EMAC) ||
		(id == DAVINCI_LPSC_EMAC_WRAPPER) ||
		(id == DAVINCI_LPSC_MDIO) ||
		(id == DAVINCI_LPSC_USB) ||
		(id == DAVINCI_LPSC_ATA) ||
		(id == DAVINCI_LPSC_VLYNQ) ||
		(id == DAVINCI_LPSC_UHPI) ||
		(id == DAVINCI_LPSC_DDR_EMIF) ||
		(id == DAVINCI_LPSC_AEMIF) ||
		(id == DAVINCI_LPSC_MMC_SD) ||
		(id == DAVINCI_LPSC_MEMSTICK) ||
		(id == DAVINCI_LPSC_McBSP) ||
		(id == DAVINCI_LPSC_GPIO)
	   )
		*mdctl |= 0x200;

	REG(PSC_PTCMD) = 0x01;

	while (REG(PSC_PTSTAT) & 0x03) {;}
	while ((*mdstat & 0x1f) != 0x03) {;}	/* Probably an overkill... */
}

#ifdef POWERUP_DSP
void dsp_on(void)
{
	int	i;

	if (REG(PSC_PDSTAT1) & 0x1f)
		return;			/* Already on */

	REG(PSC_GBLCTL) |= 0x01;
	REG(PSC_PDCTL1) |= 0x01;
	REG(PSC_PDCTL1) &= ~0x100;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_GEM * 4)) &= 0xfffffeff;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) |= 0x03;
	REG(PSC_MDCTL_BASE + (DAVINCI_LPSC_IMCOP * 4)) &= 0xfffffeff;
	REG(PSC_PTCMD) = 0x02;

	for (i = 0; i < 100; i++) {
		if (REG(PSC_EPCPR) & 0x02)
			break;
	}

	REG(PSC_CHP_SHRTSW) = 0x01;
	REG(PSC_PDCTL1) |= 0x100;
	REG(PSC_EPCCR) = 0x02;

	for (i = 0; i < 100; i++) {
		if (!(REG(PSC_PTSTAT) & 0x02))
			break;
	}

	REG(PSC_GBLCTL) &= ~0x1f;
}
#endif

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	/* arch number of the board */
	gd->bd->bi_arch_number = MACH_TYPE_DAVINCI_XENON ;

	/* address of boot parameters */
	gd->bd->bi_boot_params = LINUX_BOOT_PARAM_ADDR;

	/* Workaround for TMS320DM6446 errata 1.3.22 */
	REG(PSC_SILVER_BULLET) = 0;

	/* Power on required peripherals */
	lpsc_on(DAVINCI_LPSC_EMAC);
	lpsc_on(DAVINCI_LPSC_EMAC_WRAPPER);
	lpsc_on(DAVINCI_LPSC_MDIO);
	lpsc_on(DAVINCI_LPSC_I2C);
	lpsc_on(DAVINCI_LPSC_UART0);
	lpsc_on(DAVINCI_LPSC_TIMER1);
	lpsc_on(DAVINCI_LPSC_GPIO);
	lpsc_on(DAVINCI_LPSC_MMC_SD);

	lpsc_on(DAVINCI_LPSC_VPSSMSTR);
	lpsc_on(DAVINCI_LPSC_VPSSSLV);
#ifdef POWERUP_DSP
	/* Powerup the DSP */
	dsp_on();
#endif
	/* Bringup UART0 out of reset */
	REG(UART0_PWREMU_MGMT) = 0x0000e003;

	/* Enable GIO3.3V cells used for EMAC */
	REG(VDD3P3V_PWDN) = 0;

	/* Enable UART0 MUX lines */
	REG(PINMUX1) |= 1;

	/* Enable EMAC and AEMIF pins */
	REG(PINMUX0) = 0x80000c00;

	/* Enable I2C pin Mux */
	REG(PINMUX1) |= (1 << 7);

	/* Set the Bus Priority Register to appropriate value */
	REG(VBPR) = 0x20;

	timer_init();

#ifdef CONFIG_CMD_I2C
	gpio_set_val(GPIO_THS_ENABLE, 0);
#endif
#ifdef CONFIG_GP_HSYNC
	gpio_set_dir(CONFIG_GP_HSYNC, DIR_IN);
	gpio_set_dir(CONFIG_GP_VSYNC, DIR_IN);
#endif
	return(0);
}

int get_rom_mac (char *v_rom_mac);

extern char version_string[];
int misc_init_r (void)
{
	char macAddr[6];
	int		clk = 0;
	clk = ((REG(PLL2_PLLM) + 1) * 27) / ((REG(PLL2_DIV2) & 0x1f) + 1);
	printf ("ARM Clock : %dMHz\n", ((REG(PLL1_PLLM) + 1) * 27 ) / 2);
	printf ("DDR Clock : %dMHz\n", (clk / 2));

	if (get_rom_mac(macAddr)) {
		printf( "Mac address %02x:%02x:%02x:%02x:%02x:%02x\n",
			macAddr[0], macAddr[1], macAddr[2],
			macAddr[3], macAddr[4], macAddr[5] );
	} else
		printf( "No mac address assigned\n" );

	if (!eth_hw_init()) {
		printf("ethernet init failed!\n");
	} else {
		printf("ETH PHY   : %s\n", phy.name);
	}
#if 0
	i2c_read (0x39, 0x00, 1, (u_int8_t *)&i, 1);
	setenv ("videostd", ((i  & 0x80) ? "pal" : "ntsc"));
#endif
	setenv ("version", version_string );
#ifdef CONFIG_CMD_I2C
	gpio_set_val(GPIO_DISPLAY_SELECT, getenv("vmux_select_davinci") ? 0 : 1);
#endif
	return(0);
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	volatile unsigned int * pCfg = (unsigned int *)0x20000008;
	int addressBits = 13+2;	/* 13 row bits, 32 bits wide */
	unsigned int cfg = *pCfg;

	addressBits += (cfg&7)+8;		/* # of column address bits */
	addressBits += ((cfg>>4)&7);	/* # of bank address bits */
	addressBits -= ((cfg>>14)&1);	/* only 16 bit bus*/
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = 1<<addressBits;

	return(0);
}

extern int parse_mac
   ( char const *macString, // input
     char       *macaddr ); // output: not NULL-terminated
extern void dm644x_eth_set_mac_addr(const u_int8_t *addr);

int get_rom_mac (char *v_rom_mac)
{
   char *cmac ;
   if( NULL != ( cmac = getenv(MAC_VARIABLE) ) ){
      if (parse_mac(cmac,v_rom_mac) ){
         dm644x_eth_set_mac_addr((unsigned char*)v_rom_mac);
         return 1 ;
      }
   }
   return 0 ;
}

int set_rom_mac (char const *v_rom_mac)
{
   char cMac[20];
   sprintf( cMac, "%02x:%02x:%02x:%02x:%02x:%02x",
            v_rom_mac[0], v_rom_mac[1], v_rom_mac[2],
            v_rom_mac[3], v_rom_mac[4], v_rom_mac[5] );
   setenv(MAC_VARIABLE, cMac);
   return (0 == saveenv());
}
