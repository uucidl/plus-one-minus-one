/* a10 292
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo_driver.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 292 */



#ifndef KGO_DRIVER_H
#define KGO_DRIVER_H

/*
  a kgo driver provides access to a video display.
*/
#include <library/map.h>
#include <system/event_listener.h>
#include <system/demo.h>
#include <library/memory.h>

#ifdef LINUX
  #include <X11/Xlib.h>
#endif

typedef struct kgo_driver_t
{
    object_t super;

    int    (*new)(struct kgo_driver_t* self, 
		  char* title, unsigned int width, unsigned int height);
    int    (*destroy)(struct kgo_driver_t* self);
    void   (*start)(struct kgo_driver_t* self);
    void   (*stop)(struct kgo_driver_t* self);
    int    (*has_fd)(struct kgo_driver_t* self);
    int    (*get_fd)(struct kgo_driver_t* self);
    event_listener_t* (*get_event_listener)(struct kgo_driver_t* self);
    void   (*update_frame)(struct kgo_driver_t* self, void* frame, int event_pending);
    atom_t (*get_frame_type)(struct kgo_driver_t* self);
    void*  (*allocate_frame)(struct kgo_driver_t* self);
    int    (*configure_demo)(struct kgo_driver_t* self, demo_t* demo);

#ifdef LINUX
    Display* (*get_display) (struct kgo_driver_t* self);
    Window   (*get_window)  (struct kgo_driver_t* self);
#endif

    unsigned int frame_nbytes;
} kgo_driver_t;

CLASS_INHERIT(kgo_driver, object);

map_t* get_kgo_drivers();
void put_kgo_driver(const char* name, kgo_driver_t* driver);

/* 
   an immutable, <non operating> driver, which happens to be a video
   effect driver. (video_effect_t driver) 
*/
kgo_driver_t* null_kgo_driver_instantiate();

#endif
