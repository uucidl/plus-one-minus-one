/* a10 573
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/shared_transport.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 573 */

#ifndef KNOS_NETWORK_SHARED_TRANSPORT_H
#define KNOS_NETWORK_SHARED_TRANSPORT_H

#include "transport.h"

struct default_transport_t;

struct shared_transport_t *
shared_transport_instantiate(struct shared_transport_t *x);
struct shared_transport_t *shared_transport_get_instance();

#include "selector.h"

selector_t *shared_selector_get_instance();

#endif
