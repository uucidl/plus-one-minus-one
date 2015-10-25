/* a10 376
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/wavetable.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 376 */

#include "wavetable.h"
#include <libc/stdlib.h>
#include <libc/math.h>

static wavetable_t gaussian_w = 0;
static wavetable_t cosine = 0;
static wavetable_t triangle = 0;
static wavetable_t exponential_1 = 0;
static wavetable_t blackman_harris = 0;

#include <pure/gaussian.h>

static void wavetable_precompute(wavetable_t w)
{
    int i;
    for (i = 0; i < WAVETABLE_SIZE; i++)
        w[i].slope = (w[(i + 1) & (WAVETABLE_SIZE - 1)].value - w[i].value);
}

wavetable_t wavetable_get_gaussian()
{
    if (!gaussian_w) {
        int i;
        double sigma = WAVETABLE_SIZE / 2 / 4;
        double mu = WAVETABLE_SIZE / 2;

        /* then create */
        gaussian_w = wavetable_allocate(WAVETABLE_SIZE);

        for (i = 0; i < WAVETABLE_SIZE; i++) {
            gaussian_w[i].value = gaussian(sigma, i - mu);
            gaussian_w[i].slope = gaussian_dx(sigma, i - mu);
        }

        wavetable_precompute(gaussian_w);
    }

    return gaussian_w;
}

wavetable_t wavetable_get_exponential_1()
{
    if (!exponential_1) {
        int i;
        double target = 4.0; /*295.0;*/

        /* then create */
        exponential_1 = wavetable_allocate(WAVETABLE_SIZE);

        for (i = 0; i < WAVETABLE_SIZE; i++)
            exponential_1[i].value =
                1.0 / M_E * exp(-1.0 * (target * i / WAVETABLE_SIZE));

        wavetable_precompute(exponential_1);
    }

    return exponential_1;
}

wavetable_t wavetable_get_cosine()
{
    if (!cosine) {
        int i;
        double w = (M_PI * 2) / WAVETABLE_SIZE;

        /* then create */
        cosine = wavetable_allocate(WAVETABLE_SIZE);

        for (i = 0; i < WAVETABLE_SIZE; i++)
            cosine[i].value = sin(w * i);
        cosine[0].value = 0.0;
        cosine[WAVETABLE_SIZE - 1].value = 0.0;

        wavetable_precompute(cosine);
    }

    return cosine;
}

wavetable_t wavetable_get_triangle()
{
    if (!triangle) {
        int i;

        /* then create */
        triangle = wavetable_allocate(WAVETABLE_SIZE);

        for (i = 0; i < WAVETABLE_SIZE / 4; i++) {
            triangle[i].value = 1.0f * i / (WAVETABLE_SIZE / 4);
        }
        for (; i < 3 * WAVETABLE_SIZE / 4; i++) {
            triangle[i].value =
                1.0f - (1.0f * (i - WAVETABLE_SIZE / 4) / (WAVETABLE_SIZE / 4));
        }
        for (; i < WAVETABLE_SIZE; i++) {
            triangle[i].value =
                1.0f * (i - 3 * WAVETABLE_SIZE / 4) / (WAVETABLE_SIZE / 4) -
                1.0f;
        }

        triangle[0].value = 0.0;
        triangle[WAVETABLE_SIZE - 1].value = 0.0;

        wavetable_precompute(triangle);
    }

    return triangle;
}

/*
  this is blackman-harris -92db
*/
wavetable_t wavetable_get_blackman_harris()
{
    if (!blackman_harris) {
        int i;
        const double phi = 2.0 * M_PI / (WAVETABLE_SIZE - 1);

        blackman_harris = wavetable_allocate(WAVETABLE_SIZE);

        for (i = 0; i < WAVETABLE_SIZE; i++) {
            blackman_harris[i].value = 0.35875 - 0.48829 * cos(phi * i) +
                                       0.14128 * cos(2 * phi * i) -
                                       0.01168 * cos(3 * phi * i);
        }
        wavetable_precompute(blackman_harris);
    }

    return blackman_harris;
}

wrapper_t *wavetable_get_wrapper(wavetable_t w)
{
    return wrapper_new(WAVETABLE_BITS);
}

wavetable_t wavetable_allocate(int size)
{
    wavetable_t w = a_calloc(sizeof(wavetable_sample_t), size);

    return w;
}

void wavetable_free(wavetable_t w) { free(w); }

sample_t wavetable_get(wavetable_t w, wrapper_t *wrapper)
{
    wrapper_t work;
    int idx;

    wrapper_copy(&work, wrapper);

    idx = wrapper_get_int(&work);

    return w[idx].value;
}

sample_t wavetable_get_linear(wavetable_t w, wrapper_t *wrapper)
{
    wrapper_t work;
    int idx;
    double frac;

    wrapper_copy(&work, wrapper);

    idx = wrapper_get_int(&work);
    frac = wrapper_get_frac(&work);

    return w[idx].value + frac * w[idx].slope;
}
