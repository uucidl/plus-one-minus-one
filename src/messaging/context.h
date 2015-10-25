/* a10 803
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/context.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 803 */

#ifndef KNOS_MESSAGING_CONTEXT_H
#define KNOS_MESSAGING_CONTEXT_H

#include <library/memory.h>

typedef struct context_t {
    object_t super;

    /*
      returns true if self->ms is before (or equal) what->ms,
      and self->object is included in what->object
      (warning: NULL includes everything, and is included in everything)
    */
    int (*is_in)(struct context_t *self, struct context_t *what);

    void *object;
    double ms;
    int dispatch_now_p;
} context_t;

CLASS_INHERIT(context, object);

#endif
