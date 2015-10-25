/* a10 805
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/chance.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 805 */

#ifndef KNOS_DEMOS_LIB_CHANCE_H
#define KNOS_DEMOS_LIB_CHANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libc/stdlib.h>
#include <libc/math.h>
#include <libc/limits.h>
#include <lib/cokus.h>

static inline double unirand() { return 1.0 * randomMT() / UINT_MAX; }

static inline float unirandf() { return 1.0f * randomMT() / UINT_MAX; }

static inline double variate(double center, double variation)
{
    double value = center;

    if (variation > 0.0)
        value += (variation * 2 * (unirand() - 0.5));

    return value;
}

static inline float variatef(float center, float variation)
{
    float value = center;

    if (variation > 0.0f)
        value += (variation * 2 * (unirandf() - 0.5f));

    return value;
}

static inline void unirand2d(double *u, double *v)
{
    double x;
    double y;
    double s;

    do {
        x = -1.0 + 2.0 * unirand();
        y = -1.0 + 2.0 * unirand();
        s = x * x + y * y;
    } while (s > 1.0 || s == 0.0);

    *u = (x * x - y * y) / s;
    *v = 2.0 * x *y / s;
}

static inline void unirand2df(float *u, float *v)
{
    float x;
    float y;
    float s;

    do {
        x = -1.0f + 2.0f * unirand();
        y = -1.0f + 2.0f * unirand();
        s = x * x + y * y;
    } while (s > 1.0f || s == 0.0f);

    *u = (x * x - y * y) / s;
    *v = 2.0f * x *y / s;
}

static inline double exprand()
{
    // old: return -1.0 * log(unirand() + .1);
    return -1.0 * log10(unirand() * 0.9 + .1);
}

static inline float exprandf() { return -1.0f * log10(unirand() * 0.9 + .1); }

#ifdef __cplusplus
}
#endif

#endif
