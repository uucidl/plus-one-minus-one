/* a10 303
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/architectures/sdl/events.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 303 */



#ifndef KNOS_DEMOS_SYSTEM_ARCHITECTURES_SDL_EVENTS_H
#define KNOS_DEMOS_SYSTEM_ARCHITECTURES_SDL_EVENTS_H

#include <system/event_listener.h>
#include <SDL.h>

typedef struct sdl_event_listener_t
{
    event_listener_t super;

    void (*accept)(struct sdl_event_listener_t* self,
		   const SDL_Event* event);

    /*
      homothetic correction to the mouse coordinates reported by the motion event.
    */
    void (*set_mouse_motion_correction)(struct sdl_event_listener_t* self, double ratiox, double ratioy);

    int mouse_motion_corrected_p;
    double ratiox;
    double ratioy;
} sdl_event_listener_t;

CLASS_INHERIT(sdl_event_listener, event_listener);

#endif
