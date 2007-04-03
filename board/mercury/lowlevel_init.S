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

#define BYTES_PER_PIXEL 1	//1, 2, or 3 for mercury
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
	InitCS0_CS1	r0,sp
	InitGPIO	r0,sp
	InitIC_Clocks	r0,sp,CPU_CLOCK
	InitUARTs	r0,sp,BAUDRATE,BAUDRATE,9600
	InitChangeCPUSpeed r0,r1,sp
	InitMemory	r0,sp,r1
    mov     pc, lr

.globl _lcd_ctrl_init
//void _lcd_ctrl_init	(void *lcdbase)
_lcd_ctrl_init:
	stmdb	sp!,{r4,lr}
	SetupLcd	r0,r1,r2,r3,r4,lr
	FinalInitChangeCPUSpeed r0,r1,r2	//don't change without dma active???
	ldmia	sp!,{r4,pc}

