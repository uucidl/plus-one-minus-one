/* a10 385
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/velocity.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 385 */

#ifndef _VELOCITY_H_
#define _VELOCITY_H_

#include <library/memory.h>

typedef struct velocity_t {
    object_t super;
    /* update vel */
    void (*update)(struct velocity_t *self, struct velocity_t *instant_vel);

    float u;
    float v;
    float w;

    int inf_p; /* is velocity infinite */
} velocity_t;

CLASS_INHERIT(velocity, object)

#endif
