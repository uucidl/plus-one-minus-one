/* a10 317
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_adapter.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 317 */



#ifndef KNOS_DEMOS_SYSTEM_OPENGL_ADAPTER_H
#define KNOS_DEMOS_SYSTEM_OPENGL_ADAPTER_H

#include <system/video_effect.h>
#include <system/frame_converter.h>

/*
  adapts other kind of effects to the video frames.
*/

typedef struct opengl_adapter_t
{
    video_effect_t super;

    /*
      plug effect to this adapter, which will pull data from
      e->computes when asked for a frame. 
      if own_p is true, the adapter will consider the effect his, and will destroy it
      when himself is destroyed.
      return 1 on success, 0 on error (for example if the effect's frame type is not supported)
     */
    int (*plug_effect)(struct opengl_adapter_t* self, effect_t* e, int own_p);

    /* 
       a frame converter and one frame worth of buffer
    */
    frame_converter_t* fc;
    void* frame;
    unsigned int nbytes;

    /*
      the effect we are adapting
    */
    effect_t* effect;
    int       own_p;
} video_adapter_t;

CLASS_INHERIT(video_adapter, video_effect);

#endif
