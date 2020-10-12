/* a10 591
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/stutter.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 591 */

#include "stutter.h"

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_STUTTER);

#include <libc/stdlib.h>
#include <libc/stdio.h>

#include <lib/chance.h>

static void stutter_set_block_size(stutter_t *self, unsigned int samples)
{
    if (samples > 65536) {
        WARNING("tried to set block size to an incorrect value. ('%u')",
                 samples);
        samples = 65535;
    } else if (samples == 0) {
        samples = 2;
    }

    self->block_size = samples;

    self->block_phase %= self->block_size << 16;
    self->bblock_phase %= self->block_size;

    if (!self->block)
        self->block = calloc(sizeof(sample_t), 2 * 65536);
    if (!self->bblock)
        self->bblock = calloc(sizeof(sample_t), 2 * 65536);
}

static void stutter_set_window(stutter_t *self, stutter_window_t window)
{
    self->window_p = window ? 1 : 0;
    if (self->window_p) {
        wavetable_t wavetable = NULL;

        if (!self->window)
            self->window = osc_instantiate_toplevel(NULL);
        switch (window) {
        case GAUSSIAN:
            wavetable = wavetable_get_gaussian();
            break;
        case NONE:
            DEBUG("cannot happen");
        }

        self->window->new (self->window, wavetable, self->super.sample_rate);
        self->window->set_period(self->window, 1.0 * self->block_size /
                                                   self->super.sample_rate);
    }
}

static void stutter_set_speed(stutter_t *self, double speed)
{
    self->phase_increment = speed * 65536;
}

static void stutter_mono_computes_area(audio_effect_t *e, audio_area_t *area,
                                       double ms) __attribute__((unused));

static void stutter_mono_computes_area(audio_effect_t *e, audio_area_t *area,
                                       double ms)
{
    stutter_t *self = (stutter_t *)e;
    sample_t *samples = area->samples + area->head * self->super.frame_size;

    int i;

    /* algo:
       for each sample, grab value if not repeat, put it at current position in
       the back block.
       put value of block to current sample. when back block is full, exchange
       back block and block
       pointers
    */
    for (i = 0; i < area->frame_number; i++) {
        int wrapped_p = 0;

        sample_t w; /* window sample */
        sample_t s; /* input sample */

        if (!self->window_p)
            w = 1.0;
        else {
            w = self->window->next(self->window);
        }

        s = samples[i * self->super.frame_size];
        self->bblock[2 * self->bblock_phase] = w * s;

        s = samples[i * self->super.frame_size + 1];
        self->bblock[2 * self->bblock_phase + 1] = w * s;

        if (!self->dropout_l_p) {
            unsigned int j = self->block_phase >> 16;
            samples[i * self->super.frame_size] = self->block[2 * j];
            samples[i * self->super.frame_size + 1] = self->block[2 * j + 1];
        } else {
            samples[i * self->super.frame_size] =
                samples[i * self->super.frame_size + 1] = 0.0f;
        }

        self->block_phase += self->phase_increment;

        {
            unsigned int old_phase = self->block_phase;

            self->block_phase %= self->block_size << 16;

            if (self->block_phase != old_phase)
                wrapped_p = 1;
        }

        if (wrapped_p) {
            // decides wether to repeat or not
            if (unirand() < self->repeat_probability)
                self->repeat_l_p = self->repeat_r_p = 1;
            else
                self->repeat_l_p = self->repeat_r_p = 0;

            // decides wether to dropout or not
            if (unirand() < self->dropout_probability)
                self->dropout_l_p = self->dropout_r_p = 1;
            else
                self->dropout_l_p = self->dropout_r_p = 0;
        }

        if (!self->repeat_l_p) {
            self->bblock_phase += 1;
            if (self->bblock_phase == self->block_size) {
                // exchange blocks
                sample_t *temp = self->block;
                self->block = self->bblock;
                self->bblock = temp;
                self->bblock_phase = 0;
            }
        }
    }
}

static void stutter_stereo_computes_area(audio_effect_t *e, audio_area_t *area,
                                         double ms)
{
    stutter_t *self = (stutter_t *)e;
    sample_t *samples = area->samples + area->head * self->super.frame_size;

    int i;

    /* algo:
       for each sample, grab value if not repeat, put it at current position in
       the back block.
       put value of block to current sample. when back block is full, exchange
       back block and block
       pointers
    */
    for (i = 0; i < area->frame_number; i++) {
        int wrapped_p = 0;

        sample_t w; /* window sample */
        sample_t s; /* input sample */

        if (!self->window_p)
            w = 1.0f;
        else {
            w = self->window->next(self->window);
        }

        if (!self->repeat_l_p) {
            s = samples[i * self->super.frame_size];
            self->bblock[2 * self->bblock_phase] = w * s;
        } else {
            self->bblock[2 * self->bblock_phase] =
                self->block[2 * self->bblock_phase];
        }

        if (!self->repeat_r_p) {
            s = samples[i * self->super.frame_size + 1];
            self->bblock[2 * self->bblock_phase + 1] = w * s;
        } else {
            self->bblock[2 * self->bblock_phase + 1] =
                self->block[2 * self->bblock_phase + 1];
        }

        {
            unsigned int j = self->block_phase >> 16;

            if (!self->dropout_l_p) {
                samples[i * self->super.frame_size] = self->block[2 * j];
            } else {
                samples[i * self->super.frame_size] = 0.0f;
            }

            if (!self->dropout_r_p) {
                samples[i * self->super.frame_size + 1] =
                    self->block[2 * j + 1];
            } else {
                samples[i * self->super.frame_size + 1] = 0.0f;
            }
        }

        self->block_phase += self->phase_increment;

        {
            unsigned int old_phase = self->block_phase;

            self->block_phase %= self->block_size << 16;

            if (self->block_phase != old_phase)
                wrapped_p = 1;
        }

        if (wrapped_p) {
            // decides wether to repeat or not
            if (unirand() < self->repeat_probability)
                self->repeat_l_p = 1;
            else
                self->repeat_l_p = 0;

            if (unirand() < self->repeat_probability)
                self->repeat_r_p = 1;
            else
                self->repeat_r_p = 0;

            // decides wether to dropout or not
            if (unirand() < self->dropout_probability)
                self->dropout_l_p = 1;
            else
                self->dropout_l_p = 0;
            // decides wether to dropout or not
            if (unirand() < self->dropout_probability)
                self->dropout_r_p = 1;
            else
                self->dropout_r_p = 0;
        }

        self->bblock_phase += 1;
        if (self->bblock_phase == self->block_size) {
            // exchange blocks
            sample_t *temp = self->block;
            self->block = self->bblock;
            self->bblock = temp;
            self->bblock_phase = 0;
        }
    }
}

static void stutter_set_repeat_probability(stutter_t *self, double proba)
{
    if (proba > 1.0)
        proba = 1.0;
    else if (proba < 0.0)
        proba = 0.0;

    self->repeat_probability = proba;
}

static void stutter_set_dropout_probability(stutter_t *self, double proba)
{
    if (proba > 1.0)
        proba = 1.0;
    else if (proba < 0.0)
        proba = 0.0;

    self->dropout_probability = proba;
}

static double stutter_get_latency_ms(effect_t *zelf)
{
    stutter_t *self = (stutter_t *)zelf;
    if (self->super.super.mode != ON) {
        return 0.;
    } else {
        return 1000. * (self->block_size - (self->block_phase >> 16)) / 44100.;
    }
}

stutter_t *stutter_instantiate(stutter_t *x)
{
    stutter_t *s = stutter_instantiate_super(x);

    effect_register_instance("stutter", &s->super.super);

    s->set_block_size = stutter_set_block_size;
    s->set_speed = stutter_set_speed;
    s->set_window = stutter_set_window;
    s->set_repeat_probability = stutter_set_repeat_probability;
    s->set_dropout_probability = stutter_set_dropout_probability;
    s->super.computes_area = stutter_stereo_computes_area;
    s->super.super.get_latency_ms = stutter_get_latency_ms;

    return s;
}
