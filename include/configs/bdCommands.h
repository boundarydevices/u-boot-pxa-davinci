#ifndef __BDCOMMANDS_H__
#define __BDCOMMANDS_H__ "$Id$"
/*
 * Common command-set definition for Boundary Devices boards.
 *
 */
#include <config_cmd_default.h>
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
#define CONFIG_CMD_BMP
#define CONFIG_CMD_DHCP 
#define CONFIG_CMD_ENV 
#define CONFIG_CMD_EXT2 
#define CONFIG_CMD_FAT 
#define CONFIG_CMD_FLASH 
#define CONFIG_CMD_MISC 
#define CONFIG_CMD_MMC 
#define CONFIG_CMD_NET 
#define CONFIG_CMD_NOT 
#define CMD_XMODEM 1

#endif

