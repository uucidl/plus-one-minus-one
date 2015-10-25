/* a10 613
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/osc.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 613 */

#include <audio/wrapper.h>
#include <audio/osc.h>
#include <system/pan.h>
#include <libc/stdlib.h>

static int osc_new(osc_t *self, wavetable_t w, int sample_rate)
{
    self->wavetable = w;

    /* default parameter */
    self->phase_increment = 0.0;
    self->sample_rate = 1.0 * sample_rate;
    self->rate_sample = 1.0 / sample_rate;
    self->conv_rate = 1.0 * WAVETABLE_SIZE * self->rate_sample;
    self->wrapper = wavetable_get_wrapper(w);
    wrapper_set(self->wrapper, 0.0);

    return 1;
}

static int osc_destroy(osc_t *self)
{
    wrapper_destroy(self->wrapper);

    return 1;
}

static void osc_set_frequency(osc_t *self, double f)
{
    self->phase_increment = f * self->conv_rate;
}

static void osc_set_period(osc_t *self, double p)
{
    self->phase_increment = 1.0 / p * self->conv_rate;
}

static void osc_set_phase(osc_t *self, double phase)
{
    wrapper_set(self->wrapper, phase);
}

static sample_t osc_next(osc_t *self)
{
    sample_t s = wavetable_get_linear(self->wavetable, self->wrapper);
    wrapper_increment(self->wrapper, self->phase_increment);

    return s;
}

static sample_t osc_get(osc_t *self, double ms)
{
    wrapper_set(self->wrapper,
                ms / 1000.0 * self->sample_rate * self->phase_increment);
    return wavetable_get_linear(self->wavetable, self->wrapper);
}

osc_t *osc_instantiate(osc_t *x)
{
    osc_t *osc = osc_instantiate_super(x);

    osc->new = osc_new;
    osc->destroy = osc_destroy;
    osc->set_frequency = osc_set_frequency;
    osc->set_period = osc_set_period;
    osc->set_phase = osc_set_phase;
    osc->next = osc_next;

    return osc;
}
