/* a10 211
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/channel.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 211 */

#ifndef KNOS_NETWORK_CHANNEL_H
#define KNOS_NETWORK_CHANNEL_H

#include <library/sock_stream.h>

typedef struct channel_t {
    object_t super;
    sock_stream_t stream;
} channel_t;

CLASS_INHERIT(channel, object);

#endif
