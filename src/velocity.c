/* a10 926
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/velocity.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 926 */




#include <libc/stdlib.h>
#include "velocity.h"

static 
void update_v(velocity_t* self, velocity_t* instant_vel)
{
    self->u = instant_vel->u;
    self->v = instant_vel->v;
    self->w = instant_vel->w;
    self->inf_p = instant_vel->inf_p;
}

velocity_t* velocity_instantiate(velocity_t* x)
{
    velocity_t* v = velocity_instantiate_super (x);
    
    v->update = update_v;
    
    return v;
}
