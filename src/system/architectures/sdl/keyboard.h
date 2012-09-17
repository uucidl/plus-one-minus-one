/* a10 190
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/architectures/sdl/keyboard.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 190 */



#ifndef KNOS_DEMOS_SYSTEM_ARCHITECTURES_SDL_KEYBOARD_H
#define KNOS_DEMOS_SYSTEM_ARCHITECTURES_SDL_KEYBOARD_H

#include <SDL.h>
#include <scripting/atom.h>

/*
   call this before the rest
*/
void sdl_keyboard_initialize ();

atom_t sdl_keysym_to_symbol (SDL_keysym keysym);
int sdl_keysym_to_ascii (SDL_keysym keysym);

#endif
