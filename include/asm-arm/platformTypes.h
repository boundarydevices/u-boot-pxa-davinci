#ifndef __PLATFORMTYPES_H__
#define __PLATFORMTYPES_H__ 1
#define NEON						1
#define NEONB						2
#define BD2003						3
#define GAME_WITH_SMC				4
#define GAME_CONTROLLER				5
#define GAME_CONTROLLER_PLAITED_A1	6
#define BOUNDARY_OLD_BOARD			7  //lcd pin reordering for rgb problem, don't use VLIO(gp18 is turnstile)
#define OLD_GAME_CONTROLLER			8
#define HALOGEN						9
#define ARGON						10
#define NEON270						11
#define MERCURY						12

#if (PLATFORM_TYPE==NEONB)
#define PHYS_FLASH_BASE 0x04000000
#else
#define PHYS_FLASH_BASE 0x0
#endif

#endif
