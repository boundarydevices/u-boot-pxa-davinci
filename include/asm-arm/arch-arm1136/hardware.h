#ifndef __ASSEMBLY__
#define io_p2v(PhAdd)	   (PhAdd)
#define __REG(x)	(*((volatile u32 *)io_p2v(x)))
#define __REG2(x,y)	(*(volatile u32 *)((u32)&__REG(x) + (y)))
#else
#define __REG(x) (x)
#endif

#define MMC_STRPCL	__REG(CFG_MMC_BASE+0)  /* Control to start and stop MMC clock */
#define MMC_STAT	__REG(CFG_MMC_BASE+0x04)  /* MMC Status Register (read only) */
#define MMC_CLKRT	__REG(CFG_MMC_BASE+0x08)  /* MMC clock rate */
#define MMC_CMDAT	__REG(CFG_MMC_BASE+0x0c)  /* Command/response/data sequence control */
#define MMC_RESTO	__REG(CFG_MMC_BASE+0x10)  /* Expected response time out */
#define MMC_RDTO	__REG(CFG_MMC_BASE+0x14)  /* Expected data read time out */
#define MMC_BLKLEN	__REG(CFG_MMC_BASE+0x18)  /* Block length of data transaction */
#define MMC_NOB		__REG(CFG_MMC_BASE+0x1c)  /* Number of blocks, for block mode */
#define MMC_REVNO	__REG(CFG_MMC_BASE+0x20)
#define MMC_I_MASK	__REG(CFG_MMC_BASE+0x24)  /* Interrupt Mask */

#define MMC_CMD		__REG(CFG_MMC_BASE+0x28)  /* Index of current command */
#define MMC_ARG		__REG(CFG_MMC_BASE+0x2c)
#define MMC_RES		__REG(CFG_MMC_BASE+0x34)  /* Response FIFO (read only) */
#define MMC_RXFIFO	__REG(CFG_MMC_BASE+0x38)  /* Receive FIFO (read only) */
#define MMC_TXFIFO	__REG(CFG_MMC_BASE+0x38)  /* Transmit FIFO (write only) */

#define IO_ADDRESS(x) (x)
#define AIPS1_BASE_ADDR         0x43F00000
#define IOMUXC_BASE_ADDR        (AIPS1_BASE_ADDR + 0x000AC000)
