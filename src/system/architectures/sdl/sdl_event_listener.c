/* a10 797
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/architectures/sdl/sdl_event_listener.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 797 */




#include "events.h"

#include <scripting/dictionary.h>
#include <libc/stdlib.h>

#include <SDL.h>
#include <system/architectures/sdl/keyboard.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(SDL_EVENT_LISTENER);

static
key_event_t* key_event_fromSDL_KeyboardEvent(key_event_t* x,
					     const SDL_KeyboardEvent* sdl_ke)
{
    key_event_t* ke = NULL;
    if(sdl_ke) {
	dictionary_t* dict = dictionary_get_instance();

	ke = key_event_instantiate_toplevel (x);
	if(sdl_ke->type == SDL_KEYDOWN) {
	    ke->hierarchy[1].adverb = dict->get_atom(dict, "atom");
	    ke->hierarchy[1].verb   = dict->get_atom(dict, "down");
	} else if(sdl_ke->type == SDL_KEYUP) {
	    ke->hierarchy[1].adverb = dict->get_atom(dict, "atom");
	    ke->hierarchy[1].verb   = dict->get_atom(dict, "up");
	}
	TRACE3("key event: %c (%x)", sdl_ke->keysym.sym & 0xff, sdl_ke->keysym.sym);

	ke->hierarchy[2].adverb = dict->get_atom(dict, "atom");
	ke->hierarchy[2].verb   = sdl_keysym_to_symbol (sdl_ke->keysym);
	ke->hierarchy[3].adverb = dict->get_atom(dict, "integer");
	ke->hierarchy[3].verb   = atom_new_integer
	    (sdl_keysym_to_ascii (sdl_ke->keysym));
    }

    return ke;
}

static
mouse_event_t* mouse_event_fromSDL_MouseButtonEvent(mouse_event_t* x,
						    const SDL_MouseButtonEvent* sdl_be)
{
    mouse_event_t* be = NULL;
    if(sdl_be) {
	dictionary_t* dict = dictionary_get_instance();

	be = mouse_button_event_instantiate_toplevel(x);
	if(sdl_be->type == SDL_MOUSEBUTTONDOWN) {
	    TRACE2("button: pressed %u", sdl_be->button);
	    be->hierarchy[2].adverb = dict->get_atom(dict, "atom");
	    be->hierarchy[2].verb   = dict->get_atom(dict, "down");
	} else if(sdl_be->type == SDL_MOUSEBUTTONUP) {
	    TRACE2("button: released %u", sdl_be->button);
	    be->hierarchy[2].adverb = dict->get_atom(dict, "atom");
	    be->hierarchy[2].verb   = dict->get_atom(dict, "up");
	}

	be->hierarchy[3].adverb = dict->get_atom(dict, "atom");
	switch(sdl_be->button) {
	case 1:
	    be->hierarchy[3].verb = dict->get_atom(dict, "lmb");
	    break;
	case 2:
	    be->hierarchy[3].verb = dict->get_atom(dict, "mmb");
	    break;
	case 3:
	    be->hierarchy[3].verb = dict->get_atom(dict, "rmb");
	    break;
	default:
	    be->hierarchy[3].verb = 0; /* #undefined */
	    break;
	}

    }

    return be;
}

static
mouse_event_t* mouse_event_fromSDL_MouseMotionEvent(mouse_event_t* x,
						    const SDL_MouseMotionEvent* sdl_me)
{
    mouse_event_t* me = NULL;
    if(sdl_me) {
	dictionary_t* dict = dictionary_get_instance();
	me = mouse_move_event_instantiate_toplevel (x);
	TRACE3("motion position: %d %d", sdl_me->x, sdl_me->y);
	me->hierarchy[2].adverb = dict->get_atom(dict, "integer");
	me->hierarchy[2].verb   = atom_new_integer(sdl_me->x);
	me->hierarchy[3].adverb = dict->get_atom(dict, "integer");
	me->hierarchy[3].verb   = atom_new_integer(sdl_me->y);
    }

    return me;
}

#ifdef WIN32
  #include <windows.h>
  #include <shellapi.h>
  #include <system/architectures/win32/events.h>
  #include <SDL_syswm.h>
#elif defined(LINUX)
  #include <SDL_syswm.h>
#endif

static
void event_listener_accept(sdl_event_listener_t* self, const SDL_Event* event)
{
    if(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
	key_event_t   key_event;
	const SDL_KeyboardEvent* sdl_ke = &event->key;

	key_event_fromSDL_KeyboardEvent(&key_event, sdl_ke);
	self->super.accept(&self->super, (event_t*) &key_event);
	event_retire (&key_event);
    } else if(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
	mouse_event_t mouse_event;
	const SDL_MouseButtonEvent* sdl_be = &event->button;

	mouse_event_fromSDL_MouseButtonEvent(&mouse_event, sdl_be);
	self->super.accept(&self->super, (event_t*) &mouse_event);
	event_retire (&mouse_event);
    } else if(event->type == SDL_MOUSEMOTION) {
	mouse_event_t mouse_event;
	const SDL_MouseMotionEvent* sdl_me = &event->motion;

	mouse_event_fromSDL_MouseMotionEvent(&mouse_event, sdl_me);
	if(self->mouse_motion_corrected_p) {
	    mouse_event.hierarchy[2].verb *= self->ratiox;
	    mouse_event.hierarchy[3].verb *= self->ratioy;
	}
	self->super.accept(&self->super, (event_t*) &mouse_event);
	event_retire (&mouse_event);
#ifdef WIN32
/* drag and drop for WIN32 */
    } else if(event->type == SDL_SYSWMEVENT &&
	      event->syswm.msg->msg == WM_DROPFILES) {
      SDL_SysWMmsg* msg = event->syswm.msg;
      HDROP hdrop = (HDROP) msg->wParam;
      unsigned int i;
      unsigned int n = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);
      TRACE2("%d files dropped.", n);
      for(i=0; i<n; i++) {
	drop_event_t drop_event;
	drop_event_fromWIN32_DROPFILES_Message(&drop_event, msg->msg, hdrop, i);
	self->super.accept(&self->super, (event_t*) &drop_event);
	if(drop_event.hierarchy[2].verb) {
	  free((void*) drop_event.hierarchy[2].verb);
	}
	event_retire (&drop_event);
      }
#endif
    }

}

static
void sdl_event_listener_set_mouse_motion_correction(sdl_event_listener_t* self, double ratiox, double ratioy)
{
    self->ratiox = ratiox;
    self->ratioy = ratioy;
    self->mouse_motion_corrected_p = 1;
}

sdl_event_listener_t* sdl_event_listener_instantiate(sdl_event_listener_t* x)
{
    sdl_event_listener_t* el = sdl_event_listener_instantiate_super (x);

    sdl_keyboard_initialize ();

    el->accept			    = event_listener_accept;
    el->set_mouse_motion_correction = sdl_event_listener_set_mouse_motion_correction;

    return el;
}
