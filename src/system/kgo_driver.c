/* a10 385
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo_driver.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 385 */

#include "kgo_driver.h"

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_KGO_DRIVER);

#include <library/map.h>
#include <scripting/dictionary.h>

static map_t *drivers = 0;

map_t *get_kgo_drivers()
{
    if (!drivers) {
        drivers = map_instantiate_toplevel(NULL);
    }

    return drivers;
}

void put_kgo_driver(const char *name, kgo_driver_t *driver)
{
    map_t *m = get_kgo_drivers();
    dictionary_t *dict = dictionary_get_instance();
    atom_t a = dict->new_atom(dict, name);

    if (map_value_is_there(m->get(m, (unsigned long)a))) {
        WARNING("Redefining kgo driver '%s'\n", name);
    }

    m->put(m, (unsigned long)a, driver);
}

/* --- null driver */

static int null_kgo_driver_new(kgo_driver_t *self, char *title,
                               unsigned int width, unsigned int height)
{
    return 0;
}

static int null_kgo_driver_destroy(struct kgo_driver_t *self) { return 1; }

static int null_kgo_driver_has_fd(struct kgo_driver_t *self) { return 0; }

static int null_kgo_driver_get_fd(struct kgo_driver_t *self) { return -1; }

static event_listener_t *
null_kgo_driver_get_event_listener(struct kgo_driver_t *self)
{
    return NULL;
}

static void null_kgo_driver_update_frame(struct kgo_driver_t *self,
                                         void *buffer, int event_pending)
{
    // do nothing
}

static atom_t null_kgo_driver_get_frame_type(kgo_driver_t *self)
{
    return dictionary_get_instance()->new_atom(dictionary_get_instance(),
                                               VIDEO_EFFECT_FRAME_TYPE_NAME);
}

static void *null_kgo_driver_allocate_frame(kgo_driver_t *self) { return NULL; }

static int video_effect_kgo_driver_configure_demo(kgo_driver_t *self, demo_t *d)
{
    int error_p = 0;

    d->set_gui_event_listener(d, self->get_event_listener(self));

    /* the demo must have one effect. */
    error_p = !d->kgo_effect_root;

    if (!error_p) {
        atom_t frame_type =
            d->kgo_effect_root->get_frame_type(d->kgo_effect_root);
        dictionary_t *dict = dictionary_get_instance();

        error_p =
            (frame_type != dict->get_atom(dict, VIDEO_EFFECT_FRAME_TYPE_NAME));

        if (error_p) {
            ERROR("video effect has invalid frame type: %s. (expected %s)",
                   atom_get_cstring_value(frame_type),
                   VIDEO_EFFECT_FRAME_TYPE_NAME);
        }
    }

    error_p = error_p || !d->kgo_effect_root->new (d->kgo_effect_root);

    if (error_p) {
        ERROR("couldn't create video effect.");
        return 0;
    }

    {
        argb32_video_frame_signature_t frame_signature;

        argb32_video_frame_signature_instantiate_toplevel(&frame_signature);

        frame_signature.width = d->video_width;
        frame_signature.height = d->video_height;
        frame_signature.pitch = d->video_width;

        d->kgo_effect_root->set_frame_signature(d->kgo_effect_root,
                                                &frame_signature.super);

        argb32_video_frame_signature_retire(&frame_signature);
    }

    return !error_p;
}

kgo_driver_t *kgo_driver_instantiate(kgo_driver_t *x)
{
    kgo_driver_t *driver = kgo_driver_instantiate_super(x);

    driver->new = null_kgo_driver_new;
    driver->destroy = null_kgo_driver_destroy;
    driver->has_fd = null_kgo_driver_has_fd;
    driver->get_fd = null_kgo_driver_get_fd;
    driver->get_event_listener = null_kgo_driver_get_event_listener;
    driver->update_frame = null_kgo_driver_update_frame;
    driver->get_frame_type = null_kgo_driver_get_frame_type;
    driver->allocate_frame = null_kgo_driver_allocate_frame;
    driver->configure_demo = video_effect_kgo_driver_configure_demo;

    return driver;
}

kgo_driver_t *null_kgo_driver_instantiate()
{
    return kgo_driver_instantiate_toplevel(NULL);
}

static void kgo_null_kgo_driver_initialize() __attribute__((constructor));

static void kgo_null_kgo_driver_initialize()
{
    put_kgo_driver("null", null_kgo_driver_instantiate());
}
