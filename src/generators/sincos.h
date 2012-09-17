/* a10 691
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/generators/sincos.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 691 */



#ifndef KNOS_GENERATORS_SINCOS_H
#define KNOS_GENERATORS_SINCOS_H

#include <libc/math.h>

typedef struct
{
    float a;
    /* output */
    float sin;
    float cos;
} sincos_state_t;

static
void sincos_init(sincos_state_t* state, float divisor)
{
    state->a = 2.f*(float)sin(M_PI/divisor);
    
    state->sin = 1.0f;
    state->cos = 0.0f;
    
}

static
void sincos_next(sincos_state_t* state)
{
    state->sin -= state->a*state->cos;
    state->cos += state->a*state->sin;
}

#endif
