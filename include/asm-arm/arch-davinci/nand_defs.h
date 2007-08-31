/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * Parts shamelesly stolen from Linux Kernel source tree.
 *
 * ------------------------------------------------------------
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
#ifndef _NAND_DEFS_H_
#define _NAND_DEFS_H_

#include <asm/arch/hardware.h>

#define	MASK_CLE	0x10
#define	MASK_ALE	0x08

typedef struct  {
	u_int32_t	NRCSR;
	u_int32_t	AWCCR;
	u_int8_t	RSVD0[8];
	u_int32_t	AB1CR;
	u_int32_t	AB2CR;
	u_int32_t	AB3CR;
	u_int32_t	AB4CR;
	u_int8_t	RSVD1[32];
	u_int32_t	NIRR;
	u_int32_t	NIMR;
	u_int32_t	NIMSR;
	u_int32_t	NIMCR;
	u_int8_t	RSVD2[16];
	u_int32_t	NANDFCR;
	u_int32_t	NANDFSR;
	u_int8_t	RSVD3[8];
	u_int32_t	NANDF_ECC[4];
	u_int8_t	RSVD4[4];
	u_int32_t	IODFTECR;
	u_int32_t	IODFTGCR;
	u_int8_t	RSVD5[4];
	u_int32_t	IODFTMRLR;
	u_int32_t	IODFTMRMR;
	u_int32_t	IODFTMRMSBR;
	u_int8_t	RSVD6[20];
	u_int32_t	MODRNR;
	u_int8_t	RSVD7[76];
	u_int32_t	CE0DATA;
	u_int32_t	CE0ALE;
	u_int32_t	CE0CLE;
	u_int8_t	RSVD8[4];
	u_int32_t	CE1DATA;
	u_int32_t	CE1ALE;
	u_int32_t	CE1CLE;
	u_int8_t	RSVD9[4];
	u_int32_t	CE2DATA;
	u_int32_t	CE2ALE;
	u_int32_t	CE2CLE;
	u_int8_t	RSVD10[4];
	u_int32_t	CE3DATA;
	u_int32_t	CE3ALE;
	u_int32_t	CE3CLE;
} nand_registers;

typedef volatile nand_registers	*nandregs;

#define NAND_READ_START		0x00
#define NAND_READ_END		0x30
#define NAND_STATUS		0x70

#endif
