/* a10 219
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/stutter.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 219 */

#ifndef STUTTER_H
#define STUTTER_H

/*
  'stuttering' effect: samples blocks of audio from
  input and plays them back looped at a specified speed,
  with a determined window. the effect can also
  repeat/drop the blocks, and change their volume
*/

#include <system/effects.h>
#include <system/pan.h>
#include <audio/osc.h>

typedef enum { NONE, GAUSSIAN } stutter_window_t;

typedef struct stutter_t {
    audio_effect_t super;

    void (*set_block_size)(struct stutter_t *self, unsigned int samples);
    void (*set_speed)(struct stutter_t *self, double speed);
    void (*set_window)(struct stutter_t *self, stutter_window_t type);

    void (*set_repeat_probability)(struct stutter_t *self, double prob);
    void (*set_dropout_probability)(struct stutter_t *self, double prob);
    /* 0.0 -> 1.0, defines the amount of amplitude variation */
    void (*set_amplitude_variation)(struct stutter_t *self, double variation);

    sample_t *block;
    unsigned int
        block_phase; /* 16.16 format, allowing 65536 maximum size for blocks */
    unsigned int phase_increment; /* 16.16 format */

    sample_t *bblock; /* back block buffer */
    unsigned int bblock_phase;
    unsigned int block_size;

    int window_p;
    osc_t *window;

    int repeat_l_p;
    int repeat_r_p;
    double repeat_probability;

    int dropout_l_p;
    int dropout_r_p;
    double dropout_probability;
} stutter_t;

CLASS_INHERIT(stutter, audio_effect)

#endif
