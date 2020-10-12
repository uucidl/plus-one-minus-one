/* a10 692
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/effect.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 692 */

#include "effect.h"

#include <libc/stdlib.h>
#include <libc/stdio.h>

#include <scripting/dictionary.h>
#include <scripting/compile.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_EFFECT);

static int core_effect_new(effect_t *this)
{
    // nothing
    return 1;
}

static int core_effect_destroy(effect_t *this) { return 1; }

static void core_effect_computes(effect_t *this, void *frame, double ms)
{
    // does nothing
}

static void core_effect_set_mode(effect_t *this, emode_t mode)
{
    switch (mode) {
    case ON:
        if (this->mode != ON && this->swapped_computes)
            this->computes = this->swapped_computes;
        this->mode = ON;
        TRACE("'%s' setting mode: ON",
               atom_get_cstring_value(this->class->class));
        break;
    case OFF:
        if (this->mode == ON) {
            this->swapped_computes = this->computes;
        }
        this->computes = core_effect_computes;
        this->mode = OFF;
        TRACE("'%s' setting mode: OFF",
               atom_get_cstring_value(this->class->class));
        break;
    }
}

static void core_effect_send(effect_t *self, const char *message)
{
    context_t *c;
    bytecode_stream_t *m;

    c = context_instantiate_toplevel(NULL);
    c->object = self;
    c->dispatch_now_p = 1;

    m = compile_cstring(message, NULL);
    self->send_message(self, m, c);
}

static void core_effect_send_msg(effect_t *self, bytecode_stream_t *m,
                                 context_t *c)
{
    receiver_t *r = router_get_receiver(effect_get_router(self));
    r->receive(r, m, c);
}

/*
  default latency is 0 ms
*/
static double core_effect_get_latency_ms(effect_t *this) { return 0.; }

static atom_t core_effect_get_frame_type(effect_t *this)
{
    /* 0, the #undefined atom */
    return 0;
}

static void core_effect_set_frame_signature(effect_t *this, object_t *signature)
{
    /* nothing to do */
}

effect_t *effect_instantiate(effect_t *x)
{
    dictionary_t *d = dictionary_get_instance();
    effect_t *e = effect_instantiate_super(x);

    e->class = init_class("effect");

    e->new = core_effect_new;
    e->destroy = core_effect_destroy;
    e->computes = core_effect_computes;
    e->get_latency_ms = core_effect_get_latency_ms;
    e->get_frame_type = core_effect_get_frame_type;
    e->set_frame_signature = core_effect_set_frame_signature;
    e->mode = ON;
    e->set_mode = core_effect_set_mode;
    e->send_immediate = core_effect_send;
    e->send_message = core_effect_send_msg;

    return (effect_t *)e;
}

void effect_register_instance(const char *class_name, effect_t *instance)
{
    instance->class = init_class(class_name);
}
