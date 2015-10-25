/* a10 383
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/cokus.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 383 */

#ifndef KNOS_DEMOS_LIB_COKUS_H
#define KNOS_DEMOS_LIB_COKUS_H

#include <libc/stdint.h>

extern void seedMT(uint32_t seed);
extern uint32_t reloadMT(void);
extern uint32_t randomMT(void);

#endif
