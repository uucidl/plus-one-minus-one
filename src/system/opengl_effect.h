/* a10 993
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/opengl_effect.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 993 */



#ifndef KNOS_DEMOS_SYSTEM_OPENGL_EFFECT_H
#define KNOS_DEMOS_SYSTEM_OPENGL_EFFECT_H

#include <system/effect.h>

#define OPENGL_FRAME_TYPE_NAME "opengl_frame"

typedef struct opengl_frame_t
{
    // an opengl frame doesn't have anything in particular now..
    // since the opengl state is stored in the driver. This makes it
    // impossible to buffer opengl data in advance.
} opengl_frame_t;

typedef struct opengl_effect_t
{
    effect_t super;
} opengl_effect_t;

CLASS_INHERIT(opengl_effect, effect);

#endif
