/* a10 596
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/transport.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 596 */



#ifndef KNOS_NETWORK_TRANSPORT_H
#define KNOS_NETWORK_TRANSPORT_H

#include <network/channel.h>
#include <network/selector.h>
#include <network/transport_callbacks.h>

#include <library/vector.h>
#include <library/memory.h>

typedef struct transport_t
{
    object_t super;
    channel_t* (*connect_to)(struct transport_t* self, 
			     url_t* url, 
			     transport_callbacks_t* callbacks);

    void (*release)(struct transport_t* self, channel_t* channel);

    /* run with given channel_selector 
     */
    void (*run)(struct transport_t* self, selector_t* selector);

    /* run and wait for completion of pending io transfers 
     */
    void (*finish)(struct transport_t* self);
} transport_t;

CLASS_INHERIT(transport, object)

struct default_transport_t;

struct default_transport_t* default_transport_instantiate(struct default_transport_t* x);

#endif
