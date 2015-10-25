/* a10 640
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/event_listener.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 640 */

#ifndef KNOS_DEMOS_SYSTEM_EVENT_LISTENER_H
#define KNOS_DEMOS_SYSTEM_EVENT_LISTENER_H

#include <library/map.h>
#include <system/event.h>
#include <library/memory.h>

typedef struct filtered_listener_t {
    object_t super;
    signature_t signature;
    struct event_listener_t *listener;

    void (*set_signature_from_cstring)(struct filtered_listener_t *self,
                                       const char *s);
} filtered_listener_t;

CLASS_INHERIT(filtered_listener, object);

typedef struct event_listener_t {
    object_t super;

    void (*accept)(struct event_listener_t *self, const event_t *e);
    void (*add_child)(struct event_listener_t *self,
                      filtered_listener_t *listener);
    void (*remove_child)(struct event_listener_t *self,
                         filtered_listener_t *listener);

    map_t childs; /* map of filtered_listener_t* */
} event_listener_t;

CLASS_INHERIT(event_listener, object);

#endif
