/* a10 353
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/alogic.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 353 */

#ifndef ALOGIC_H
#define ALOGIC_H

/*
  "audio logic"

  uses a small 8bit cpu equipped with standard logic and numeric
  functions to create samples
*/

#include <system/effects.h>
#include <generators/8bit-cpu.h>
#include "chamberlin_2p.h"

typedef struct alogic_t {
    audio_effect_t super;
    _8pu_t state;
    chamberlin_2p_t filter;

    unsigned int i;  /* sample count for frequency effects */
    unsigned int nz; /* first frequency */
    unsigned int ny; /* second frequency */

    gen_t nexter;            /* generator compiled from the cpu */
    unsigned char *and_code; /* actual code */
    unsigned char *xor_code; /* actual code */
    unsigned char *block;    /* temporary buffer for mixing */
} alogic_t;

CLASS_INHERIT(alogic, audio_effect)

#endif
