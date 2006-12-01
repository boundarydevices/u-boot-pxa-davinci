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

#include <config.h>
#include <version.h>

#define BAUDRATE 115200
#include <configs/select.h>
#include <asm/BigMacro.h>
#include <asm/lcdPanels2.h>
#include <asm/arch/sysMacros.h>

/*
 * 	Memory setup
 */

.globl lowlevel_init
lowlevel_init:

    mov      r10, lr

    InitCS0_CS1	r0,sp
    InitGPIO	r0,sp
    InitIC_Clocks	r0,sp
    InitUART	r0,sp,UART_BASE,BAUDRATE
    InitUART	r0,sp,UART_BASE+0x600000,9600
    InitChangeCPUSpeed r0,r1,sp
    InitMemory	r0,sp,r1

    mov     pc, lr
