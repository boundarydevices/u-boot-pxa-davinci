/*
 * TI DaVinci (TMS320DM644x) I2C driver.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * --------------------------------------------------------
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

#include <common.h>

#ifdef CONFIG_DRIVER_DAVINCI_I2C

#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>

#define MAC_VARIABLE "ethaddr"

#define GP_BANK0_OFFSET	0x10
#define GP_BANK_LENGTH	0x28
#define GP_DIR 0x00
#define GP_OUT 0x04
#define GP_SET 0x08
#define GP_CLR 0x0C
#define GP_IN  0x10
#define GPIO_SCL 43
#define GPIO_SDA 44

static void gpio_set_val_dir(unsigned int gp, int val, int output)
{
	unsigned int mask = (1 << (gp & 0x1f));
	unsigned int bank = (gp >> 5);
	int base = DAVINCI_GPIO_BASE+GP_BANK0_OFFSET+(bank * GP_BANK_LENGTH);
	int set_clear = base + (val ? GP_SET : GP_CLR);
	unsigned int dir = REG(base + GP_DIR);
	if (output) {
		dir &= ~mask;
		REG(set_clear) = mask;
		REG(base + GP_DIR) = dir;
	} else {
		dir |= mask;
		REG(base + GP_DIR) = dir;
		REG(set_clear) = mask;
	}
}

static int gpio_get_val(unsigned int gp)
{
	unsigned int bank = (gp >> 5);
	int base = DAVINCI_GPIO_BASE+GP_BANK0_OFFSET+(bank * GP_BANK_LENGTH);
	return (REG(base + GP_IN) >> (gp & 0x1f)) & 1;
}
/*
 * return 0 if already free, 1 if freed, -1 if failed to free
 */
static int DoGpioFreeBus(void)
{
	int retval = 0;
	gpio_set_val_dir(GPIO_SCL, 1, 0);	/* high, input */
	gpio_set_val_dir(GPIO_SDA, 1, 0);	/* high, input */

	/* Disable I2C, make gpios again */
	REG(PINMUX1) &= ~(1 << 7);

	if (gpio_get_val(GPIO_SDA) == 0) {
		int i = 0;
		/* SDA is low, toggling clock */
		retval = 1;
		do {
			gpio_set_val_dir(GPIO_SCL, 0, 1);	/* low, output */
			udelay(1000);
			gpio_set_val_dir(GPIO_SCL, 1, 0);	/* high, input */
			udelay(1000);
			if (gpio_get_val(GPIO_SDA))
				break;
			i++;
			if (i>=16) {
				printf("Error could not make SDA high\r\n");
				retval = -1;
				break;
			}
		} while (1);
	}
	/* Enable I2C pin Mux */
	REG(PINMUX1) |= (1 << 7);
	return retval;
}

static int wait_for_bus(void)
{
	int	stat, timeout;

	REG(I2C_STAT) = 0xffff;

	for (timeout = 0; timeout < 20; timeout++) {
		stat = REG(I2C_STAT);
		if (!(stat & I2C_STAT_BB)) {
			REG(I2C_STAT) = 0xffff;
			return(0);
		}

		REG(I2C_STAT) = stat;
		udelay(50000);
	}

	REG(I2C_STAT) = 0xffff;
	printf("%s: timeout %x, trying bus reset\n", __func__, stat);
	if (stat & I2C_STAT_BB) {
		REG(I2C_CON) = 0;
		udelay(5000);
		printf("toggling enable to hopefully clear busy\r\n");
		REG(I2C_CON) = I2C_CON_EN;
	}
	if (REG(I2C_STAT) & I2C_STAT_BB) {
		printf("resetting i2c\r\n");
		i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
		if (DoGpioFreeBus() == 1) {
			printf("%s: toggling scl freed bus\n", __func__);
		}
	}
	return(1);
}

#define I2C_STAT_NACK_AL (I2C_STAT_NACK | I2C_STAT_AL)

static int poll_i2c_irq(int mask)
{
	int	stat, timeout;
	int	sum_stat = 0;
	int	stop = 0;
	int	clear_mask;
	mask |= I2C_STAT_NACK_AL;
	clear_mask = mask & ~(I2C_STAT_RRDY | I2C_STAT_XRDY);
	for (timeout = 0; timeout < 200; timeout++) {
		udelay(1000);
		stat = REG(I2C_STAT);
		sum_stat |= stat;
		if (stat & mask) {
			if (stat & clear_mask)
				REG(I2C_STAT) = stat & clear_mask;
			if (stat & I2C_STAT_NACK_AL) {
				stop = 1;
				mask |= I2C_STAT_ARDY;
				if (mask & I2C_STAT_XRDY)
					REG(I2C_CON) |= I2C_CON_STP | (1 << 7); /* repeat mode */
				if (mask & I2C_STAT_RRDY)
					REG(I2C_CON) |= I2C_CON_STP | (1 << 7) | (1 << 15); /* NACK */
			}
			if (stop) {
				if (stat & I2C_STAT_RRDY){
					/* Throw away data */
					REG(I2C_DRR);
				}
				if (!(stat & (I2C_STAT_ARDY | I2C_STAT_SCD)))
					continue;
				REG(I2C_CON) |= I2C_CON_STP;
				printf("%s: nack %x %x\n", __func__, stat, sum_stat);
				return -1;
			}
			return sum_stat;
		}
	}

	REG(I2C_STAT) = 0xffff;
	if (stop) {
		REG(I2C_CON) |= I2C_CON_STP;
		printf("%s: stop timeout %x %x\n", __func__, stat, sum_stat);
	} else {
		printf("%s: timeout, trying i2c reset %x %x\n", __func__, stat, sum_stat);
		i2c_init(CFG_I2C_SPEED, CFG_I2C_SLAVE);
		if (DoGpioFreeBus() == 1) {
			printf("%s: toggling scl freed bus\n", __func__);
		}
	}
	return -1;
}


void flush_rx(void)
{
	int	dummy;

	while (1) {
		if (!(REG(I2C_STAT) & I2C_STAT_RRDY))
			break;

		dummy = REG(I2C_DRR);
		REG(I2C_STAT) = I2C_STAT_RRDY;
		udelay(1000);
	}
}


void i2c_init(int speed, int slaveadd)
{
	u_int32_t	div, psc;

	REG(I2C_CON) = 0;
	udelay (50000);

	psc = 2;
	div = (CFG_PERIPHERAL_CLOCK / ((psc + 1) * speed)) - 10;	/* SCLL + SCLH */
	REG(I2C_PSC) = psc;			/* 27MHz / (2 + 1) = 9MHz */
	REG(I2C_SCLL) = (div * 50) / 100;	/* 50% Duty */
	REG(I2C_SCLH) = div - REG(I2C_SCLL);

	REG(I2C_OA) = slaveadd;
	REG(I2C_CNT) = 0;

	/* Interrupts must be enabled or I2C module won't work */
	REG(I2C_IE) = I2C_IE_SCD_IE | I2C_IE_XRDY_IE |
		I2C_IE_RRDY_IE | I2C_IE_ARDY_IE | I2C_IE_NACK_IE;

	/* Now enable I2C controller (get it out of reset) */
	REG(I2C_CON) = I2C_CON_EN;

	udelay(1000);
}


int i2c_probe(u_int8_t chip)
{
	int	rc = 1;

	if (chip == REG(I2C_OA)) {
		return(rc);
	}

	REG(I2C_CON) = 0;
	if (wait_for_bus()) {return(1);}

	/* try to read one byte from current (or only) address */
	REG(I2C_CNT) = 1;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = (I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP);
	udelay (50000);

	if (!(REG(I2C_STAT) & I2C_STAT_NACK)) {
		rc = 0;
		flush_rx();
		REG(I2C_STAT) = 0xffff;
	} else {
		REG(I2C_STAT) = 0xffff;
		REG(I2C_CON) |= I2C_CON_STP;
		udelay(20000);
		if (wait_for_bus()) {return(1);}
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	return(rc);
}


int i2c_read(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	int tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __FUNCTION__, alen);
		return(1);
	}

	if (wait_for_bus()) {return(1);}

	if (alen != 0) {
		/* Start address phase */
		tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX;
		REG(I2C_CNT) = alen;
		REG(I2C_SA) = chip;
		REG(I2C_CON) = tmp;

		tmp = poll_i2c_irq(I2C_STAT_XRDY);
		if (tmp < 0) return 1;


		switch (alen) {
			case 2:
				/* Send address MSByte */
				if (tmp & I2C_STAT_XRDY) {
					REG(I2C_DXR) = (addr >> 8) & 0xff;
				} else {
					REG(I2C_CON) = 0;
					return(1);
				}

				tmp = poll_i2c_irq(I2C_STAT_XRDY);
				if (tmp < 0) return 1;
				/* No break, fall through */
			case 1:
				/* Send address LSByte */
				if (tmp & I2C_STAT_XRDY) {
					REG(I2C_DXR) = addr & 0xff;
				} else {
					REG(I2C_CON) = 0;
					return(1);
				}

				tmp = poll_i2c_irq(I2C_STAT_XRDY | I2C_STAT_ARDY);
				if (tmp < 0) return 1;
		}
	}

	/* Address phase is over, now read 'len' bytes and stop */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_STP;
	REG(I2C_CNT) = len & 0xffff;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = tmp;

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(I2C_STAT_RRDY | I2C_STAT_ROVR);
		if (tmp < 0) {
			printf("%s: read %i bytes a\n", __func__, i);
			return 1;
		}

		if (tmp & I2C_STAT_RRDY) {
			buf[i] = REG(I2C_DRR);
		} else {
			printf("%s: read %i bytes b\n", __func__, i);
			REG(I2C_CON) = 0;
			return(1);
		}
	}

	tmp = poll_i2c_irq(I2C_STAT_SCD);
	if (tmp < 0) {
		printf("%s: read %i bytes c\n", __func__, i);
		return 1;
	}

	if (!(tmp & I2C_STAT_SCD)) {
		printf("%s: read %i bytes d\n", __func__, i);
		REG(I2C_CON) = 0;
		return(1);
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	REG(I2C_CON) = 0;

	return(0);
}


int i2c_write(u_int8_t chip, u_int32_t addr, int alen, u_int8_t *buf, int len)
{
	u_int32_t	tmp;
	int		i;

	if ((alen < 0) || (alen > 2)) {
		printf("%s(): bogus address length %x\n", __FUNCTION__, alen);
		return(1);
	}
	if (len < 0) {
		printf("%s(): bogus length %x\n", __FUNCTION__, len);
		return(1);
	}

	if (wait_for_bus()) {return(1);}

	/* Start address phase */
	tmp = I2C_CON_EN | I2C_CON_MST | I2C_CON_STT | I2C_CON_TRX | I2C_CON_STP;
	REG(I2C_CNT) = (alen == 0) ? len & 0xffff : (len & 0xffff) + alen;
	REG(I2C_SA) = chip;
	REG(I2C_CON) = tmp;

	switch (alen) {
		case 2:
			/* Send address MSByte */
			tmp = poll_i2c_irq(I2C_STAT_XRDY);
			if (tmp < 0) return 1;

			if (tmp & I2C_STAT_XRDY) {
				REG(I2C_DXR) = (addr >> 8) & 0xff;
			} else {
				REG(I2C_CON) = 0;
				return(1);
			}
			/* No break, fall through */
		case 1:
			/* Send address LSByte */
			tmp = poll_i2c_irq(I2C_STAT_XRDY);
			if (tmp < 0) return 1;

			if (tmp & I2C_STAT_XRDY) {
				REG(I2C_DXR) = addr & 0xff;
			} else {
				REG(I2C_CON) = 0;
				return(1);
			}
	}

	for (i = 0; i < len; i++) {
		tmp = poll_i2c_irq(I2C_STAT_XRDY);
		if (tmp < 0) return 1;

		if (tmp & I2C_STAT_XRDY) {
			REG(I2C_DXR) = buf[i];
		} else {
			return(1);
		}
	}

	tmp = poll_i2c_irq(I2C_STAT_SCD);
	if (tmp < 0) return 1;

	if (!(tmp & I2C_STAT_SCD)) {
		REG(I2C_CON) = 0;
		return(1);
	}

	flush_rx();
	REG(I2C_STAT) = 0xffff;
	REG(I2C_CNT) = 0;
	REG(I2C_CON) = 0;

	return(0);
}


u_int8_t i2c_reg_read(u_int8_t chip, u_int8_t reg)
{
	u_int8_t	tmp;

	i2c_read(chip, reg, 1, &tmp, 1);
	return(tmp);
}


void i2c_reg_write(u_int8_t chip, u_int8_t reg, u_int8_t val)
{
	u_int8_t	tmp;

	i2c_write(chip, reg, 1, &tmp, 1);
}

#endif /* CONFIG_DRIVER_DAVINCI_I2C */
