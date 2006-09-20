/*
 *  linux/drivers/mmc/mmc_pxa.h
 *
 *  Author: Vladimir Shebordaev, Igor Oblakov
 *  Copyright:  MontaVista Software Inc.
 *
 *  $Id: mmc.h,v 1.3 2005/04/16 17:05:19 ericn Exp $
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#ifndef __MMC_PXA_P_H__
#define __MMC_PXA_P_H__

/* PXA-250 MMC controller registers */

/* MMC_STRPCL */
#define MMC_STRPCL_STOP_CLK     	(0x0001UL)
#define MMC_STRPCL_START_CLK		(0x0002UL)

/* MMC_STAT */
#define MMC_STAT_END_CMD_RES		(0x01UL << 13)
#define MMC_STAT_PRG_DONE       	(0x01UL << 12)
#define MMC_STAT_DATA_TRAN_DONE     (0x01UL << 11)
#define MMC_STAT_CLK_RUNNING		(0x01UL << 8)

#define MMC_STAT_RECV_FIFO_FULL     (0x01UL << 7)
#define MMC_STAT_XMIT_FIFO_EMPTY    (0x01UL << 6)
#define MMC_STAT_RES_CRC_ERROR      (0x01UL << 5)
#define MMC_STAT_CRC_READ_ERROR     (0x01UL << 3)
#define MMC_STAT_CRC_WRITE_ERROR    (0x01UL << 2)
#define MMC_STAT_TIME_OUT_RESPONSE  (0x01UL << 1)
#define MMC_STAT_READ_TIME_OUT      (0x01UL)


#define MMC_STAT_ERRORS (MMC_STAT_RES_CRC_ERROR \
	|MMC_STAT_CRC_READ_ERROR|MMC_STAT_TIME_OUT_RESPONSE \
	|MMC_STAT_READ_TIME_OUT|MMC_STAT_CRC_WRITE_ERROR)
	
//MPCTL=0x04001800 0b0    0 0001 0000000000 00 0110 0000000000
//					BRMO	 PD		MFd	        MFi	   MFn
// Fref      x 2 x ((MFi + MFn/MFd)/PD) = Fvco
//32768*1024 x 2 x ((  6 +   0/(0+1))/(1+1))
//32768*1024 x 6 = 201326592 Hz = 201.3 MHz

//UPCTL=0x04112014 0b0    0 0001 0000010001 00 1000 0000010100(USB PLL control)
//					BRMO	 PD		MFd	        MFi	   MFn
// Fref      x 2 x ((MFi + MFn/MFd)/PD) = Fvco
//32768*1024 x 2 x ((  8 +  20/(17+1))/(1+1)) = fraction error


//
//SPCTL=0x04043001 0b0    0 0001 0000000100 00 1100 0000000001
//					BRMO	 PD		MFd	        MFi	   MFn
// Fref      x 2 x ((MFi + MFn/MFd)/PD) = Fvco
//32768*1024 x 2 x (( 12 +   1/(4+1))/(1+1)) =
//32768*1024 x 2 x (( 12 +   1/5)/2) =
//32768*1024 x 2 x 6.1 =
//32768*1024 x 12.2 = 409364070 Hz = 409.3 MHz

//PDR0 (Post Divider register 0) PER_PODF (peripheral postdivisor) = 7 (meaning 8)
//409.3/(7+1) = 51.170508 MHz

/* MMC_CLKRT */
//divisor to get to 25M, (51.17M/2 = 25.58M)
#define MMC_CLKRT_25MHZ	 		((0<<4) + (2-1))
#define MMC_CLKRT_20MHZ	 		((0<<4) + (3-1))	//17M
#define MMC_CLKRT_10MHZ	 		((0<<4) + (5-1))	//10.2M
#define MMC_CLKRT_5MHZ	  		((0<<4) + (10-1))
#define MMC_CLKRT_2_5MHZ		((1<<4) + (10-1))
#define MMC_CLKRT_1_25MHZ       ((2<<4) + (10-1))
#define MMC_CLKRT_0_625MHZ      ((4<<4) + (10-1))
#define MMC_CLKRT_0_3125MHZ     ((8<<4) + (10-1))

/* MMC_SPI */
#define MMC_SPI_DISABLE	 		(0x00UL)
#define MMC_SPI_EN	  		(0x01UL)
#define MMC_SPI_CS_EN	   		(0x01UL << 2)
#define MMC_SPI_CS_ADDRESS      	(0x01UL << 3)
#define MMC_SPI_CRC_ON	  		(0x01UL << 1)

/* MMC_CMDAT */
#define MMC_CMDAT_4BIT_MODE		(2<<8)
//#define MMC_CMDAT_MMC_DMA_EN		(0x0001UL << 7)
#define MMC_CMDAT_INIT	  		(1<<7)
//#define MMC_CMDAT_STREAM		(0x0001UL << 4)
#define MMC_CMDAT_WRITE	 		(1<<4)
#define MMC_CMDAT_READ	  		(0<<4)
#define MMC_CMDAT_DATA_EN       (1<<3)
#define MMC_CMDAT_R1	    	(1)
#define MMC_CMDAT_R2	    	(2)
#define MMC_CMDAT_R3	    	(3)

#define MMC_CMDAT_BLOCK	 		0	//not part of this register
#define MMC_CMDAT_BUSY	  		0	//not part of this register

/* MMC_RESTO */
#define MMC_RES_TO_MAX	  		(0x007fUL) /* [6:0] */

/* MMC_RDTO */
#define MMC_READ_TO_MAX	 		(0x0ffffUL) /* [15:0] */

/* MMC_BLKLEN */
#define MMC_BLK_LEN_MAX	 		(0x03ffUL) /* [9:0] */

/* MMC_PRTBUF */
#define MMC_PRTBUF_BUF_PART_FULL       	(0x01UL)
#define MMC_PRTBUF_BUF_FULL		(0x00UL    )

/* MMC_I_MASK */
//#define MMC_I_MASK_TXFIFO_WR_REQ	(0x01UL << 6)
//#define MMC_I_MASK_RXFIFO_RD_REQ	(0x01UL << 5)
//#define MMC_I_MASK_CLK_IS_OFF	   	(0x01UL << 4)
//#define MMC_I_MASK_STOP_CMD	 	(0x01UL << 3)
#define MMC_I_MASK_END_CMD_RES	  	(0x01UL << 2)
#define MMC_I_MASK_PRG_DONE	 	(0x01UL << 1)
#define MMC_I_MASK_DATA_TRAN_DONE       (0x01UL)
#define MMC_I_MASK_ALL	      		(0x07fUL)



/* MMC_CMD */
#define MMC_CMD_INDEX_MAX       	(0x006fUL)  /* [5:0] */
#define CMD(x)  (x)

#define MMC_DEFAULT_RCA			1

#define MMC_BLOCK_SIZE			512
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
#define SD_STATUS     13         /* 0x0D */

#define MMC_MAX_BLOCK_SIZE		512

#define MMC_R1_IDLE_STATE		0x01
#define MMC_R1_ERASE_STATE		0x02
#define MMC_R1_ILLEGAL_CMD		0x04
#define MMC_R1_COM_CRC_ERR		0x08
#define MMC_R1_ERASE_SEQ_ERR		0x01
#define MMC_R1_ADDR_ERR			0x02
#define MMC_R1_PARAM_ERR		0x04

#define MMC_R1B_WP_ERASE_SKIP		0x0002
#define MMC_R1B_ERR			0x0004
#define MMC_R1B_CC_ERR			0x0008
#define MMC_R1B_CARD_ECC_ERR		0x0010
#define MMC_R1B_WP_VIOLATION		0x0020
#define MMC_R1B_ERASE_PARAM		0x0040
#define MMC_R1B_OOR			0x0080
#define MMC_R1B_IDLE_STATE		0x0100
#define MMC_R1B_ERASE_RESET		0x0200
#define MMC_R1B_ILLEGAL_CMD		0x0400
#define MMC_R1B_COM_CRC_ERR		0x0800
#define MMC_R1B_ERASE_SEQ_ERR		0x1000
#define MMC_R1B_ADDR_ERR		0x2000
#define MMC_R1B_PARAM_ERR		0x4000

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

typedef struct mmc_csd
{
	uchar	ecc:2,
		file_format:2,
		tmp_write_protect:1,
		perm_write_protect:1,
		copy:1,
		file_format_grp:1;
	uint64_t content_prot_app:1,
		rsvd3:4,
		write_bl_partial:1,
		write_bl_len:4,
		r2w_factor:3,
		default_ecc:2,
		wp_grp_enable:1,
		wp_grp_size:5,
		erase_grp_mult:5,
		erase_grp_size:5,
		c_size_mult1:3,
		vdd_w_curr_max:3,
		vdd_w_curr_min:3,
		vdd_r_curr_max:3,
		vdd_r_curr_min:3,
		c_size:12,
		rsvd2:2,
		dsr_imp:1,
		read_blk_misalign:1,
		write_blk_misalign:1,
		read_bl_partial:1;

	ushort	read_bl_len:4,
		ccc:12;
	uchar	tran_speed;
	uchar	nsac;
	uchar	taac;
	uchar	rsvd1:2,
  		spec_vers:4,
		csd_structure:2;
} mmc_csd_t;


typedef struct sd_status {
   ulong  prot_size ;
   ushort card_type ;
   ushort bus_width:2,
          secured_mode:1,
          unused0: 13 ;
} sd_status_t ;

extern uchar *
mmc_cmd(ushort cmd, uint arg, ushort cmdat);

#endif /* __MMC_PXA_P_H__ */
