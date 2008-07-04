#ifndef __SM501_H__
#define __SM501_H__

/*
 * sm501.h
 *
 * Defines constants and macros for the SM-501 Graphics Controller.
 *
 */

extern unsigned long const mmioStart ;
extern unsigned long const mmioLength ;

#define SMIR_GPIO_32_63_CONTROL  0x0000c
#define SMIR_PWRM0_GATE          0x00040
#define SMIR_PWRM1_GATE          0x00048
#define SMIR_POWER_MODE_CONTROL  0x00054

#define READ_SM501_REG( addr ) *( (unsigned long volatile *)((addr)+mmioStart) )
#define STUFF_SM501_REG( addr, value ) *( (unsigned long volatile *)((addr)+mmioStart) ) = (value)

#endif
