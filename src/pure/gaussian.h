/* a10 560
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/pure/gaussian.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 560 */

#ifndef PURE_GAUSSIAN_H
#define PURE_GAUSSIAN_H

/**
 * Gaussian function
 */

#include <math.h> // for expf

#define PURE_GAUSSIAN_EXP_05 (0.60653065971263342426311737654032)

// this is the value of gaussian (sigma, -/+sigma)
#define gaussian_at_sigma (PURE_GAUSSIAN_EXP_05)

/**
 * Gaussian function, centered around x = 0.f
 *
 * sigma in ]0..+inf[ controls the width of the peak.
 *
 * x in [-sigma .. +sigma] yields values > 0.5f
 */
static inline float gaussian(float const sigma, float const x)
{
    float const u = x / sigma;

    return expf(-0.5f * u * u);
}

/**
 * First derivative of the gaussian function.
 */
static inline float gaussian_dx(float const sigma, float const x)
{
    float const u = x / sigma;

    return -u / sigma * expf(-0.5f * u * u);
}

#endif
