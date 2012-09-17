/* a10 997
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/transport_callbacks.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 997 */



#ifndef KNOS_NETWORK_TRANSPORT_CALLBACKS_H
#define KNOS_NETWORK_TRANSPORT_CALLBACKS_H

#include <network/channel.h>

/*
  callback based io
*/

typedef struct transport_callbacks_t
{
    object_t super;

    /* on read, this callback receives the input in a buffer of nbytes */
    void (*read)(struct transport_callbacks_t* self, 
		 channel_t* channel, char* buffer, int nbytes);

    /* on write, the callback receives the output buffer and its size 
     *
     * returns the written size
     */
    int (*write)(struct transport_callbacks_t* self, 
		 channel_t* channel, char* buffer, int nbytes);

    /* on connection closed, the callback acts as a notification */
    void (*close)(struct transport_callbacks_t* self, 
		  channel_t* channel);
} transport_callbacks_t;

CLASS_INHERIT(transport_callbacks, object)

#endif
