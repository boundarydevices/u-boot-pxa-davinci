/*
 * U-boot - cache.c
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

/* for now: just dummy functions to satisfy the linker */
#include <config.h>
#include <common.h>
#include <asm/blackfin.h>
#include "cache.h"

void flush_cache(unsigned long dummy1, unsigned long dummy2)
{
	if ((dummy1 >= L1_ISRAM) && (dummy1 < L1_ISRAM_END))
		return;
	if ((dummy1 >= DATA_BANKA_SRAM) && (dummy1 < DATA_BANKA_SRAM_END))
		return;
	if ((dummy1 >= DATA_BANKB_SRAM) && (dummy1 < DATA_BANKB_SRAM_END))
		return;

	if (icache_status())
		blackfin_icache_flush_range((void*)dummy1, (void*)(dummy1 + dummy2));
	if (dcache_status())
		blackfin_dcache_flush_range((void*)dummy1, (void*)(dummy1 + dummy2));

	return;
}
