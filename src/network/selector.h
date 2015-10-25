/* a10 263
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/selector.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 263 */

#ifndef KNOS_NETWORK_SELECTOR_H
#define KNOS_NETWORK_SELECTOR_H

#include <network/channel.h>
#include <network/transport_callbacks.h>
#include <library/memory.h>

/*
   an interface around the 'select' call
*/

typedef struct selector_t {
    object_t super;
    /* register channel in this selector */
    void (*register_channel)(struct selector_t *self, channel_t *channel,
                             transport_callbacks_t *callbacks);
    void (*unregister_channel)(struct selector_t *self, channel_t *channel);

    /* run starts the selector, this may be in another thread,
       another process, or an in process function */
    void (*run)(struct selector_t *self);
} selector_t;

CLASS_INHERIT(selector, object)

#endif
