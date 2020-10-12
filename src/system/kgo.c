/* a10 536
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 536 */

#include "kgo.h"
#include <scripting/dictionary.h>
#include "kgo_driver.h"

#include <library/map_impl.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_KGO);

static kgo_driver_t *driver = 0;

static void initialize_driver() __attribute__((constructor));

static void initialize_driver() { driver = null_kgo_driver_instantiate(); }

int kgo_open(const char *driver_name, char *title, int width, int height)
{
    dictionary_t *d = dictionary_get_instance();
    atom_t a = d->get_atom(d, driver_name);
    map_t *m = get_kgo_drivers();
    map_value_t kk_d;
    int ret = 0;

    kk_d = m->get(m, (unsigned long)a);
    if (map_value_is_there(kk_d)) {
        driver = map_value_obtain(kk_d);
        ret = driver->new (driver, title, width, height);
    } else {
        ERROR("couldn't find driver %s", driver_name);
    }

    return ret;
}

int kgo_close(void) { return driver->destroy(driver); }

void kgo_start(void)
{
    if (driver->start)
        driver->start(driver);
}

void kgo_stop(void)
{
    if (driver->stop)
        driver->stop(driver);
}

int kgo_get_fd(void)
{
    if (driver->has_fd(driver)) {
        return driver->get_fd(driver);
    } else {
        return -1;
    }
}

event_listener_t *kgo_get_event_listener()
{
    return driver->get_event_listener(driver);
}

void kgo_update(void *frame, int event_pending)
{
    driver->update_frame(driver, frame, event_pending);
}

int kgo_configure_demo(demo_t *demo)
{
    return driver->configure_demo(driver, demo);
}

atom_t kgo_get_frame_type() { return driver->get_frame_type(driver); }

void *kgo_allocate_frame() { return driver->allocate_frame(driver); }

kgo_driver_t *kgo_get_current_driver() { return driver; }

kgo_driver_t *kgo_get_driver(atom_t a)
{
    map_t *m = get_kgo_drivers();
    map_iterator_t *it = m->get_iterator(m, NULL);
    kgo_driver_t *a_driver = NULL;
    if (it) {
        map_value_t v;

        while (map_value_is_there(v = map_iterator_next(it))) {
            kgo_driver_t *d = map_value_obtain(v);
            if (a == d->get_frame_type(d)) {
                a_driver = d;
                break;
            }
        }

        map_iterator_retire(it);
    }

    return a_driver;
}
