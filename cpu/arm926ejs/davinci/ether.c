/*
 * Ethernet driver for TI TMS320DM644x (DaVinci) chips.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelessly stolen from TI's dm644x_emac.c. Original copyright
 * follows:
 *
 * ----------------------------------------------------------------------------
 *
 * dm644x_emac.c
 *
 * TI DaVinci (DM644X) EMAC peripheral driver source for DV-EVM
 *
 * Copyright (C) 2005 Texas Instruments.
 *
 * ----------------------------------------------------------------------------
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * ----------------------------------------------------------------------------

 * Modifications:
 * ver. 1.0: Sep 2005, Anant Gole - Created EMAC version for uBoot.
 * ver  1.1: Nov 2005, Anant Gole - Extended the RX logic for multiple descriptors
 *
 */
#include <common.h>
#include <command.h>
#include <net.h>
#include <miiphy.h>
#include <asm/arch/emac_defs.h>

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_CMD_NET

unsigned int	emac_dbg = 0;
#define debug_emac(fmt,args...)	if (emac_dbg) printf(fmt,##args)

/* Internal static functions */
static int dm644x_eth_hw_init (void);
static int dm644x_eth_open (void);
static int dm644x_eth_close (void);
static int dm644x_eth_send_packet (volatile void *packet, int length);
static int dm644x_eth_rcv_packet (void);
static void dm644x_eth_mdio_enable(void);

static int gen_init_phy(int phy_addr);
static int gen_is_phy_connected(int phy_addr);
static int gen_get_link_speed(int phy_addr);
static int gen_auto_negotiate(int phy_addr);

/* Wrappers exported to the U-Boot proper */
int eth_hw_init(void)
{
	return(dm644x_eth_hw_init());
}

int eth_init(bd_t * bd)
{
	return(dm644x_eth_open());
}

void eth_halt(void)
{
	dm644x_eth_close();
}

int eth_send(volatile void *packet, int length)
{
	return(dm644x_eth_send_packet(packet, length));
}

int eth_rx(void)
{
	return(dm644x_eth_rcv_packet());
}

void eth_mdio_enable(void)
{
	dm644x_eth_mdio_enable();
}
/* End of wrappers */


static u_int8_t dm644x_eth_mac_addr[] __attribute__ ((aligned (4))) = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };


/* EMAC Addresses */
static volatile emac_regs	*adap_emac = (emac_regs *)EMAC_BASE_ADDR;
static volatile ewrap_regs	*adap_ewrap = (ewrap_regs *)EMAC_WRAPPER_BASE_ADDR;
static volatile mdio_regs	*adap_mdio = (mdio_regs *)EMAC_MDIO_BASE_ADDR;

/* EMAC descriptors */
static volatile emac_desc	*emac_rx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_RX_DESC_BASE);
static volatile emac_desc	*emac_tx_desc = (emac_desc *)(EMAC_WRAPPER_RAM_ADDR + EMAC_TX_DESC_BASE);
static volatile emac_desc	*emac_rx_active_head;
static volatile emac_desc	*emac_rx_active_tail;
static int			emac_rx_queue_active;

static volatile emac_desc	*emac_tx_free_head;
static volatile emac_desc	*emac_tx_free_tail;
static volatile emac_desc	*emac_tx_active_head;
static volatile emac_desc	*emac_tx_active_tail;

/* Receive packet buffers */
static unsigned char		emac_rx_buffers[EMAC_MAX_RX_BUFFERS * (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)];

/* PHY address for a discovered PHY (0xff - not found) */
static volatile u_int8_t	active_phy_addr = 0xff;

phy_t				phy;

/*
 * This function must be called before emac_open() if you want to override
 * the default mac address.
 */
void dm644x_eth_set_mac_addr(const u_int8_t *addr)
{
	int i;

	for (i = 0; i < sizeof (dm644x_eth_mac_addr); i++) {
		dm644x_eth_mac_addr[i] = addr[i];
	}
	adap_emac->MACSRCADDRHI = *((unsigned int*)dm644x_eth_mac_addr);
	adap_emac->MACSRCADDRLO = *((unsigned short*)(dm644x_eth_mac_addr+4));
}

static void dm644x_eth_mdio_enable(void)
{
	u_int32_t	clkdiv;

	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;

	adap_mdio->CONTROL = (clkdiv & 0xff) |
		MDIO_CONTROL_ENABLE |
		MDIO_CONTROL_FAULT |
		MDIO_CONTROL_FAULT_ENABLE;

	while (adap_mdio->CONTROL & MDIO_CONTROL_IDLE) {;}
}

/*
 * Tries to find an active connected PHY. Returns 1 if address if found.
 * If no active PHY (or more than one PHY) found returns 0.
 * Sets active_phy_addr variable.
 */
static int dm644x_eth_phy_detect(void)
{
	u_int32_t	phy_act_state;
	int		i;

	active_phy_addr = 0xff;

	if ((phy_act_state = adap_mdio->ALIVE) == 0)
		return(0);				/* No active PHYs */

	debug_emac("dm644x_eth_phy_detect(), ALIVE = 0x%08x\n", phy_act_state);

	for (i = 0; i < 32; i++) {
		if (phy_act_state & (1 << i)) {
			if (phy_act_state & ~(1 << i))
				return(0);		/* More than one PHY */
			else {
				active_phy_addr = i;
				return(1);
			}
		}
	}

	return(0);	/* Just to make GCC happy */
}


/* Read a PHY register via MDIO inteface. Returns 1 on success, 0 otherwise */
int dm644x_eth_phy_read(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t *data)
{
	int	tmp;

	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

	adap_mdio->USERACCESS0 = MDIO_USERACCESS0_GO |
				MDIO_USERACCESS0_WRITE_READ |
				((reg_num & 0x1f) << 21) |
				((phy_addr & 0x1f) << 16);

	/* Wait for command to complete */
	while ((tmp = adap_mdio->USERACCESS0) & MDIO_USERACCESS0_GO) {;}

	if (tmp & MDIO_USERACCESS0_ACK) {
		*data = tmp & 0xffff;
		return(1);
	}

	*data = -1;
	return(0);
}

/* Write to a PHY register via MDIO inteface. Blocks until operation is complete. */
int dm644x_eth_phy_write(u_int8_t phy_addr, u_int8_t reg_num, u_int16_t data)
{

	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

	adap_mdio->USERACCESS0 = MDIO_USERACCESS0_GO |
				MDIO_USERACCESS0_WRITE_WRITE |
				((reg_num & 0x1f) << 21) |
				((phy_addr & 0x1f) << 16) |
				(data & 0xffff);

	/* Wait for command to complete */
	while (adap_mdio->USERACCESS0 & MDIO_USERACCESS0_GO) {;}

	return(1);
}

/* PHY functions for a generic PHY */
static int gen_init_phy(int phy_addr)
{
	int	ret = 1;

	if (gen_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = gen_get_link_speed(phy_addr);
	}

	return(ret);
}

static int gen_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return(dm644x_eth_phy_read(phy_addr, PHY_PHYIDR1, &dummy));
}

static int gen_get_link_speed(int phy_addr)
{
	u_int16_t	tmp;

	if (dm644x_eth_phy_read(phy_addr, MII_STATUS_REG, &tmp) && (tmp & 0x04))
		return(1);

	return(0);
}

static int gen_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;

	if (!dm644x_eth_phy_read(phy_addr, PHY_BMCR, &tmp))
		return(0);

	/* Restart Auto_negotiation  */
	tmp |= PHY_BMCR_AUTON;
	dm644x_eth_phy_write(phy_addr, PHY_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (!dm644x_eth_phy_read(phy_addr, PHY_BMSR, &tmp))
		return(0);

	if (!(tmp & PHY_BMSR_AUTN_COMP))
		return(0);

	return(gen_get_link_speed(phy_addr));
}
/* End of generic PHY functions */


#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
static int dm644x_mii_phy_read(char *devname, unsigned char addr, unsigned char reg, unsigned short *value)
{
	return(dm644x_eth_phy_read(addr, reg, value) ? 0 : 1);
}

static int dm644x_mii_phy_write(char *devname, unsigned char addr, unsigned char reg, unsigned short value)
{
	return(dm644x_eth_phy_write(addr, reg, value) ? 0 : 1);
}

int dm644x_eth_miiphy_initialize(bd_t *bis)
{
	miiphy_register(phy.name, dm644x_mii_phy_read, dm644x_mii_phy_write);

	return(1);
}
#endif

/*
 * This function initializes the emac hardware. It does NOT initialize
 * EMAC modules power or pin multiplexors, that is done by board_init()
 * much earlier in bootup process. Returns 1 on success, 0 otherwise.
 */
static int dm644x_eth_hw_init(void)
{
	u_int32_t	phy_id;
	u_int16_t	tmp;
	int		i;

	dm644x_eth_mdio_enable();

	for (i = 0; i < 256; i++) {
		if (adap_mdio->ALIVE)
			break;
		udelay(10);
	}

	if (i >= 256) {
		printf("No ETH PHY detected!!!\n");
		return(0);
	}

	/* Find if a PHY is connected and get it's address */
	if (!dm644x_eth_phy_detect())
		return(0);

	/* Get PHY ID and initialize phy_ops for a detected PHY */
	if (!dm644x_eth_phy_read(active_phy_addr, PHY_PHYIDR1, &tmp)) {
		active_phy_addr = 0xff;
		return(0);
	}

	phy_id = (tmp << 16) & 0xffff0000;

	if (!dm644x_eth_phy_read(active_phy_addr, PHY_PHYIDR2, &tmp)) {
		active_phy_addr = 0xff;
		return(0);
	}

	phy_id |= tmp & 0x0000ffff;

	switch (phy_id) {
		case PHY_LXT972:
			sprintf(phy.name, "LXT972 @ 0x%02x", active_phy_addr);
			phy.init = lxt972_init_phy;
			phy.is_phy_connected = lxt972_is_phy_connected;
			phy.get_link_speed = lxt972_get_link_speed;
			phy.auto_negotiate = lxt972_auto_negotiate;
			break;
		case PHY_DP83848:
			sprintf(phy.name, "DP83848 @ 0x%02x", active_phy_addr);
			phy.init = dp83848_init_phy;
			phy.is_phy_connected = dp83848_is_phy_connected;
			phy.get_link_speed = dp83848_get_link_speed;
			phy.auto_negotiate = dp83848_auto_negotiate;
			break;
		default:
			sprintf(phy.name, "GENERIC @ 0x%02x", active_phy_addr);
			phy.init = gen_init_phy;
			phy.is_phy_connected = gen_is_phy_connected;
			phy.get_link_speed = gen_get_link_speed;
			phy.auto_negotiate = gen_auto_negotiate;
	}

	return(1);
}


/* Eth device open */
static int dm644x_eth_open(void)
{
	dv_reg_p		addr;
	u_int32_t		clkdiv, cnt;
	volatile emac_desc	*desc;

	debug_emac("+ emac_open\n");

	/* Reset EMAC module and disable interrupts in wrapper */
	adap_emac->SOFTRESET = 1;
	while (adap_emac->SOFTRESET != 0) {;}
	adap_ewrap->EWCTL = 0;
	for (cnt = 0; cnt < 5; cnt++) {
		clkdiv = adap_ewrap->EWCTL;
	}


	adap_emac->TXCONTROL = 0x01;
	adap_emac->RXCONTROL = 0x01;

	/* Set MAC Addresses & Init multicast Hash to 0 (disable any multicast receive) */
	/* Using channel 0 only - other channels are disabled */
	adap_emac->MACINDEX = 0;
	adap_emac->MACADDRHI = *((unsigned int*)dm644x_eth_mac_addr);
	adap_emac->MACADDRLO = *((unsigned short*)(dm644x_eth_mac_addr+4));

	adap_emac->MACHASH1 = 0;
	adap_emac->MACHASH2 = 0;

	/* Set source MAC address - REQUIRED */
	adap_emac->MACSRCADDRHI = *((unsigned int*)dm644x_eth_mac_addr);
	adap_emac->MACSRCADDRLO = *((unsigned short*)(dm644x_eth_mac_addr+4));

	/* Set DMA 8 TX / 8 RX Head pointers to 0
	 * TX0HDP - TX7HDP, followed immediately by
	 * RX0HDP - RX7HDP, followed immediately by TX0CP
	 */
	addr= &adap_emac->TX0HDP;
	for(cnt = 0; cnt < 8*2; cnt++) {
		*addr++ = 0;
	}

	/* Set DMA 8 TXCP / 8 RXCP pointers to no interrupt
	 * TX0CP - TX7CP, followed immediately by
	 * RX0CP - RX7CP
	 */
	for(cnt = 0; cnt < 8*2; cnt++) {
		*addr = *addr;
		addr++;
	}

	/* Clear Statistics (do this before setting MacControl register) */
	addr = &adap_emac->RXGOODFRAMES;
	for(cnt = 0; cnt < EMAC_NUM_STATS; cnt++)
		*addr++ = 0;

	/* No multicast addressing */
	adap_emac->MACHASH1 = 0;
	adap_emac->MACHASH2 = 0;

	/* Create RX queue and set receive process in place */
	desc = emac_rx_desc;
	emac_rx_active_head = desc;
	for (cnt = 0; cnt < EMAC_MAX_RX_BUFFERS; cnt++) {
		desc->next = (desc + 1);
		desc->buffer = &emac_rx_buffers[cnt * (EMAC_MAX_ETHERNET_PKT_SIZE + EMAC_PKT_ALIGN)];
		desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		desc++;
	}

	/* Set the last descriptor's "next" parameter to 0 to end the RX desc list */
	desc--;
	desc->next = 0;
	emac_rx_active_tail = desc;
	emac_rx_queue_active = 1;

	desc = emac_tx_desc;
	emac_tx_free_head = desc;
	for (cnt = 0; cnt < EMAC_MAX_TX_BUFFERS; cnt++) {
		desc->next = (desc + 1);
		desc->pkt_flag_len = 0;
		desc++;
	}
	/* Set the last descriptor's "next" parameter to 0 to end the TX desc list */
	desc--;
	desc->next = 0;
	emac_tx_free_tail = desc;

	/* Enable TX/RX */
	adap_emac->RXMAXLEN = EMAC_MAX_ETHERNET_PKT_SIZE;
	adap_emac->RXBUFFEROFFSET = 0;

	/* No fancy configs - Use this for promiscous for debug - EMAC_RXMBPENABLE_RXCAFEN_ENABLE */
	adap_emac->RXMBPENABLE = EMAC_RXMBPENABLE_RXBROADEN | (3<<22);

	/* Enable ch 0 only */
	adap_emac->RXUNICASTSET = 0x01;

	/* Enable MII interface and Full duplex mode */
	adap_emac->MACCONTROL = (EMAC_MACCONTROL_MIIEN_ENABLE | EMAC_MACCONTROL_FULLDUPLEX_ENABLE);

	/* Init MDIO & get link state */
	clkdiv = (EMAC_MDIO_BUS_FREQ / EMAC_MDIO_CLOCK_FREQ) - 1;
	adap_mdio->CONTROL = ((clkdiv & 0xff) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULT);

	if (!phy.auto_negotiate(active_phy_addr))
		return(0);

	/* Start receive process */
	adap_emac->RX0HDP = (u_int32_t)emac_rx_desc;

	debug_emac("- emac_open\n");

	return(1);
}

/* EMAC Channel Teardown */
static void dm644x_eth_ch_teardown(int ch)
{
	dv_reg		dly = 0xff;
	dv_reg		cp;

	debug_emac("+ emac_ch_teardown\n");

	if (ch == EMAC_CH_TX) {
		/* Init TX channel teardown */
		adap_emac->TXTEARDOWN = 1;
		for (cp = 0; cp != 0xfffffffc; cp = adap_emac->TX0CP) {
			/* Wait here for Tx teardown completion interrupt to occur
			 * Note: A task delay can be called here to pend rather than
			 * occupying CPU cycles - anyway it has been found that teardown
			 * takes very few cpu cycles and does not affect functionality */
			 dly--;
			 udelay(1);
			 if (dly == 0)
				break;
		}
		adap_emac->TX0CP = cp;
		adap_emac->TX0HDP = 0;
	} else {
		/* Init RX channel teardown */
		adap_emac->RXTEARDOWN = 1;
		for (cp = 0; cp != 0xfffffffc; cp = adap_emac->RX0CP) {
			/* Wait here for Rx teardown completion interrupt to occur
			 * Note: A task delay can be called here to pend rather than
			 * occupying CPU cycles - anyway it has been found that teardown
			 * takes very few cpu cycles and does not affect functionality */
			 dly--;
			 udelay(1);
			 if (dly == 0)
				break;
		}
		adap_emac->RX0CP = cp;
		adap_emac->RX0HDP = 0;
	}

	debug_emac("- emac_ch_teardown\n");
}

/* Eth device close */
static int dm644x_eth_close(void)
{
	debug_emac("+ emac_close\n");

	dm644x_eth_ch_teardown(EMAC_CH_TX);	/* TX Channel teardown */
	dm644x_eth_ch_teardown(EMAC_CH_RX);	/* RX Channel teardown */

	/* Reset EMAC module and disable interrupts in wrapper */
	adap_emac->SOFTRESET = 1;
	while (adap_emac->SOFTRESET != 0) {;}
	adap_ewrap->EWCTL = 0;
	/* Set source MAC address - REQUIRED */
	adap_emac->MACSRCADDRHI = *((unsigned int*)dm644x_eth_mac_addr);
	adap_emac->MACSRCADDRLO = *((unsigned short*)(dm644x_eth_mac_addr+4));

	debug_emac("- emac_close\n");
	return(1);
}

static emac_desc* QueuePacket(volatile void *packet, int length)
{
	volatile emac_desc* desc = emac_tx_free_head;
	if (desc) {
		emac_tx_free_head = desc->next;
		if (!emac_tx_free_head) {
			emac_tx_free_tail = 0;
		}
		/* Populate the TX descriptor */
		desc->next = 0;
		desc->buffer = (u_int8_t *) packet;
		desc->buff_off_len = (length & 0xffff);
		desc->pkt_flag_len = ((length & 0xffff) |
				      EMAC_CPPI_SOP_BIT |
				      EMAC_CPPI_OWNERSHIP_BIT |
				      EMAC_CPPI_EOP_BIT);
		/* Send the packet */
		if (emac_tx_active_tail) {
			emac_tx_active_tail->next = desc;
			if (emac_tx_active_tail->pkt_flag_len & EMAC_CPPI_EOQ_BIT) {
				emac_tx_active_tail->pkt_flag_len &= ~EMAC_CPPI_EOQ_BIT;
				adap_emac->TX0HDP = (unsigned int)desc;
			}
			emac_tx_active_tail = desc;
		} else {
			adap_emac->TX0HDP = (unsigned int)desc;
			emac_tx_active_tail = emac_tx_active_head = desc;
		}
	}
	return desc;
}
/*
 * This function sends a single packet on the network and returns
 * positive number (number of bytes transmitted) or negative for error
 */
static int dm644x_eth_send_packet (volatile void *packet, int length)
{
	volatile emac_desc* desc;
	int ret_status = -1;
	ulong cur_time;
	ulong start_time;
	ulong elapsed_time;
	ulong link_check_time;

	/* Check packet size and if < EMAC_MIN_ETHERNET_PKT_SIZE, pad it up */
	if (length < EMAC_MIN_ETHERNET_PKT_SIZE) {
		length = EMAC_MIN_ETHERNET_PKT_SIZE;
	}
	desc = QueuePacket(packet,length);
	/* Wait for packet to complete or link down */
	link_check_time = start_time = get_timer(0);
	do {
		volatile emac_desc* d;
		d = emac_tx_active_head;
		if (!d) {
			ret_status = length;	/* descriptor was processed in nested call */
			break;
		}
		if ((d->pkt_flag_len & EMAC_CPPI_OWNERSHIP_BIT)==0) {
			emac_tx_active_head = d->next;
			if (d->next) {
				/* probably not necessary, but to be very safe*/
				if (d->pkt_flag_len & EMAC_CPPI_EOQ_BIT) {
					adap_emac->TX0HDP = (unsigned int)d->next;
				}
			} else {
				emac_tx_active_tail = 0;
			}
			adap_emac->TX0CP = (unsigned int)d;

			d->next = 0;
			d->pkt_flag_len = 0;
			if (emac_tx_free_tail) {
				emac_tx_free_tail->next = d;
				emac_tx_free_tail = d;
			} else {
				emac_tx_free_tail = emac_tx_free_head = d;
			}
			if (desc) {
				if (d==desc) {
					ret_status = length;
					break;
				}
			} else {
				desc = QueuePacket(packet,length);
			}
			continue;
		}
		cur_time = get_timer(0);
		elapsed_time = cur_time - link_check_time;
		if (elapsed_time > (CFG_HZ>>4)) {
			link_check_time = cur_time;
			if (!phy.get_link_speed (active_phy_addr)) {
				dm644x_eth_ch_teardown (EMAC_CH_TX);
				printf ("WARN: dm644x_eth_send_packet: No link\n");
				break;
			}
		}
		elapsed_time = cur_time - start_time;
		if (elapsed_time > CFG_HZ) {
			dm644x_eth_ch_teardown (EMAC_CH_TX);
			printf ("WARN: dm644x_eth_send_packet: 1 second timeout\n");
			break;
		}
	} while (1);
	return (ret_status);
}

/*
 * This function handles receipt of a packet from the network
 */
static int dm644x_eth_rcv_packet (void)
{
	volatile emac_desc *rx_curr_desc;
	volatile emac_desc *curr_desc;
	volatile emac_desc *tail_desc;
	int status, ret = 0;

	rx_curr_desc = emac_rx_active_head;
	status = rx_curr_desc->pkt_flag_len;
	if ((rx_curr_desc) && ((status & EMAC_CPPI_OWNERSHIP_BIT) == 0)) {
		if (status & EMAC_CPPI_RX_ERROR_FRAME) {
			/* Error in packet - discard it and requeue desc */
			printf("rcv_pkt: Error in packet %x\n", status);
			ret = -1;
		} else {
			NetReceive (rx_curr_desc->buffer,
				    (rx_curr_desc->buff_off_len & 0xffff));
			ret = rx_curr_desc->buff_off_len & 0xffff;
		}

		/* Ack received packet descriptor */
		adap_emac->RX0CP = (unsigned int) rx_curr_desc;
		curr_desc = rx_curr_desc;
		emac_rx_active_head = rx_curr_desc->next;

		if (status & EMAC_CPPI_EOQ_BIT) {
			if (emac_rx_active_head) {
				adap_emac->RX0HDP =
					(unsigned int) emac_rx_active_head;
			} else {
				emac_rx_queue_active = 0;
				printf ("INFO:emac_rcv_packet: RX Queue not active\n");
			}
		}

		/* Recycle RX descriptor */
		rx_curr_desc->buff_off_len = EMAC_MAX_ETHERNET_PKT_SIZE;
		rx_curr_desc->pkt_flag_len = EMAC_CPPI_OWNERSHIP_BIT;
		rx_curr_desc->next = 0;

		if (emac_rx_active_head == 0) {
			printf ("INFO: emac_rcv_pkt: active queue head = 0\n");
			emac_rx_active_head = curr_desc;
			emac_rx_active_tail = curr_desc;
			if (emac_rx_queue_active != 0) {
				adap_emac->RX0HDP =
					(unsigned int) emac_rx_active_head;
				printf ("INFO: emac_rcv_pkt: active queue head = 0, HDP fired\n");
				emac_rx_queue_active = 1;
			}
		} else {
			tail_desc = emac_rx_active_tail;
			emac_rx_active_tail = curr_desc;
			tail_desc->next = curr_desc;
			status = tail_desc->pkt_flag_len;
			if (status & EMAC_CPPI_EOQ_BIT) {
				adap_emac->RX0HDP = (unsigned int) curr_desc;
				status &= ~EMAC_CPPI_EOQ_BIT;
				tail_desc->pkt_flag_len = status;
			}
		}
	}
	return ret;
}

#endif /* CONFIG_CMD_NET */

#endif /* CONFIG_DRIVER_TI_EMAC */
