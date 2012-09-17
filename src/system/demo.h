/* a10 567
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/demo.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 567 */



#ifndef DEMO_H
#define DEMO_H

#include "effects.h"

#include <messaging/router.h>
#include <system/event_listener.h>

/*
  demo effective configuration.
*/

typedef struct demo_t
{
    object_t super;

    char* title;

    double current_ms; /* current time */
    /* end of the demo in milliseconds, -1 if the demo shall be unlimited. */
    double end_ms;
    int running_p; /* is the demo running */

    void (*set_unlimited_duration) (struct demo_t* self);

    void (*tick) (struct demo_t* self, const double timeMs);

    int video_width;
    int video_height;
    int video_buffers; /* number of allocated buffers */
    double video_frame_ms; /* current period in ms of a video frame */

    /* push method for updating the screen from outside an effect */
    void (*update)(struct demo_t* self, uint32_t* videobuffer);

    char *audio_device;
    int  audio_sample_rate;
    int  audio_frame_size; /* size in sample of one buffer */
    int  audio_buffers; /* number of allocated buffers */

    int nosound; /* disables sound */
    int osc;     /* enables osc subsystem */
    int midi_p;  /* enables midi subsystem */

    void (*set_video_effect)(struct demo_t* demo, effect_t* e);
    void (*set_audio_effect)(struct demo_t* demo, effect_t* e);

    effect_t* kgo_effect_root; /* root video effect */
    effect_t* pan_effect_root; /* root audio effect */

    /*
      private api, used by the driver to set attach its listener to the demo
    */
    void (*set_gui_event_listener)(struct demo_t* demo, event_listener_t* e);

    void (*set_midi_event_listener)(struct demo_t* demo, event_listener_t* e);

    /* forwards all ui events to its childs */
    event_listener_t* ptc_listener;

    /* forwards all midi events to its childs */
    event_listener_t* midi_listener;

    /* global event listener */
    event_listener_t event_listener;
    filtered_listener_t event_filter;

    /* global router to which classes can attaches themselves */
    router_t router;
    /* compile and send a message to self */
    void (*send_immediate)(struct demo_t* self, const char* message);
    void (*send_message)(struct demo_t* self, 
			 bytecode_stream_t* msg,
			 context_t* context);

    /* a callback called by pan if an overflow occurs */
    void (*pan_overflow_callback)(sample_t sample);
} demo_t;

CLASS_INHERIT(demo, object)

demo_t* demo_get_instance();

#endif
