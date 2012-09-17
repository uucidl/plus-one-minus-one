/* a10 356
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/gscreen.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 356 */



#ifndef KNOS_DEMOS_1_1_GSCREEN_H
#define KNOS_DEMOS_1_1_GSCREEN_H

#include <system/effects.h>
#include "ps.h"

typedef struct
{
    video_effect_t super;
    
    int w,     h; /* image width / height */
    int winc,  hinc; /* increments for cells */
    int s; /* size */
    unsigned char** amp; /* image as loaded */

    double r; /* some offset in the rotation */

    ps_t* ps; /* particle system */
} gscreen_t;

CLASS_INHERIT(gscreen, video_effect)

#endif
