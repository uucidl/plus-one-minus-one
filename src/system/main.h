/* a10 500
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/main.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 500 */

#ifndef MAIN_H
#define MAIN_H

#ifdef MACOSX
// for redefinition of main
#include <SDL.h>
#endif

#include "demo.h"
#include "pan.h" // for sample type

/*
  the main loop, pulling frames from the
  effects defined in the demo object
*/
int main_loop(demo_t *d);

#endif
