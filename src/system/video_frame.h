/* a10 183
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_frame.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 183 */



#ifndef KNOS_DEMOS_SYSTEM_VIDEO_FRAME_H
#define KNOS_DEMOS_SYSTEM_VIDEO_FRAME_H

#include <library/memory.h>

#define VIDEO_EFFECT_FRAME_TYPE_NAME "argb32[]"

/*
  signature for video effect's frames
 */
typedef struct argb32_video_frame_signature_t
{
    object_t super;
    unsigned int width;  /* width of window */
    unsigned int height; /* height of window */
    unsigned int pitch;  /* size of one scanline */
} argb32_video_frame_signature_t;

CLASS_INHERIT (argb32_video_frame_signature, object);

#endif
