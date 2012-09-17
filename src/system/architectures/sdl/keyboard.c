/* a10 740
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/architectures/sdl/keyboard.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 740 */




#include "keyboard.h"

#include <library/map.h>
#include <scripting/dictionary.h>

static map_t keyMap;

static int init_p = 1;

static void define (const char* name, SDLKey key) {
    dictionary_t* dictionary = dictionary_get_instance ();
    keyMap.put (&keyMap, key, (void*) dictionary->new_atom (dictionary, name));
}

static void initialize_keyMap () {
    map_instantiate_toplevel (&keyMap);
    
    define ("backspace", SDLK_BACKSPACE);
    define ("tab", SDLK_TAB);
    //define ("linefeed", );
    define ("clear", SDLK_CLEAR);
    define ("return", SDLK_RETURN);
    //define ("pause", );
    define ("scroll-lock", SDLK_SCROLLOCK);
    define ("sys-req", SDLK_SYSREQ);
    define ("escape", SDLK_ESCAPE);
    define ("delete", SDLK_DELETE);
    
    define ("f1", SDLK_F1);
    define ("f2", SDLK_F2);
    define ("f3", SDLK_F3);
    define ("f4", SDLK_F4);
    define ("f5", SDLK_F5);
    define ("f6", SDLK_F6);
    define ("f7", SDLK_F7);
    define ("f8", SDLK_F8);
    define ("f9", SDLK_F9);
    define ("f10", SDLK_F10);
    define ("f11", SDLK_F11);
    define ("f12", SDLK_F12);

    define ("left-shift", SDLK_LSHIFT);
    define ("right-shift", SDLK_RSHIFT);
    define ("left-control", SDLK_LCTRL);
    define ("right-control", SDLK_RCTRL);
    define ("left-meta", SDLK_LMETA);
    define ("right-meta", SDLK_RMETA);
    define ("left-alt", SDLK_LALT);
    define ("right-alt", SDLK_RALT);

    define ("home", SDLK_HOME);
    define ("left", SDLK_LEFT);
    define ("up", SDLK_UP);
    define ("right", SDLK_RIGHT);
    define ("down", SDLK_DOWN);
    //    define ("previous", XK_Prior);
    define ("page-up", SDLK_PAGEUP);
    //    define ("next", XK_Next);
    define ("page-down", SDLK_PAGEDOWN);
    define ("end", SDLK_END);
    //    define ("begin", XK_Begin);
}

static atom_t char_atom;

void sdl_keyboard_initialize () {
    if (init_p) {
	SDL_EnableUNICODE (1);
	initialize_keyMap ();
	char_atom = dictionary_get_instance () -> new_atom 
	    (dictionary_get_instance (), "char");
							    
	init_p = 0;
    }
}

atom_t sdl_keysym_to_symbol (SDL_keysym keysym) {
    map_value_t value = keyMap.get (&keyMap, keysym.sym);
    if (map_value_is_there (value)) {
	return (atom_t) map_value_obtain (value);
    } else {
	return char_atom;
    }
}

int sdl_keysym_to_ascii (SDL_keysym keysym) {
    return keysym.unicode & 0xff;;
}

