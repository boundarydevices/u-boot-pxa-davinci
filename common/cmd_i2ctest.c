/*
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
#ifdef CMD_I2CTEST
#include <asm/arch/hardware.h>
#include <command.h>

#define I2C_BASE 0x40301680
/*
#define ICR_UR		(1<<14)		//Unit Reset
#define ICR_GCD		(1<<7)		//General call Disable
#define ICR_IUE		(1<<6)		//unit enable
#define ICR_TB		(1<<3)		//transfer byte

#define ISR_BED		(1<<10)		//Bus Error Detected
#define ISR_SAD		(1<<9)		//Slave Address Detected
#define ISR_GCAD	(1<<8)		//General Call Address Detected
#define ISR_IRF		(1<<7)		//Receive Full
#define ISR_ITE		(1<<6)		//Transmit Empty
#define ISR_ALD		(1<<5)		//Arbitration Loss Detected
#define ISR_SSD		(1<<4)		//Slave Stop Detected
#define ISR_IBB		(1<<3)		//I2C Bus Busy
#define ISR_UB		(1<<2)		//Unit Busy
#define ISR_ACKNAK	(1<<1)
#define ISR_RWM		(1<<0)		//Read/Write Mode (1: master read/ slave transmit)
*/
#define I2C_ICR_INIT	(ICR_BEIE | ICR_IRFIE | ICR_ITEIE | ICR_GCD | ICR_SCLE)
#define I2C_ISR_INIT		0x7FF

struct I2CStruct
{
	unsigned long volatile ibmr;	//80
	unsigned long volatile resv1;
	unsigned long volatile idbr;	//88
	unsigned long volatile resv2;
	unsigned long volatile icr;		//90
	unsigned long volatile resv3;
	unsigned long volatile isr;		//98
	unsigned long volatile resv4;
	unsigned long volatile isar;	//a0
};
#define GMU_SLAVE_ADDR 0x50
#define GPIO_I2C_POWER_STATUS 1		//input: 1 means I2C bus has power now
#define GPIO_I2C_POWER_ENABLE 4		//output: 1 means we provide 12v to I2c bus
#define I2C_ACTIVE_LEVEL	 1		// high means I2C power is on
///////////////////////////////
#define GP_BITMASK(gp)  (1 << ((gp) & 0x1f) )
#define GAFR_BITMASK(val,gp)  (val << (((gp) & 0x0f) << 1) )

#define GPLR_ADDR(gp) (&GPLR0)[gp>>5]
#define GPSR_ADDR(gp) (&GPSR0)[gp>>5]
#define GPCR_ADDR(gp) (&GPCR0)[gp>>5]
#define	INT_LEVEL(name)		((GPIO_##name >= 0) ? ( GPLR_ADDR(GPIO_##name) & GP_BITMASK(GPIO_##name) ) : 0)
#define	INT_LEVEL_N(name)	((GPIO_##name >= 0) ? ( (~GPLR_ADDR(GPIO_##name)) & GP_BITMASK(GPIO_##name) ) : 0)

#define IS_ACTIVE(activeLevel,gpio) ((activeLevel) ? INT_LEVEL(gpio) : INT_LEVEL_N(gpio))

#define GPIO_SET(val,gp)   if (val) GPSR_ADDR(gp) = GP_BITMASK(gp); else GPCR_ADDR(gp) = GP_BITMASK(gp)
#define SET_GPIO(val,name) if (GPIO_##name >= 0) {GPIO_SET(val,GPIO_##name);}

#define MAX_BUFFER 64

int lcd_ClearScreen(void);
void lcd_putc(const char c);

void lcd_Lecho(unsigned const char * p)
{
	unsigned char c;
	printf(p);
	do {
		c = *p++;
		if (c==0) break;
		lcd_putc((const char)c);
	} while (1);
}

void lcd_printHexByte(unsigned char c)
{
	unsigned char c1 = (c>>4)&0xf;
	unsigned char c2 = c&0xf;
	if (c1 >= 10) c1 += 'a' - '0' - 10;
	if (c2 >= 10) c2 += 'a' - '0' - 10;
	printf("%02x",c);
	lcd_putc(c1+'0');
	lcd_putc(c2+'0');
}
void lcd_printInt(unsigned int val)
{
	unsigned char buf[16];
	unsigned int rem;
	unsigned char* p = &buf[15];
	*p= 0;
	do {
		rem = val % 10;
		val = val/10;
		p--; *p = rem + '0';
	} while (val);
	lcd_Lecho(p);
}
void resetI2c(struct I2CStruct* p,int slaveAddr)
{
	p->icr &= ~ICR_IUE;		/* disable unit */
	p->icr |= ICR_UR;			/* reset the unit */
	udelay(100);
	p->icr &= ~ICR_IUE;		/* disable unit */
#ifdef CONFIG_CPU_MONAHANS
	CKENB |= (CKENB_4_I2C); /*  | CKENB_1_PWM1 | CKENB_0_PWM0); */
#else /* CONFIG_CPU_MONAHANS */
	CKEN |= CKEN14_I2C;		/* set the global I2C clock on */
#endif
	p->isar = slaveAddr;	/* set our slave address */
	p->icr = I2C_ICR_INIT;		/* set control register values */
	p->isr = I2C_ISR_INIT;		/* set clear interrupt bits */
	p->icr |= ICR_IUE;			/* enable unit */
	udelay(100);

//	p->icr = ICR_UR;	//reset
//	p->isr = ISR_BED|ISR_SAD|ISR_GCAD|ISR_IRF|ISR_ITE|ISR_ALD|ISR_SSD;	//clear the write 1 to clear bits
//	p->icr = 0;			//reset finished
//	p->isar = slaveAddr;	//low bit is r/w
//	p->icr = ICR_GCD|ICR_IUE;	//Don't respond to general call, enable unit
}

int i2ctest_bin (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int powerOn = IS_ACTIVE(I2C_ACTIVE_LEVEL,I2C_POWER_STATUS); 
	int rcode = 0;
	int isr;
	struct I2CStruct* p = (struct I2CStruct*)I2C_BASE;
	unsigned char buffer[MAX_BUFFER];
	int index;
	int i;
	int slaveAddr = GMU_SLAVE_ADDR>>1;
	unsigned int sclToggleCnt=0;
	unsigned int sdaToggleCnt=0;
	unsigned int lastIbmr=0;
	unsigned int ibmr=0;
	unsigned int loop=0;
	ulong startTime;
	ulong curTime;
	ulong timeOutTicks = (5* CFG_HZ);	//5 seconds timeout
	lcd_ClearScreen();
	lcd_Lecho("\n");
	lastIbmr = p->ibmr;
	if (powerOn) {
		rcode = 1;
		lcd_Lecho("Error, I2C power is already ON, trying to turn off\n");
	} else {
		lcd_Lecho("OK, I2C power defaulted to off\n");
	}
	SET_GPIO(0,I2C_POWER_ENABLE)	//try to remove power
	for (i=0; i<600; i++) { //delay up to 60 seconds
		udelay(100000);	//.1 second
		powerOn = IS_ACTIVE(I2C_ACTIVE_LEVEL,I2C_POWER_STATUS); 
		if (!powerOn) {
			lcd_Lecho(" power off detected, delaying to make sure\n");
			for (i=0; i<50; i++) { //delay 5 seconds
				udelay(100000);	//.1 second
			}
			powerOn = IS_ACTIVE(I2C_ACTIVE_LEVEL,I2C_POWER_STATUS); 
			if (powerOn) {
				rcode = 1;
				lcd_Lecho("Error, could not turn power off\n");
			}
			break;
		}
	}
	if (!powerOn) {
		int i=0;
		SET_GPIO(1,I2C_POWER_ENABLE)	//we provide 12 volts
		do {
			udelay(1000);	//delay 1ms
			powerOn = IS_ACTIVE(I2C_ACTIVE_LEVEL,I2C_POWER_STATUS); 
			if (powerOn) break;
			i++;
		} while (i<10);
		if (powerOn) {
			lcd_Lecho("OK, I2C power enabled\n");
		} else {
			rcode = 1;
			lcd_Lecho("Error, I2C power NOT on!!!\n");
		}
	}
	
	resetI2c(p,slaveAddr);
	lcd_Lecho("Waiting for message targeted to GMU\n");
	startTime = get_timer(0);
	do {
		isr = p->isr;
		if (isr & ISR_SAD) {
			//Slave address detected
			p->isr = isr & ~0xf;	//clear the write 1 to clear bits
			if ((isr & ISR_GCAD)==0) break;	//break if not general call
		}
		loop++;
		if (loop>=100) {
			loop =0;
			curTime = get_timer(0);
			if ( (curTime-startTime) > timeOutTicks) {
				rcode = 1;
				lcd_Lecho("Error, message timeout scl toggles:");
				lcd_printInt(sclToggleCnt);
				lcd_Lecho(" sda toggles:");
				lcd_printInt(sdaToggleCnt);
				lcd_Lecho(" sa:");
				lcd_printHexByte(slaveAddr<<1);
				lcd_Lecho("\n");
				if (1) {
					goto exit1;
				} else {
					slaveAddr++;
					if (slaveAddr>=0x80) slaveAddr=1;
					startTime = curTime;
					resetI2c(p,slaveAddr);
					sclToggleCnt = 0;
					sdaToggleCnt = 0;
				}
			}
		}
		ibmr = p->ibmr;
		lastIbmr = lastIbmr ^ ibmr;
		if (lastIbmr & 2) sclToggleCnt++;
		if (lastIbmr & 1) sdaToggleCnt++;
		lastIbmr = ibmr;
//		printf("isr:%x ibmr:%x sa:%x\n",isr,ibmr,slaveAddr<<1);
	} while (1);

	lcd_Lecho("OK, slave address match:");
	lcd_printHexByte(slaveAddr<<1);
	lcd_Lecho("\n");
	if (isr & 1) {
		rcode = 1;
		lcd_Lecho("Error, read transaction not expected!!!\n");
	} else {
		index = 0;
		p->icr = ICR_GCD|ICR_IUE|ICR_TB;
		do {
			isr = p->isr;
			p->isr = isr & ~0xf;	//clear the write 1 to clear bits
			if (isr & ISR_IRF) {
				buffer[index++] = (unsigned char)p->idbr;
				if (index>=MAX_BUFFER) break;
				p->icr = ICR_GCD|ICR_IUE|ICR_TB;
			}
			if (isr & ISR_SSD) break;
			curTime = get_timer(startTime);
			if (curTime > timeOutTicks) {
				rcode = 1;
				lcd_Lecho("Error, stop timeout\n");
				break;
			}
		} while (1);
		i = 0;
		lcd_Lecho("OK, Message bytes:");
		while (i<index) {
			lcd_printHexByte(buffer[i]);
			lcd_Lecho(" ");
			i++;
		}
		lcd_Lecho("\n");
	}

exit1:
	if (rcode) {
		SET_GPIO(0,I2C_POWER_ENABLE)	//try to remove power if failed test
		lcd_Lecho("Removing power\n");
		lcd_Lecho("Failed\n");
	} else {
		lcd_Lecho("Passed\n");
		for (i=0; i<50; i++) udelay(100000);	//delay 5 seconds
		SET_GPIO(0,I2C_POWER_ENABLE)	//try to remove power
	}
	return rcode;
}

U_BOOT_CMD(
	i2ctest, 1, 0,	i2ctest_bin,
	"i2ctest - test power & receive\n",
	"\n"
);

#endif
