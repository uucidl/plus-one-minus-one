/* a10 396
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan_driver.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 396 */

#include "pan_driver.h"

#include <logging.h>
LOG_NEW_DEFAULT_SUBCATEGORY(PAN_DRIVER, KNOS_DEMOS_SYSTEM);

#include <library/map.h>
#include <scripting/dictionary.h>

static map_t *drivers = 0;

map_t *get_pan_drivers()
{
    if (!drivers) {
        drivers = map_instantiate_toplevel(NULL);
    }

    return drivers;
}

void put_pan_driver(const char *name, pan_driver_t *driver)
{
    map_t *m = get_pan_drivers();
    dictionary_t *dict = dictionary_get_instance();
    atom_t a = dict->new_atom(dict, name);

    if (map_value_is_there(m->get(m, (unsigned long)a))) {
        WARNING("Redefining pan driver '%s'\n", name);
    }

    m->put(m, (unsigned long)a, driver);
}

static int pan_driver_configure_demo(pan_driver_t *p, demo_t *demo)
{
    if (!demo->pan_effect_root)
        return 0;

    {
        /* check frame type */
        dictionary_t *dict = dictionary_get_instance();
        atom_t frame_type =
            demo->pan_effect_root->get_frame_type(demo->pan_effect_root);

        if (frame_type != dict->get_atom(dict, AUDIO_EFFECT_FRAME_TYPE_NAME)) {
            ERROR("Invalid frame type for audio effect. (%s) Expected %s.",
                   atom_get_cstring_value(frame_type),
                   AUDIO_EFFECT_FRAME_TYPE_NAME);
            return 0;
        }
    }

    if (!demo->pan_effect_root->new (demo->pan_effect_root)) {
        ERROR("couldn't create audio effect.");
        return 0;
    }

    demo->audio_sample_rate = p->get_sample_rate(p);
    demo->audio_frame_size = p->get_samples_number(p);

    {
        // shh, it's a secret
        audio_area_audio_frame_signature_t signature;

        audio_area_audio_frame_signature_instantiate_toplevel(&signature);

        signature.sample_rate = demo->audio_sample_rate;
        signature.max_frame_number = demo->audio_frame_size;
        signature.frame_size = 2;
        demo->pan_effect_root->set_frame_signature(demo->pan_effect_root,
                                                   &signature.super);

        audio_area_audio_frame_signature_retire(&signature);
    }

    return 1;
}

pan_driver_t *pan_driver_instantiate(pan_driver_t *x)
{
    pan_driver_t *p = pan_driver_instantiate_super(x);

    p->configure_demo = pan_driver_configure_demo;

    return p;
}

/* --- null driver */

#include <library/time.h>

typedef struct null_driver_t {
    pan_driver_t super;

    int sample_rate;
    double start_ms;
    double end_ms;
    int running_p;
} null_driver_t;

CLASS_INHERIT(null_driver, pan_driver);

static int null_pan_driver_new(pan_driver_t *zelf, const char *device,
                               int sample_rate)
{
    null_driver_t *self = (null_driver_t *)zelf;

    self->sample_rate = sample_rate;

    return 1;
}

static int null_pan_driver_destroy(pan_driver_t *self) { return 1; }

static void null_pan_driver_start(pan_driver_t *zelf)
{
    null_driver_t *self = (null_driver_t *)zelf;

    self->start_ms = get_milliseconds();
    self->running_p = 1;
}

static void null_pan_driver_stop(pan_driver_t *zelf)
{
    null_driver_t *self = (null_driver_t *)zelf;

    self->end_ms = get_milliseconds();
    self->running_p = 0;
}

static void null_pan_driver_update(pan_driver_t *self, sample_t *samples)
{
    // nothing to be done
}

static int null_pan_driver_get_sample_rate(pan_driver_t *zelf)
{
    null_driver_t *self = (null_driver_t *)zelf;

    return self->sample_rate;
}

static int null_pan_driver_get_samples_number(pan_driver_t *zelf) { return 32; }

static int null_pan_driver_return0(pan_driver_t *self) { return 0; }

static long int null_pan_driver_returnlong0(pan_driver_t *self) { return 0; }

static double null_pan_driver_get_time(pan_driver_t *zelf)
{
    null_driver_t *self = (null_driver_t *)zelf;
    double ms = self->start_ms;

    if (self->running_p) {
        ms -= get_milliseconds();
    } else {
        ms -= self->end_ms;
    }

    return -ms;
}

null_driver_t *null_driver_instantiate(null_driver_t *x)
{
    null_driver_t *p = null_driver_instantiate_super(NULL);

    p->super.new = null_pan_driver_new;
    p->super.destroy = null_pan_driver_destroy;
    p->super.start = null_pan_driver_start;
    p->super.stop = null_pan_driver_stop;
    p->super.get_samples_number = null_pan_driver_get_samples_number;
    p->super.get_sample_rate = null_pan_driver_get_sample_rate;
    p->super.has_fd = null_pan_driver_return0;
    p->super.get_fd = null_pan_driver_returnlong0;
    p->super.is_ready = null_pan_driver_return0;
    p->super.update = null_pan_driver_update;
    p->super.get_time = null_pan_driver_get_time;

    return p;
}

pan_driver_t *null_pan_driver_instantiate()
{
    null_driver_t *p = null_driver_instantiate_toplevel(NULL);
    return &p->super;
}

static void pan_null_pan_driver_initialize() __attribute__((constructor));

static void pan_null_pan_driver_initialize()
{
    put_pan_driver("null", null_pan_driver_instantiate());
}
