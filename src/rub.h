/* a10 416
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/rub.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 416 */

#ifndef KNOS_DEMOS_RUB_H
#define KNOS_DEMOS_RUB_H

#include "blur.h"

typedef struct rub_t {
    blur_t super;
} rub_t;

CLASS_INHERIT(rub, blur);

#endif
