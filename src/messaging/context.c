/* a10 874
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/context.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 874 */

#include <messaging/context.h>

#include <libc/stdlib.h>

static int context_is_in(context_t *self, context_t *what)
{
    int ret_p = 1;

    if (self == what)
        return 1;
    else if (what->object && self->object) {
        ret_p = ret_p && (self->object == what->object);
    }

    ret_p = ret_p && (self->ms <= what->ms);

    return ret_p;
}

context_t *context_instantiate(context_t *x)
{
    context_t *c = context_instantiate_super(x);

    c->is_in = context_is_in;

    c->object = NULL;
    c->ms = 0.0;

    return c;
}
