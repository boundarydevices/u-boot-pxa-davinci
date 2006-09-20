/*
 * (c) 2004 Sascha Hauer <sascha@saschahauer.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#if defined (CONFIG_IMX_SERIAL)


#ifndef CONFIG_IMX_SERIAL_NONE

#ifndef UART_BASE

#include <asm/arch/imx-regs.h>
#if defined CONFIG_IMX_SERIAL1
#define UART_BASE IMX_UART1_BASE
#elif defined CONFIG_IMX_SERIAL2
#define UART_BASE IMX_UART2_BASE
#else
#error "define CONFIG_IMX_SERIAL1, CONFIG_IMX_SERIAL2 or CONFIG_IMX_SERIAL_NONE or UART_BASE"
#endif

#endif

struct imx_serial {
	volatile uint32_t urxd[16];
	volatile uint32_t utxd[16];
	volatile uint32_t ucr1;
	volatile uint32_t ucr2;
	volatile uint32_t ucr3;
	volatile uint32_t ucr4;
	volatile uint32_t ufcr;
	volatile uint32_t usr1;
	volatile uint32_t usr2;
	volatile uint32_t uesc;
	volatile uint32_t utim;
	volatile uint32_t ubir;
	volatile uint32_t ubmr;
	volatile uint32_t ubrc;
#ifndef CONFIG_IMX31
	volatile uint32_t bipr[4];
	volatile uint32_t bmpr[4];
#else
	volatile uint32_t uonems;
#endif
	volatile uint32_t uts;
};

/* UART Control Register Bit Fields.*/
#ifndef URXD_CHARRDY
#define  URXD_CHARRDY    (1<<15)
#define  URXD_ERR        (1<<14)
#define  URXD_OVRRUN     (1<<13)
#define  URXD_FRMERR     (1<<12)
#define  URXD_BRK        (1<<11)
#define  URXD_PRERR      (1<<10)
#define  UCR1_ADEN       (1<<15) /* Auto dectect interrupt */
#define  UCR1_ADBR       (1<<14) /* Auto detect baud rate */
#define  UCR1_TRDYEN     (1<<13) /* Transmitter ready interrupt enable */
#define  UCR1_IDEN       (1<<12) /* Idle condition interrupt */
#define  UCR1_RRDYEN     (1<<9)	 /* Recv ready interrupt enable */
#define  UCR1_RDMAEN     (1<<8)	 /* Recv ready DMA enable */
#define  UCR1_IREN       (1<<7)	 /* Infrared interface enable */
#define  UCR1_TXMPTYEN   (1<<6)	 /* Transimitter empty interrupt enable */
#define  UCR1_RTSDEN     (1<<5)	 /* RTS delta interrupt enable */
#define  UCR1_SNDBRK     (1<<4)	 /* Send break */
#define  UCR1_TDMAEN     (1<<3)	 /* Transmitter ready DMA enable */
#define  UCR1_UARTCLKEN  (1<<2)	 /* UART clock enabled */
#define  UCR1_DOZE       (1<<1)	 /* Doze */
#define  UCR1_UARTEN     (1<<0)	 /* UART enabled */
#define  UCR2_ESCI     	 (1<<15) /* Escape seq interrupt enable */
#define  UCR2_IRTS  	 (1<<14) /* Ignore RTS pin */
#define  UCR2_CTSC  	 (1<<13) /* CTS pin control */
#define  UCR2_CTS        (1<<12) /* Clear to send */
#define  UCR2_ESCEN      (1<<11) /* Escape enable */
#define  UCR2_PREN       (1<<8) /* Parity enable */
#define  UCR2_PROE       (1<<7) /* Parity odd/even */
#define  UCR2_STPB       (1<<6)	/* Stop */
#define  UCR2_WS         (1<<5)	/* Word size */
#define  UCR2_RTSEN      (1<<4)	/* Request to send interrupt enable */
#define  UCR2_TXEN       (1<<2)	/* Transmitter enabled */
#define  UCR2_RXEN       (1<<1)	/* Receiver enabled */
#define  UCR2_SRST 	 (1<<0)	/* SW reset */
#define  UCR3_DTREN 	 (1<<13) /* DTR interrupt enable */
#define  UCR3_PARERREN   (1<<12) /* Parity enable */
#define  UCR3_FRAERREN   (1<<11) /* Frame error interrupt enable */
#define  UCR3_DSR        (1<<10) /* Data set ready */
#define  UCR3_DCD        (1<<9)  /* Data carrier detect */
#define  UCR3_RI         (1<<8)  /* Ring indicator */
#define  UCR3_TIMEOUTEN  (1<<7)  /* Timeout interrupt enable */
#define  UCR3_RXDSEN	 (1<<6)  /* Receive status interrupt enable */
#define  UCR3_AIRINTEN   (1<<5)  /* Async IR wake interrupt enable */
#define  UCR3_AWAKEN	 (1<<4)  /* Async wake interrupt enable */
#define  UCR3_REF25 	 (1<<3)  /* Ref freq 25 MHz */
#define  UCR3_REF30 	 (1<<2)  /* Ref Freq 30 MHz */
#define  UCR3_INVT  	 (1<<1)  /* Inverted Infrared transmission */
#define  UCR3_BPEN  	 (1<<0)  /* Preset registers enable */
#define  UCR4_CTSTL_32   (32<<10) /* CTS trigger level (32 chars) */
#define  UCR4_INVR  	 (1<<9)  /* Inverted infrared reception */
#define  UCR4_ENIRI 	 (1<<8)  /* Serial infrared interrupt enable */
#define  UCR4_WKEN  	 (1<<7)  /* Wake interrupt enable */
#define  UCR4_REF16 	 (1<<6)  /* Ref freq 16 MHz */
#define  UCR4_IRSC  	 (1<<5) /* IR special case */
#define  UCR4_TCEN  	 (1<<3) /* Transmit complete interrupt enable */
#define  UCR4_BKEN  	 (1<<2) /* Break condition interrupt enable */
#define  UCR4_OREN  	 (1<<1) /* Receiver overrun interrupt enable */
#define  UCR4_DREN  	 (1<<0) /* Recv data ready interrupt enable */
#define  UFCR_RXTL_SHF   0      /* Receiver trigger level shift */
#define  UFCR_RFDIV      (7<<7) /* Reference freq divider mask */
#define  UFCR_TXTL_SHF   10     /* Transmitter trigger level shift */
#define  USR1_PARITYERR  (1<<15) /* Parity error interrupt flag */
#define  USR1_RTSS  	 (1<<14) /* RTS pin status */
#define  USR1_TRDY  	 (1<<13) /* Transmitter ready interrupt/dma flag */
#define  USR1_RTSD  	 (1<<12) /* RTS delta */
#define  USR1_ESCF  	 (1<<11) /* Escape seq interrupt flag */
#define  USR1_FRAMERR    (1<<10) /* Frame error interrupt flag */
#define  USR1_RRDY       (1<<9)	/* Receiver ready interrupt/dma flag */
#define  USR1_TIMEOUT    (1<<7)	/* Receive timeout interrupt status */
#define  USR1_RXDS  	 (1<<6)	/* Receiver idle interrupt flag */
#define  USR1_AIRINT	 (1<<5)	/* Async IR wake interrupt flag */
#define  USR1_AWAKE 	 (1<<4)	/* Aysnc wake interrupt flag */
#define  USR2_ADET  	 (1<<15) /* Auto baud rate detect complete */
#define  USR2_TXFE  	 (1<<14) /* Transmit buffer FIFO empty */
#define  USR2_DTRF  	 (1<<13) /* DTR edge interrupt flag */
#define  USR2_IDLE  	 (1<<12) /* Idle condition */
#define  USR2_IRINT 	 (1<<8)	/* Serial infrared interrupt flag */
#define  USR2_WAKE  	 (1<<7)	/* Wake */
#define  USR2_RTSF  	 (1<<4)	/* RTS edge interrupt flag */
#define  USR2_TXDC  	 (1<<3)	/* Transmitter complete */
#define  USR2_BRCD  	 (1<<2)	/* Break condition */
#define  USR2_ORE        (1<<1)	/* Overrun error */
#define  USR2_RDR        (1<<0)	/* Recv data ready */
#define  UTS_FRCPERR	 (1<<13) /* Force parity error */
#define  UTS_LOOP        (1<<12) /* Loop tx and rx */
#define  UTS_TXEMPTY	 (1<<6)	/* TxFIFO empty */
#define  UTS_RXEMPTY	 (1<<5)	/* RxFIFO empty */
#define  UTS_TXFULL 	 (1<<4)	/* TxFIFO full */
#define  UTS_RXFULL 	 (1<<3)	/* RxFIFO full */
#define  UTS_SOFTRST	 (1<<0)	/* Software reset */
#endif

void serial_setbrg (void)
{
	serial_init();
}

extern void imx_gpio_mode(int gpio_mode);

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init (void)
{
#ifndef CONFIG_IMX31
	struct imx_serial* base = (struct imx_serial *)UART_BASE;
#ifdef CONFIG_IMX_SERIAL1
	imx_gpio_mode(PC11_PF_UART1_TXD);
	imx_gpio_mode(PC12_PF_UART1_RXD);
#else
	imx_gpio_mode(PB30_PF_UART2_TXD);
	imx_gpio_mode(PB31_PF_UART2_RXD);
#endif

	/* Disable UART */
	base->ucr1 &= ~UCR1_UARTEN;

	/* Set to default POR state */

	base->ucr1 = 0x00000004;
	base->ucr2 = 0x00000000;
	base->ucr3 = 0x00000000;
	base->ucr4 = 0x00008040;
	base->uesc = 0x0000002B;
	base->utim = 0x00000000;
	base->ubir = 0x00000000;
	base->ubmr = 0x00000000;
	base->uts  = 0x00000000;
	/* Set clocks */
	base->ucr4 |= UCR4_REF16;

	/* Configure FIFOs */
	base->ufcr = 0xa81;

#ifndef CONFIG_IMX31
	/* Set the numerator value minus one of the BRM ratio */
	base->ubir = (CONFIG_BAUDRATE / 100) - 1;
	/* Set the denominator value minus one of the BRM ratio	*/
	base->ubmr = 10000 - 1;
#else

	base->ubir = 16-1;
	base->ubmr = (CONFIG_PERIPH_CLK/CONFIG_BAUDRATE) - 1;
#endif

	/* Set to 8N1 */
	base->ucr2 &= ~UCR2_PREN;
	base->ucr2 |= UCR2_WS;
	base->ucr2 &= ~UCR2_STPB;

	/* Ignore RTS */
	base->ucr2 |= UCR2_IRTS;

	/* Enable UART */
	base->ucr1 |= UCR1_UARTEN | UCR1_UARTCLKEN;

	/* Enable FIFOs */
	base->ucr2 |= UCR2_SRST | UCR2_RXEN | UCR2_TXEN;

  	/* Clear status flags */
	base->usr2 |= USR2_ADET  |
	          USR2_DTRF  |
	          USR2_IDLE  |
	          USR2_IRINT |
	          USR2_WAKE  |
	          USR2_RTSF  |
	          USR2_BRCD  |
	          USR2_ORE   |
	          USR2_RDR;

  	/* Clear status flags */
	base->usr1 |= USR1_PARITYERR |
	          USR1_RTSD      |
	          USR1_ESCF      |
	          USR1_FRAMERR   |
	          USR1_AIRINT    |
	          USR1_AWAKE;
#endif
	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is successful, the character read is
 * written into its argument c.
 */
int serial_getc (void)
{
	struct imx_serial* base = (struct imx_serial *)UART_BASE;
	unsigned char ch;

	while(base->uts & UTS_RXEMPTY);

	ch = (char)base->urxd[0];

	return ch;
}

#ifdef CONFIG_HWFLOW
static int hwflow = 0; /* turned off by default */
int hwflow_onoff(int on)
{
}
#endif

/*
 * Output a single byte to the serial port.
 */
void serial_putc (char c)
{
	struct imx_serial* base = (struct imx_serial *)UART_BASE;

	do {
		/* Wait for Tx FIFO not full */
		while (base->uts & UTS_TXFULL);
		base->utxd[0] = c;
		if (c == '\n') c = '\r'; /* If \n, also do \r */
		else break;
	} while (1);
}

void serial_puts (const char *s)
{
	while (*s) {
		serial_putc (*s++);
	}
}
/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc (void)
{
	struct imx_serial* base = (struct imx_serial *)UART_BASE;

	/* If receive fifo is empty, return false */
	if (base->uts & UTS_RXEMPTY)
		return 0;
	return 1;
}

void serial_waitTxComplete(void)
{
	struct imx_serial* base = (struct imx_serial *)UART_BASE;
	do {
	} while ((base->usr2 & USR2_TXDC)==0);
}
#endif /* CONFIG_IMX_SERIAL_NONE */
#endif /* defined CONFIG_IMX */
