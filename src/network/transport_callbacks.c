/* a10 351
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/transport_callbacks.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 351 */




#include <network/transport_callbacks.h>

transport_callbacks_t* transport_callbacks_instantiate (transport_callbacks_t* x) {
    return transport_callbacks_instantiate_super (x);
}
