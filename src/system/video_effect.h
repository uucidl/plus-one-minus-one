/* a10 261
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_effect.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 261 */

#ifndef KNOS_DEMOS_SYSTEM_VIDEO_EFFECT_H
#define KNOS_DEMOS_SYSTEM_VIDEO_EFFECT_H

#include <system/video_frame.h>
#include <system/effect.h>

typedef struct video_effect_t {
    effect_t super;

    /* width is width and pitch of frame,
       height is height and height of frame */
    void (*set_frame_size)(struct video_effect_t *self, int width, int height,
                           int pitch);

    int width;
    int height;
    int pitch;
} video_effect_t;

CLASS_INHERIT(video_effect, effect);

/*
  example video effect that clears the frame before computing the 'next effect'
*/
typedef struct video_blank_t {
    video_effect_t super;
    void (*set_next_effect)(struct video_blank_t *self, video_effect_t *ae);
    /* an auxiliary video_effect that is called
       after the buffer blanking. useful in end of chain */
    video_effect_t *next;
} video_blank_t;

CLASS_INHERIT(video_blank, video_effect);

#endif
