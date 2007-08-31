/*
 * NAND driver for TI DaVinci based boards.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Based on Linux DaVinci NAND driver by TI. Original copyright follows:
 */

/*
 *
 * linux/drivers/mtd/nand/nand_davinci.c
 *
 * NAND Flash Driver
 *
 * Copyright (C) 2006 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
 *
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
 *
 *  Overview:
 *   This is a device driver for the NAND flash device found on the
 *   DaVinci board which utilizes the Samsung k9k2g08 part.
 *
 Modifications:
 ver. 1.0: Feb 2005, Vinod/Sudhakar
 -
 *
 */

#include <common.h>

#ifdef CFG_USE_NAND
#if !defined(CFG_NAND_LEGACY)

#include <nand.h>
#include <asm/arch/nand_defs.h>
#include <asm/arch/emif_defs.h>

extern struct nand_chip nand_dev_desc[CFG_MAX_NAND_DEVICE];


unsigned char nandCtlSetClr[] = {
	0x00,		//1 - NAND_CTL_SETNCE, 2 - NAND_CTL_CLRNCE, Select/Deselect Chip Select (nCE)
	MASK_CLE,	//3 - NAND_CTL_SETCLE, 4 - NAND_CTL_CLRCLE, Select/Deselect the command latch (CLE)
	MASK_ALE,	//5 - NAND_CTL_SETALE, 6 - NAND_CTL_CLRALE, Select/Deselect the address latch (ALE)
	0x00,		//7 - NAND_CTL_SETWP,  8 - NAND_CTL_CLRWP,  Set/Clear write protection (WP) Not used!
};

static void nand_davinci_hwcontrol(struct mtd_info *mtd, int cmd)
{
	cmd--;
	if ((cmd>=0)&&(cmd<8)) {
		struct		nand_chip *nand = mtd->priv;
		u_int32_t	addr = (u_int32_t)nand->IO_ADDR_W;
		if (cmd&1) addr &= ~nandCtlSetClr[cmd>>1];
		else addr |= nandCtlSetClr[cmd>>1];
		nand->IO_ADDR_W = (void *)addr;
//		DEBUG (MTD_DEBUG_LEVEL3, "cmd:0x%x newAddress: 0x%x\n",cmd+1,addr);
	} else {
		DEBUG (MTD_DEBUG_LEVEL0, "Invalid command 0x%x\n",cmd+1);
	}
}

/* Set WP on deselect, write enable on select */
static void nand_davinci_select_chip(struct mtd_info *mtd, int chip)
{
#ifdef SONATA_BOARD_GPIOWP
#define GPIO_SET_DATA01	0x01c67018
#define GPIO_CLR_DATA01	0x01c6701c
#define GPIO_NAND_WP	(1 << 4)
	if (chip < 0) {
		REG(GPIO_CLR_DATA01) |= GPIO_NAND_WP;
	} else {
		REG(GPIO_SET_DATA01) |= GPIO_NAND_WP;
	}
#endif
}

#ifdef CFG_NAND_HW_ECC
#ifdef CFG_NAND_LARGEPAGE
static struct nand_oobinfo davinci_nand_oobinfo = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 12,
	.eccpos = {8, 9, 10, 24, 25, 26, 40, 41, 42, 56, 57, 58},
	.oobfree = { {2, 6}, {12, 12}, {28, 12}, {44, 12}, {60, 4} }
};
#elif defined(CFG_NAND_SMALLPAGE)
static struct nand_oobinfo davinci_nand_oobinfo = {
	.useecc = MTD_NANDECC_AUTOPLACE,
	.eccbytes = 3,
	.eccpos = {0, 1, 2},
	.oobfree = { {6, 2}, {8, 8} }
};
#else
#error "Either CFG_NAND_LARGEPAGE or CFG_NAND_SMALLPAGE must be defined!"
#endif


static u_int32_t nand_davinci_readecc(struct mtd_info *mtd, u_int32_t chipNum)
{
	u_int32_t	ecc = 0;
	emifregs	emif_base_addr;

	emif_base_addr = (emifregs)DAVINCI_ASYNC_EMIF_CNTRL_BASE;
	ecc = emif_base_addr->NANDF_ECC[chipNum];
	return(ecc);
}
static void nand_davinci_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct		nand_chip *nand = mtd->priv;
	u_int32_t	addr = (u_int32_t)nand->IO_ADDR_R;
	u_int32_t   chipNum=(addr-CFG_NAND_BASE)>>25;		//0 - cs2, 1 - cs3, 2 - cs4, 3 - cs5
	emifregs	emif_addr;
	int		dummy;
	if (chipNum>=4) return;

//	DEBUG (MTD_DEBUG_LEVEL3, "enable_hwecc\n");
	emif_addr = (emifregs)DAVINCI_ASYNC_EMIF_CNTRL_BASE;
	dummy = nand_davinci_readecc(mtd, chipNum);	//reset ecc to 0
	emif_addr->NANDFCR |= (1 << (8+chipNum));	//start ECC on chip select region+2
}

static int nand_davinci_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	struct		nand_chip *nand = mtd->priv;
	u_int32_t	addr = (u_int32_t)nand->IO_ADDR_R;
	u_int32_t   chipNum=(addr-CFG_NAND_BASE)>>25;		//0 - cs2, 1 - cs3, 2 - cs4, 3 - cs5
	if (chipNum>=4) return -1;
	u_int32_t	tmp = nand_davinci_readecc(mtd, chipNum);
#ifdef CONFIG_MTD_DEBUG
	//calculate it ourself and compare
	{
		unsigned int i = 0x00000fff;	//2**12 bits/ecc
		unsigned int j = 512;			//512 bytes/ecc
		unsigned int ecc = 0;
		do {
			unsigned int k=i;
			unsigned int v = *dat++;
			do {
				if (v&1) ecc ^= k;
				k += (1<<16);
				k--;
				v >>= 1;
			} while (v);
			i += 8<<16;
			i -= 8;
			j--;
		} while (j);
		if (tmp!=ecc) {
			printf("Hardware Ecc: %x, Calculated: %x\r\n",tmp,ecc);
			tmp=ecc;
		}
	}
#endif
	tmp = (tmp&0x0fff)|((tmp&0x0fff0000)>>4);		//squeeze 0 middle bits out so that it fits in 3 bytes
	tmp = ~tmp;									//invert so that erased block ecc is correct
	*ecc_code++ = (u_char)(tmp);
	*ecc_code++ = (u_char)(tmp >> 8);
	*ecc_code++ = (u_char)(tmp >> 16);
	return(0);
}


static int nand_davinci_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	u_int32_t	eccNand = read_ecc[0] | (read_ecc[1]<<8) | (read_ecc[2]<< 16);
	u_int32_t	eccCalc = calc_ecc[0] | (calc_ecc[1]<<8) | (calc_ecc[2]<< 16);
	u_int32_t	diff = eccCalc ^ eccNand;
	if (diff) {
		if ((((diff>>12)^diff)&0xfff)!=0xfff) {
			/* Uncorrectable error */
			DEBUG (MTD_DEBUG_LEVEL0, "ECC UNCORRECTED_ERROR %x\n",diff);
			return(-1);
		}
		/* Correctable error */
		DEBUG (MTD_DEBUG_LEVEL0, "Correcting single bit ECC error at offset: %d, bit: %d\n", diff>>(12+3), ((diff>>12)&7));
		dat[diff>>(12+3)] ^= (1 << ((diff>>12)&7));
	}
	return(0);
}
#endif

#ifdef NAND_GPIO_READY_LIST
static unsigned char nandGpioReadyList[] = {NAND_GPIO_READY_LIST};
#endif

static int nand_davinci_dev_ready(struct mtd_info *mtd)
{
#ifdef NAND_GPIO_READY_LIST
#define GP_DIR 0x00
#define GP_OUT 0x04
#define GP_SET 0x08
#define GP_CLR 0x0C
#define GP_IN  0x10

	struct		nand_chip *nand = mtd->priv;
	u_int32_t	addr = (u_int32_t)nand->IO_ADDR_R;
	u_int32_t   chipNum=(addr-CFG_NAND_BASE)>>25;		//0 - cs2, 1 - cs3, 2 - cs4, 3 - cs5
	if (chipNum<4) {
		unsigned int gp = nandGpioReadyList[chipNum];
		if (gp) {
			unsigned int bank = (gp>>5);
			volatile unsigned int* p = (unsigned int*)(DAVINCI_GPIO_BASE+(bank*0x28)+0x10);
			int ret = (p[GP_IN>>2] >> (gp&0x1f))&1;
			DEBUG (MTD_DEBUG_LEVEL3, "Ready: %d\n", ret);
			return ret;
		}
	}
#endif
	emifregs	emif_addr = (emifregs)DAVINCI_ASYNC_EMIF_CNTRL_BASE;
	return (emif_addr->NANDFSR & 0x1);
}

/**
 * nand_davinci_waitfunc - [DEFAULT]  wait until the command is done
 * @mtd:	MTD device structure
 * @this:	NAND chip structure
 * @state:	state to select the max. timeout value
 *
 * Wait for command done. This applies to erase and program only
 * Erase can take up to 400ms and program up to 20ms according to
 * general NAND and SmartMedia specs
 *
*/
static int nand_davinci_waitfunc(struct mtd_info *mtd, struct nand_chip *this, int state)
{
	ulong start = get_timer(0);
	volatile u_int8_t * p = (volatile u_int8_t *)((unsigned int)this->IO_ADDR_R);
	unsigned long timeout = (state == FL_ERASING)? ((CFG_HZ * 400) / 1000) : ((CFG_HZ * 20) / 1000);
#if 0	//enable this if you don't trust your ready pin.
	unsigned long mwait = (state == FL_ERASING)? 400 : 20;
	udelay(mwait*1000);
#endif

	p[MASK_CLE] = NAND_CMD_STATUS;

	while (1) {
		if (get_timer(start) > timeout) {
			printf("!!!Nand wait Timeout!!!\n");
			return 1;
		}
		if (nand_davinci_dev_ready(mtd)) break;
	}
	return (*p);
}

static void nand_flash_init(void)
{
	u_int32_t	acfg1 = 0x3ffffffc;
	u_int32_t	acfg2 = 0x3ffffffc;
	u_int32_t	acfg3 = 0x3ffffffc;
	u_int32_t	acfg4 = 0x3ffffffc;
	emifregs	emif_regs;

	/*------------------------------------------------------------------*
	 *  NAND FLASH CHIP TIMEOUT @ 459 MHz                               *
	 *                                                                  *
	 *  AEMIF.CLK freq   = PLL1/6 = 459/6 = 76.5 MHz                    *
	 *  AEMIF.CLK period = 1/76.5 MHz = 13.1 ns                         *
	 *                                                                  *
	 *------------------------------------------------------------------*/
	 acfg1 = 0
	 	| (0 << 31 )	/* selectStrobe */
	 	| (0 << 30 )	/* extWait */
	 	| (1 << 26 )	/* writeSetup	10 ns */
	 	| (3 << 20 )	/* writeStrobe	40 ns */
	 	| (1 << 17 )	/* writeHold	10 ns */
	 	| (1 << 13 )	/* readSetup	10 ns */
	 	| (5 << 7 )	/* readStrobe	60 ns */
	 	| (1 << 4 )	/* readHold	10 ns */
	 	| (3 << 2 )	/* turnAround	?? ns */
	 	| (0 << 0 )	/* asyncSize	8-bit bus */
	 	;

	emif_regs = (emifregs)DAVINCI_ASYNC_EMIF_CNTRL_BASE;

	emif_regs->AWCCR |= 0x10000000;
	emif_regs->AB1CR = acfg1;	/* 0x08244128 */;
	emif_regs->AB2CR = acfg2;
	emif_regs->AB3CR = acfg3;
	emif_regs->AB4CR = acfg4;
	emif_regs->NANDFCR = 0x00000101;
}

int board_nand_init(struct nand_chip *nand)
{
#ifdef NAND_GPIO_READY_LIST
	struct		nand_chip *nand = mtd->priv;
	u_int32_t	addr = (u_int32_t)nand->IO_ADDR_R;
	u_int32_t   chipNum=(addr-CFG_NAND_BASE)>>25;		//0 - cs2, 1 - cs3, 2 - cs4, 3 - cs5
	if (chipNum<4) {
		unsigned int gp = nandGpioReadyList[chipNum];
		if (gp) {
			unsigned int bank = (gp>>5);
			volatile unsigned int* p = (unsigned int*)(DAVINCI_GPIO_BASE+(bank*0x28)+0x10);
			int val;
			int bitNum = (gp&0x1f);
			p[GP_DIR>>2] |= 1<<bitNum;		//make sure it's an input
			val = (p[GP_IN>>2] >> bitNum)&1;
			DEBUG (MTD_DEBUG_LEVEL3, "Bank: %x curVal:%x inReg:%p=%x\n", bank,val,&p[GP_IN>>2],p[GP_IN>>2]);
		}
	}
#endif

	DEBUG (MTD_DEBUG_LEVEL3, "Nand base read:%p write:%p\n",nand->IO_ADDR_R,nand->IO_ADDR_W);
	nand->chip_delay  = 0;
	nand->select_chip = nand_davinci_select_chip;
#ifdef CFG_NAND_USE_FLASH_BBT
	nand->options	  = NAND_USE_FLASH_BBT;
#endif
#ifdef CFG_NAND_HW_ECC
	nand->eccmode     = NAND_ECC_HW3_512;
	nand->autooob	  = &davinci_nand_oobinfo;
	nand->calculate_ecc = nand_davinci_calculate_ecc;
	nand->correct_data  = nand_davinci_correct_data;
	nand->enable_hwecc  = nand_davinci_enable_hwecc;
#else
	nand->eccmode     = NAND_ECC_SOFT;
#endif

	/* Set address of hardware control function */
	nand->hwcontrol = nand_davinci_hwcontrol;

	nand->dev_ready = nand_davinci_dev_ready;
	nand->waitfunc = nand_davinci_waitfunc;

	nand_flash_init();

	return(0);
}

#else
#error "U-Boot legacy NAND support not available for DaVinci chips"
#endif
#endif	/* CFG_USE_NAND */
