/*
 * (C) Copyright 2003
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
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
//#define MMC_DEBUG
//#define DEBUG 1
#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>
#include <command.h>
#define MMC_BLOCK_SHIFT			9
#define MMC_BLOCK_SIZE			(1<<MMC_BLOCK_SHIFT)

#ifdef CONFIG_MMC

#ifdef CONFIG_IMX31
#include "mx31_pins.h"
#include "iomux.h"
void gpio_sdhc_active(int module)
{
	if (module==0) {
		mxc_request_iomux(MX31_PIN_SD1_CLK, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
		mxc_request_iomux(MX31_PIN_SD1_CMD, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
		mxc_request_iomux(MX31_PIN_SD1_DATA0, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
		mxc_request_iomux(MX31_PIN_SD1_DATA1, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
		mxc_request_iomux(MX31_PIN_SD1_DATA2, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
		mxc_request_iomux(MX31_PIN_SD1_DATA3, OUTPUTCONFIG_FUNC,INPUTCONFIG_FUNC);
	} else {
		/* TODO:what are the pins for SDHC2? */
	}
}
#endif


extern int
fat_register_device(block_dev_desc_t *dev_desc, int part_no);

static block_dev_desc_t mmc_dev;

block_dev_desc_t * mmc_get_dev(int dev)
{
	return (dev == 0) ? &mmc_dev : NULL;
}

/*
 * FIXME needs to read cid and csd info to determine block size
 * and other parameters
 */
static uchar mmc_buf[MMC_BLOCK_SIZE] __attribute__ ((aligned (32)));
static uchar resp[20] __attribute__ ((aligned (32)));
static mmc_csd_t mmc_csd;
static int startBlock = 0;
static char mmc_ready = 0;
static char f4BitMode = 0;
static char bHighCapacity = 0;

#ifdef CONFIG_IMX31
#define CLEAR_END_CMD_STAT MMC_STAT = 0xc0007e2f;	//MMC_STAT_END_CMD_RES | MMC_STAT_RES_CRC_ERROR | MMC_STAT_TIME_OUT_RESPONSE;
#define DISABLE_CLKSTOP_INT
#define	WAIT_FOR_CLOCK_TO_STOP  while ((MMC_STAT & MMC_STAT_CLK_RUNNING));
#define SET_MMC_ARG(a) MMC_ARG = a
#else
#define CLEAR_END_CMD_STAT
#define DISABLE_CLKSTOP_INT  MMC_I_MASK = ~MMC_I_MASK_CLK_IS_OFF;
#define	WAIT_FOR_CLOCK_TO_STOP	while (!(MMC_I_REG & MMC_I_REG_CLK_IS_OFF));
#define SET_MMC_ARG(a) MMC_ARGH = (a>>16); MMC_ARGL = (unsigned short)a
#endif

static void stop_clock( void )
{
	DISABLE_CLKSTOP_INT
	MMC_STRPCL = MMC_STRPCL_STOP_CLK;
//	debug("waiting for clock to stop\n");
	WAIT_FOR_CLOCK_TO_STOP
//	debug("clock stopped\n");
}

uchar *mmc_cmd(ushort cmd, uint arg, ushort cmdat)
{
	volatile u32 * res_fifo = &MMC_RES;
	unsigned short* prsp;
	ulong status;
	int words, i;

	debug("mmc_cmd %x %x %x\n", cmd, arg, cmdat);
	stop_clock();

	MMC_CMD    = cmd;
	SET_MMC_ARG(arg);
	if (f4BitMode) cmdat |= MMC_CMDAT_4BIT_MODE;
	MMC_CMDAT  = cmdat;
	CLEAR_END_CMD_STAT
	MMC_I_MASK = ~MMC_I_MASK_END_CMD_RES;
	MMC_STRPCL = MMC_STRPCL_START_CLK;
#ifdef CONFIG_IMX31
//	debug("waiting for END_CMD_RES\n");
	while (!(MMC_STAT & MMC_STAT_END_CMD_RES));
//	debug("found END_CMD_RES\n");
#else
	while (!(MMC_I_REG & MMC_I_REG_END_CMD_RES));
#endif

	status = MMC_STAT;
	debug("MMC status %lx\n", status);
	if (0) if (status & MMC_STAT_RES_CRC_ERROR) {
		MMC_STAT = MMC_STAT_RES_CRC_ERROR;	//write 1 to clear
		printf( "mmc_cmd response crc error: cmd: 0x%x, args: 0x%08x, status 0x%lx\n", cmd, arg, status );
		return 0;
	}
	if (status & MMC_STAT_TIME_OUT_RESPONSE) {
		MMC_STAT = MMC_STAT_TIME_OUT_RESPONSE; //write 1 to clear
		printf( "mmc_cmd timeout: cmd: 0x%x, args: 0x%08x, status 0x%lx\n", cmd, arg, status );
		return 0;
	}

	switch (cmdat & 0x3) {
		case MMC_CMDAT_R1:
		case MMC_CMDAT_R3:
			words = 3;
			prsp = (unsigned short*)(resp+(words<<1));
			i = words;
			while (i--) {
				*(--prsp) = (unsigned short)(*res_fifo);	//only low order 16 bits
			}
			break;

		case MMC_CMDAT_R2:
#ifdef CONFIG_IMX31
			{
				unsigned long rem=0;
				unsigned long tmp;
				words = 9;	//first byte of 136 bit response not stored, leaving 128 bits (8 words)
				prsp = (unsigned short*)(resp+(words<<1));
				i = words-1;
				while (i--) {
					tmp = (unsigned short)(*res_fifo);	//only low order 16 bits
					*(--prsp) = (unsigned short)(rem|(tmp>>8));
					rem = tmp<<8;
				}
				*(--prsp) = (unsigned short)rem;
			}
#else
			words = 9;
			prsp = (unsigned short*)(resp+(words<<1));
			i = words;
			while (i--) {
				*(--prsp) = (unsigned short)(*res_fifo);	//only low order 16 bits
			}
#endif
			break;
		//case 0 - no response to command is default
		default:
			*((unsigned int *)resp) = 0;
			return resp;
	}

#ifdef MMC_DEBUG
	for (i=0; i<words*2; i += 1) {
		printf("MMC resp[%d] = %02x\n", i, resp[i]);
	}
#endif
	return resp;
}

static void mmc_setblklen( ulong blklen )
{
	static ulong prevLen = -1UL ;
	if( blklen != prevLen )
	{
		/* set block len */
		mmc_cmd( MMC_CMD_SET_BLOCKLEN, blklen, MMC_CMDAT_R1);
		prevLen = blklen ;
	}
}

int mmc_ReadFifo(unsigned int* pDst,ulong len)
{
	unsigned int val;
	ulong status;
#ifdef CONFIG_IMX31
	MMC_STAT = MMC_STAT_DATA_TRAN_DONE;
	{
		unsigned int volatile *rxFIFO = (unsigned int volatile *)&(MMC_RXFIFO);
		while (len) {
			unsigned int stat = MMC_STAT;
			if (stat & MMC_STAT_RECV_FIFO_FULL) {
	   			int bytes = min((f4BitMode)?64:16,len);	//16 (1-bit mode), 64(4-bit mode)
	   			int rem = bytes&3;
//				debug("r%i",bytes);
   				len -= bytes;
   				bytes &= ~3;
   				while(bytes){
   					*pDst++ = *rxFIFO;
   					bytes -= 4;
   				}
   				if (rem) {
					if (rem&2) {
						val = *(unsigned short volatile *)rxFIFO;
						if (rem&1) val |= (*(unsigned char volatile *)rxFIFO)<<16;
					} else val = *(unsigned char volatile *)rxFIFO;
					*pDst = val;
					break;
   				}
			} else if (stat & MMC_STAT_ERRORS) break;
			else {
//				debug("w%x",stat);
//				udelay(100000);
			}
		}
	}
	while (!(MMC_STAT & MMC_STAT_DATA_TRAN_DONE)) {
//			debug("w2");
//			udelay(100000);
	}
#else
	MMC_I_MASK = ~MMC_I_MASK_RXFIFO_RD_REQ;
	{
		unsigned char volatile *rxFIFO = (unsigned char volatile *)&(MMC_RXFIFO);
		while (len) {
			if (MMC_I_REG & MMC_I_REG_RXFIFO_RD_REQ) {
	   			int bytes = min(32,len);
   				len -= bytes;
#ifdef CONFIG_PXA27X
   				if (bytes==32) {
   					pDst[0] = *((unsigned int volatile *)rxFIFO);
   					pDst[1] = *((unsigned int volatile *)rxFIFO);
   					pDst[2] = *((unsigned int volatile *)rxFIFO);
   					pDst[3] = *((unsigned int volatile *)rxFIFO);

   					pDst[4] = *((unsigned int volatile *)rxFIFO);
   					pDst[5] = *((unsigned int volatile *)rxFIFO);
   					pDst[6] = *((unsigned int volatile *)rxFIFO);
   					pDst[7] = *((unsigned int volatile *)rxFIFO);
   					pDst += 8;
   				} else
#endif
   				{
					while (bytes>=4) {
						// read in the byte from the FIFO
#ifdef CONFIG_PXA27X
						*pDst++ = *((unsigned int volatile *)rxFIFO);
#else
						val = *rxFIFO;
						val |= (*rxFIFO << 8);
						val |= (*rxFIFO << 16);
						val |= (*rxFIFO << 24);
						*pDst++ = val;
#endif
						bytes-=4;
					}
					if (bytes) {
						val = *rxFIFO;
						if (bytes&2) {
							val |= (*rxFIFO << 8);
							if (bytes&1) {
								val |= (*rxFIFO << 16);
							}
						}
						*pDst = val;
						break;
					}
   				}
   				//ignore MMC_STAT_RES_CRC_ERROR, PXA270 has bug with R2 responses
			} else if (MMC_STAT & (MMC_STAT_ERRORS & ~MMC_STAT_RES_CRC_ERROR)) break;
		}
	}
	MMC_I_MASK = ~MMC_I_MASK_DATA_TRAN_DONE;
	while (!(MMC_I_REG & MMC_I_REG_DATA_TRAN_DONE));
#endif
	status = MMC_STAT;
	if (status & (MMC_STAT_ERRORS & ~MMC_STAT_RES_CRC_ERROR)) {
		printf("MMC_STAT error %lx\n", status);
		return -1;
	}
	return 0;
}
static int mmc_block_read(uchar *dst, ulong blockNum, ulong len)
{
	uchar *resp;
	if (len == 0) {
		return 0;
	}

	debug("mmc_block_rd dst %lx blk# %lx len %ld\n", (ulong)dst, blockNum, len);
	mmc_setblklen( len );

	/* send read command */
	MMC_STRPCL = MMC_STRPCL_STOP_CLK;
	MMC_RDTO = 0xffff;
	MMC_NOB = 1;
	MMC_BLKLEN = len;
	blockNum += startBlock;
	if (!bHighCapacity) blockNum <<= MMC_BLOCK_SHIFT;
	resp = mmc_cmd(MMC_CMD_READ_BLOCK, blockNum,
			MMC_CMDAT_R1|MMC_CMDAT_READ|MMC_CMDAT_BLOCK|MMC_CMDAT_DATA_EN);

	return mmc_ReadFifo((unsigned int*)dst,len);
}


int
/****************************************************/
mmc_block_write(ulong blockNum, uchar *src, int len)
/****************************************************/
{
	uchar *resp;
	ulong status;
	unsigned int* pSrc = (unsigned int*)src;
	unsigned int val;
	if (len == 0) {
		return 0;
	}

	debug("mmc_block_wr blockNum %lx src %lx len %d\n", blockNum, (ulong)pSrc, len);

	/* set block len */
	resp = mmc_cmd(MMC_CMD_SET_BLOCKLEN, len, MMC_CMDAT_R1);


	/* send write command */
	MMC_STRPCL = MMC_STRPCL_STOP_CLK;
	MMC_NOB = 1;
	MMC_BLKLEN = len;
	blockNum += startBlock;
	if (!bHighCapacity) blockNum <<=MMC_BLOCK_SHIFT;
	resp = mmc_cmd(MMC_CMD_WRITE_BLOCK, blockNum,
			MMC_CMDAT_R1|MMC_CMDAT_WRITE|MMC_CMDAT_BLOCK|MMC_CMDAT_DATA_EN);

#ifdef CONFIG_IMX31
	MMC_STAT = MMC_STAT_PRG_DONE;
	{
		unsigned int volatile *txFIFO = (unsigned int volatile *)&(MMC_TXFIFO);
		while (len) {
			if (MMC_STAT & MMC_STAT_XMIT_FIFO_EMPTY) {
		   		int bytes = min((f4BitMode)?64:16,len);	//16 (1-bit mode), 64(4-bit mode)
	   			int rem = bytes&3;
   				len -= bytes;
   				bytes &= ~3;
   				while(bytes){
   					*txFIFO = *pSrc++;
   					bytes -= 4;
   				}
   				if (rem) {
   					val = *pSrc;
					if (rem&2) {
						*((unsigned short volatile *)txFIFO) = (unsigned short)val;
						if (rem&1) *((unsigned char volatile *)txFIFO) = (unsigned char)(val>>16);
					} else *((unsigned char volatile *)txFIFO) = (unsigned char)val);
					break;
   				}
			} else if (MMC_STAT & MMC_STAT_ERRORS) break;
		}
	}
	if (!(MMC_STAT & MMC_STAT_ERRORS)) {
		while (!(MMC_STAT & MMC_STAT_PRG_DONE));
	}
#else
	MMC_I_MASK = ~MMC_I_MASK_TXFIFO_WR_REQ;
	{
		unsigned char volatile *txFIFO = (unsigned char volatile *)&(MMC_TXFIFO);
		while (len) {
			if (MMC_I_REG & MMC_I_REG_TXFIFO_WR_REQ) {
				int bytes = min(32,len);
				len -= bytes;
#ifdef CONFIG_PXA27X
   				if (bytes==32) {
   					*((unsigned int volatile*)txFIFO) = pSrc[0];
   					*((unsigned int volatile*)txFIFO) = pSrc[1];
   					*((unsigned int volatile*)txFIFO) = pSrc[2];
   					*((unsigned int volatile*)txFIFO) = pSrc[3];

   					*((unsigned int volatile*)txFIFO) = pSrc[4];
   					*((unsigned int volatile*)txFIFO) = pSrc[5];
   					*((unsigned int volatile*)txFIFO) = pSrc[6];
   					*((unsigned int volatile*)txFIFO) = pSrc[7];
   					pSrc += 8;
   				} else
#endif
				{
					int b = bytes;
					while (b>=4) {
#ifdef CONFIG_PXA27X
	   					*((unsigned int volatile*)txFIFO) = *pSrc++;
#else
						val = *pSrc++;
						*txFIFO = (unsigned char)val;
						*txFIFO = (unsigned char)(val >> 8);
						*txFIFO = (unsigned char)(val >> 16);
						*txFIFO = (unsigned char)(val >> 24);
#endif
						b-=4;
					}
					if (b) {
						val = *pSrc;
						*txFIFO = (unsigned char)val;
						if (b&2) {
							*txFIFO = (unsigned char)(val>>8);
							if (b&1) {
								*txFIFO = (unsigned char)(val>>16);
							}
						}
						break;
					}
					if (bytes < 32) MMC_PRTBUF = MMC_PRTBUF_BUF_PART_FULL;
				}
			}
			status = MMC_STAT;
			if (status & MMC_STAT_ERRORS) {
				printf("MMC_STAT error %lx\n", status);
				return -1;
			}
		}
	}
	MMC_I_MASK = ~MMC_I_MASK_DATA_TRAN_DONE;
	while (!(MMC_I_REG & MMC_I_REG_DATA_TRAN_DONE));
	MMC_I_MASK = ~MMC_I_MASK_PRG_DONE;
	while (!(MMC_I_REG & MMC_I_REG_PRG_DONE));
#endif
	status = MMC_STAT;
	if (status & MMC_STAT_ERRORS) {
		printf("MMC_STAT error %lx\n", status);
		return -1;
	}
	return 0;
}


int
/****************************************************/
mmc_read(ulong src, uchar *dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, blockStart, blockEnd;
	ulong mmc_block_size;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;

	src -= CFG_MMC_BASE;
	end = src + size;
	part_start = src & (MMC_BLOCK_SIZE-1);
	part_end = end & (MMC_BLOCK_SIZE-1);
	blockStart = src>>MMC_BLOCK_SHIFT;
	blockEnd = end>>MMC_BLOCK_SHIFT;

	/* all block aligned accesses */
	debug("dst %lx end %lx pstart %lx pend %lx blockStart %lx blockEnd %lx\n",
		(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
			(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_read(mmc_buf, blockStart, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf+part_start, part_len);
		dst += part_len;
		blockStart++;
	}
	debug("dst %lx end %lx pstart %lx pend %lx blockStart %lx blockEnd %lx\n",
		(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
	for (; blockStart < blockEnd; blockStart++, dst += mmc_block_size) {
		debug("al dst %lx end %lx pstart %lx pend %lx blockStart %lx blockEnd %lx\n",
			(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_read((uchar *)(dst), blockStart, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
	if (part_end) {
		debug("pe dst %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
			(ulong)dst, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_read(mmc_buf, blockEnd, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(dst, mmc_buf, part_end);
	}
	return 0;
}

int
/****************************************************/
mmc_write(uchar *src, ulong dst, int size)
/****************************************************/
{
	ulong end, part_start, part_end, part_len, blockStart, blockEnd;
	ulong mmc_block_size;

	if (size == 0) {
		return 0;
	}

	if (!mmc_ready) {
		printf("Please initial the MMC first\n");
		return -1;
	}

	mmc_block_size = MMC_BLOCK_SIZE;

	dst -= CFG_MMC_BASE;
	end = dst + size;
	part_start = dst & (MMC_BLOCK_SIZE-1);
	part_end = end & (MMC_BLOCK_SIZE-1);
	blockStart = dst>>MMC_BLOCK_SHIFT;
	blockEnd = end>>MMC_BLOCK_SHIFT;

	/* all block aligned accesses */
	debug("src %p end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, end, part_start, part_end, blockStart, blockEnd);
	if (part_start) {
		part_len = mmc_block_size - part_start;
		debug("ps src %lx end %lx pstart %lx pend %lx astart %lx aend %lx\n",
			(ulong)src, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_read(mmc_buf, blockStart, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf+part_start, src, part_len);
		if ((mmc_block_write(blockStart, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
		blockStart++;
		src += part_len;
	}
	debug("src %p end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, end, part_start, part_end, blockStart, blockEnd);
	for (; blockStart < blockEnd; src += mmc_block_size, blockStart++) {
		debug("al src %p end %lx pstart %lx pend %lx astart %lx aend %lx\n",
			src, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_write(blockStart, (uchar *)src, mmc_block_size)) < 0) {
			return -1;
		}
	}
	debug("src %p end %lx pstart %lx pend %lx astart %lx aend %lx\n",
		src, end, part_start, part_end, blockStart, blockEnd);
	if (part_end) {
		debug("pe src %p end %lx pstart %lx pend %lx astart %lx aend %lx\n",
			src, end, part_start, part_end, blockStart, blockEnd);
		if ((mmc_block_read(mmc_buf, blockEnd, mmc_block_size)) < 0) {
			return -1;
		}
		memcpy(mmc_buf, src, part_end);
		if ((mmc_block_write(blockEnd, mmc_buf, mmc_block_size)) < 0) {
			return -1;
		}
	}
	return 0;
}

ulong
/****************************************************/
mmc_bread(int dev_num, ulong blknr, ulong blkcnt, void *dst)
/****************************************************/
{
	debug( "read %lu blocks at block #%lu\n", blkcnt, blknr );
   	
	if( 0 < blkcnt ) {
		if( 0 != getenv( "mblock" ) ) {
			int mmc_block_size = MMC_BLOCK_SIZE;
			ulong src = blknr * mmc_block_size + CFG_MMC_BASE;

			mmc_read(src, (uchar *)dst, blkcnt*mmc_block_size);
		} else {
			ulong curBlock = (blknr+startBlock);
			ulong status ;
			unsigned int* pDst = (unsigned int*)dst;
			MMC_RDTO   = 0xffff;
			MMC_BLKLEN = MMC_BLOCK_SIZE ;
			MMC_NOB    = blkcnt ;
         
			mmc_setblklen( MMC_BLOCK_SIZE );

			if (!bHighCapacity) curBlock <<= MMC_BLOCK_SHIFT;
			mmc_cmd( MMC_CMD_RD_BLK_MULTI, curBlock,
                  MMC_CMDAT_R1|MMC_CMDAT_READ|MMC_CMDAT_BLOCK|MMC_CMDAT_DATA_EN );

#ifdef CONFIG_IMX31
			MMC_STAT = MMC_STAT_DATA_TRAN_DONE;
			{
				unsigned int val;
				unsigned int volatile *rxFIFO = (unsigned int *)&(MMC_RXFIFO);
				// read the data
				for( blknr = 0 ; blknr < blkcnt ; blknr++ ) {
					unsigned len = MMC_BLOCK_SIZE ;
					while (len) {
						if (MMC_STAT & MMC_STAT_RECV_FIFO_FULL) {
					   		int bytes = min((f4BitMode)?64:16,len);	//16 (1-bit mode), 64(4-bit mode)
				   			int rem = bytes&3;
							len -= bytes;
							bytes &= ~3;
							while(bytes){
								*pDst++ = *rxFIFO;
								bytes -= 4;
							}
							if (rem) {
								if (rem&2) {
									val = *(unsigned short volatile *)rxFIFO;
									if (rem&1) val |= (*(unsigned char volatile *)rxFIFO)<<16;
								} else val = *(unsigned char volatile *)rxFIFO;
								*pDst = val;
								break;
							}
						} else if (MMC_STAT & MMC_STAT_ERRORS) break;
					}
				} // for each block
			}
			while (!(MMC_STAT & MMC_STAT_DATA_TRAN_DONE));
#else
			MMC_I_MASK = ~MMC_I_MASK_RXFIFO_RD_REQ;	//only enable RXFIFO_RD_REQ interrupt
			{
				unsigned char volatile *rxFIFO = (unsigned char *)&(MMC_RXFIFO);
				// read the data
				for( blknr = 0 ; blknr < blkcnt ; blknr++ ) {
					unsigned len = MMC_BLOCK_SIZE ;
					while (len) {
						len -= 32 ;
						while( (MMC_I_REG & MMC_I_REG_RXFIFO_RD_REQ) == 0 ) { }
#ifdef CONFIG_PXA27X
	   					pDst[0] = *((unsigned int volatile *)rxFIFO);
	   					pDst[1] = *((unsigned int volatile *)rxFIFO);
						pDst[2] = *((unsigned int volatile *)rxFIFO);
						pDst[3] = *((unsigned int volatile *)rxFIFO);

						pDst[4] = *((unsigned int volatile *)rxFIFO);
						pDst[5] = *((unsigned int volatile *)rxFIFO);
						pDst[6] = *((unsigned int volatile *)rxFIFO);
						pDst[7] = *((unsigned int volatile *)rxFIFO);
						pDst += 8;
#else
						{
							unsigned int val;
							int bytes = 32;
							while (bytes) {
								// read in the byte from the FIFO
								val = *rxFIFO;
								val |= (*rxFIFO << 8);
								val |= (*rxFIFO << 16);
								val |= (*rxFIFO << 24);
								*pDst++ = val;
								bytes-=4;
							}
						}
#endif
					}
				} // for each block
			}
			MMC_I_MASK = ~MMC_I_MASK_DATA_TRAN_DONE;	//only enable DATA_TRAN_DONE interrupt
			while (!(MMC_I_REG & MMC_I_REG_DATA_TRAN_DONE));
#endif
			status = MMC_STAT;
			if (status & MMC_STAT_ERRORS) {
				printf("MMC_STAT error %lx\n", status);
				return -1;
			}
//			printf( "completed mread... now stop\n" );

			mmc_cmd( MMC_CMD_STOP, 0, MMC_CMDAT_R1);

		} // multi-block read
	} // or why bother?
	return blkcnt;
}

static void dumpResponse( uchar *resp, unsigned bytes )
{
   debug( "rsp: " );
   if( resp )
   {
      while( bytes-- )
         debug( "%02X ", *resp++ );
      debug( "\n" );
   }
   else
      debug( "NULL\n" );
}
unsigned char* mmc_reset()
{
	unsigned char *resp;
	int i=0;
	do {
		resp = mmc_cmd(0, 0, MMC_CMDAT_INIT|0);	//reset
		if (resp) break;
		printf( "mmc_reset error\n" );
		i++;
		if (i>=10) break;
	} while (1);
	udelay(50);
	f4BitMode = 0;
	bHighCapacity = 0;
	return resp;
}

int SDCard_test( void )
{
	unsigned char *resp ;
	int highCapacityAllowed = 0;
	int cmdatInit = MMC_CMDAT_INIT;
	int bRetry=0;
	resp = mmc_reset();
	resp = mmc_cmd(8, (1<<8)|0xaa, cmdatInit|MMC_CMDAT_R1);	//request 2.7-3.6 volts, r7 is same length as r1
	if (resp) {
		highCapacityAllowed = 1<<30;
	}

	do {
		resp = mmc_cmd(SD_APP_CMD55, 0, cmdatInit|MMC_CMDAT_R1);
		if( !resp ) {
			resp = mmc_reset();
			resp = mmc_cmd(SD_APP_CMD55, 0, cmdatInit|MMC_CMDAT_R1);
			if (!resp ) {
				if (!bRetry) {
					printf( "SDInitErr1\n" );
					return -ENODEV ;
				}
			}
		}
		
		if (resp) {
			//bit 21 means 3.3 to 3.4 Volts
			resp = mmc_cmd(SD_APP_CMD41, (1<<21)|highCapacityAllowed, cmdatInit|MMC_CMDAT_R3);
			if ( !resp ) {
				if (!bRetry) {
					printf( "SDInitErr2\n" );
					return -ENODEV ;
				}
			} else {
				if ((resp[5] == 0x3f) && ( (resp[4]&0xbf) == 0x80)) break;
				cmdatInit = 0;
			}
		}
		bRetry++;
		udelay(1000);
		if (bRetry > 1000) {
			if (resp) printf("response %02x %02x %02x %02x %02x %02x\n",resp[0],resp[1],resp[2],resp[3],resp[4],resp[5]);
			return -ENODEV;
		}
	} while (1);
	if (highCapacityAllowed) if (resp[4]&0x40) bHighCapacity = 1;
	return 0 ;
}

#ifdef DEBUG
static void print_mmc_csd( struct mmc_csd *csd )
{
   printf( "ecc: %u\n", csd->ecc );
   printf( "file_format: %u\n", csd->file_format );
   printf( "tmp_write_protect: %u\n", csd->tmp_write_protect );
   printf( "perm_write_protect: %u\n", csd->perm_write_protect );
   printf( "copy: %u\n", csd->copy );
   printf( "file_format_grp: %u\n", csd->file_format_grp );
	printf( "content_prot_app: %u\n", csd->content_prot_app );
   printf( "rsvd3: %u\n", csd->rsvd3 );
   printf( "write_bl_partial: %u\n", csd->write_bl_partial );
   printf( "write_bl_len: %u\n", csd->write_bl_len );
   printf( "r2w_factor: %u\n", csd->r2w_factor );
   printf( "default_ecc: %u\n", csd->default_ecc );
   printf( "wp_grp_enable: %u\n", csd->wp_grp_enable );
   printf( "wp_grp_size: %u\n", csd->wp_grp_size );
   printf( "erase_grp_mult: %u\n", csd->erase_grp_mult );
   printf( "erase_grp_size: %u\n", csd->erase_grp_size );
   printf( "c_size_mult1: %u\n", csd->c_size_mult1 );
   printf( "vdd_w_curr_max: %u\n", csd->vdd_w_curr_max );
   printf( "vdd_w_curr_min: %u\n", csd->vdd_w_curr_min );
   printf( "vdd_r_curr_max: %u\n", csd->vdd_r_curr_max );
   printf( "vdd_r_curr_min: %u\n", csd->vdd_r_curr_min );
   printf( "c_size: %u\n", csd->c_size );
   printf( "rsvd2: %u\n", csd->rsvd2 );
   printf( "dsr_imp: %u\n", csd->dsr_imp );
   printf( "read_blk_misalign: %u\n", csd->read_blk_misalign );
   printf( "write_blk_misalign: %u\n", csd->write_blk_misalign );
   printf( "read_bl_partial: %u\n", csd->read_bl_partial );
   printf( "read_bl_len: %u\n", csd->read_bl_len );
   printf( "ccc: %u\n", csd->ccc );
   printf( "tran_speed %u\n", csd->tran_speed );
   printf( "nsac; %u\n", csd->nsac );
	printf( "taac; %u\n", csd->taac );
   printf( "rsvd1: %u\n", csd->rsvd1 );
   printf( "spec_vers: %u\n", csd->spec_vers );
   printf( "csd_structure: %u\n",  csd->csd_structure );
}
#endif 

#define DOS_PART_MAGIC_OFFSET	0x1fe
#define DOS_FS_TYPE_OFFSET	0x36
#define MSDOS_LABEL_MAGIC1	0x55
#define MSDOS_LABEL_MAGIC2	0xAA

struct bpb { // see http://staff.washington.edu/dittrich/misc/fatgen103.pdf
   unsigned char  jump[3];				//0x00
   char           oemName[8];			//0x03
   unsigned short bytesPerSector ;		//0x0b
   unsigned char  sectorsPerCluster ;	//0x0d
   unsigned short reservedSectorCount ;	//0x0e
   unsigned char  numFats ;				//0x10
   unsigned short rootEntCount ;		//0x11
   unsigned short totalSec16 ;			//0x13
   unsigned char  media ; 				//0x15 - 0xF8
   unsigned short fatSz16 ; 			//0x16
   unsigned short secPerTrack ;			//0x18
   unsigned short numHeads ;			//0x1a
   unsigned long  hiddenSectors ;		//0x1c
   unsigned long  totalSectors32 ;		//0x20
   unsigned char  driveNum ;			//0x24
   unsigned char  reserved1 ; 			//0x25 0x00
   unsigned char  bootSig ; 			//0x26 0x29 
   unsigned long  volumeId ;			//0x27
   char           volumeLabel[11];
   char           fileSysType[8];
} __attribute__((packed));

#define isprint(__c) (((__c)>=0x20)&&((__c)<=0x7f))

#ifdef DEBUG
static void ShowBpb(uchar* data,int blockNum)
{
	struct bpb const *bootParams = (struct bpb *)data ;
	unsigned j ;
	for ( j = 0 ; j < MMC_BLOCK_SIZE; j++ ) {
		if ( 0 == ( j & 0x0f ) ) printf( "%04x   ", j );
		printf( "%02x ", data[j] );
		if ( 7 == ( j & 7 ) ) printf( "  " );
		if( 0x0f == ( j & 0x0f ) ) {
			unsigned b ;
			for( b = j-15 ; b <= j ; b++ ) {
				uchar c = data[b];
				if( isprint(c) ) printf( "%c", c );
				else printf( "." );
				if( 7 == ( b & 7 ) ) printf( " " );
			}
			printf( "\n" );
		}
	}
	printf( "Invalid MBR at %d\n",blockNum );
	printf( "---> Boot Parameter block\n" );
	printf( "jump %02x %02x %02x\n", bootParams->jump[0],bootParams->jump[1],bootParams->jump[2]);
	printf( "bytesPerSector: %04x\n", bootParams->bytesPerSector );
	printf( "sectorsPerCluster: %02x\n", bootParams->sectorsPerCluster );
	printf( "reservedSectors %04x\n", bootParams->reservedSectorCount );
	printf( "numFats: %02x\n", bootParams->numFats );
	printf( "rootEntCount: %04x\n", bootParams->rootEntCount );
	printf( "totalSec16: %04x\n", bootParams->totalSec16 );
	printf( "media: %02x\n", bootParams->media );
	printf( "fatsz16: %04x", bootParams->fatSz16 );
	printf( "secPerTrack: %04x\n", bootParams->secPerTrack );
	printf( "numHeads = %04x\n", bootParams->numHeads );
	printf( "hidden = %08lx\n", bootParams->hiddenSectors );
	printf( "totalSec32 = %08lx\n", bootParams->totalSectors32 );
	printf( "drive #%u\n", bootParams->driveNum );
	printf( "reserved1: %02x\n", bootParams->reserved1 );
	printf( "bootSig: %02x\n", bootParams->bootSig );
	printf( "volume: %08lx\n", bootParams->volumeId );
}
#endif

static int find_mbr( int max_blocks, lbaint_t* pmax)
{
	uchar closeData[MMC_BLOCK_SIZE];
	int blockNum=-1;
	struct partition part ;
	int i ;
	ulong curBlock = 0 ;

	printf( "---- searching %d blocks for MBR\n", max_blocks );
	memset( &part, 0, sizeof(part));

	for( i = 0 ; i < 10 ; i++, curBlock++ ) {
		uchar data[MMC_BLOCK_SIZE];
		if( 0 == mmc_block_read(data, curBlock, sizeof(data) )) {
			if( (data[DOS_PART_MAGIC_OFFSET] == MSDOS_LABEL_MAGIC1 ) &&
				(data[DOS_PART_MAGIC_OFFSET + 1] == MSDOS_LABEL_MAGIC2 ) ) {            
				memcpy( &part, data+0x1be, sizeof(part));
				if( (('\x00' == part.boot_ind ) || ('\x80' == part.boot_ind )) &&
					(10 > part.head ) && ( part.end_head >= part.head ) ) {
					printf( "partition info found at block %u\n", i );
					printf( "boot:%02x head:%02x sec:%02x cyl:%02x sys:%02x endh:%02x ends:%02x endc:%02x start:%08x, count:%08x\n",
						part.boot_ind, part.head, part.sector, part.cyl,
						part.sys_ind, part.end_head, part.end_sector, part.end_cyl,
						part.start_sect, part.nr_sects );
					printf( "MBR found at block %d\n", i );
					if (pmax) *pmax = part.nr_sects;
					return part.start_sect ;
				} else {
					memcpy( closeData, data, MMC_BLOCK_SIZE);
					blockNum = i;
					break;
				}
			}
		} else {
			printf( "!!! Error reading mmc block %u\n", i );
			break;
		}
	}

	if (blockNum>=0) {
		struct bpb const *bootParams = (struct bpb *)closeData ;
#ifdef DEBUG
		ShowBpb( closeData,blockNum);
#endif
		part.boot_ind = 0 ;
		part.head = 0 ; 
		part.sector = 2 ;
		part.cyl = 0 ;
		part.sys_ind = 6 ;
		part.end_head = bootParams->numHeads ;
		part.end_sector = 0xe0 ;
		part.end_cyl = 0xc9 ;
		part.start_sect = 0 ;
		part.nr_sects = bootParams->totalSectors32 ;
		printf( "partition info faked\n" );
		printf( "boot:%02x head:%02x sec:%02x cyl:%02x sys:%02x endh:%02x ends:%02x endc:%02x start:%08x, count:%08x\n",
			part.boot_ind, part.head, part.sector, part.cyl,
			part.sys_ind, part.end_head, part.end_sector, part.end_cyl,
			part.start_sect, part.nr_sects );
		if (pmax) *pmax = part.nr_sects;
		return 0;
	}
	printf( "MBR not found!\n" );
	return -1 ;
}

static unsigned const mmcClks[] = {
	20000,
	10000,
	5000,
	2500,
	1250,
	625,
	313
};

#define EREAD_ERR 30

int mmc_init__(int verbose)
{
 	int retries, rc = -ENODEV;
	ushort rca = MMC_DEFAULT_RCA ;
	char isSD = 0 ;
	char allowed4bit=0;
	uchar *resp;
	mmc_cid_t *cid ;
	mmc_csd_t *csd ;
#ifdef CONFIG_IMX31
	gpio_sdhc_active(0);
#endif

#ifdef CONFIG_LUBBOCK
	set_GPIO_mode( GPIO6_MMCCLK_MD );
	set_GPIO_mode( GPIO8_MMCCS0_MD );
#endif

#ifndef CONFIG_IMX31
	CKEN |= CKEN12_MMC; /* enable MMC unit clock */
#endif

#if defined(CONFIG_ADSVIX)
	/* turn on the power */
	GPCR(114) = GPIO_bit(114);
	udelay(1000);
#endif

	mmc_csd.c_size = 0;
	mmc_ready = 0;
	f4BitMode = 0;	//default to 1bit mode
	bHighCapacity = 0;
	startBlock = 0 ;

	MMC_CLKRT  = MMC_CLKRT_0_3125MHZ;
	MMC_RESTO  = MMC_RES_TO_MAX;
#ifndef CONFIG_IMX31
	MMC_SPI    = MMC_SPI_DISABLE;
#endif

	if( 0 == SDCard_test() ) {
		printf( "SD card detected!\n" );
		isSD = 1 ;
	} else {
		isSD = 0 ;
		/* reset */
		resp = mmc_reset();
		resp = mmc_cmd(1, 0x00ffc000, MMC_CMDAT_INIT|MMC_CMDAT_BUSY|MMC_CMDAT_R3);
		if( 0 == resp ) {
			printf( "MMC CMD1 error\n" );
			return -1 ;
		}
		printf( "init: " ); dumpResponse( resp, 6 );
		retries = 0 ;
		do {
#ifdef CONFIG_PXA27X
			udelay(100);
#else
			udelay(50);
#endif
			resp = mmc_cmd(1, 0x00ffff00, MMC_CMDAT_BUSY|MMC_CMDAT_R3);
			debug( "cmd1: " ); dumpResponse( resp, 6 );
			retries++ ;
		} while( resp && ( 0 == ( resp[4] & 0x80 ) ) );
    
		if( 0 == resp ) {
			printf( "MMC CMD1 error2\n" );
			return -1 ;
		}
      
		do {
			udelay(100);
			resp = mmc_cmd(1, 0x00ffff00, MMC_CMDAT_BUSY|MMC_CMDAT_R3);
			debug( "cmd1: " ); dumpResponse( resp, 6 );
			retries++ ;
		} while( resp && ( 0 != ( resp[4] & 0x80 ) ) );
            
		printf( "after busy: %s, %d retries\n", 
			resp ? "have INIT response" : "no INIT response",
			retries );
	}

	/* try to get card id */
	resp = mmc_cmd(2, 0, MMC_CMDAT_R2);
	if( !resp ) {
		printf( "Bad CMDAT_R2 response\n" );
		return -1 ;
	}
	/* TODO configure mmc driver depending on card attributes */
	cid = (mmc_cid_t *)resp;
	if (verbose) {
		printf("MMC found. Card desciption is:\n");
		printf("Manufacturer ID = %02x%02x%02x\n",
						cid->id[0], cid->id[1], cid->id[2]);
		printf("HW/FW Revision = %x %x\n",cid->hwrev, cid->fwrev);
		cid->hwrev = cid->fwrev = 0;	/* null terminate string */
		printf("Product Name = %s\n",cid->name);
		printf("Serial Number = %02x%02x%02x\n",
						cid->sn[0], cid->sn[1], cid->sn[2]);
		printf("Month = %d\n",cid->month);
		printf("Year = %d\n",1997 + cid->year);
	}
	sprintf((char*)mmc_dev.product,"%s",cid->name);
	sprintf((char*)mmc_dev.vendor,"Man %02x%02x%02x Snr %02x%02x%02x",
		cid->id[0], cid->id[1], cid->id[2],
		cid->sn[0], cid->sn[1], cid->sn[2]);
	sprintf((char*)mmc_dev.revision,"%x %x",cid->hwrev, cid->fwrev);

	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_dev.blksz = 512;
	mmc_dev.lba = 0x10000;	//temp default
#if 0
	sprintf(mmc_dev.vendor,"Man %02x%02x%02x Snr %02x%02x%02x",
			cid->id[0], cid->id[1], cid->id[2],
			cid->sn[0], cid->sn[1], cid->sn[2]);
	sprintf(mmc_dev.product,"%s",cid->name);
	sprintf(mmc_dev.revision,"%x %x",cid->hwrev, cid->fwrev);
#endif
	mmc_dev.removable = 0;
	mmc_dev.block_read = mmc_bread;

	/* MMC exists, get CSD too */
	resp = mmc_cmd(MMC_CMD_SET_RCA, MMC_DEFAULT_RCA<<16, MMC_CMDAT_R1);
   
	if (!resp ) {
		printf( "no SET_RCA response\n" );
		return -1 ;
	}
	rca = ( isSD )? (((ushort)resp[4] << 8 ) | resp[3]) : MMC_DEFAULT_RCA ;

#if 0
/*
 * According to a Toshiba doc, the following is supposed to give
 * the size of the 'protected' area (so we can ignore it).

 * Unfortunately, I can't get the numbers to add up, so we walk
 * til we find an MBR instead.
 */
	if ( isSD ) {
		sd_status_t *status ;
		int i ;
		printf( "sending CMD55\n" );
		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if( !resp ) {
			printf( "Error 0x%04x sending APP CMD\n", MMC_STAT );
			return -1 ;
		}
		printf( "have CMD55 response\n" );
		memset( resp, 0, 20 );

		resp = mmc_cmd(SD_STATUS, rca<<16, MMC_CMDAT_R1 );
		if ( !resp ) {
			printf( "Error reading SD_STATUS\n" );
			return -1 ;
		}
       	printf( "SDSTATUS returned\n" );
       	for( i = 0 ; i < 16 ; i++ ) printf( "%02x ", resp[i] );
		printf( "\n" );
		status = (sd_status_t *)resp ;
		printf( "bus_width:     %u\n", status->bus_width );
		printf( "secured_mode:  %u\n", status->secured_mode );
		printf( "unused0:       %x\n", status->unused0 );
		printf( "card_type:     %x\n", status->card_type );
		printf( "prot_size:     %lx\n", status->prot_size );
	}
#endif

	MMC_STRPCL = MMC_STRPCL_STOP_CLK;
	DISABLE_CLKSTOP_INT
	WAIT_FOR_CLOCK_TO_STOP

#ifdef CONFIG_IMX31
	MMC_CLKRT = MMC_CLKRT_25MHZ;
#elif defined(CONFIG_PXA27X)
	if( getenv("sdclk10") ){
		MMC_CLKRT = MMC_CLKRT_10MHZ ;
	} else if( getenv("sdclk3" ) ){
		MMC_CLKRT = MMC_CLKRT_0_3125MHZ ;
	} else {
		MMC_CLKRT = MMC_CLKRT_20MHZ ;
	}
	printf( "using %u kHz SD clock (change with sdclk10 or sdclk3)\n", mmcClks[MMC_CLKRT] );
#else
	MMC_CLKRT = MMC_CLKRT_20MHZ;	/* 20 MHz */
#endif
	resp = mmc_cmd(7, rca<<16, MMC_CMDAT_R1);
	if( !resp ) {
		printf( "Error selecting RCA %x\n", rca );
		return -1 ;
	}

	resp = mmc_cmd(7, 0, MMC_CMDAT_R1);
	if( !resp ) {
		// this is normal
	}

	resp = mmc_cmd(MMC_CMD_SEND_CSD, rca<<16, MMC_CMDAT_R2);
	if (!resp) {
		printf( "Error reading CSD\n" );
		return -1 ;
	}

	csd = (mmc_csd_t *)resp;
	memcpy(&mmc_csd, csd, sizeof(*csd));
	rc = 0;

#ifdef DEBUG
	dumpResponse( resp, sizeof( *csd ) );
	print_mmc_csd( csd );
#endif 

	resp = mmc_cmd(7, rca<<16, MMC_CMDAT_R1);
	if( !resp ) {
		printf( "Error selecting RCA %x\n", rca );
		return -1 ;
	}

//#define CONFIG_1WIRE_ONLY
#ifndef CONFIG_1WIRE_ONLY
#if defined(CONFIG_IMX31)||defined(CONFIG_PXA27X)
	if (isSD) {
		int busWidthMode = 0;
		allowed4bit = 1;
		if( 0 == getenv("sd1bit") ){
			busWidthMode = 2;	//4 bit wide
		}
		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if (resp) {
			resp = mmc_cmd(6, busWidthMode, MMC_CMDAT_R1);
		}
		if (resp){
			f4BitMode = (busWidthMode)? 1 : 0;	//switch to 4bit mode
		} else {
			printf( "Error selecting %i bit mode\n",(busWidthMode)?4:1);
		}
		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if (resp) {
			/* send read command */
			unsigned char buf[64];
			unsigned char* p = buf;
			MMC_STRPCL = MMC_STRPCL_STOP_CLK;
			MMC_RDTO = 0xffff;
			MMC_NOB = 1;
			MMC_BLKLEN = 64;
			resp = mmc_cmd(13, 0, MMC_CMDAT_R2|MMC_CMDAT_READ|MMC_CMDAT_BLOCK|MMC_CMDAT_DATA_EN); 	//get SD Status
			memset(p,0,64);
			mmc_ReadFifo((unsigned int*)p,64);
#if 1
			if ((p[0] >> 6)!= busWidthMode) {
				printf( "!!!!!Error selecting bus width of %i bits\n",(busWidthMode)? 4 : 1);
				resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
				if (resp) {
					resp = mmc_cmd(6, 0, MMC_CMDAT_R1);
					f4BitMode = 0;	//back to 1 bit mode
					setenv( "sd1bit", "1" );
				}
			}
#else			
			for (i=0; i<8; i++) {
				printf( "%02x %02x %02x %02x %02x %02x %02x %02x\n",p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7]);
				p += 8;
			}
#endif
		}
	}
#endif
#endif
	mmc_ready = 1;
	if (f4BitMode) {
		printf( "using 4-bit SD card transfers\n");
	} else if (allowed4bit) {
		printf( "!!!!!***** using 1-bit SD card transfers *****!!!!!\n");
	} else {
		printf( "using 1-bit transfers\n");
	}

	{
		int i=0;
		startBlock = 0;
		do {
			int sb = find_mbr(mmc_csd.c_size,&mmc_dev.lba);
			if (sb>=0) {
				startBlock = sb;
				break;
			}
			i++;
			if (i>=2) {
				mmc_ready = 0;
				return -EREAD_ERR;
			}
		} while (1);
	}
	

	printf( "registering device: startBlock == %d, isSD ? %s\n", 
		startBlock, isSD ? "yes" : "no" );

	fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */

	return rc;
}
int mmc_init(int verbose)
{
	int rc = mmc_init__(verbose);
	if (rc==-EREAD_ERR) {
		setenv("sd1bit","1");
		rc = mmc_init__(verbose);
	}
	return rc;
}
int
mmc_ident(block_dev_desc_t *dev)
{
	return 0;
}

int
mmc2info(ulong addr)
{
	/* FIXME hard codes to 32 MB device */
	if (addr >= CFG_MMC_BASE && addr < CFG_MMC_BASE + 0x02000000) {
		return 1;
	}
	return 0;
}

#ifdef CONFIG_CMD_MMC

int do_mmc_detect (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef CONFIG_IMX31
#define GPIO1_BASE 0x53fcc000
#define GPIO_PSR	0x08
#define CARD_DETECT_BIT 2
#define WRITE_PROTECT_BIT 3
	unsigned long level = *((unsigned long *)(GPIO1_BASE+GPIO_PSR)) ;
	return ((level&(1<<CARD_DETECT_BIT))!=0);	//0 means detected
#else

#define GP_INDEX(gp) ((gp<96)? (gp>>5) : 0x40)
#define GPLRx(gp) __REG((0x40e00000+(GP_INDEX(gp)<<2)))

#ifdef CONFIG_PXA27X
#define GPIO_SDMMC_CARD_DETECT		10
#define GPIO_SDMMC_WRITE_PROTECT	38
#else
#define GPIO_SDMMC_CARD_DETECT		36
#define GPIO_SDMMC_WRITE_PROTECT	38
#endif

	unsigned long gplrx = GPLRx(GPIO_SDMMC_CARD_DETECT);
	int rval = ( 0 != (gplrx & (1<<(GPIO_SDMMC_CARD_DETECT&0x1f))) ); 
	debug("Checking for MMC card: %lx, %d\n", gplrx, rval );
	return rval ;
#endif
}

U_BOOT_CMD(
	mmcdet,	  1,	0,	do_mmc_detect,
	"mmcdet  - detect mmc card\n",
	NULL
);

int do_mmc_wp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
#ifdef CONFIG_IMX31
	unsigned long level = *((unsigned long *)(GPIO1_BASE+GPIO_PSR)) ;
	return ((level&(1<<WRITE_PROTECT_BIT))==0);	//0 means protected
#else
	unsigned long gplrx = GPLRx(GPIO_SDMMC_WRITE_PROTECT);
	int rval = ( 0 == (gplrx & (1<<(GPIO_SDMMC_WRITE_PROTECT&0x1f))) ); 
	debug("Checking MMC write protect: %lx, %d\n", gplrx, rval );
	return rval ;
#endif   
}

U_BOOT_CMD(
	mmcwp,	  1,	0,	do_mmc_wp,
	"mmcwp   - detect mmc write protect\n",
	NULL
);

#endif

#endif	/* CONFIG_MMC */
