/* a10 90
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/tap.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 90 */

#include "tap.h"

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_TAP);

#include <libc/stdlib.h>
#include <libc/stdio.h>

static void tapin_set_size(tapin_t *self, unsigned int size)
{
    if (size) {
        int n = size - self->line_size;
        sample_t *samples;

        self->line = realloc(self->line, size * 2 * sizeof(sample_t));

        samples = self->line + self->line_size;
        // now zeroes newly allocated area
        if (n > 0) {
            while (n--) {
                *samples++ = 0.0f;
                *samples++ = 0.0f;
            }
        }
    } else {
        if (self->line)
            free(self->line);
        self->line = NULL;
    }
    self->line_size = size;
    self->line_phase = 0;
}

static void tapin_computes_area(audio_effect_t *e, audio_area_t *area,
                                double ms)
{
    tapin_t *self = (tapin_t *)e;
    sample_t *samples = area->samples + area->head * self->super.frame_size;
    int i;

    if (self->line) {
        /* fill the circular buffer */
        for (i = 0; i < area->frame_number; i++) {
            self->line[2 * self->line_phase] =
                samples[i * self->super.frame_size];
            self->line[2 * self->line_phase + 1] =
                samples[i * self->super.frame_size + 1];

            self->line_phase += 1;
            while (self->line_phase >= self->line_size)
                self->line_phase -= self->line_size;
        }
    }
}

tapin_t *tapin_instantiate(tapin_t *x)
{
    tapin_t *t = tapin_instantiate_super(x);

    t->set_size = tapin_set_size;
    t->super.computes_area = tapin_computes_area;

    return t;
}

static void tapout_wraparound(tapout_t *self)
{
    if (self->tapin) {
        unsigned int phase = self->out_phase;
        self->out_phase %= self->tapin->line_size;
        if (phase != self->out_phase)
            DEBUG("wrap-around done.");
    }
}

static void tapout_set_tapin(tapout_t *self, tapin_t *tapin)
{
    self->tapin = tapin;
    tapout_wraparound(self);
}

static void tapout_set_offset(tapout_t *self, unsigned int offset)
{
    self->out_phase = offset;
    tapout_wraparound(self);
}

static void tapout_computes_area(audio_effect_t *e, audio_area_t *area,
                                 double ms)
{
    tapout_t *self = (tapout_t *)e;
    sample_t *samples = area->samples + area->head * self->super.frame_size;
    int i;

    if (self->tapin) {
        /* fill the circular buffer */
        for (i = 0; i < area->frame_number; i++) {
            samples[self->super.frame_size * i] =
                self->tapin->line[2 * self->out_phase];
            samples[self->super.frame_size * i + 1] =
                self->tapin->line[2 * self->out_phase + 1];

            self->out_phase += 1;
            while (self->out_phase >= self->tapin->line_size)
                self->out_phase -= self->tapin->line_size;
        }
    }
}

tapout_t *tapout_instantiate(tapout_t *x)
{
    tapout_t *t = tapout_instantiate_super(x);

    t->set_tapin = tapout_set_tapin;
    t->set_offset = tapout_set_offset;

    t->super.computes_area = tapout_computes_area;

    return t;
}
