/*
 * (C) Copyright 2003
 * Texas Instruments <www.ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002-2004
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * (C) Copyright 2004
 * Philippe Robin, ARM Ltd. <philippe.robin@arm.com>
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <arm926ejs.h>

typedef volatile struct {
	u_int32_t	pid12;
	u_int32_t	emumgt_clksped;
	u_int32_t	gpint_en;
	u_int32_t	gpdir_dat;
	u_int32_t	tim12;
	u_int32_t	tim34;
	u_int32_t	prd12;
	u_int32_t	prd34;
	u_int32_t	tcr;
	u_int32_t	tgcr;
	u_int32_t	wdtcr;
	u_int32_t	tlgc;
	u_int32_t	tlmr;
} davinci_timer;

davinci_timer * const timer = (davinci_timer *)CFG_TIMERBASE;

#define PRESCALE 16
int timer_init(void)
{
	/* We are using timer34 in unchained 32-bit mode, full speed */
	timer->tcr = 0x0;
	timer->tgcr = 0x0;
	timer->tgcr = 0x06 | ((PRESCALE-1)<<8);	/* prescale divide by 16 */
	timer->tim34 = 0x0;
	timer->prd34 = 0xffffffff;
	timer->tcr = 2 << 22;
	return(0);
}

ulong get_timer(ulong base)
{
	return(timer->tim34 - base);
}

void set_timer(ulong t)
{
	/* nop */
}


void reset_timer(void)
{
	timer->tcr = 0x0;
	timer->tim34 = 0;
	timer->tcr = 2 << 22;
}

void udelay(unsigned long usec)
{
	ulong tmo;
	ulong endtime;
	signed long diff;

	if (usec >= 1000) {
		tmo = usec / 1000;
		tmo *= CFG_HZ;
		tmo /= 1000;
	} else {
		tmo = usec * CFG_HZ;
		tmo /= (1000*1000);
	}

	endtime = timer->tim34 + tmo;

	do {
		ulong now = timer->tim34;
		diff = endtime - now;
	} while (diff >= 0);
}

/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return timer->tim34;
}

/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return CFG_HZ;
}
