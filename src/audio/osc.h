/* a10 315
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/osc.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 315 */



#ifndef KNOS_AUDIO_OSC_H
#define KNOS_AUDIO_OSC_H

#include <audio/wavetable.h>
#include <system/pan.h>
#include <library/memory.h>

/*

oscillator

*/

typedef struct osc_t
{
    object_t super;
    wavetable_t wavetable;
    double      phase_increment;
    wrapper_t*  wrapper;
    double      sample_rate;
    double      rate_sample;
    double      conv_rate;

    int (*new)(struct osc_t* self, wavetable_t w, int sample_rate);
    int (*destroy)(struct osc_t* self);
    void (*set_frequency)(struct osc_t* self, double freq);
    /* period in seconds */
    void (*set_period)(struct osc_t* self, double period);
    void (*set_phase)(struct osc_t* self, double phase);
    sample_t (*next)(struct osc_t* self);
} osc_t;

CLASS_INHERIT(osc, object)

#endif
