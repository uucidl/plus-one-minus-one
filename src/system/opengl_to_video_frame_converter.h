/* a10 889
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/opengl_to_video_frame_converter.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 889 */



#ifndef KNOS_DEMOS_SYSTEM_OPENGL_TO_VIDEO_FRAME_CONVERTER_H
#define KNOS_DEMOS_SYSTEM_OPENGL_TO_VIDEO_FRAME_CONVERTER_H

#include <system/frame_converter.h>
#include <system/video_effect.h>
#include <system/opengl_effect.h>

/*
  Converts an opengl frame to a pixel video frame.. This
  implementation is inherently broken, as it relies on glReadPixels
  returning the data just being rendered ... which only works if the
  current context is the proper one. It also is unable to
  asynchroneously convert the opengl data. (opengl state + commands +
  rendering cannot be buffered)

  It also doesn't know where the opengl data is been written in the
  framebuffer (which could be of any size and is configured when the
  driver opens the gl context)

  So right now, it captures data from the (0, 0) point with the width
  and height of the destination video frame as the region to copy.
 */
typedef struct opengl_to_video_frame_converter_t
{
    frame_converter_t super;

    unsigned int width;
    unsigned int height;
    unsigned int pitch;
} opengl_to_video_frame_converter_t;

CLASS_INHERIT(opengl_to_video_frame_converter, frame_converter);

#endif
