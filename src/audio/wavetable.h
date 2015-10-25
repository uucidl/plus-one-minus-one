/* a10 620
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/wavetable.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 620 */

#ifndef WAVETABLE_H
#define WAVETABLE_H

#include <system/pan.h> /* for data types */
#include "wrapper.h"

#define WAVETABLE_BITS 12
#define WAVETABLE_SIZE (1 << WAVETABLE_BITS)

typedef struct {
    sample_t value;
    sample_t slope;
} wavetable_sample_t;

typedef wavetable_sample_t *wavetable_t;

/*
   wave table generators:

   on the first get, allocates and computes the table, afterwards,
   always returned the already computed and allocated table.
*/

wavetable_t wavetable_get_gaussian();
wavetable_t wavetable_get_cosine();
wavetable_t wavetable_get_triangle();
wavetable_t wavetable_get_exponential_1();
wavetable_t wavetable_get_blackman_harris();

wrapper_t *wavetable_get_wrapper(wavetable_t w);
/* get sample at phase */
sample_t wavetable_get(wavetable_t w, wrapper_t *wrapper);
/* get sample at phase interpolating linearly */
sample_t wavetable_get_linear(wavetable_t w, wrapper_t *wrapper);

/*
  internal functions
*/
wavetable_t wavetable_allocate(int size);
void wavetable_free(wavetable_t w);
int wavetable_size(wavetable_t w);

#endif
