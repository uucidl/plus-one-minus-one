/* a10 726
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/main.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 726 */

#include "main.h"

#if defined(LINUX) || defined(MACOSX)

extern int default_main_loop(demo_t* demo);
#  define os_main_loop default_main_loop

#elif defined(WIN32)

extern int win32_main_loop(demo_t* demo);
#  define os_main_loop win32_main_loop

#else

#error Unknown architecture!

#endif

int main_loop(demo_t* demo)
{
	return os_main_loop(demo);
}
