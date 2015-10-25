/* a10 693
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vloo.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 693 */

#ifndef KNOS_DEMOS_1_1_VLOO_H
#define KNOS_DEMOS_1_1_VLOO_H

#include <system/effects.h>
#include "ps.h"

/*
 * effect inspired by golan levin`s `floo`
 */

typedef struct vloo_t {
    video_effect_t super;

    /* add a new singularity to use */
    struct vloo_singularity_t *(*new_singularity)(struct vloo_t *self,
                                                  double now);

    ps_t *ps;                                 /* particle system         */
    struct vloo_singularity_t *singularities; /* singularities           */
    int singularity_number;                   /* number of next created
                                                 singularity */
} vloo_t;

CLASS_INHERIT(vloo, video_effect)

#endif
