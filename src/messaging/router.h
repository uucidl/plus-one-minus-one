/* a10 269
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/router.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 269 */

#ifndef KNOS_MESSAGING_ROUTER_H
#define KNOS_MESSAGING_ROUTER_H

#include <messaging/receiver.h>
#include <library/map.h>

typedef struct router_slot_t {
    atom_t name;
    receiver_t *receiver;
} router_slot_t;

/*
  a routing receiver object
*/

typedef struct router_t {
    receiver_t super;

    void (*set_child)(struct router_t *self, atom_t name, receiver_t *children);

    void (*receive_backup)(receiver_t *self, bytecode_stream_t *message,
                           context_t *context);

    map_t children; /* router_slot_t* */
} router_t;

CLASS_INHERIT(router, receiver);

/*
   router that supports the '*' wildcard
*/
router_t *wildcard_router_instantiate(router_t *x);

static inline router_t *wildcard_router_instantiate_toplevel(router_t *x)
{
    if (x) {
        router_to_object(x)->magic = 0;
    }
    return wildcard_router_instantiate(x);
}

/* accessors */
static inline receiver_t *router_get_receiver(router_t *r)
    __attribute__((unused));

static inline receiver_t *router_get_receiver(router_t *r) { return &r->super; }

#endif
