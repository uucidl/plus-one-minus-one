/* a10 834
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/audio_effect.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 834 */

#include "audio_effect.h"

#include <libc/stdlib.h>
#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_AUDIO_EFFECT);

audio_area_audio_frame_signature_t *
audio_area_audio_frame_signature_instantiate(
    audio_area_audio_frame_signature_t *x)
{
    return audio_area_audio_frame_signature_instantiate_super(x);
}

static atom_t frame_type = 0;

static int audio_effect_new(effect_t *self) { return 1; }

static int audio_effect_destroy(effect_t *self) { return 1; }

static void silence_computes_area(audio_effect_t *self, audio_area_t *area,
                                  double ms)
{
    int len = self->frame_size * area->frame_number;
    sample_t *__restrict__ samples =
        area->samples + area->head * self->frame_size;
    int i;

    for (i = 0; i < len; i++)
        samples[i] = 0.0f;
}

static void audio_effect_computes(effect_t *self, void *content, double ms)
{
    audio_effect_t *x = (audio_effect_t *)self;
    audio_area_t *area = (audio_area_t *)content;

    x->computes_area(x, area, ms);
}

static void audio_effect_computes_in_parts(effect_t *self, void *content,
                                           double ms)
{
    audio_effect_t *x = (audio_effect_t *)self;
    audio_area_t *area_in = content;
    audio_area_t area_out;

    double next_event_ms;
    double now = ms;

    receiver_t *r = router_get_receiver(effect_get_router(self));

    int count = area_in->frame_number;
    area_out = *area_in;

    while (count > 0) {
        context_t c;

        /* eval pending events for each selector */
        {
            context_instantiate_toplevel(&c);
            c.object = x;
            c.ms = now;

            r->eval_all_pending(r, &c);
            context_retire(&c);
        }

        /* determine when next event will happen */
        {
            message_node_t *node;

            context_instantiate_toplevel(&c);
            c.object = x;
            /* position ms at end of area_in */
            next_event_ms =
                ms + 1000.0 * (area_in->frame_number + 1) / x->sample_rate;
            c.ms = next_event_ms;

            node = r->peek_any_pending(r, &c);
            if (node) {
                next_event_ms = node->context->ms;
            }
        }

        if (next_event_ms < c.ms) {
            area_out.frame_number =
                (next_event_ms - now) * x->sample_rate / 1000.0;
            area_out.frame_number += 1;
            /* clip it */
            area_out.frame_number =
                area_out.frame_number > count ? count : area_out.frame_number;
        } else {
            area_out.frame_number = count;
        }

        x->computes_area(x, &area_out, now);
        area_out.head += area_out.frame_number;
        count -= area_out.frame_number;
        now += area_out.frame_number * 1000.0 / x->sample_rate;
        if (count > 0)
            TRACE2("cut at: %f", now);

        context_retire(&c);
    }
}

static void audio_effect_set_area_parameters(audio_effect_t *self, int srate,
                                             int frames, int frame_size)
{
    self->sample_rate = srate;
    self->frame_number = frames;
    self->frame_size = frame_size;
}

static atom_t audio_effect_get_frame_type(effect_t *self) { return frame_type; }

static void audio_effect_set_frame_signature(effect_t *self,
                                             object_t *signature)
{
    audio_effect_t *ae = (audio_effect_t *)self;
    audio_area_audio_frame_signature_t *sig =
        (audio_area_audio_frame_signature_t *)signature;
    ae->set_area_parameters(ae, sig->sample_rate, sig->max_frame_number,
                            sig->frame_size);
}

audio_effect_t *audio_effect_instantiate(audio_effect_t *x)
{
    audio_effect_t *e = audio_effect_instantiate_super(x);

    if (!frame_type) {
        frame_type = dictionary_get_instance()->new_atom(
            dictionary_get_instance(), AUDIO_EFFECT_FRAME_TYPE_NAME);
    }

    e->super.new = audio_effect_new;
    e->super.destroy = audio_effect_destroy;
    e->super.computes = audio_effect_computes_in_parts;
    e->super.set_frame_signature = audio_effect_set_frame_signature;
    e->super.get_frame_type = audio_effect_get_frame_type;

    e->computes_area = silence_computes_area;
    e->set_area_parameters = audio_effect_set_area_parameters;

    return e;
}

static void audio_silence_computes(effect_t *self, void *content, double ms)
{
    audio_silence_t *as = (audio_silence_t *)self;
    audio_effect_computes(self, content, ms);

    if (as->next) {
        effect_t *e = (effect_t *)as->next;
        e->computes(e, content, ms);
    }
}

static void audio_silence_set_area_parameters(audio_effect_t *self, int srate,
                                              int frames, int frame_size)
{
    audio_silence_t *as = (audio_silence_t *)self;
    audio_effect_set_area_parameters(self, srate, frames, frame_size);

    if (as->next) {
        as->next->set_area_parameters(as->next, as->super.sample_rate,
                                      as->super.frame_number,
                                      as->super.frame_size);
    }
}

static void audio_silence_set_next(audio_silence_t *self, audio_effect_t *next)
{
    self->next = next;
    if (next && self->super.sample_rate && self->super.frame_number &&
        self->super.frame_size) {
        self->next->set_area_parameters(self->next, self->super.sample_rate,
                                        self->super.frame_number,
                                        self->super.frame_size);
    }
}

audio_silence_t *audio_silence_instantiate(audio_silence_t *x)
{
    audio_silence_t *as = audio_silence_instantiate_super(x);

    as->super.super.computes = audio_silence_computes;
    as->super.set_area_parameters = audio_silence_set_area_parameters;
    as->set_next_effect = audio_silence_set_next;

    return as;
}
