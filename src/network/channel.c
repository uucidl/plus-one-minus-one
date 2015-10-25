/* a10 600
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/channel.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 600 */

#include <network/channel.h>

#include <libc/stdlib.h>
#include <libc/string.h>

channel_t *channel_instantiate(channel_t *x)
{
    channel_t *c = channel_instantiate_super(x);

    sock_stream_instantiate_toplevel(&c->stream);

    return c;
}
