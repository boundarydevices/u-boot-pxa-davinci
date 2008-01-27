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

#define MMC_CMDAT_BUSY		(1<<8)

#define MMC_CMDAT_R1		(1<<9)
#define MMC_CMDAT_R2		(2<<9)
#define MMC_CMDAT_R3		(3<<9)
#define MMC_CMDAT_RSP_MASK	(3<<9)

#define MMC_CMDAT_READ		(0<<11)
#define MMC_CMDAT_WRITE		(1<<11)
#define MMC_CMDAT_DATA_EN	(1<<13)
#define MMC_CMDAT_INIT		(1<<14)

typedef struct mmc_registers {
	unsigned long volatile MMCCTL;		/* 0x00 */
	unsigned long volatile MMCCLK;		/* 0x04 */
	unsigned long volatile MMCST0;		/* 0x08 */
	unsigned long volatile MMCST1;		/* 0x0C */

	unsigned long volatile MMCIM;		/* 0x10 */
	unsigned long volatile MMCTOR;		/* 0x14 */
	unsigned long volatile MMCTOD;		/* 0x18 */
	unsigned long volatile MMCBLEN;		/* 0x1C */

	unsigned long volatile MMCNBLK;		/* 0x20 */
	unsigned long volatile MMCNBLC;		/* 0x24 */
	unsigned long volatile MMCDRR;		/* 0x28 */
	unsigned long volatile MMCDXR;		/* 0x2C */

	unsigned long volatile MMCCMD;		/* 0x30 */
	unsigned long volatile MMCARGHL;	/* 0x34 */
	unsigned long volatile MMCRSP01;	/* 0x38 */
	unsigned long volatile MMCRSP23;	/* 0x3C */

	unsigned long volatile MMCRSP45;	/* 0x40 */
	unsigned long volatile MMCRSP67;	/* 0x44 */
	unsigned long volatile MMCDRSP;		/* 0x48 */
	unsigned long volatile spare1;		/* 0x4C */

	unsigned long volatile MMCCIDX;		/* 0x50 */
	unsigned long volatile spare2;		/* 0x54 */
	unsigned long volatile spare3;		/* 0x58 */
	unsigned long volatile spare4;		/* 0x5C */

	unsigned long volatile spare5;		/* 0x60 */
	unsigned long volatile spare6;		/* 0x64 */
	unsigned long volatile spare7;		/* 0x68 */
	unsigned long volatile spare8;		/* 0x6C */

	unsigned long volatile spare9;		/* 0x70 */
	unsigned long volatile MMCFIFOCTL;	/* 0x74 */
} mmc_registers_t;

#if 1
#define MMCFIFOCTL_FIFOLEV (0<<2)	/* 16 bytes in fifo when DRRDY/DXRDY gets set */
#else
#define MMCFIFOCTL_FIFOLEV (1<<2)	/* 32 bytes in fifo when DRRDY/DXRDY gets set */
#endif

#define MMCCTL_BIGENDIAN_WRITE      (1<<10)
#define MMCCTL_BIGENDIAN_READ       (1<<9)
#define MMCCTL_DATEG_MASK           (7<<6)
#define MMCCTL_DATEG_DISABLED       (0<<6)
#define MMCCTL_DATEG_RISING         (1<<6)
#define MMCCTL_DATEG_FALLING        (2<<6)
#define MMCCTL_DATEG_BOTH           (3<<6)
#define MMCCTL_BUSWIDTH_4           (1<<2)
#define MMCCTL_CMDRST               (1<<1)
#define MMCCTL_DATRST               (1<<0)

typedef enum {
	MMCSD_EVENT_TRDONE = (1 << 12),
	MMCSD_EVENT_READ = (1 << 10),
	MMCSD_EVENT_WRITE = (1 << 9),
	MMCSD_EVENT_ERROR_CMDCRC = (1 << 7),
	MMCSD_EVENT_ERROR_DATACRC = ((1 << 6) | (1 << 5)),
	MMCSD_EVENT_ERROR_CMDTIMEOUT = (1 << 4),
	MMCSD_EVENT_ERROR_DATATIMEOUT = (1 << 3),
	MMCSD_EVENT_EOFCMD = (1 << 2),
	MMCSD_EVENT_CARD_EXITBUSY = (1 << 1),
	MMCSD_EVENT_BLOCK_XFERRED = (1 << 0)
} mmcsdevent;

#define MMCSD_EVENT_ERROR (MMCSD_EVENT_ERROR_DATATIMEOUT \
                          |MMCSD_EVENT_ERROR_CMDTIMEOUT \
                          |MMCSD_EVENT_ERROR_DATACRC \
                          |MMCSD_EVENT_ERROR_CMDCRC)
#define MMCSD_TIME_OUT_COUNT 0x1ffff

#define DAVINCI_MMCCLK_ENABLE (1<<8)

#define DAVINCI_ST1_MMCCLK_STOPPED (1<<1)

typedef struct intc_registers_t {
	unsigned int fiq0;	/* 0x0 */
	unsigned int fiq1;	/* 0x4 */
	unsigned int irq0;	/* 0x8 */
	unsigned int irq1;	/* 0xC */
	unsigned int fiqentry;	/* 0x10 */
	unsigned int irqentry;	/* 0x14 */
	unsigned int eint0;	/* 0x18 */
	unsigned int eint1;	/* 0x1C */
	unsigned int inctl;	/* 0x20 */
	unsigned int eabase;	/* 0x24 */
	unsigned int resv1;	/* 0x28 */
	unsigned int resv2;	/* 0x2C */
	unsigned int intpri0;	/* 0x30 */
	unsigned int intpri1;	/* 0x34 */
	unsigned int intpri2;	/* 0x38 */
	unsigned int intpri3;	/* 0x3C */
	unsigned int intpri4;	/* 0x30 */
	unsigned int intpri5;	/* 0x34 */
	unsigned int intpri6;	/* 0x38 */
	unsigned int intpri7;	/* 0x3C */
} intc_registers;

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
