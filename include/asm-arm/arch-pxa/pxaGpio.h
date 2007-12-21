#include "../platformTypes.h"

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

	.ifdef __ARMASM
.macro	SPEC_GP gp_,dir,level,alt
SPEC_\gp_	EQU	\dir+(\level<<8)+(\alt<<16)
.endm
	.else
.macro	SPEC_GP gp_,dir,level,alt
	.set	SPEC_\gp_,\dir+(\level<<8)+(\alt<<16)
.endm
	.endif
// *****************************************************************************************
.macro	CREATE_MASK_DIR name,prefix,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,p30,p31
	.ifdef __ARMASM
	LCLA	mask
	.endif
	.set	mask,((\prefix\p0&1)<<(\p0&0x1f))
	.set	mask,mask | ((\prefix\p1&1)<<(\p1&0x1f))
	.set	mask,mask | ((\prefix\p2&1)<<(\p2&0x1f))
	.set	mask,mask | ((\prefix\p3&1)<<(\p3&0x1f))
	.set	mask,mask | ((\prefix\p4&1)<<(\p4&0x1f))
	.set	mask,mask | ((\prefix\p5&1)<<(\p5&0x1f))
	.set	mask,mask | ((\prefix\p6&1)<<(\p6&0x1f))
	.set	mask,mask | ((\prefix\p7&1)<<(\p7&0x1f))
	.set	mask,mask | ((\prefix\p8&1)<<(\p8&0x1f))
	.set	mask,mask | ((\prefix\p9&1)<<(\p9&0x1f))
	.set	mask,mask | ((\prefix\p10&1)<<(\p10&0x1f))
	.set	mask,mask | ((\prefix\p11&1)<<(\p11&0x1f))
	.set	mask,mask | ((\prefix\p12&1)<<(\p12&0x1f))
	.set	mask,mask | ((\prefix\p13&1)<<(\p13&0x1f))
	.set	mask,mask | ((\prefix\p14&1)<<(\p14&0x1f))
	.set	mask,mask | ((\prefix\p15&1)<<(\p15&0x1f))
	.set	mask,mask | ((\prefix\p16&1)<<(\p16&0x1f))
	.set	mask,mask | ((\prefix\p17&1)<<(\p17&0x1f))
	.set	mask,mask | ((\prefix\p18&1)<<(\p18&0x1f))
	.set	mask,mask | ((\prefix\p19&1)<<(\p19&0x1f))
	.set	mask,mask | ((\prefix\p20&1)<<(\p20&0x1f))
	.set	mask,mask | ((\prefix\p21&1)<<(\p21&0x1f))
	.set	mask,mask | ((\prefix\p22&1)<<(\p22&0x1f))
	.set	mask,mask | ((\prefix\p23&1)<<(\p23&0x1f))
	.set	mask,mask | ((\prefix\p24&1)<<(\p24&0x1f))
	.set	mask,mask | ((\prefix\p25&1)<<(\p25&0x1f))
	.set	mask,mask | ((\prefix\p26&1)<<(\p26&0x1f))
	.set	mask,mask | ((\prefix\p27&1)<<(\p27&0x1f))
	.set	mask,mask | ((\prefix\p28&1)<<(\p28&0x1f))
	.set	mask,mask | ((\prefix\p29&1)<<(\p29&0x1f))
	.set	mask,mask | ((\prefix\p30&1)<<(\p30&0x1f))
	.set	mask,mask | ((\prefix\p31&1)<<(\p31&0x1f))
	.ifdef __ARMASM
\name	EQU mask
	.else
	.set	\name,mask
	.endif
.endm

.macro	CREATE_MASK_LEVEL name,prefix,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22,p23,p24,p25,p26,p27,p28,p29,p30,p31
	.ifdef __ARMASM
	LCLA	mask
	.endif
	.set	mask,(((\prefix\p0>>8)&1)<<(\p0&0x1f))
	.set	mask,mask | (((\prefix\p1>>8)&1)<<(\p1&0x1f))
	.set	mask,mask | (((\prefix\p2>>8)&1)<<(\p2&0x1f))
	.set	mask,mask | (((\prefix\p3>>8)&1)<<(\p3&0x1f))
	.set	mask,mask | (((\prefix\p4>>8)&1)<<(\p4&0x1f))
	.set	mask,mask | (((\prefix\p5>>8)&1)<<(\p5&0x1f))
	.set	mask,mask | (((\prefix\p6>>8)&1)<<(\p6&0x1f))
	.set	mask,mask | (((\prefix\p7>>8)&1)<<(\p7&0x1f))
	.set	mask,mask | (((\prefix\p8>>8)&1)<<(\p8&0x1f))
	.set	mask,mask | (((\prefix\p9>>8)&1)<<(\p9&0x1f))
	.set	mask,mask | (((\prefix\p10>>8)&1)<<(\p10&0x1f))
	.set	mask,mask | (((\prefix\p11>>8)&1)<<(\p11&0x1f))
	.set	mask,mask | (((\prefix\p12>>8)&1)<<(\p12&0x1f))
	.set	mask,mask | (((\prefix\p13>>8)&1)<<(\p13&0x1f))
	.set	mask,mask | (((\prefix\p14>>8)&1)<<(\p14&0x1f))
	.set	mask,mask | (((\prefix\p15>>8)&1)<<(\p15&0x1f))
	.set	mask,mask | (((\prefix\p16>>8)&1)<<(\p16&0x1f))
	.set	mask,mask | (((\prefix\p17>>8)&1)<<(\p17&0x1f))
	.set	mask,mask | (((\prefix\p18>>8)&1)<<(\p18&0x1f))
	.set	mask,mask | (((\prefix\p19>>8)&1)<<(\p19&0x1f))
	.set	mask,mask | (((\prefix\p20>>8)&1)<<(\p20&0x1f))
	.set	mask,mask | (((\prefix\p21>>8)&1)<<(\p21&0x1f))
	.set	mask,mask | (((\prefix\p22>>8)&1)<<(\p22&0x1f))
	.set	mask,mask | (((\prefix\p23>>8)&1)<<(\p23&0x1f))
	.set	mask,mask | (((\prefix\p24>>8)&1)<<(\p24&0x1f))
	.set	mask,mask | (((\prefix\p25>>8)&1)<<(\p25&0x1f))
	.set	mask,mask | (((\prefix\p26>>8)&1)<<(\p26&0x1f))
	.set	mask,mask | (((\prefix\p27>>8)&1)<<(\p27&0x1f))
	.set	mask,mask | (((\prefix\p28>>8)&1)<<(\p28&0x1f))
	.set	mask,mask | (((\prefix\p29>>8)&1)<<(\p29&0x1f))
	.set	mask,mask | (((\prefix\p30>>8)&1)<<(\p30&0x1f))
	.set	mask,mask | (((\prefix\p31>>8)&1)<<(\p31&0x1f))
	.ifdef __ARMASM
\name	EQU mask
	.else
	.set	\name,mask
	.endif
.endm

.macro	CREATE_MASK_ALT name,prefix,p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15
	.ifdef __ARMASM
	LCLA	mask
	.endif
	.set	mask,(((\prefix\p0>>16)&3)<<((\p0&0x0f)<<1))
	.set	mask,mask | (((\prefix\p1>>16)&3)<<((\p1&0x0f)<<1))
	.set	mask,mask | (((\prefix\p2>>16)&3)<<((\p2&0x0f)<<1))
	.set	mask,mask | (((\prefix\p3>>16)&3)<<((\p3&0x0f)<<1))
	.set	mask,mask | (((\prefix\p4>>16)&3)<<((\p4&0x0f)<<1))
	.set	mask,mask | (((\prefix\p5>>16)&3)<<((\p5&0x0f)<<1))
	.set	mask,mask | (((\prefix\p6>>16)&3)<<((\p6&0x0f)<<1))
	.set	mask,mask | (((\prefix\p7>>16)&3)<<((\p7&0x0f)<<1))
	.set	mask,mask | (((\prefix\p8>>16)&3)<<((\p8&0x0f)<<1))
	.set	mask,mask | (((\prefix\p9>>16)&3)<<((\p9&0x0f)<<1))
	.set	mask,mask | (((\prefix\p10>>16)&3)<<((\p10&0x0f)<<1))
	.set	mask,mask | (((\prefix\p11>>16)&3)<<((\p11&0x0f)<<1))
	.set	mask,mask | (((\prefix\p12>>16)&3)<<((\p12&0x0f)<<1))
	.set	mask,mask | (((\prefix\p13>>16)&3)<<((\p13&0x0f)<<1))
	.set	mask,mask | (((\prefix\p14>>16)&3)<<((\p14&0x0f)<<1))
	.set	mask,mask | (((\prefix\p15>>16)&3)<<((\p15&0x0f)<<1))
	.ifdef __ARMASM
\name	EQU mask
	.else
	.set	\name,mask
	.endif
.endm

#if (PLATFORM_TYPE==NEON)||(PLATFORM_TYPE==NEONB)||(PLATFORM_TYPE==BD2003)||(PLATFORM_TYPE==BOUNDARY_OLD_BOARD)
#define ALT_LCD 2
#include "pxaGpio25x.h"
#else
#if (PLATFORM_TYPE==HALOGEN)||(PLATFORM_TYPE==ARGON)||(PLATFORM_TYPE==NEON270)||(PLATFORM_TYPE==OXYGEN)
#define ALT_LCD 2
#include "pxaGpio27x.h"
#else
#if (PLATFORM_TYPE==GAME_WITH_SMC)||(PLATFORM_TYPE==GAME_CONTROLLER)
#include "pxaGpioGame.h"
#else
error Unsupplorted PLATFORM_TYPE
#endif
#endif
#endif
