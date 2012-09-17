/* a10 997
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/main-macosx.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 997 */




/*
  the main loop / buffering
*/

#include "demo.h"
#include "effect.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM);

#include "kgo.h"
#include "pan.h"
#include "midi.h"

#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/math.h>
#include <libc/stdint.h>

static demo_t* demo;

static void* videobuffer = NULL;

void ptc_cleanup_callback(void)
{
    INFO1("done.");
    DEBUG2("total time: %f\n", demo->current_ms);

    free(videobuffer);
    
    demo->kgo_effect_root->destroy (demo->kgo_effect_root);
    demo->pan_effect_root->destroy (demo->pan_effect_root);

    pan_stop();
    pan_close();
}

int macosx_main_loop(demo_t* d)
{
    effect_t* video_fx;

    demo = d;

    if(!demo->nosound) {
	if(!pan_open(PAN_DEFAULT, demo->audio_device, demo->audio_sample_rate)) {
	    ERROR1("audio: cannot open.");
	    return 1;
	}
    } else {
	INFO1("audio: no sound mode.");
	pan_open("null", demo->audio_device, demo->audio_sample_rate);
    }

    if(!pan_configure_demo(demo)) {
	ERROR1("couldn't configure demo.");
	return 1;
    }

    if(!kgo_open(KGO_DEFAULT, demo->title, demo->video_width, demo->video_height)) {
      ERROR1("video: cannot open.");	
      
      return 1;
    }

    if(!kgo_configure_demo(demo)) {
	ERROR1("couldn't configure demo.");
	return 1;
    }

    videobuffer =       
	kgo_allocate_frame();

    video_fx = demo->kgo_effect_root;
    video_fx->computes (video_fx, videobuffer, 0.0);
    kgo_update (videobuffer, 0);

    if (demo->midi_p) {
	if (!midi_open ()) {
	    ERROR1 ("couldn't configure midi.");
	    return 1;
	}
	demo->set_midi_event_listener (demo, midi_get_event_listener ());
    }
    
    pan_start();
    while (demo->running_p)
    {
      double ms = pan_gettime();
      demo->video_frame_ms = demo->video_frame_ms + (ms - demo->current_ms);
      demo->video_frame_ms /= 2;

      demo->tick (demo, ms);
      
      video_fx->computes(video_fx, videobuffer, demo->current_ms);

      kgo_update (videobuffer, 1);
    }

    if (demo->midi_p) {
	midi_close ();
    }

    pan_close();
    kgo_close();    
    
    return 0;
}
















