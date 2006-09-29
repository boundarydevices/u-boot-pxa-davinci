/*
 * (C) Copyright 2006
 * Boundary Devices
 * <feedback@boundarydevices.com>
 * Configuation settings for the TI OMAP Innovator board.
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

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IMX31
#define CONFIG_LCD		1
//#define BOARD_LATE_INIT		1
#define CFG_PREFER_SERIAL_CONSOLE 1
/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CFG_MEMTEST_START	0x80000000
#define CFG_MEMTEST_END		0x81000000

#define EPIT1_BASE	0x53f94000		//timer1
#define EPIT2_BASE	0x53f98000		//timer2
#define EP_CR 0
#define EP_SR 4
#define EP_LR 8
#define EP_CMPR 0x0c
#define EP_CNT 0x10
#define CFG_HZ		32000	//use the 32k clock

#define CONFIG_CMDLINE_TAG		1	/* enable passing of ATAGs  */
#define CONFIG_SETUP_MEMORY_TAGS	1
#define CONFIG_INITRD_TAG	 1
#define CONFIG_MISC_INIT_R		1	/* call misc_init_r during start up */


/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN		(CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_MMU_SPACE_RESERVED (1<<14)


#define PHYSICAL_SDRAM 0x80000000	//to CS0 address is 512M max
#define PHYSICAL_CS0 0xA0000000 //128 meg space
#define PHYSICAL_CS1 0xA8000000 //128 meg space
#define PHYSICAL_CS2 0xB0000000	//32 meg space
#define PHYSICAL_CS3 0xB2000000	//32 meg space
#define PHYSICAL_CS4 0xB4000000	//32 meg space
#define PHYSICAL_CS5 0xB6000000	//32 meg space

/*
 * Hardware drivers
 */
#if 0
#define CONFIG_NET__ 1
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC_USE_32_BIT	//can use 32 bit even though bus is 16
#define CONFIG_SMC91111_BASE    (PHYSICAL_CS4+0x0300)
#undef CONFIG_SMC91111_EXT_PHY
#endif

#define CONFIG_IMX_SERIAL
#define UART1_BASE 0x43f90000
#define UART2_BASE 0x43f94000
#define UART3_BASE 0x5000c000
#define UART4_BASE 0x43fb0000
#define UART5_BASE 0x43fb4000

#define UART_BASE UART1_BASE
#define CONFIG_BAUDRATE		115200
#define CONFIG_PERIPH_CLK	51170508
/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

#define CFG_MMC_BASE		0x50004000
#define CONFIG_DOS_PARTITION	1

#define CFG_NAND_BOOT 1

#ifdef CFG_NAND_BOOT
#define NAND_CMDS (CFG_CMD_NAND | CFG_CMD_JFFS2)
#define CONFIG_JFFS2_NAND	1	/* jffs2 on nand support */
#else
#define NAND_CMDS 0
#endif

#ifdef CONFIG_NET__
#define NET_CMDS (CFG_CMD_NET | CFG_CMD_DHCP | CFG_CMD_IMI | CFG_CMD_PING)
#else
#define NET_CMDS 0
#endif

#ifdef CONFIG_LCD
#define VIDEO_CMDS CFG_CMD_BMP
#else
#define VIDEO_CMDS 0
#endif
//CFG_CMD_FLASH |
//#define CONFIG_LCDPANEL	/* Dynamic LCD Panel Support */

#define CONFIG_COMMANDS  (NET_CMDS | NAND_CMDS | VIDEO_CMDS | \
	CFG_CMD_BDI | CFG_CMD_MEMORY | CFG_CMD_ENV | \
	CFG_CMD_FAT | CFG_CMD_NOT | CFG_CMD_MISC | \
	CFG_CMD_MMC | CFG_CMD_AUTOSCRIPT) //| CFG_CMD_EXT2 | CFG_CMD_USB )

#define CONFIG_MMC		1
#define CMD_XMODEM 1
////////
//a prototype Bourne shell grammar parser
#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "
////////////
/* #define CONFIG_BOOTP_MASK	CONFIG_BOOTP_DEFAULT */

/* this must be included AFTER the definition of CONFIG_COMMANDS (if any) */
#include <cmd_confdefs.h>

/* Use the new NAND code. (BOARDLIBS = drivers/nand/libnand.a required) */
#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices */
#define NAND_MAX_CHIPS		1
#define CFG_NAND_BASE		0xb8000000
#define NFC_BUFFER 0x0
#define NFC_SPARE 0x0800
#define NFC_REGS 0x0e00

#define CFG_NAND_16BIT		1		//1 is true, 0 is false(8bit)
#define CFG_NAND_2K_PAGE	1		//1 is true, 0 is false(512 byte page)

/* NAND debugging */
#define CFG_DFC_DEBUG1 /* usefull */
#define CFG_DFC_DEBUG2  /* noisy */
#define CFG_DFC_DEBUG3  /* extremly noisy  */
#define CONFIG_MTD_DEBUG
#define CONFIG_MTD_DEBUG_VERBOSE 1

#define CFG_NAND_PROG_ERASE_TO	3000	//in uSecs
#define CFG_NAND_OTHER_TO		 100	//in uSecs


#define CONFIG_JFFS2_NAND 1
#define CONFIG_JFFS2_NAND_DEV 0                 /* nand device jffs2 lives on */
#define CONFIG_JFFS2_NAND_OFF 0                 /* start of jffs2 partition */
#define CONFIG_JFFS2_NAND_SIZE 2*1024*1024      /* size of jffs2 partition */

#if 0
#define CONFIG_NEW_NAND_CODE
#define CFG_NAND_BASE_LIST	{ CFG_NAND_BASE }
/* nand timeout values */
#define CFG_NAND_SENDCMD_RETRY	3
#undef NAND_ALLOW_ERASE_ALL	/* Allow erasing bad blocks - don't use */

#define ADDR_COLUMN		1
#define ADDR_PAGE		2
#define ADDR_COLUMN_PAGE	3

#define NAND_ChipID_UNKNOWN	0x00
#define NAND_MAX_FLOORS		1
#endif
///////////////////////////////////////////////////////////////////////////

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"while not mmcdet ; do cls ; lecho \"insert SD card\" ; sleep 1 ; done ; cls ; " \
                                "if mmcwp ; then lecho \"write protected\" ; else lecho \"not write protected\" ; fi ; " \
                                "mmcinit; " \
                                "if fatload mmc 0 80000000 init.scr ; then autoscr 80000000 ; fi"
#define CONFIG_BOOTARGS		"console=ttymxc0,115200 DEBUG=1 ENV=/etc/bashrc init=/linuxrc rw root=/dev/ram0"

/*
 * Miscellaneous configurable options
 */
#define CFG_LONGHELP				/* undef to save memory */
#define CFG_PROMPT	"Mercury # "	/* Monitor Command Prompt */
#define CFG_CBSIZE	256			/* Console I/O Buffer Size*/
/* Print Buffer Size */
#define CFG_PBSIZE	(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)
#define CFG_MAXARGS	16			/* max number of command args */
#define CFG_BARGSIZE	CFG_CBSIZE		/* Boot Argument Buffer Size*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */
#define CFG_LOAD_ADDR	0x7fc0	/* default load address */

/*-----------------------------------------------------------------------
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*-----------------------------------------------------------------------
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0x80000000	/* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE 	0x04000000	/* 64 MB */

/*-----------------------------------------------------------------------
 * FLASH and environment organization

 * Top varies according to amount fitted
 * Reserve top 4 blocks of flash
 * - ARM Boot Monitor
 * - Unused
 * - SIB block
 * - U-Boot environment
 *
 */
#define CFG_FLASH_BASE		PHYSICAL_CS0
#define CFG_MAX_FLASH_SECT 	64
#define CFG_MAX_FLASH_BANKS	1		/* max number of memory banks */
#define PHYS_FLASH_SIZE 	0x01000000	/* 16MB */
#define CFG_FLASH_ERASE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2*CFG_HZ)	/* Timeout for Flash Write */

#define CFG_MONITOR_LEN		0x00100000

#if 1
//#define CFG_NO_FLASH
#define CFG_ENV_IS_IN_NAND (1)
#define CFG_ENV_SECT_SIZE		0x20000		/* 128KB */
#define CFG_ENV_SIZE			8192		/* 8KB */
#define CFG_ENV_OFFSET			0x40000
#define CFG_ENV_OFFSET_REDUND	0x60000
#else
//#define CFG_ENV_IS_IN_FLASH	(1)
/*
 * Move up the U-Boot & monitor area if more flash is fitted.
 * If this U-Boot is to be run on Integrators with varying flash sizes,
 * drivers/cfi_flash.c::flash_init() can read the Integrator CP_FLASHPROG
 * register and dynamically assign CFG_ENV_ADDR & CFG_MONITOR_BASE
 * - CFG_MONITOR_BASE is set to indicate that the environment is not
 * embedded in the boot monitor(s) area
 */
#define CFG_ENV_SECT_SIZE	0x40000		/* 256KB */
#define CFG_ENV_SIZE		8192		/* 8KB */

#define __FLASH_END (CFG_FLASH_BASE+PHYS_FLASH_SIZE)
#define CFG_ENV_ADDR		(__FLASH_END - CFG_MONITOR_LEN)
#define CFG_MONITOR_BASE	(CFG_ENV_ADDR + CFG_ENV_SECT_SIZE)
#endif

/*-----------------------------------------------------------------------
 */

/*
 * The ARM boot monitor initializes the board.
 * However, the default U-Boot code also performs the initialization.
 * If desired, this can be prevented by defining SKIP_LOWLEVEL_INIT
 * - see documentation supplied with board for details of how to choose the
 * image to run at reset/power up
 * e.g. whether the ARM Boot Monitor runs before U-Boot
 */
#define CONFIG_SKIP_LOWLEVEL_INIT

/*
 * The ARM boot monitor does not relocate U-Boot.
 * However, the default U-Boot code performs the relocation check,
 * and may relocate the code if the memory map is changed.
 * If necessary this can be prevented by defining SKIP_RELOCATE_UBOOT

#define SKIP_CONFIG_RELOCATE_UBOOT

 */
/*-----------------------------------------------------------------------
 * There are various dependencies on the core module (CM) fitted
 * Users should refer to their CM user guide
 * - when porting adjust u-boot/Makefile accordingly
 * to define the necessary CONFIG_ s for the CM involved
 * see e.g. cp_926ejs_config
 */

#include "armcoremodule.h"

/*
 * If CONFIG_SKIP_LOWLEVEL_INIT is not defined &
 * the core module has a CM_INIT register
 * then the U-Boot initialisation code will
 * e.g. ARM Boot Monitor or pre-loader is repeated once
 * (to re-initialise any existing CM_INIT settings to safe values).
 *
 * This is usually not the desired behaviour since the platform
 * will either reboot into the ARM monitor (or pre-loader)
 * or continuously cycle thru it without U-Boot running,
 * depending upon the setting of Integrator/CP switch S2-4.
 *
 * However it may be needed if Integrator/CP switch S2-1
 * is set OFF to boot direct into U-Boot.
 * In that case comment out the line below.
#undef	CONFIG_CM_INIT
 */

#endif /* __CONFIG_H */
