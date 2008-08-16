/*
 * (C) Copyright 2008
 * Troy Kisky, Boundary Devices
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

#define GP_SPI_ENABLE	CONFIG_OKW_GP_SPI_ENABLE
#define GP_SPI_CLK	CONFIG_OKW_GP_SPI_CLK
#define GP_SPI_DATA	CONFIG_OKW_GP_SPI_DATA

#define GP_INDEX(gp) ((gp<96)? (gp>>5) : 0x40)

#define GPLRx_OFFSET 0x00	//level
#define GPDRx_OFFSET 0x0c	//direction
#define GPSRx_OFFSET 0x18	//set
#define GPCRx_OFFSET 0x24	//clear
#define GRERx_OFFSET 0x30	//rising edge detect
#define GFERx_OFFSET 0x3c	//falling edge detect
#define GEDRx_OFFSET 0x48	//edge detect status
#define GAFRx_OFFSET 0x54	//alternate function

#define GPIO_BASE 0x40e00000
#define __REG(a) *(volatile unsigned int *)(a)
#define GPLRx(gp) __REG((GPIO_BASE + GPLRx_OFFSET + (GP_INDEX(gp)<<2)))
#define GPDRx(gp) __REG((GPIO_BASE + GPDRx_OFFSET + (GP_INDEX(gp)<<2)))
#define GPSRx(gp) __REG((GPIO_BASE + GPSRx_OFFSET + (GP_INDEX(gp)<<2)))
#define GPCRx(gp) __REG((GPIO_BASE + GPCRx_OFFSET + (GP_INDEX(gp)<<2)))
#define GRERx(gp) __REG((GPIO_BASE + GRERx_OFFSET + (GP_INDEX(gp)<<2)))
#define GFERx(gp) __REG((GPIO_BASE + GFERx_OFFSET + (GP_INDEX(gp)<<2)))
#define GEDRx(gp) __REG((GPIO_BASE + GEDRx_OFFSET + (GP_INDEX(gp)<<2)))
#define GAFRx(gp) __REG((GPIO_BASE + GAFRx_OFFSET + ((gp>>4)<<2)))

struct af_saved
{
	unsigned int af_enable;
	unsigned int af_clk;
	unsigned int af_data;

	unsigned int dir_enable;
	unsigned int dir_clk;
	unsigned int dir_data;
};

void okaya_setup(struct af_saved* paf)
{
	paf->af_enable = GAFRx(GP_SPI_ENABLE);
	paf->af_clk = GAFRx(GP_SPI_CLK);
	paf->af_data = GAFRx(GP_SPI_DATA);

	paf->dir_enable = GPDRx(GP_SPI_ENABLE);
	paf->dir_clk = GPDRx(GP_SPI_CLK);
	paf->dir_data = GPDRx(GP_SPI_DATA);

	GPSRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f));
	GPSRx(GP_SPI_CLK) = (1<<(GP_SPI_CLK&0x1f));
	GPSRx(GP_SPI_DATA) = (1<<(GP_SPI_DATA&0x1f));

	GPDRx(GP_SPI_ENABLE) |= (1<<(GP_SPI_ENABLE&0x1f));
	GPDRx(GP_SPI_CLK) |= (1<<(GP_SPI_CLK&0x1f));
	GPDRx(GP_SPI_DATA) |= (1<<(GP_SPI_DATA&0x1f));

	GAFRx(GP_SPI_ENABLE) &= ~(3<<((GP_SPI_ENABLE&0x0f)<<1));
	GAFRx(GP_SPI_CLK) &= ~(3<<((GP_SPI_CLK&0x0f)<<1));
	GAFRx(GP_SPI_DATA) &= ~(3<<((GP_SPI_DATA&0x0f)<<1));
}

void okaya_restore(struct af_saved* paf)
{
	GPDRx(GP_SPI_ENABLE) = paf->dir_enable;
	GPDRx(GP_SPI_CLK) = paf->dir_clk;
	GPDRx(GP_SPI_DATA) = paf->dir_data;

	GAFRx(GP_SPI_ENABLE) = paf->af_enable;
	GAFRx(GP_SPI_CLK) = paf->af_clk;
	GAFRx(GP_SPI_DATA) = paf->af_data;
}
int okaya_shift(unsigned int value, int len)
{
	GPCRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f));

	do {
		len--;
		if (len<0) break;
		GPCRx(GP_SPI_CLK) = (1<<(GP_SPI_CLK&0x1f));
		if (value & (1<<len)) {
			GPSRx(GP_SPI_DATA) = (1<<(GP_SPI_DATA&0x1f));
		} else {
			GPCRx(GP_SPI_DATA) = (1<<(GP_SPI_DATA&0x1f));
		}
		GPSRx(GP_SPI_CLK) = (1<<(GP_SPI_CLK&0x1f));
		GPSRx(GP_SPI_CLK) = (1<<(GP_SPI_CLK&0x1f)); //delay
	} while (1);
	GPSRx(GP_SPI_DATA) = (1<<(GP_SPI_DATA&0x1f));
	GPSRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f));
	GPSRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f)); //delay
	GPSRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f)); //delay
	GPSRx(GP_SPI_ENABLE) = (1<<(GP_SPI_ENABLE&0x1f)); //delay
	return 0;
}
int okaya_write(unsigned int reg_addr, unsigned int value)
{
	okaya_shift((0x70<<16)|reg_addr, 24);
	okaya_shift((0x72<<16)|value, 24);
	return 0;
}
/*
 * okw register value
 *
 * Syntax:
 * okw [register] [value]
 */

int do_okw(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc >= 3) {
		unsigned int reg_addr = 0;
		unsigned int value = 0;
		reg_addr = simple_strtoul(argv[1], NULL, 10);
		value = simple_strtoul(argv[2], NULL, 10);
		if ((reg_addr<0x10000)&&(value<0x10000)) {
			struct af_saved af;
			okaya_setup(&af);
			okaya_write(reg_addr,value);
			okaya_restore(&af);
		} else {
			printf("both parameters 16 bits only\n");
		}
		return 0;
	} else if (argc == 1) {
		struct af_saved af;
		okaya_setup(&af);
		okaya_write(0x01,0x7300);
		okaya_write(0x02,0x0200);
		okaya_write(0x03,0x6364);
		okaya_write(0x04,0x04c7);
		okaya_write(0x05,0xfc80);
		okaya_write(0x0a,0x4008);
		okaya_write(0x0d,0x3229);
		okaya_write(0x0e,0x3200);
		okaya_write(0x1e,0x00d2);
		okaya_restore(&af);
		return 0;
	} else {
		printf("Invalid number of parameters\n");
	}
	return 1;
}

/***************************************************/

U_BOOT_CMD(
	okw,	5,	0,	do_okw,
	"okw <register> <value> - write to okaya lcd panel with bit-banged spi interface\n",
	"register - 16 bit register\n"
	"value  - 16 bit data\n"
);
