/*
 * Module davinci_mmc.c
 *
 * This module defines the driver interfaces for the
 * TI Davinci:
 *
 * From include/mmc.h:
 
      int mmc_init(int verbose);
      int mmc_read(ulong src, uchar *dst, int size);
      int mmc_write(uchar *src, ulong dst, int size);
      int mmc2info(ulong addr);
 
 *
 * Declared separately in include/asm/arch_x/mmc.h
 *
      uchar *mmc_cmd(ushort cmd, uint arg, ushort cmdat);

 * It has only been tested on TMS320DM6446 (DVEVM) and 
 * TMS320DM6443 (Xenon) boards.
 *
 * Change History : 
 *
 * $Log$
 *
 * Copyright Boundary Devices, Inc. 2007
 */

#include <config.h>
#include <common.h>
#include <asm/arch/hardware.h>

#if defined(CONFIG_CMD_MMC) && defined(DAVINCI_MMC_SD_BASE)
#include <fat.h>
#include <mmc.h>
#include <asm/arch/mmc.h>
#include <asm/arch/edma.h>

/**************************************************************************
 * Global Function Prototypes
 **************************************************************************/
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

static intc_registers volatile *const pintc = (intc_registers volatile *)DAVINCI_ARM_INTC_BASE ;
#define IRQ_MMCINT       26
#define IRQ_MASK (1<<IRQ_MMCINT)

#define INT_ASSERTED (0==(pintc->irq0&IRQ_MASK))
#define CLEAR_INT pintc->irq0 = IRQ_MASK
#define ENABLE_INT pintc->eint0 = IRQ_MASK 

extern void lpsc_on(unsigned int id);

#define DAVINCI_MMCCTL *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x00)
#define DAVINCI_MMCCLK *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x04)
#define DAVINCI_MMCST0 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x08)
#define DAVINCI_MMCST1 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x0C)
#define DAVINCI_MMCIM *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x10)
#define DAVINCI_MMCTOR *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x14)
#define DAVINCI_MMCTOD *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x18)
#define DAVINCI_MMCBLEN *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x1C)
#define DAVINCI_MMCNBLK *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x20)
#define DAVINCI_MMCNBLC *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x24)
#define DAVINCI_MMCDRR *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x28)
#define DAVINCI_MMCDXR *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x2C)
#define DAVINCI_MMCCMD *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x30)
#define DAVINCI_MMCARGHL *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x34)
#define DAVINCI_MMCRSP01 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x38)
#define DAVINCI_MMCRSP23 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x3C)
#define DAVINCI_MMCRSP45 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x40)
#define DAVINCI_MMCRSP67 *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x44)
#define DAVINCI_MMCDRSP *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x48)
#define DAVINCI_MMCCIDX *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x50)
#define DAVINCI_MMCFIFOCTL *(unsigned long volatile *)(DAVINCI_MMC_SD_BASE+0x74)

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

#define MMC_CMDAT_INIT (1<<14)
#define MMC_CMDAT_DATA_EN (1<<13)
#define MMC_CMDAT_READ (0<<11)
#define MMC_CMDAT_WRITE (1<<11)
#define MMC_CMDAT_BUSY (1<<8)

#define SD_APP_CMD55  55         /* 0x37 */
#define SD_APP_CMD41  41         /* 0x29 */
#define SD_STATUS     13         /* 0x0D */

static const unsigned int tran_exp[] = {
	10000,		100000,		1000000,	10000000,
	0,		0,		0,		0
};

static const unsigned char tran_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

static const unsigned int tacc_exp[] = {
	1,	10,	100,	1000,	10000,	100000,	1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

#define UNSTUFF_BITS(resp,start,size)					\
	({								\
		const int __size = size;				\
		const unsigned long __mask = (__size < 32 ? 1 << __size : 0) - 1;	\
		const int __off = 3 - ((start) / 32);			\
		const int __shft = (start) & 31;			\
		unsigned long __res;						\
									\
		__res = resp[__off] >> __shft;				\
		if (__size + __shft > 32){				\
			__res |= resp[__off-1] << ((32 - __shft) % 32);	\
      } \
		__res & __mask;						\
	})

static block_dev_desc_t mmc_dev;
static mmc_csd_t mmc_csd;
static int allowed4bit = 0 ;
static char f4BitMode = 0;

#ifdef DEBUG
#define DPRINT(args...)	printf(args)
#else
#define DPRINT(args...)
#endif
#define PRINT(args...)	printf(args)

block_dev_desc_t *  mmc_get_dev(int dev)
{
    return ((block_dev_desc_t *)&mmc_dev);
}

static void dumpResponse( unsigned char *resp )
{
   unsigned long const *longs = (unsigned long const *)resp ;
   unsigned i ;
   for( i = 0 ; i < 16 ; i++ ){
      printf( "%02x ", resp[i] );
   }
   printf( "\n" );
   for( i = 0 ; i < 16 ; i++ ){
      char c = resp[i];
      printf( " %c ", ( ( ' ' <= c ) && ( '~' >= c ) ) ? c : '.' );
   }
   printf( "\n" );
   printf( "%08x%08x%08x%08x\n", longs[0], longs[1], longs[2], longs[3] );
}

static int SDCard_test( void )
{
	unsigned char *resp ;
	int highCapacityAllowed = 0;
	int cmdatInit = 0 ; // MMC_CMDAT_INIT;
	int bRetry=0;
	resp = mmc_cmd(8, (1<<8)|0xaa, cmdatInit|MMC_CMDAT_R1);	//request 2.7-3.6 volts, r7 is same length as r1
	if (resp) {
		highCapacityAllowed = 1<<30;
      if( highCapacityAllowed )
         printf( "High capacity supported!\n" );
      else
         printf( "Low capacity!\n" );
	}

	do {
		resp = mmc_cmd(SD_APP_CMD55, 0, cmdatInit|MMC_CMDAT_R1);
		if( !resp ) {
			resp = mmc_cmd(SD_APP_CMD55, 0, cmdatInit|MMC_CMDAT_R1);
			if (!resp ) {
				if (!bRetry) {
					printf( "SDInitErr1\n" );
					return -1 ;
				}
			}
		}

		if (resp) {
			//bit 21 means 3.3 to 3.4 Volts
//cmdatInit = 0 ;
			resp = mmc_cmd(SD_APP_CMD41, (1<<21)|highCapacityAllowed, cmdatInit|MMC_CMDAT_R3);
			if ( !resp ) {
				if (!bRetry) {
					printf( "SDInitErr2\n" );
					return -1 ;
				}
			} else {
				if( ((resp[3]&0x3f) == 0x3f) && ((resp[4]&0xbf) == 0x80) ){
               break;
            }
            else if( 0 ){
               printf( "unexpected response %02x %02x\n", resp[3], resp[4] );
               dumpResponse(resp);
            }
				cmdatInit = 0;
			}
		}
		bRetry++;
		udelay(5000);
		if (bRetry > 20) {
			return -1;
		}
//      if (resp) printf("response %02x %02x %02x %02x %02x %02x\n",resp[0],resp[1],resp[2],resp[3],resp[4],resp[5]);
	} while (1);
	if (highCapacityAllowed) 
      if (resp[4]&0x40) 
         printf( "High-capacity\n" );
	return 0 ;
}

static void print_mmc_csd( mmc_csd_t const *csd )
{
   printf( "mmca_vsn: %x\n", csd->mmca_vsn );
   printf( "cmdclass: %x\n", csd->cmdclass );
   printf( "tacc_clks: %x\n", csd->tacc_clks );
   printf( "tacc_ns: %x\n", csd->tacc_ns );
   printf( "r2w_factor: %x\n", csd->r2w_factor );
   printf( "max_dtr: %x\n", csd->max_dtr );
   printf( "read_blkbits: %x\n", csd->read_blkbits );
   printf( "write_blkbits: %x\n", csd->write_blkbits );
   printf( "capacity: %x\n", csd->capacity );
   printf( "read_partial: %x\n", csd->read_partial );
   printf( "read_misalign: %x\n", csd->read_misalign );
   printf( "write_partial: %x\n", csd->write_partial );
   printf( "write_misalign: %x\n", csd->write_misalign );
}

static void stop_clock( void )
{
   /* Disable clock */
   DAVINCI_MMCCLK &= ~DAVINCI_MMCCLK_ENABLE ;
/*
   while( !(DAVINCI_MMCST1 & DAVINCI_ST1_MMCCLK_STOPPED) )
      DAVINCI_MMCCLK &= ~DAVINCI_MMCCLK_ENABLE ; // wait for clock to stop
*/
}

static void start_clock( void )
{
   /* Enable clock */
   DAVINCI_MMCCLK |= DAVINCI_MMCCLK_ENABLE ;

   while( DAVINCI_MMCST1 & DAVINCI_ST1_MMCCLK_STOPPED)
      ; // wait for clock to start
}

static void set_clock( unsigned hz )
{
   unsigned long clock ;
   unsigned long clkrt ;
   
   /* Set clock to specified rate
    *    Input clock is 27MHz * (PLL1M+1)
    */
   clock = (27000000*(REGVALUE(PLL1_PLLM)+1))/6 ;  // SYSCLK5
   
   /* Showing my work:
    *       From the manual:     400kHz = clock/(2*(clkrt+1));
    *                            (2*(clkrt+1)) = clock/400kHz ;
    *                            clkrt+1 = clock/800kHz ;
    *                            clkrt = (clock/800kHz)-1 ;
    */
   clkrt = (clock/(2*hz))-1 ;
   DAVINCI_MMCCLK = (DAVINCI_MMCCLK & ~0xFF) | clkrt ;
}

static void mmc_setblklen( ulong blklen )
{
   static ulong prevLen = -1UL ;
   if( blklen != prevLen )
   {
      /* set block len */
      unsigned char *resp = mmc_cmd( MMC_CMD_SET_BLOCKLEN, blklen, MMC_CMDAT_R1);
      if( resp ){
         prevLen = blklen ;
         DAVINCI_MMCBLEN = blklen ;
      }
      else
         printf( "Error setting blocklen to %u\n", blklen );
   }
}

static void resetFIFO(void)
{
   DAVINCI_MMCCTL |= MMCCTL_CMDRST | MMCCTL_DATRST ;
   DAVINCI_MMCFIFOCTL = 1 ; // reset FIFO
   DAVINCI_MMCCTL &= ~(MMCCTL_CMDRST | MMCCTL_DATRST);
}

static unsigned mmc_ReadFifo(unsigned int* pDst,ulong len)
{
   unsigned numRead = 0 ;
   unsigned long prevStat0 = 0xDEADBEEF ;
   unsigned long prevStat1 = 0xDEADBEEF ;
   DPRINT( "reading %lu longwords from fifo\n", len );
   while( len ){
      unsigned long stat0, stat1 ;

      while( !INT_ASSERTED ){
         if( tstc() ){
            getc();
            DPRINT( "bailing...\n" );
            resetFIFO();
            return numRead ;
         }
      }
      CLEAR_INT ;
      
      stat0 = DAVINCI_MMCST0 ;
      stat1 = DAVINCI_MMCST1 ;
      if( (stat1 & (1<<6))
          ||
          (stat0 & MMCSD_EVENT_READ) ){
         // 128 bits == 16 bytes == 4 longwords
         //    depth flag 0 == 128 bits
         //    depth flag 1 == 256 bits
         unsigned depth = 4*(1+((DAVINCI_MMCFIFOCTL&4)>>2));
         while( depth && len ){
            *pDst++ = DAVINCI_MMCDRR ;
            --len ;
            numRead++ ;
            --depth ;
            DPRINT( "%08lx\n", pDst[-1] );
         }
      }
      else if( stat0 & MMCSD_EVENT_READ ){
         *pDst++ = DAVINCI_MMCDRR ;
         --len ;
         numRead++ ;
         DPRINT( "%08lx\n", pDst[-1] );
      }
      else {
         if( stat1 & (1<<3) )
            DPRINT( "DRFULL\n" );
         if( stat0 & MMCSD_EVENT_ERROR ){
            DPRINT( "FIFO Error %08x\n", stat0 );
         }
      }
      if( stat0 != prevStat0 ){
         DPRINT( "stat0: %08lx\n", stat0 );
         prevStat0 = stat0 ;
      }
      if( stat1 != prevStat1 ){
         DPRINT( "stat1: %08lx\n", stat1 );
         prevStat1 = stat1 ;
      }
   }
   
   return numRead ;
}

unsigned long  mmc_bread (int dev_unused, unsigned long start, lbaint_t blkcnt, void *buffer) {
   unsigned i ;
   unsigned long blocksRead = 0 ;
   unsigned const longsPerBlock = mmc_dev.blksz/sizeof(unsigned long);
   unsigned long blockNum = start ;
   uchar *resp;
   if (blkcnt == 0) {
          return 0;
   }

   mmc_setblklen( mmc_dev.blksz );

   for( i = 0 ; i < blkcnt ; i++ ){
        resp = mmc_cmd(MMC_CMD_READ_BLOCK, blockNum*mmc_dev.blksz, MMC_CMDAT_R1|MMC_CMDAT_READ|MMC_CMDAT_DATA_EN);
        if( resp ){
           unsigned longsRead = mmc_ReadFifo((unsigned int*)buffer,longsPerBlock);

           if( longsPerBlock == longsRead ){
#if 0
              unsigned i ;
              printf( "read block %lu\n", blockNum );
              for( i = 0 ; i < longsPerBlock ; i++ ){
                  printf( "%08lx ", ((unsigned long *)buffer)[i] );
                  if( 1 == (i & 1) ){
                     printf( "  " );
                  }
                  if( 3 == (i & 3) ){
                     printf( "\n" );
                  }
              }
#endif
              blocksRead++ ;
              blockNum++ ;
              buffer = ((unsigned long *)buffer) + longsPerBlock ;
           }
           else {
              printf( "short read of block %lu (%u of %u longs)\n", blockNum, longsRead, longsPerBlock );
              break ;
           }
        }
        else
           break ;
   }

   return blocksRead ;
}

int mmc_init(int verbose)
{
   unsigned long status ;
   unsigned long prevStat ;
   unsigned char *resp ;
	unsigned int e, m, csd_struct;
	mmc_cid_t cid ;
	char isSD = 0 ;
   unsigned long *longs ;
   ushort rca = MMC_DEFAULT_RCA ;
   unsigned long raw_cid[4];

   lpsc_on(15);

   ENABLE_INT ;

   /* No interrupts */
   DAVINCI_MMCIM = 0 ;

   /* Reset */
   DAVINCI_MMCCTL = MMCCTL_CMDRST | MMCCTL_DATRST ;

   stop_clock();
   set_clock(400000);

   /* 
    * timeout registers (max for now)
    */
   DAVINCI_MMCTOR = 0x1fff ;     // TOD upper 0x1f, TOR 0xff
   DAVINCI_MMCTOD = 0xffff ;

   start_clock();

   /* Out of reset */
   DAVINCI_MMCCTL &= ~(MMCCTL_CMDRST | MMCCTL_DATRST);

   /* Send clock cycles, poll completion */
   DAVINCI_MMCARGHL = 0x0;
   DAVINCI_MMCCMD   = 0x4000;
   status = 0;
   prevStat = 0x01234 ;
   while (!(status & (MMCSD_EVENT_EOFCMD))) {
      status = DAVINCI_MMCST0 ;
      if( status != prevStat ){
         prevStat = status ; 
      }
      if( tstc() ){
         getc();
         printf( "bailing...\n" );
         break ;
      }
   }

   /* Send SD_SEND_IF_COND command to set voltage ?? */
   DAVINCI_MMCARGHL = 0x01AA ;
   DAVINCI_MMCCMD   = 0x0608 ;
   status = 0 ;
   prevStat = 0x01234 ;
   while (!(status & (MMCSD_EVENT_EOFCMD|MMCSD_EVENT_ERROR))) {
      status = DAVINCI_MMCST0 ;
      if( status != prevStat ){
         prevStat = status ; 
      }
      if( tstc() ){
         getc();
         printf( "bailing...\n" );
         break ;
      }
   }

   isSD = (0 == SDCard_test() );

   printf( "%s\n", isSD ? "SD Card" : "Not SD" );
   if( !isSD )
      return -1 ;

   /* try to get card id */
	resp = mmc_cmd(2, 0, MMC_CMDAT_R2);
	if( !resp ) {
		printf( "Bad CMDAT_R2 response\n" );
   	resp = mmc_cmd(2, 0, MMC_CMDAT_R2);
   	if( !resp ) {
   		printf( "Still bad CMDAT_R2 response\n" );
   		return -1 ;
      }
	}

   memcpy( raw_cid, resp, sizeof(raw_cid) );

	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_dev.blksz = 512;
	mmc_dev.lba = 0x10000;	//temp default

	/* MMC exists, get CSD too */
	resp = mmc_cmd(MMC_CMD_SET_RCA, MMC_DEFAULT_RCA<<16, MMC_CMDAT_R1);

	if (!resp ) {
		printf( "no SET_RCA response\n" );
		return -1 ;
	}
	rca = ( isSD )? (((ushort)resp[4] << 8 ) | resp[3]) : MMC_DEFAULT_RCA ;

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

   longs = (unsigned long *)resp ;
	csd_struct = UNSTUFF_BITS(longs, 126, 2);
	switch (csd_struct) {
	case 0:
		m = UNSTUFF_BITS(longs, 115, 4);
		e = UNSTUFF_BITS(longs, 112, 3);
		mmc_csd.tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		mmc_csd.tacc_clks	 = UNSTUFF_BITS(longs, 104, 8) * 100;

		m = UNSTUFF_BITS(longs, 99, 4);
		e = UNSTUFF_BITS(longs, 96, 3);
		mmc_csd.max_dtr	  = tran_exp[e] * tran_mant[m];
		mmc_csd.cmdclass	  = UNSTUFF_BITS(longs, 84, 12);

		e = UNSTUFF_BITS(longs, 47, 3);
		m = UNSTUFF_BITS(longs, 62, 12);
		mmc_csd.capacity	  = (1 + m) << (e + 2);

		mmc_csd.read_blkbits = UNSTUFF_BITS(longs, 80, 4);
		mmc_csd.read_partial = UNSTUFF_BITS(longs, 79, 1);
		mmc_csd.write_misalign = UNSTUFF_BITS(longs, 78, 1);
		mmc_csd.read_misalign = UNSTUFF_BITS(longs, 77, 1);
		mmc_csd.r2w_factor = UNSTUFF_BITS(longs, 26, 3);
		mmc_csd.write_blkbits = UNSTUFF_BITS(longs, 22, 4);
		mmc_csd.write_partial = UNSTUFF_BITS(longs, 21, 1);
		break;
	case 1:
		/*
		 * This is a block-addressed SDHC card. Most
		 * interesting fields are unused and have fixed
		 * values. To avoid getting tripped by buggy cards,
		 * we assume those fixed values ourselves.
		 */
		mmc_csd.tacc_ns	 = 0; /* Unused */
		mmc_csd.tacc_clks	 = 0; /* Unused */

		m = UNSTUFF_BITS(longs, 99, 4);
		e = UNSTUFF_BITS(longs, 96, 3);
		mmc_csd.max_dtr	  = tran_exp[e] * tran_mant[m];
		mmc_csd.cmdclass	  = UNSTUFF_BITS(longs, 84, 12);

		m = UNSTUFF_BITS(longs, 48, 22);
		mmc_csd.capacity     = (1 + m) << 10;

		mmc_csd.read_blkbits = 9;
		mmc_csd.read_partial = 0;
		mmc_csd.write_misalign = 0;
		mmc_csd.read_misalign = 0;
		mmc_csd.r2w_factor = 4; /* Unused */
		mmc_csd.write_blkbits = 9;
		mmc_csd.write_partial = 0;
		break;
	default:
		printf("unrecognised CSD structure version %d\n", csd_struct);
	}
   
   if( verbose )
      print_mmc_csd( &mmc_csd );

	memset(&cid, 0, sizeof(struct mmc_cid));

	/*
	 * SD doesn't currently have a version field so we will
	 * have to assume we can parse this.
	 */
	cid.manfid		= UNSTUFF_BITS(raw_cid, 120, 8);
	cid.oemid			= UNSTUFF_BITS(raw_cid, 104, 16);
	cid.prod_name[0]		= UNSTUFF_BITS(raw_cid, 96, 8);
	cid.prod_name[1]		= UNSTUFF_BITS(raw_cid, 88, 8);
	cid.prod_name[2]		= UNSTUFF_BITS(raw_cid, 80, 8);
	cid.prod_name[3]		= UNSTUFF_BITS(raw_cid, 72, 8);
	cid.prod_name[4]		= UNSTUFF_BITS(raw_cid, 64, 8);
	cid.hwrev			= UNSTUFF_BITS(raw_cid, 60, 4);
	cid.fwrev			= UNSTUFF_BITS(raw_cid, 56, 4);
	cid.serial		= UNSTUFF_BITS(raw_cid, 24, 32);
	cid.year			= UNSTUFF_BITS(raw_cid, 12, 8);
	cid.month			= UNSTUFF_BITS(raw_cid, 8, 4);
	cid.year += 2000; /* SD cards year offset */

   if( verbose ){
   	printf( "manfid:    %08x\n", cid.manfid );
   	printf( "prod_name: %s\n", cid.prod_name );
   	printf( "serial:    %d\n", cid.serial );
   	printf( "oemid:     %d\n", cid.oemid );
   	printf( "date:      %d/%d\n", cid.month, cid.year );
   	printf( "hwrev:     %x\n", cid.hwrev );
   	printf( "fwrev:     %x\n", cid.fwrev );
   }

   printf( "%s: %u blocks of %u bytes == %u MB\n", 
           cid.prod_name, 
           mmc_csd.capacity, 
           1<<mmc_csd.read_blkbits,
           mmc_csd.capacity*(1<<mmc_csd.read_blkbits)/(1024*1024) );

	resp = mmc_cmd(7, rca<<16, MMC_CMDAT_R1);
	if( !resp ) {
		printf( "Error selecting RCA %x\n", rca );
		return -1 ;
	}

	if (isSD) {
		int busWidthMode = 0;
		allowed4bit = 1;
		if( 0 == getenv("sd1bit") ){
			busWidthMode = 2;	//4 bit wide
		}
		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if (resp) {
			resp = mmc_cmd(SD_APP_CMD_SET_BUS_WIDTH, busWidthMode, MMC_CMDAT_R1);
         if( !resp )
            printf( "error setting bus width\n" );
		}
		if (resp){
			f4BitMode = (busWidthMode)? 1 : 0;	//switch to 4bit mode
		} else {
			printf( "Error selecting %i bit mode\n",(busWidthMode)?4:1);
		}
      if( f4BitMode )
         DAVINCI_MMCCTL |= MMCCTL_BUSWIDTH_4;
      else
         DAVINCI_MMCCTL &= ~MMCCTL_BUSWIDTH_4;

      resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
      if (resp) {
			/* send read command */
			unsigned char buf[64];
			unsigned char* p = buf;
         DAVINCI_MMCFIFOCTL = 1 ; // reset FIFO

         DAVINCI_MMCNBLC = 1 ;
         DAVINCI_MMCBLEN = sizeof(buf);
         resp = mmc_cmd(13, 0, MMC_CMDAT_R2|MMC_CMDAT_READ|MMC_CMDAT_DATA_EN); 	//get SD Status
         if( resp ){
            unsigned bytesRead = 0 ;
            unsigned const longsToRead = sizeof(buf)/sizeof(unsigned int);
   			memset(p,0,64);
   			bytesRead = sizeof(unsigned int)*mmc_ReadFifo((unsigned int*)p,longsToRead);
            DAVINCI_MMCFIFOCTL = 1 ; // reset FIFO
            DAVINCI_MMCFIFOCTL = 0 ; // reset FIFO
            if ((p[0] >> 6)!= busWidthMode) {
                    printf( "!!!!!Error selecting bus width of %i bits\n",(busWidthMode)? 4 : 1);
                    resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
                    if (resp) {
                            resp = mmc_cmd(6, 0, MMC_CMDAT_R1);
                            f4BitMode = 0;	//back to 1 bit mode
                            setenv( "sd1bit", "1" );
                    }
            }
            else
               printf( "bus width == %u\n", f4BitMode ? 4 : 1 );
         }
         else
            printf( "Error reading bytes in %u bit mode\n", f4BitMode ? 4 : 1 );
         
/*
*/
         mmc_dev.block_read = mmc_bread;

         stop_clock();
         set_clock(20000000);
         start_clock();
         
         fat_register_device(&mmc_dev,1); /* partitions start counting with 1 */
         
         return 0 ;
      } // response to APP55
   } // SD card
   return -1 ;
}

int mmc_read(ulong src, uchar *dst, int size)
{
    printf( "%s: %lx, %p, size %u\n", __FUNCTION__, src, dst, size );
    return -1 ;
}

int mmc_write(uchar *src, ulong dst, int size)
{
   printf( "%s: %p, %lx, size %u\n", __FUNCTION__, src, dst, size );
   return -1 ;
}

int mmc2info(ulong addr)
{
   printf( "%s: %lx\n", __FUNCTION__, addr );
   return -1 ;
}

uchar *mmc_cmd(ushort cmd, uint arg, ushort cmdat)
{
   static unsigned char resp[20];
   unsigned status ;
   unsigned prevStat ;

   /* Send SD_SEND_IF_COND command to set voltage ?? */
//    printf( "cmd: %04x, args: %08x, flags %04x\n", cmd, arg, cmdat );
   if( cmdat & MMC_CMDAT_INIT )
      cmd |= MMC_CMDAT_INIT ;
   if( cmdat & MMC_CMDAT_BUSY )
      cmd |= MMC_CMDAT_BUSY ;
   cmd |= ((cmdat&3) << 9);

   cmd |= (cmdat & MMC_CMDAT_DATA_EN);
   cmd |= (cmdat & MMC_CMDAT_WRITE);

   status = DAVINCI_MMCST0 ; // flush old status

//   DAVINCI_MMCFIFOCTL = 1 ; // reset FIFO
   DAVINCI_MMCIM = MMCSD_EVENT_ERROR_CMDCRC |
					    MMCSD_EVENT_ERROR_DATACRC |
					    MMCSD_EVENT_ERROR_CMDTIMEOUT |
					    MMCSD_EVENT_ERROR_DATATIMEOUT ;
   if(cmdat & MMC_CMDAT_DATA_EN){
      DAVINCI_MMCIM |= MMCSD_EVENT_BLOCK_XFERRED ;
      if(cmdat & MMC_CMDAT_WRITE){
         DAVINCI_MMCIM |= MMCSD_EVENT_WRITE ;
      }
      else {
         DAVINCI_MMCIM |= MMCSD_EVENT_READ ;
      }
   }
// printf( "cmd == 0x%04x, arg = %08x, int %d, irq %x\n", cmd, arg, INT_ASSERTED, DAVINCI_MMCIM );
if( INT_ASSERTED )
   CLEAR_INT ;
memset(resp,0xcc,sizeof(resp));
   DAVINCI_MMCARGHL = arg ;
   DAVINCI_MMCCMD   = cmd ;

   if(0 != (cmdat & MMC_CMDAT_DATA_EN))
      return resp ;
   status = 0 ;
   prevStat = 0x01234 ;
   while (!(status & (MMCSD_EVENT_EOFCMD|MMCSD_EVENT_ERROR))) {
      status = DAVINCI_MMCST0 ;
      if( status != prevStat ){
//         printf( "status == 0x%lx..., cmdat %04x, int %d\n", status, cmdat, INT_ASSERTED );
         prevStat = status ; 
      }
      if( ( 0 != (cmdat & MMC_CMDAT_DATA_EN))
          &&
          (status & MMCSD_EVENT_READ) ){
//         printf( "data ready\n" );
         break ;
      }
      if( tstc() ){
         getc();
         printf( "bailing...\n" );
         break ;
      }
   }
#if 0
   printf( "%04x/%08lx/%08lx\n", status, DAVINCI_MMCST0, DAVINCI_MMCST1 );
   printf( "rspIdx == %08x\n", DAVINCI_MMCCIDX );
   printf( "rsp01 == 0x%08lx\n", DAVINCI_MMCRSP01 );
   printf( "rsp23 == 0x%08lx\n", DAVINCI_MMCRSP23 );
   printf( "rsp45 == 0x%08lx\n", DAVINCI_MMCRSP45 );
   printf( "rsp67 == 0x%08lx\n", DAVINCI_MMCRSP67 );
#endif

   if( INT_ASSERTED )
      CLEAR_INT ;
   if( status & MMCSD_EVENT_ERROR ){
//      printf( "MMCSD error cmd %u %x.%08lx/%08lx\n", cmd, status, DAVINCI_MMCST0, DAVINCI_MMCST1 );
      return 0 ;
   }

   switch (cmdat & 0x3) {
		case MMC_CMDAT_R1:
		case MMC_CMDAT_R3: {
         unsigned long rv = DAVINCI_MMCRSP45 ;
         memcpy(resp, &rv, 1);
         rv = DAVINCI_MMCRSP67 ;
         memcpy(resp+1,&rv,sizeof(rv));
         break ;
      }

		case MMC_CMDAT_R2: {
         unsigned bytes = 16 ;
         unsigned long volatile *addr = &DAVINCI_MMCRSP01 ;
         unsigned char *out = resp + 12 ;
         while( 0 < bytes ){
            unsigned long v = *addr++ ;
            memcpy(out,&v,sizeof(v));
            out -= sizeof(v);
            bytes -= sizeof(v);
         }
         break ;
      }
		//case 0 - no response to command is default
		default: {
         unsigned v = 0 ;
         memcpy( resp, &v, sizeof(v));
      }
	}

//   printf( "rstat 0x%08lx/0x%08lx\n", DAVINCI_MMCST0, DAVINCI_MMCST1 );
	return resp;
}

#endif
