#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * neon.h
 *
 * This header file declares the configuration constants for the Boundary
 * Devices Neon board.
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
 *
 * Copyright Boundary Devices, Inc. 2005
 */
#include "select.h"
#if (PLATFORM_TYPE==HALOGEN)||(PLATFORM_TYPE==ARGON)||(PLATFORM_TYPE==NEON270)
#define CONFIG_PXA270		1	/* This is a PXA270 CPU    */
#define CONFIG_PXA27X		1	/* Which is a PXA27X */
#else
#define CONFIG_PXA250		1	/* This is an PXA250 CPU    */
#endif

#define CONFIG_SETUP_MEMORY_TAGS 1
#define CONFIG_INITRD_TAG	 1


#if (PLATFORM_TYPE==NEON)||(PLATFORM_TYPE==NEONB)
//These don't use the built-in pxa lcd controller
#else
#define CONFIG_PXALCD          1     /* Allow PXA display controller as well */
#endif

/*
 * High Level Configuration Options
 * (easy to change)
 */
#define CONFIG_NEON		1	/* on a Neon Board	    */
#define CONFIG_SM501		1
#define PXALCD          1     /* Allow PXA display controller as well */
// #define CONFIG_LCD		1
#define CONFIG_LCD_MULTI 1

#define CONFIG_MMC		1
#define BOARD_LATE_INIT		1

#undef CONFIG_USE_IRQ			/* we don't need IRQ/FIQ stuff */

/*
 * Size of malloc() pool
 */
#define CFG_MALLOC_LEN	    (CFG_ENV_SIZE + 128*1024)
#define CFG_GBL_DATA_SIZE	128	/* size in bytes reserved for initial data */
#define CFG_MMU_SPACE_RESERVED (1<<14)

/*
 * Hardware drivers
 */
#define CONFIG_DRIVER_SMC91111
#define CONFIG_SMC91111_BASE 0x10000300
#define CONFIG_SMC_USE_32_BIT

/************************************************************
 * USB support
 ************************************************************/
#define LITTLEENDIAN		1  /* Needed by usb_ohci.c */
#define CFG_DEVICE_DEREGISTER	1  /* Needed by usb_kbd */
#define CONFIG_DOS_PARTITION	1
#define CONFIG_USB_OHCI		1
#define CONFIG_USB_KEYBOARD	1
#define CONFIG_USB_STORAGE	1

/*
 * select serial console configuration
 */
#define CONFIG_FFUART	       1       /* we use FFUART on NEON */

/* allow to overwrite serial and ethaddr */
//#define CONFIG_ENV_OVERWRITE

#define CONFIG_BAUDRATE		115200
#define CONFIG_SUPPORT_VFAT
#define CONFIG_LCDPANEL	/* Dynamic LCD Panel Support */

#include <config_cmd_default.h>
#define CONFIG_CMD_DFL
#define CONFIG_CMD_MMC
#define CONFIG_CMD_FAT
#define CONFIG_CMD_FLASH
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_ENV
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_USB
#define CONFIG_CMD_NOT
#define CONFIG_CMD_MISC
#define CONFIG_CMD_BMP
#define CONFIG_CMD_XMODEM
#define CONFIG_CMD_I2CTEST
#define CONFIG_CMD_ECHO

#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_BOOTD
#undef CONFIG_CMD_LOADS
#undef CONFIG_CMD_LOADB
#undef CONFIG_CMD_ITEST
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_DIAG
#undef CONFIG_CMD_DATE
#undef CONFIG_CMD_BOOTP
#undef CONFIG_CMD_NFS

#ifdef CONFIG_CMD_I2CTEST
#define CONFIG_BOOTDELAY	0
#define CONFIG_BOOTCOMMAND	\
"if i2ctest ; then " \
	"lecho \"i2ctest succeeded\" ; " \
	"while not mmcdet ; do cls ; lecho \"insert SD card\" ; sleep 1 ; done ; " \
	"lecho \"SD card Detected\" ; " \
	"while mmcwp ; do cls ; lecho \"remove write protect\" ; sleep 1 ; done ; " \
	"lecho \"not write protected\" ; " \
	"mmcinit ; " \
	"if fatload mmc 0 A0030000 bnk.nb0 ; then g A0030000 ; else lecho \"file bnk.nb0 not found\" ; fi ; " \
"else lecho \"i2ctest failed\" ; fi ; "
#if (PLATFORM_TYPE==NEONB)
#define CONFIG_BOOTDELAY	1
#endif

#else

#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTCOMMAND	"mmcinit && fatload mmc 0 a0000000 init.scr && autoscr a0000000 ; fi"
#endif

#define CONFIG_BOOTARGS		"console=ttyS0,115200 DEBUG=1 ENV=/etc/bashrc init=/linuxrc rw mtdparts=phys:1024k(armboot),256k(params),-(rootfs1) root=/dev/mtdblock3 rootfstype=cramfs"
#define CONFIG_CMDLINE_TAG
#define CONFIG_CMDLINE_EDITING

#define CONFIG_GZIP
#define CONFIG_CMD_GUNZIP
#define CONFIG_CMDLINE_EDITING

#define	CONFIG_AUTOBOOT_KEYED		/* Enable password protection */
#define	CONFIG_AUTOBOOT_PROMPT		"\nEnter password - autoboot in %d sec...\n"
#define	CONFIG_AUTOBOOT_DELAY_STR	"\x1b\x1b\x1b"

#include "lcdPanelChoice.h"

#define LCD_BPP			LCD_COLOR8


#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	230400		/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	2		/* which serial port to use */
#endif

/*
 * Miscellaneous configurable options
 */
#define CFG_HUSH_PARSER		1
#define CFG_PROMPT_HUSH_PS2	"> "

#define CFG_LONGHELP				/* undef to save memory		*/
#ifdef CFG_HUSH_PARSER
#define CFG_PROMPT		"$ "		/* Monitor Command Prompt */
#else
#define CFG_PROMPT		"=> "		/* Monitor Command Prompt */
#endif

#define MAX_CMDBUF_SIZE		512
#define CFG_CBSIZE		MAX_CMDBUF_SIZE		/* Console I/O Buffer Size	*/
#define CFG_PBSIZE (CFG_CBSIZE+sizeof(CFG_PROMPT)+16) /* Print Buffer Size */
#define CFG_MAXARGS		16		/* max number of command args	*/
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size	*/
#define CFG_DEVICE_NULLDEV	1

#define CFG_MEMTEST_START	0xa0400000	/* memtest works on	*/
#define CFG_MEMTEST_END		0xa0800000	/* 4 ... 8 MB in DRAM	*/

#undef	CFG_CLKS_IN_HZ		/* everything, incl board info, in Hz */

#define CFG_LOAD_ADDR		0xa0030000	/* default load address */

#define CFG_HZ			3686400		/* incrementer freq: 3.6864 MHz */
#define CFG_CPUSPEED		0x161		/* set core clock to 400/200/100 MHz */

						/* valid baudrates */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
#define CFG_MMC_BASE		0xF0000000

/*
 * Stack sizes
 *
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE	(128*1024)	/* regular stack */
#ifdef CONFIG_USE_IRQ
#define CONFIG_STACKSIZE_IRQ	(4*1024)	/* IRQ stack */
#define CONFIG_STACKSIZE_FIQ	(4*1024)	/* FIQ stack */
#endif

/*
 * Physical Memory Map
 */
#define CONFIG_NR_DRAM_BANKS	1	   /* we have 1 bank of DRAM */
#define PHYS_SDRAM_1		0xa0000000 /* SDRAM Bank #1 */
#define PHYS_SDRAM_1_SIZE	0x04000000 /* 64 MB */

#define PHYS_FLASH_1		0x00000000 /* Flash Bank #1 */
#define PHYS_FLASH_2		0x04000000 /* Flash Bank #2 */
#define PHYS_FLASH_SIZE		0x02000000 /* 32 MB */
#define PHYS_FLASH_BANK_SIZE	0x02000000 /* 32 MB Banks */
#define PHYS_FLASH_SECT_SIZE	0x00040000 /* 256 KB sectors (x2) */

#define CFG_DRAM_BASE		0xa0000000
#define CFG_DRAM_SIZE		0x04000000


/*
 * Memory settings
 */
#define CFG_MSC0_VAL		0x23F223F2
#define CFG_MSC1_VAL		0x3FF1A441
#define CFG_MSC2_VAL		0x7FF97FF1
#define CFG_MDCNFG_VAL		0x00001AC9
#define CFG_MDREFR_VAL		0x00018018
#define CFG_MDMRS_VAL		0x00000000

/*
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks		*/
#define CFG_MAX_FLASH_SECT	128  /* max number of sectors on one chip    */

/* timeout values are in ticks */
#define CFG_FLASH_ERASE_TOUT	(25*CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(25*CFG_HZ) /* Timeout for Flash Write */

/*
 * Environment is saved in flash at offset 1MB
 */
#define CFG_ENV_IS_IN_FLASH	1
#define CFG_FLASH_BASE     0
#define CFG_ENV_ADDR		   ((CFG_FLASH_BASE)+0x100000)	/* Addr of Environment Sector	*/
#define CFG_ENV_OFFSET     ((CFG_ENV_ADDR)-(CFG_FLASH_BASE))
#define CFG_ENV_SIZE		   PHYS_FLASH_SECT_SIZE	/* Total Size of Environment Sector	*/

/*
 * GPIO settings
 */
#define CFG_GPSR0_VAL		0x00008000
#define CFG_GPSR1_VAL		0x00FC0382
#define CFG_GPSR2_VAL		0x0001FFFF
#define CFG_GPCR0_VAL		0x00000000
#define CFG_GPCR1_VAL		0x00000000
#define CFG_GPCR2_VAL		0x00000000
#define CFG_GPDR0_VAL		0x0060A800
#define CFG_GPDR1_VAL		0x00FF0382
#define CFG_GPDR2_VAL		0x0001C000
#define CFG_GAFR0_L_VAL		0x98400000
#define CFG_GAFR0_U_VAL		0x00002950
#define CFG_GAFR1_L_VAL		0x000A9558
#define CFG_GAFR1_U_VAL		0x0005AAAA
#define CFG_GAFR2_L_VAL		0xA0000000
#define CFG_GAFR2_U_VAL		0x00000002

#define CFG_PSSR_VAL		0x20

/*
 * PCMCIA and CF Interfaces
 */
#define CFG_MECR_VAL		0x00000000
#define CFG_MCMEM0_VAL		0x00010504
#define CFG_MCMEM1_VAL		0x00010504
#define CFG_MCATT0_VAL		0x00010504
#define CFG_MCATT1_VAL		0x00010504
#define CFG_MCIO0_VAL		0x00004715
#define CFG_MCIO1_VAL		0x00004715

#endif

