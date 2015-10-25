/* a10 484
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/demo.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 484 */

#include "demo.h"

#include <libc/stdlib.h>
#include <scripting/compile.h>

#include <system/kgo.h>

#include <libc/math.h>

static int demo_new(demo_t *self)
{
    self->title = "";
    self->running_p = 1;
    self->video_width = 640;
    self->video_height = 480;
    self->video_buffers = 2;
    self->video_frame_ms = 20; /* 50hz */

    self->audio_device = "default";
    self->audio_sample_rate = 44100;
    self->audio_buffers = 8;

    self->end_ms = 24. * 3600. * 1000. * M_PI / 1024.;

    self->set_audio_effect(self, &audio_silence_instantiate(NULL)->super.super);
    self->set_video_effect(self, &video_blank_instantiate(NULL)->super.super);

    return 1;
}

static void demo_set_unlimited_duration(struct demo_t *self)
{
    self->end_ms = -1;
}

static void demo_tick(struct demo_t *self, const double timeMs)
{
    self->current_ms = timeMs;
    if (self->end_ms >= 0. && self->current_ms > self->end_ms) {
        self->running_p = 0;
    }
}

static void demo_update(demo_t *self, uint32_t *videobuffer)
{
    if (videobuffer)
        kgo_update(videobuffer, 1);
}

static void demo_send(demo_t *self, const char *message)
{
    context_t *c = NULL;
    bytecode_stream_t *m;

    c = context_instantiate_toplevel(NULL);
    c->object = self;
    c->dispatch_now_p = 1;

    m = compile_cstring(message, NULL);
    self->send_message(self, m, c);
}

static void demo_send_msg(demo_t *self, bytecode_stream_t *m, context_t *c)
{
    receiver_t *r = &self->router.super;
    r->receive(r, m, c);
}

static void demo_set_video_effect(demo_t *self, effect_t *e)
{
    self->kgo_effect_root = e;
}

static void demo_set_audio_effect(demo_t *self, effect_t *e)
{
    self->pan_effect_root = e;
}

static void demo_set_gui_event_listener(demo_t *self, event_listener_t *e)
{
    if (self->ptc_listener != e) {
        self->ptc_listener = e;
        self->ptc_listener->add_child(self->ptc_listener, &self->event_filter);
    }
}

static void demo_set_midi_event_listener(demo_t *self, event_listener_t *e)
{
    if (self->midi_listener != e) {
        self->midi_listener = e;
        self->midi_listener->add_child(self->midi_listener,
                                       &self->event_filter);
    }
}

demo_t *demo_instantiate(demo_t *x)
{
    demo_t *d = demo_instantiate_super(x);

    wildcard_router_instantiate_toplevel(&d->router);

    event_listener_instantiate_toplevel(&d->event_listener);
    filtered_listener_instantiate_toplevel(&d->event_filter);
    d->event_filter.listener = &d->event_listener;

    d->event_filter.signature.super.size = 1;
    d->event_filter.signature.super.hierarchy = malloc(sizeof(bytecode_t) * 1);

    {
        dictionary_t *dict = dictionary_get_instance();
        d->event_filter.signature.super.hierarchy[0].adverb =
            dict->get_atom(dict, "atom");
        d->event_filter.signature.super.hierarchy[0].verb =
            dict->get_atom(dict, "*");
    }

    d->set_video_effect = demo_set_video_effect;
    d->set_audio_effect = demo_set_audio_effect;
    d->set_gui_event_listener = demo_set_gui_event_listener;
    d->set_midi_event_listener = demo_set_midi_event_listener;
    d->set_unlimited_duration = demo_set_unlimited_duration;
    d->tick = demo_tick;
    d->update = demo_update;
    d->send_immediate = demo_send;
    d->send_message = demo_send_msg;

    return d;
}

static demo_t *demo = NULL;

demo_t *demo_get_instance()
{
    if (!demo) {
        demo = demo_instantiate_toplevel(NULL);
        demo_new(demo);
    }

    return demo;
}
