/*
 * davinci_mmc.h
 *
 * This header file declares the external interfaces needed
 * by the "mmc" command (common/cmd_mmc.c).
 *
 * Change History : 
 *
 * $Log$
 *
 *
 * Copyright Boundary Devices, Inc. 2007
 */
#ifndef __DAVINCI_MMC_H__
#define __DAVINCI_MMC_H__

#define MMC_CMDAT_R1	    		(0x0001UL)
#define MMC_CMDAT_R2	    		(0x0002UL)
#define MMC_CMDAT_R3	    		(0x0003UL)

#if 0
typedef struct mmc_cid
{
/* FIXME: BYTE_ORDER */
   uchar year:4,
   month:4;
   uchar sn[3];
   uchar fwrev:4,
   hwrev:4;
   uchar name[6];
   uchar id[3];
} mmc_cid_t;

typedef struct _mmc_csd {
	uchar  csd_structure;
	uchar  spec_vers;
	uchar  taac;
	uchar  nsac;
	uchar  tran_speed;
	ushort ccc;
	uchar  read_bl_len;
	uchar  read_bl_partial;
	uchar  write_blk_misalign;
	uchar  read_blk_misalign;
	uchar  dsr_imp;
	ushort c_size;
	uchar  vdd_r_curr_min;
	uchar  vdd_r_curr_max;
	uchar  vdd_w_curr_min;
	uchar  vdd_w_curr_max;
	uchar  c_size_mult;
	union {
		struct { /* MMC system specification version 3.1 */
			uchar  erase_grp_size;
			uchar  erase_grp_mult;
		} v31;
		struct { /* MMC system specification version 2.2 */
			uchar  sector_size;
			uchar  erase_grp_size;
		} v22;
	} erase;
	uchar  wp_grp_size;
	uchar  wp_grp_enable;
	uchar  default_ecc;
	uchar  r2w_factor;
	uchar  write_bl_len;
	uchar  write_bl_partial;
	uchar  file_format_grp;
	uchar  copy;
	uchar  perm_write_protect;
	uchar  tmp_write_protect;
	uchar  file_format;
	uchar  ecc;
} mmc_csd_t ;
#endif

typedef struct mmc_csd {
	unsigned char		mmca_vsn;
	unsigned short		cmdclass;
	unsigned short		tacc_clks;
	unsigned int		tacc_ns;
	unsigned int		r2w_factor;
	unsigned int		max_dtr;
	unsigned int		read_blkbits;
	unsigned int		write_blkbits;
	unsigned int		capacity;
	unsigned int		read_partial:1,
				read_misalign:1,
				write_partial:1,
				write_misalign:1;
} mmc_csd_t ;

typedef struct mmc_cid {
	unsigned int		manfid;
	char			prod_name[8];
	unsigned int		serial;
	unsigned short		oemid;
	unsigned short		year;
	unsigned char		hwrev;
	unsigned char		fwrev;
	unsigned char		month;
} mmc_cid_t ;

extern uchar *mmc_cmd(ushort cmd, uint arg, ushort cmdat);

#define MMC_DEFAULT_RCA			1

#define MMC_CMD_RESET			0
#define MMC_CMD_SEND_OP_COND		1
#define MMC_CMD_ALL_SEND_CID 		2
#define MMC_CMD_SET_RCA			3
#define MMC_CMD_SEND_CSD 		9
#define MMC_CMD_SEND_CID 		10
#define MMC_CMD_STOP		12
#define MMC_CMD_SEND_STATUS		13
#define MMC_CMD_SET_BLOCKLEN		16
#define MMC_CMD_READ_BLOCK		17
#define MMC_CMD_RD_BLK_MULTI		18
#define MMC_CMD_WRITE_BLOCK		24

#define SD_APP_CMD55  55         /* 0x37 */
#define SD_APP_CMD41  41         /* 0x29 */
#define SD_APP_CMD_SET_BUS_WIDTH 6
#define SD_STATUS     13         /* 0x0D */

#endif
