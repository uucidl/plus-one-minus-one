/* a10 583
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/blur.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 583 */

#ifndef KNOS_DEMOS_BLUR_H
#define KNOS_DEMOS_BLUR_H

#include <system/effects.h>

#include <libc/stdint.h>

enum blur_mode_t { NORMAL, MASKED };

typedef struct blur_t {
    video_effect_t super;

    int (*set_length)(struct blur_t *self, int length);
    void (*set_mode)(struct blur_t *self, enum blur_mode_t mode);
    void (*set_mask)(struct blur_t *self, int mask);

    int n; /* blur length */
    int p; /* blur power (n = 2^p) */

    int mask; /* mask used for 'masked' blur */

    /* buffer to record previous occurences of
       pixels (for the fixing running sum) */
    int32_t *buffer;
    int buffer_chunk; /* number of chunks allocated */
} blur_t;

CLASS_INHERIT(blur, video_effect)

#endif
