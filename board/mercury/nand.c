#include <common.h>
#if (CONFIG_COMMANDS & CFG_CMD_NAND)
#include <nand.h>

#ifdef CFG_DFC_DEBUG1
# define DFC_DEBUG1(fmt, args...) printf(fmt, ##args)
#else
# define DFC_DEBUG1(fmt, args...)
#endif

#ifdef CFG_DFC_DEBUG2
# define DFC_DEBUG2(fmt, args...) printf(fmt, ##args)
#else
# define DFC_DEBUG2(fmt, args...)
#endif

#ifdef CFG_DFC_DEBUG3
# define DFC_DEBUG3(fmt, args...) printf(fmt, ##args)
#else
# define DFC_DEBUG3(fmt, args...)
#endif

struct nfc_regs
{
	volatile unsigned short bufSize;				//0x00
	volatile unsigned short reserved;			//0x02
	volatile unsigned short bufAddr;				//0x04
	volatile unsigned short flashAddr;			//0x06
	volatile unsigned short flashCmd;			//0x08
	volatile unsigned short config;				//0x0a
	volatile unsigned short eccStatus;			//0x0c
	volatile unsigned short eccRsltMain;			//0x0e
	volatile unsigned short eccRsltSpare;		//0x10
	volatile unsigned short writeProtect;		//0x12
	volatile unsigned short startBlk;			//0x14
	volatile unsigned short endBlk;				//0x16
	volatile unsigned short writeProtectStatus;	//0x18
	volatile unsigned short flashConfig1;		//0x1a
	volatile unsigned short flashConfig2;		//0x1c
};

#define CFG1_NFC_RESET (1<<6)
#define CFG1_INT_MASK (1<<4)
#define CFG1_ECC_EN (1<<3)
#define CFG1_SP_EN (1<<2)

#define CFG2_INT		(1<<15)
#define CFG2_FDO_PAGE	(1<<3)
#define CFG2_FDO_ID		(2<<3)
#define CFG2_FDO_STATUS	(4<<3)
#define CFG2_FDI		(1<<2)
#define CFG2_FADD		(1<<1)
#define CFG2_FCMD		(1<<0)

#define NFC_REGS_BASE   (CFG_NAND_BASE + NFC_REGS)
#define NFC_BUFFER_BASE (CFG_NAND_BASE + NFC_BUFFER)
#define NFC_SPARE_BASE  (CFG_NAND_BASE + NFC_SPARE)

int lastReadColumn = 0;
int lastReadPage = 0;

static int bufIndex = -1;
static int bufValidIndex = 0;
static int errIndex;
static int bufReadInProgress=0;
static unsigned int goodBuffer=0;
static unsigned int goodBufferMask=0;

static u_char dfc_read_byte(struct mtd_info *mtd)
{
	unsigned short* nfcBuf = (unsigned short*)NFC_BUFFER_BASE ;
	unsigned int val;
	if (CFG_NAND_16BIT) {
		val = nfcBuf[bufIndex];	//id bytes skip a byte in 16x flashes,
	} else {
		val = nfcBuf[bufIndex>>1];	//byte reads not allowed
		if (bufIndex & 1) val = val >> 8;
	}
	bufIndex++;
	DFC_DEBUG3("dfc_read_byte: %x\n", val);
	return (u_char)val;
}

static void dfc_write_byte(struct mtd_info *mtd, u_char byte)
{
	printf("dfc_write_byte: UNIMPLEMENTED.\n");
}
static u16 dfc_read_word(struct mtd_info *mtd)
{
	printf("dfc_read_word: UNIMPLEMENTED.\n");
	return 0;
}
static void dfc_write_word(struct mtd_info *mtd, u16 word)
{
	printf("dfc_write_word: UNIMPLEMENTED.\n");
}

static int dfc_wait_event(unsigned long timeout_uSec);

static int WaitForAndCorrectPage(void)
{
	int s=0;
	dfc_wait_event(CFG_NAND_OTHER_TO);
	if (!bufReadInProgress) {
		printf("No read in progress\n");
		return -1;
	}
	bufReadInProgress=0;
	{
		struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
		s = nfc->eccStatus;
		if (s & 0x0f) {
			if ((s & 0x0c)==1) {
				unsigned short* nfcBuf = (unsigned short*)(NFC_BUFFER_BASE+bufValidIndex) ; //byte accesses not allowed
				int bit = nfc->eccRsltMain & 0x0fff;
				nfcBuf[bit>>4] ^= (1<<(bit&0x0f));
				printf("Correctable Main ecc error, addr:0x%x s:0x%x\n",(lastReadPage<<11)+bufValidIndex,s);
				s &= ~0x0c;
			}
			if ((s & 0x03)==1) {
				unsigned short* nfcSpare = (unsigned short*)(NFC_SPARE_BASE+(bufValidIndex>>(9-4))) ; //byte accesses not allowed
				int bit = nfc->eccRsltSpare & 0x01f;
				nfcSpare[(bit>>4)] ^= (1<<(bit&0x0f));
				printf("Correctable Spare ecc error, addr:0x%x s:0x%x\n",(lastReadPage<<11)+bufValidIndex,s);
				s &= ~0x03;
			}
		}
		if ((s & 0x0f)==0) {
			goodBuffer |= 1<<(bufValidIndex >> 9);
		} else {
			errIndex = bufValidIndex;
			goodBuffer &= ~(1<<(bufValidIndex >> 9));
		}
	}
	bufValidIndex += 512;
	if (CFG_NAND_2K_PAGE) {
		if (bufValidIndex < 2048){
			struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
			nfc->bufAddr = bufValidIndex >> 9;
			nfc->flashConfig2 = CFG2_FDO_PAGE;	//start page read (528 bytes)
			bufReadInProgress=1;
		} else if (bufValidIndex == 2048) bufValidIndex += 64;
	} else if (bufValidIndex == 512) bufValidIndex += 16;
	return s;
}

//#define OVERLAP_IO

static void dfc_read_buf(struct mtd_info *mtd, u_char* buf, int len)
{
	unsigned short* nfcBuf = (unsigned short*)(NFC_BUFFER_BASE + bufIndex); //byte accesses not allowed
	unsigned short* t = (unsigned short*)buf;
	int limit = (CFG_NAND_2K_PAGE)? (2048+64):(512+16);
	int s = 0;
//	DFC_DEBUG3("dfc_read_buf: reading %d bytes.\n", len);

	if ( (bufIndex & 1) || (((int)t)&1) || ((bufIndex+len)>limit) ) {
		if ((((int)t)&1)==0) {
			t[0] = 0xbeef;
			t[1] = 0xdead;
		}
		if (bufIndex!=-1) {
			bufIndex = -1;
			if ((bufIndex+len)>limit) printf("dfc_read_buf: too much data requested\n");
			else printf("dfc_read_buf: odd alignment\n");
		}
		return;
	}
	do {
		int cp = bufValidIndex - bufIndex;
		if (cp > len) cp = len;
		if (cp > 0) {
			len -= cp;
			bufIndex += cp;
#ifndef OVERLAP_IO
			dfc_wait_event(CFG_NAND_OTHER_TO);	//both accessing different areas of the ram 
												//seems to cause problems
#endif
			if ( ((int)t) & 2) {
				if (cp>1) {
					*t++ = *nfcBuf++;
					cp -= 2;
				}
			}
			if ( ((int)nfcBuf) & 2) {
				while (cp>1) {
					*t++ = *nfcBuf++;
					cp -= 2;
				}
			} else {
				while (cp>3) {
					*((unsigned int*)t)++ = *((unsigned int*)nfcBuf)++;
					cp -= 4;
				}
				if (cp & 2) *t++ = *nfcBuf++;
			}
			if (cp & 1) *((unsigned char*)t) = (unsigned char)(*nfcBuf);
		}
		if (len==0) break;

		s = WaitForAndCorrectPage();
		if (s<0) return;
	} while (1);
	
	if (s & 0x0f) {
		printf("dfc_read_buf: Uncorrectable ecc error, addr:0x%x s:0x%x\n",(lastReadPage<<11)+errIndex,s);
	}
}
/*
 * Write buf to the DFC Controller Data Buffer
 */
static void dfc_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	unsigned short* nfcBuf = (unsigned short*)(NFC_BUFFER_BASE+bufIndex) ;
	unsigned short* t = (unsigned short*)buf;
	int limit = (CFG_NAND_2K_PAGE)? (2048+64):(512+16);
//	DFC_DEBUG2("dfc_write_buf: writing %d bytes starting with 0x%x from %p\n", len, *((unsigned long*) buf),buf);

	bufIndex += len;
	if (bufIndex>limit) {
		printf("dfc_write_buf: too much data\n");
		return;
	}

	if ( ((int)buf) & 3) {
		while (len>1) {
			*nfcBuf++ = *t++;
			len -= 2;
		}
	} else {
		while (len>3) {
			*((unsigned int*)nfcBuf)++ = *((unsigned int*)t)++;
			len -= 4;
		}
		if (len & 2) *nfcBuf++ = *t++;
	}
	if (len & 1) *nfcBuf = *((unsigned char*)t);
}


static void dfc_hwcontrol(struct mtd_info *mtdinfo, int cmd)
{
	return;
}

unsigned long CalcTicks(unsigned long usec);

/* wait_event with timeout */
static int dfc_wait_event(unsigned long timeout_uSec)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	ulong start = get_timer(0);
	ulong timeout;

	timeout = CalcTicks(timeout_uSec);
	while(1) {
		if (nfc->flashConfig2 & CFG2_INT) return 1;
		udelay(1);
		if (get_timer(start) > timeout) break;
	}
	DFC_DEBUG1("dfc_wait_event: TIMEOUT %i uSec\n", timeout_uSec);
	return -1;
}

/* this function is called after Programm and Erase Operations to
 * check for success or failure */
static int dfc_wait(struct mtd_info *mtd, struct nand_chip *nc, int state)
{
	unsigned long ndsr=0;
	unsigned long timeout_uSec = 0;
	if ((state == FL_ERASING) || (state == FL_WRITING)) {
		timeout_uSec = CFG_NAND_PROG_ERASE_TO;
	}

	ndsr = dfc_wait_event(timeout_uSec);
	if (ndsr<0) return 1;
	return 0;
}
static void dfc_preset(void)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	nfc->config = 2;	//unlocked internal ram buffer
	nfc->startBlk = 0;
	nfc->endBlk = 0x800;
	nfc->writeProtect = 4;
	nfc->flashConfig1 = CFG1_INT_MASK;
}

static void dfc_setCmd(int cmd,unsigned long timeout_uSec)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	nfc->flashCmd = (unsigned short)cmd;
	nfc->flashConfig2 = CFG2_FCMD;
	dfc_wait_event(timeout_uSec);
}
static void dfc_setFlashAddr(int col,int row)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	int i=0;
	while(i<5) {
		if (i==2) col = row;
		nfc->flashAddr = (unsigned short)(col & 0xff); col >>= 8;
		nfc->flashConfig2 = CFG2_FADD;
		dfc_wait_event(CFG_NAND_OTHER_TO);
		i++;
	}
}
static void dfc_setFlashRow(int row)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	int i=0;
	while(i<3) {
		nfc->flashAddr = (unsigned short)(row & 0xff); row >>= 8;
		nfc->flashConfig2 = CFG2_FADD;
		dfc_wait_event(CFG_NAND_OTHER_TO);
		i++;
	}
}
static void dfc_setFlashAddr0(void)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	nfc->flashAddr = 0;
	nfc->flashConfig2 = CFG2_FADD;
	dfc_wait_event(CFG_NAND_OTHER_TO);
}
unsigned int dfc_statusRead(void)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	dfc_preset();
	nfc->bufAddr = 0;	//1st buffer

	dfc_setCmd(NAND_CMD_STATUS,CFG_NAND_OTHER_TO);
	nfc->flashConfig2 = CFG2_FDO_STATUS;
	dfc_wait_event(CFG_NAND_OTHER_TO);
	return *((unsigned int *)NFC_BUFFER_BASE);
}

/* cmdfunc send commands to the DFC */
static void dfc_cmdfunc(struct mtd_info *mtd, unsigned command,
			int column, int page_addr)
{
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	/* register struct nand_chip *this = mtd->priv; */

	bufIndex = 0;
	bufValidIndex = 0;
	if (command!=NAND_CMD_RESET)
		dfc_wait_event(CFG_NAND_OTHER_TO);	//wait for any pending commands to finish

	switch (command) {
	case NAND_CMD_READ0:
	{
//		DFC_DEBUG3("dfc_cmdfunc: NAND_CMD_READ0, page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
		lastReadPage = page_addr;
		dfc_preset();
		nfc->bufAddr = 0;	//1st buffer
		dfc_setCmd(NAND_CMD_READ0,CFG_NAND_OTHER_TO);
//read entire 512 byte segment for ECC to work!!!!!
		bufIndex = column;
		dfc_setFlashAddr(0,page_addr);
		dfc_setCmd(NAND_CMD_READSTART,CFG_NAND_OTHER_TO);
		nfc->flashConfig2 = CFG2_FDO_PAGE;	//start page read (528 bytes)
		bufReadInProgress=1;
		goodBufferMask=1;
		goodBuffer=0;
		break;
	}
	case NAND_CMD_READ1:
		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_READ1 unimplemented!\n");
		break;
	case NAND_CMD_READOOB:
		DFC_DEBUG1("dfc_cmdfunc: NAND_CMD_READOOB unimplemented!\n");
		break;
	case NAND_CMD_READID:
//		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_READID.\n");
		dfc_preset();
		nfc->bufAddr = 0;	//1st buffer
		dfc_setCmd(NAND_CMD_READID,CFG_NAND_OTHER_TO);
		dfc_setFlashAddr0();
		nfc->flashConfig1 = CFG1_INT_MASK;
		nfc->bufAddr = 0;	//1st buffer
		
		nfc->flashConfig2 = CFG2_FDO_ID;
		dfc_wait_event(CFG_NAND_OTHER_TO);
		break;
	case NAND_CMD_ERASE1:
		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_ERASE1,  page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
		dfc_preset();
		nfc->bufAddr = 0;	//1st buffer
		dfc_setCmd(NAND_CMD_ERASE1,CFG_NAND_OTHER_TO);
		dfc_setFlashRow(page_addr);
		dfc_setCmd(NAND_CMD_ERASE2,CFG_NAND_PROG_ERASE_TO);
		break;
	case NAND_CMD_ERASE2:
		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_ERASE2 empty due to multicmd.\n");
		break;
	case NAND_CMD_SEQIN:
//		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_SEQIN,  page_addr: 0x%x, column: 0x%x.\n", page_addr, column);
		bufIndex = column;
		dfc_preset();
		nfc->bufAddr = 0;	//1st buffer
		dfc_setCmd(NAND_CMD_SEQIN,CFG_NAND_OTHER_TO);
		dfc_setFlashAddr(0,page_addr);
		break;
	case NAND_CMD_PAGEPROG:
	{
		int i=0;
		int limit = (CFG_NAND_2K_PAGE)? 4:1;
		nfc->flashConfig1 = CFG1_ECC_EN;	//automatically fill in correct ECC to write
		while (i<limit) {
			unsigned long* nfcBuf =  (unsigned long*)(NFC_BUFFER_BASE + (i<<9));
			unsigned long* nfcSpare = (unsigned long*)(NFC_SPARE_BASE + (i<<4));
//			DFC_DEBUG2("NAND_CMD_PAGEPROG,  1st buffer val:0x%8x spare:0x%8x\n", *nfcBuf,*nfcSpare);
			nfc->bufAddr = i++;	//1st buffer
			nfc->flashConfig2 = CFG2_FDI;
			dfc_wait_event(CFG_NAND_PROG_ERASE_TO);
		}
		dfc_setCmd(NAND_CMD_PAGEPROG,CFG_NAND_PROG_ERASE_TO);
		break;
	}
	case NAND_CMD_STATUS:
		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_STATUS.\n");
		dfc_statusRead();
		break;
	case NAND_CMD_RESET:
//		DFC_DEBUG2("dfc_cmdfunc: NAND_CMD_RESET.\n");
		dfc_preset();
		dfc_setCmd(NAND_CMD_RESET,CFG_NAND_OTHER_TO);
		break;
	default:
		printk("dfc_cmdfunc: error, unsupported command.\n");
		break;
	}
//	DFC_DEBUG3("dfc_cmdfunc: exit\n");
}
#define POS_BYTES(n) 6+(16*n),7+(16*n),8+(16*n)
#define FREE_BYTES(n) {0x0c+(16*n),4}
static struct nand_oobinfo delta_oob = {
	.useecc = MTD_NANDECC_AUTOPL_USR, /* MTD_NANDECC_PLACEONLY, */
	.eccbytes = 3*4,
	.eccpos = {POS_BYTES(0),POS_BYTES(1),POS_BYTES(2),POS_BYTES(3)},
	.oobfree = { FREE_BYTES(0), FREE_BYTES(1),FREE_BYTES(2),FREE_BYTES(3)}
};

static uint8_t scan_ff_pattern[] = { 0xff, 0xff };

static struct nand_bbt_descr delta_bbt_descr = {
	.options = 0,
	.offs = 0,
	.len = 2,
	.pattern = scan_ff_pattern
};

void dfc_enable_hwecc(struct mtd_info *mtd, int mode)
{
}
int dfc_correct_data(struct mtd_info *mtd, u_char *dat, u_char *read_ecc, u_char *calc_ecc)
{
	unsigned int mask = goodBufferMask;
	goodBufferMask = mask<<1;
	return (mask&goodBuffer)?0:-1;
}
int dfc_calculate_ecc(struct mtd_info *mtd, const u_char *dat, u_char *ecc_code)
{
	return 0;
}

void board_nand_init(struct nand_chip *nand)
{
	unsigned int* nfcBuf = (unsigned int*)NFC_BUFFER_BASE ;
	struct nfc_regs* nfc = (struct nfc_regs*)NFC_REGS_BASE ;
	
	nand->hwcontrol = dfc_hwcontrol;
/*	nand->dev_ready = dfc_device_ready; */
	nand->eccmode = NAND_ECC_HW3_512;
	nand->options = NAND_BUSWIDTH_16;
	nand->calculate_ecc=dfc_calculate_ecc;
	nand->correct_data=dfc_correct_data;
	nand->enable_hwecc=dfc_enable_hwecc;
	nand->waitfunc = dfc_wait;
	nand->read_byte = dfc_read_byte;
	nand->write_byte = dfc_write_byte;
	nand->read_word = dfc_read_word;
	nand->write_word = dfc_write_word;
	nand->read_buf = dfc_read_buf;
	nand->write_buf = dfc_write_buf;

	nand->cmdfunc = dfc_cmdfunc;
	nand->autooob = &delta_oob;
	nand->badblock_pattern = &delta_bbt_descr;
	printf("Reseting NAND Flash...\n");
	nfc->flashConfig1 = CFG1_NFC_RESET;
	udelay(500);
	dfc_cmdfunc(NULL, NAND_CMD_RESET, 0,0);
	udelay(500);

	dfc_preset();	//unlock ram
	nfcBuf[0] = 0;
	nfcBuf[1] = 0;
	if (nfcBuf[0] != 0) {
		printf("NFCBUF not allowing write!\n");
	}
#if 0
	do {
		dfc_cmdfunc(NULL, NAND_CMD_READID, 0,0);
		udelay(1000);
		if (nfcBuf[0]) break;
	} while (1);
#endif
}
#endif
