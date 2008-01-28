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


typedef struct mmc_globals {
	uchar resp[20] __attribute__ ((aligned (32)));
	block_dev_desc_t mmc_dev;
	mmc_csd_t mmc_csd;
	int allowed4bit;
	char f4BitMode;
	ulong prevLen;
} mmc_globals_t;

static mmc_globals_t mmc_g;

/**************************************************************************
 * Global Function Prototypes
 **************************************************************************/
static mmc_registers_t volatile *const pmmc = (mmc_registers_t volatile *)DAVINCI_MMC_SD_BASE;
static intc_registers volatile *const pintc = (intc_registers volatile *)DAVINCI_ARM_INTC_BASE;
#define IRQ_MMCINT       26
#define IRQ_MASK (1<<IRQ_MMCINT)

#define INT_ASSERTED (0==(pintc->irq0&IRQ_MASK))
#define CLEAR_INT pintc->irq0 = IRQ_MASK
#define ENABLE_INT pintc->eint0 = IRQ_MASK 

extern void lpsc_on(unsigned int id);


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
static unsigned long inline UNSTUFF_BITS(unsigned long* resp,int start,int size)
{
	unsigned long val;
	unsigned int shift = (start&0x1f);
	start >>= 5;
	val = resp[start] >> shift;
	if ((shift + size) > 32) val |= resp[start+1] << (32-shift);
	if (size<32) val &= ((1<<size)-1);
	return val;
}

#ifdef DEBUG
#define DPRINT(args...)	printf(args)
#else
#define DPRINT(args...)
#endif
#define PRINT(args...)	printf(args)

block_dev_desc_t *  mmc_get_dev(int dev)
{
	return ((block_dev_desc_t *)&mmc_g.mmc_dev);
}

static void dumpResponse( unsigned char *resp )
{
	unsigned long const *longs = (unsigned long const *)resp ;
	unsigned i ;
	for( i = 0 ; i <= 16 ; i++ ){
		printf( "%02x ", resp[i] );
	}
	printf( "\n" );
	for( i = 0 ; i <= 16 ; i++ ){
		char c = resp[i];
		printf( " %c ", ( ( ' ' <= c ) && ( '~' >= c ) ) ? c : '.' );
	}
	printf( "\n" );
	printf( "%08x %08x %08x %08x\n", longs[0], longs[1], longs[2], longs[3] );
}

static void resetFIFO(void)
{
	pmmc->MMCCTL |= MMCCTL_CMDRST | MMCCTL_DATRST ;
	pmmc->MMCFIFOCTL = 1|MMCFIFOCTL_FIFOLEV ; // reset FIFO
	pmmc->MMCCTL &= ~(MMCCTL_CMDRST | MMCCTL_DATRST);
}

unsigned char* mmc_reset(void)
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
	mmc_g.prevLen = 0;
	mmc_g.f4BitMode = 0;
//	mmc_g.bHighCapacity = 0;
	return resp;
}

static int SDCard_test( void )
{
	unsigned char *resp ;
	int highCapacityAllowed = 0;
	int cmdatInit = MMC_CMDAT_INIT;
	int bRetry=0;
	resp = mmc_reset();
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
			resp = mmc_reset();
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
			resp = mmc_cmd(SD_APP_CMD41, (1<<21)|highCapacityAllowed, cmdatInit|MMC_CMDAT_R3);
			if ( !resp ) {
				if (!bRetry) {
					printf( "SDInitErr2\n" );
					return -1 ;
				}
			} else {
				if( ((resp[5]&0x3f) == 0x3f) && ((resp[4]&0xbf) == 0x80) ) break;
				cmdatInit = 0;
//				printf( "unexpected response %02x %02x\n", resp[5], resp[4] );
//				printf("response %02x %02x %02x %02x %02x %02x\n",resp[0],resp[1],resp[2],resp[3],resp[4],resp[5]);
//				dumpResponse(resp);
			}
		}
		bRetry++;
		udelay(1000);
		if (bRetry > 1000) {
			if (resp) printf("response %02x %02x %02x %02x %02x %02x\n",resp[0],resp[1],resp[2],resp[3],resp[4],resp[5]);
			return -1;
		}
	} while (1);
	if (highCapacityAllowed) {
		if (resp[4]&0x40) {
			printf( "High-capacity\n" );
//			bHighCapacity = 1;
		}
	}
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
	pmmc->MMCCLK &= ~DAVINCI_MMCCLK_ENABLE ;
/*
	while( !(pmmc->MMCST1 & DAVINCI_ST1_MMCCLK_STOPPED) )
		pmmc->MMCCLK &= ~DAVINCI_MMCCLK_ENABLE ; // wait for clock to stop
*/
}

static void start_clock( void )
{
	/* Enable clock */
	pmmc->MMCCLK |= DAVINCI_MMCCLK_ENABLE ;

	while( pmmc->MMCST1 & DAVINCI_ST1_MMCCLK_STOPPED)
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
	pmmc->MMCCLK = (pmmc->MMCCLK & ~0xFF) | clkrt ;
}

static void mmc_setblklen( ulong blklen )
{
	if( blklen != mmc_g.prevLen ) {
		/* set block len */
		unsigned char *resp = mmc_cmd( MMC_CMD_SET_BLOCKLEN, blklen, MMC_CMDAT_R1);
		if( resp ) {
			mmc_g.prevLen = blklen ;
			pmmc->MMCBLEN = blklen ;
		} else {
         printf( "Error setting blocklen to %u\n", blklen );
		}
	}
}

static unsigned mmc_ReadFifo(unsigned int* pDst,ulong len)
{
	unsigned numRead = 0 ;
	unsigned long prevStat0 = 0xDEADBEEF ;
	unsigned long prevStat1 = 0xDEADBEEF ;
	//depth flag 0 == 128 bits == 16 bytes == 4 longwords
	//depth flag 1 == 256 bits == 32 bytes == 8 longwords
	unsigned fifoDepth = (pmmc->MMCFIFOCTL&4)? 8 : 4;
	DPRINT( "reading %lu longwords from fifo\n", len );
	while( len ){
		unsigned long stat0, stat1 ;
		int i=0;

		while (i<0x100000) {
			if (INT_ASSERTED) {
				CLEAR_INT ;
				i=0;
				break;
			}
			i++;
		}
		if (i) {
			if( tstc() ){
				getc();
				DPRINT( "bailing...\n" );
				resetFIFO();
				return numRead ;
			} else {
				DPRINT( "Time out waiting for interrupt level\n" );
			}
		}
      
		stat0 = pmmc->MMCST0 ;
		stat1 = pmmc->MMCST1 ;
		if (stat0 & MMCSD_EVENT_READ) { /* Is Fifo to Interrupt level or end of transfer? */
			if (len >= fifoDepth) {
	            *pDst++ = pmmc->MMCDRR ;
	            *pDst++ = pmmc->MMCDRR ;
	            *pDst++ = pmmc->MMCDRR ;
	            *pDst++ = pmmc->MMCDRR ;
	            if (fifoDepth==8) {
		            *pDst++ = pmmc->MMCDRR ;
		            *pDst++ = pmmc->MMCDRR ;
		            *pDst++ = pmmc->MMCDRR ;
		            *pDst++ = pmmc->MMCDRR ;
	            }
	            len -= fifoDepth;
	            numRead+= fifoDepth;
			} else {
				numRead+= len;
				while ( len ){
					*pDst++ = pmmc->MMCDRR ;
					len-- ;
					DPRINT( "%08lx\n", pDst[-1] );
				}
			}
		} else {
			if( stat1 & (1<<3) )
				DPRINT( "DRFULL\n" );
		}
		if( stat0 & MMCSD_EVENT_ERROR ){
			DPRINT( "FIFO Error %08x\n", stat0 );
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

unsigned long  mmc_bread (int dev_unused, unsigned long start, lbaint_t blkcnt, void *buffer)
{
	unsigned i ;
	unsigned long blocksRead = 0 ;
	unsigned const longsPerBlock = mmc_g.mmc_dev.blksz/sizeof(unsigned long);
	unsigned long blockNum = start ;
	uchar *resp;
	if (blkcnt == 0) {
		return 0;
	}

	mmc_setblklen( mmc_g.mmc_dev.blksz );

	for( i = 0 ; i < blkcnt ; i++ ){
		resp = mmc_cmd(MMC_CMD_READ_BLOCK, blockNum*mmc_g.mmc_dev.blksz, MMC_CMDAT_R1|MMC_CMDAT_READ|MMC_CMDAT_DATA_EN);
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
			} else {
				printf( "short read of block %lu (%u of %u longs)\n", blockNum, longsRead, longsPerBlock );
				break ;
			}
		} else {
           break ;
		}
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

	memset(&mmc_g,0,sizeof(mmc_g));

	lpsc_on(15);

	ENABLE_INT ;

	/* No interrupts */
	pmmc->MMCIM = 0 ;

	/* Reset */
	pmmc->MMCCTL = MMCCTL_CMDRST | MMCCTL_DATRST ;

	stop_clock();
	set_clock(400000);

	/* 
	 * timeout registers (max for now)
	 */
	pmmc->MMCTOR = 0x1fff ;     // TOD upper 0x1f, TOR 0xff
	pmmc->MMCTOD = 0xffff ;

	start_clock();

	/* Out of reset */
	pmmc->MMCCTL &= ~(MMCCTL_CMDRST | MMCCTL_DATRST);

	/* Send clock cycles, poll completion */
	pmmc->MMCARGHL = 0x0;
	pmmc->MMCCMD   = 0x4000;
	status = 0;
	prevStat = 0x01234 ;
	while (!(status & (MMCSD_EVENT_EOFCMD))) {
		status = pmmc->MMCST0 ;
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
	pmmc->MMCARGHL = 0x01AA ;
	pmmc->MMCCMD   = 0x0608 ;
	status = 0 ;
	prevStat = 0x01234 ;
	while (!(status & (MMCSD_EVENT_EOFCMD|MMCSD_EVENT_ERROR))) {
		status = pmmc->MMCST0 ;
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
	mmc_g.mmc_dev.if_type = IF_TYPE_MMC;
	mmc_g.mmc_dev.part_type = PART_TYPE_DOS;
	mmc_g.mmc_dev.dev = 0;
	mmc_g.mmc_dev.lun = 0;
	mmc_g.mmc_dev.type = 0;
	/* FIXME fill in the correct size (is set to 32MByte) */
	mmc_g.mmc_dev.blksz = 512;
	mmc_g.mmc_dev.lba = 0x10000;	//temp default

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
		mmc_g.mmc_csd.tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		mmc_g.mmc_csd.tacc_clks	 = UNSTUFF_BITS(longs, 104, 8) * 100;

		m = UNSTUFF_BITS(longs, 99, 4);
		e = UNSTUFF_BITS(longs, 96, 3);
		mmc_g.mmc_csd.max_dtr	  = tran_exp[e] * tran_mant[m];
		mmc_g.mmc_csd.cmdclass	  = UNSTUFF_BITS(longs, 84, 12);

		e = UNSTUFF_BITS(longs, 47, 3);
		m = UNSTUFF_BITS(longs, 62, 12);
		mmc_g.mmc_csd.capacity	  = (1 + m) << (e + 2);

		mmc_g.mmc_csd.read_blkbits = UNSTUFF_BITS(longs, 80, 4);
		mmc_g.mmc_csd.read_partial = UNSTUFF_BITS(longs, 79, 1);
		mmc_g.mmc_csd.write_misalign = UNSTUFF_BITS(longs, 78, 1);
		mmc_g.mmc_csd.read_misalign = UNSTUFF_BITS(longs, 77, 1);
		mmc_g.mmc_csd.r2w_factor = UNSTUFF_BITS(longs, 26, 3);
		mmc_g.mmc_csd.write_blkbits = UNSTUFF_BITS(longs, 22, 4);
		mmc_g.mmc_csd.write_partial = UNSTUFF_BITS(longs, 21, 1);
		break;
	case 1:
		/*
		 * This is a block-addressed SDHC card. Most
		 * interesting fields are unused and have fixed
		 * values. To avoid getting tripped by buggy cards,
		 * we assume those fixed values ourselves.
		 */
		mmc_g.mmc_csd.tacc_ns	 = 0; /* Unused */
		mmc_g.mmc_csd.tacc_clks	 = 0; /* Unused */

		m = UNSTUFF_BITS(longs, 99, 4);
		e = UNSTUFF_BITS(longs, 96, 3);
		mmc_g.mmc_csd.max_dtr	  = tran_exp[e] * tran_mant[m];
		mmc_g.mmc_csd.cmdclass	  = UNSTUFF_BITS(longs, 84, 12);

		m = UNSTUFF_BITS(longs, 48, 22);
		mmc_g.mmc_csd.capacity     = (1 + m) << 10;

		mmc_g.mmc_csd.read_blkbits = 9;
		mmc_g.mmc_csd.read_partial = 0;
		mmc_g.mmc_csd.write_misalign = 0;
		mmc_g.mmc_csd.read_misalign = 0;
		mmc_g.mmc_csd.r2w_factor = 4; /* Unused */
		mmc_g.mmc_csd.write_blkbits = 9;
		mmc_g.mmc_csd.write_partial = 0;
		break;
	default:
		dumpResponse(resp);
		printf("unrecognised CSD structure version %d from MMC_CMD_SEND_CSD\n", csd_struct);
	}
   
	if( verbose )
		print_mmc_csd( &mmc_g.mmc_csd );

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
			mmc_g.mmc_csd.capacity, 
			1<<mmc_g.mmc_csd.read_blkbits,
			mmc_g.mmc_csd.capacity*(1<<mmc_g.mmc_csd.read_blkbits)/(1024*1024) );

	resp = mmc_cmd(7, rca<<16, MMC_CMDAT_R1);
	if( !resp ) {
		printf( "Error selecting RCA %x\n", rca );
		return -1 ;
	}

	if (isSD) {
		int busWidthMode = 0;
		mmc_g.allowed4bit = 1;
		if (0 == getenv("sd1bit") ){
			busWidthMode = 2;	//4 bit wide
		}
		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if (resp) {
			resp = mmc_cmd(SD_APP_CMD_SET_BUS_WIDTH, busWidthMode, MMC_CMDAT_R1);
			if( !resp )
				printf( "error setting bus width\n" );
		}
		if (resp){
			mmc_g.f4BitMode = (busWidthMode)? 1 : 0;	//switch to 4bit mode
		} else {
			printf( "Error selecting %i bit mode\n",(busWidthMode)?4:1);
		}
		if( mmc_g.f4BitMode )
			pmmc->MMCCTL |= MMCCTL_BUSWIDTH_4;
		else
			pmmc->MMCCTL &= ~MMCCTL_BUSWIDTH_4;

		resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
		if (resp) {
			/* send read command */
			unsigned char buf[64];
			unsigned char* p = buf;
			pmmc->MMCFIFOCTL = 1|MMCFIFOCTL_FIFOLEV ; // reset FIFO

			pmmc->MMCNBLC = 1 ;
			pmmc->MMCBLEN = sizeof(buf);
			resp = mmc_cmd(13, 0, MMC_CMDAT_R2|MMC_CMDAT_READ|MMC_CMDAT_DATA_EN); 	//get SD Status
			if (resp) {
				unsigned bytesRead = 0 ;
				unsigned const longsToRead = sizeof(buf)/sizeof(unsigned int);
				memset(p,0,64);
				bytesRead = sizeof(unsigned int)*mmc_ReadFifo((unsigned int*)p,longsToRead);
				pmmc->MMCFIFOCTL = 1|MMCFIFOCTL_FIFOLEV ; // reset FIFO
				pmmc->MMCFIFOCTL = 0|MMCFIFOCTL_FIFOLEV ; // reset FIFO
				if ((p[0] >> 6)!= busWidthMode) {
					printf( "!!!!!Error selecting bus width of %i bits\n",(busWidthMode)? 4 : 1);
					resp = mmc_cmd(SD_APP_CMD55, rca<<16, MMC_CMDAT_R1);
					if (resp) {
						resp = mmc_cmd(6, 0, MMC_CMDAT_R1);
						mmc_g.f4BitMode = 0;	//back to 1 bit mode
						setenv( "sd1bit", "1" );
					}
				} else {
					printf( "bus width == %u\n", mmc_g.f4BitMode ? 4 : 1 );
				}
			} else {
				printf( "Error reading bytes in %u bit mode\n", mmc_g.f4BitMode ? 4 : 1 );
			}
         
/*
*/
			mmc_g.mmc_dev.block_read = mmc_bread;

			stop_clock();
			set_clock(20000000);
			start_clock();
         
			fat_register_device(&mmc_g.mmc_dev,1); /* partitions start counting with 1 */
         
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
	unsigned status ;
	unsigned prevStat ;

	/* Send SD_SEND_IF_COND command to set voltage ?? */
//	printf( "cmd: %04x, args: %08x, flags %04x\n", cmd, arg, cmdat );
	cmd |= cmdat;
	status = pmmc->MMCST0 ; // flush old status

//	pmmc->MMCFIFOCTL = 1|MMCFIFOCTL_FIFOLEV ; // reset FIFO
	pmmc->MMCIM = MMCSD_EVENT_ERROR_CMDCRC |
		MMCSD_EVENT_ERROR_DATACRC |
		MMCSD_EVENT_ERROR_CMDTIMEOUT |
		MMCSD_EVENT_ERROR_DATATIMEOUT ;
	if (cmdat & MMC_CMDAT_DATA_EN){
		pmmc->MMCIM |= MMCSD_EVENT_BLOCK_XFERRED ;
		if (cmdat & MMC_CMDAT_WRITE){
			pmmc->MMCIM |= MMCSD_EVENT_WRITE ;
		} else {
			pmmc->MMCIM |= MMCSD_EVENT_READ ;
		}
	}
//	printf( "cmd == 0x%04x, arg = %08x, int %d, irq %x\n", cmd, arg, INT_ASSERTED, pmmc->MMCIM );
	if( INT_ASSERTED ) CLEAR_INT ;
	memset(mmc_g.resp,0xcc,sizeof(mmc_g.resp));
	pmmc->MMCARGHL = arg ;
	pmmc->MMCCMD   = cmd ;

	if (0 != (cmdat & MMC_CMDAT_DATA_EN)) return mmc_g.resp ;
	status = 0 ;
	prevStat = 0x01234 ;
	while (!(status & (MMCSD_EVENT_EOFCMD|MMCSD_EVENT_ERROR))) {
		status = pmmc->MMCST0 ;
		if (status != prevStat) {
//			printf( "status == 0x%lx..., cmdat %04x, int %d\n", status, cmdat, INT_ASSERTED );
			prevStat = status ; 
		}
		if ( ( 0 != (cmdat & MMC_CMDAT_DATA_EN)) &&
				(status & MMCSD_EVENT_READ) ){
//			printf( "data ready\n" );
			break ;
		}
		if( tstc() ){
			getc();
			printf( "bailing...\n" );
			break ;
		}
	}
#if 0
	printf( "%04x/%08lx/%08lx\n", status, pmmc->MMCST0, pmmc->MMCST1 );
	printf( "rspIdx == %08x\n", pmmc->MMCCIDX );
	printf( "rsp01 == 0x%08lx\n", pmmc->MMCRSP01 );
	printf( "rsp23 == 0x%08lx\n", pmmc->MMCRSP23 );
	printf( "rsp45 == 0x%08lx\n", pmmc->MMCRSP45 );
	printf( "rsp67 == 0x%08lx\n", pmmc->MMCRSP67 );
#endif

	if( INT_ASSERTED ) CLEAR_INT ;
	if( status & MMCSD_EVENT_ERROR ){
//		printf( "MMCSD error cmd %u %x.%08lx/%08lx\n", cmd, status, pmmc->MMCST0, pmmc->MMCST1 );
		return 0 ;
	}

	switch (cmdat & MMC_CMDAT_RSP_MASK) {
		case MMC_CMDAT_R1:
		case MMC_CMDAT_R3: {
			unsigned long *dst = (unsigned long *)mmc_g.resp ;
			unsigned long rv[2];
			rv[0] = pmmc->MMCRSP45;
			rv[1] = pmmc->MMCRSP67;
			dst[0] = (((unsigned char*)rv)[0+2]) | /* MMCRSP5  */
					(rv[1]<<8);					  /* MMCRSP67 */
			dst[1] = (((unsigned char*)rv)[4+3]) | /* MMCRSP7 high */
					(((unsigned char)pmmc->MMCCIDX)<<8);
			break ;
		}

		case MMC_CMDAT_R2: {
			unsigned long *dst = (unsigned long *)mmc_g.resp ;
			dst[0] = pmmc->MMCRSP01;
			dst[1] = pmmc->MMCRSP23;
			dst[2] = pmmc->MMCRSP45;
			dst[3] = pmmc->MMCRSP67;
			mmc_g.resp[16] = (unsigned char)pmmc->MMCCIDX;
			break ;
		}
		//case 0 - no response to command is default
		default: {
			*((unsigned int *)mmc_g.resp) = 0;
		}
	}

//	printf( "rstat 0x%08lx/0x%08lx\n", pmmc->MMCST0, pmmc->MMCST1 );
	return mmc_g.resp;
}

#endif
