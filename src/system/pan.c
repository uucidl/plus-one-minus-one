/* a10 470
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 470 */




#include "pan.h"
#include <scripting/dictionary.h>
#include "pan_driver.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_PAN);

static pan_driver_t* driver = 0;

static
void initialize_driver() __attribute__ ((constructor));

static
void initialize_driver()
{
    driver = null_pan_driver_instantiate();
}

int pan_open(const char* driver_name, const char* device, int sample_rate)
{
    dictionary_t* d = dictionary_get_instance ();
    atom_t a = d->get_atom (d, driver_name);
    map_t* m = get_pan_drivers ();
    map_value_t pp_d;
    int ret = 0;

    pp_d = m->get (m, (unsigned long) a);
    if (map_value_is_there (pp_d)) {
	driver = map_value_obtain (pp_d);
	ret = driver->new (driver, device, sample_rate);
    } else {
      WARNING2("driver '%s' not found.", driver_name);
    }

    return ret;
}

/*
  start device
*/
void pan_start()
{
    driver->start(driver);
}

/*
  stop playing
*/
void pan_stop()
{
    driver->stop(driver);
}

int pan_close()
{
    return driver->destroy(driver);
}

int pan_get_samples_number()
{
    return driver->get_samples_number(driver);
}

int pan_get_sample_rate()
{
    return driver->get_sample_rate(driver);
}

long int pan_get_fd()
{
    return driver->get_fd(driver);
}

int pan_is_ready()
{
    return driver->is_ready(driver);
}

void pan_update(sample_t* samples)
{
    driver->update(driver, samples);
}

double pan_gettime()
{
    return driver->get_time(driver);
}

int pan_configure_demo(demo_t* demo)
{
    return driver->configure_demo(driver, demo);
}
