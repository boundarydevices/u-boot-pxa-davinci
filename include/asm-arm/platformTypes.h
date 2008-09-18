#ifndef __PLATFORMTYPES_H__
#define __PLATFORMTYPES_H__ 1
#define NEON				1
#define NEONB				2
#define BD2003				3
#define GAME_WITH_SMC			4
#define GAME_CONTROLLER			5
#define GAME_CONTROLLER_PLAITED_A1	6
#define BOUNDARY_OLD_BOARD		7  //lcd pin reordering for rgb problem, don't use VLIO(gp18 is turnstile)
#define OLD_GAME_CONTROLLER		8
#define H1				9
#define HALOGEN				10
#define ARGON				11
#define NEON270				12
#define HYDROGEN			13
#define MICROAVL			14
#define OXYGEN				15
#define MERCURY				16
#define DAVINCI				17

#if (PLATFORM_TYPE==NEON)
#include "plat_NEON.h"
#elif (PLATFORM_TYPE==NEONB)
#include "plat_NEONB.h"
#elif (PLATFORM_TYPE==BD2003)
#include "plat_BD2003.h"
#elif (PLATFORM_TYPE==GAME_WITH_SMC)
#include "plat_GAME_WITH_SMC.h"
#elif (PLATFORM_TYPE==GAME_CONTROLLER)
#include "plat_GAME_CONTROLLER.h"
#elif (PLATFORM_TYPE==GAME_CONTROLLER_PLAITED_A1)
#include "plat_GAME_CONTROLLER_PLAITED_A1.h"
#elif (PLATFORM_TYPE==BOUNDARY_OLD_BOARD)
#include "plat_BOUNDARY_OLD_BOARD.h"
#elif (PLATFORM_TYPE==OLD_GAME_CONTROLLER)
#include "plat_OLD_GAME_CONTROLLER.h"
#elif (PLATFORM_TYPE==H1)
#include "plat_H1.h"
#elif (PLATFORM_TYPE==HALOGEN)
#include "plat_HALOGEN.h"
#elif (PLATFORM_TYPE==ARGON)
#include "plat_ARGON.h"
#elif (PLATFORM_TYPE==NEON270)
#include "plat_NEON270.h"
#elif (PLATFORM_TYPE==HYDROGEN)
#include "plat_HYDROGEN.h"
#elif (PLATFORM_TYPE==MICROAVL)
#include "plat_MICROAVL.h"
#elif (PLATFORM_TYPE==OXYGEN)
#include "plat_MICROAVL.h"
#elif (PLATFORM_TYPE==MERCURY)
#include "plat_MERCURY.h"
#elif (PLATFORM_TYPE==DAVINCI)
#include "plat_DAVINCI.h"
#endif

#endif
