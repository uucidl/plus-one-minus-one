/* a10 466
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/icefx.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 466 */



#ifndef ICEFX_H
  #define ICEFX_H

#include <system/effects.h>
#include <freetype/ft_renderer.h>

#include "vector.h"

#include "ice.h"

typedef struct icefx_t
{
    video_effect_t super;

    void (*throw)(struct icefx_t* self, double now);

    int count;
    ice_t* ice; /* ice objects */
    unsigned int alpha;

    double onset_ms;

    ft_renderer_t renderer;

    vector2d_t center;
    double rmax;
    int width;
    int height;
} icefx_t;

CLASS_INHERIT(icefx, video_effect)

#endif
