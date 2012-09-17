/* a10 425
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan_driver.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 425 */



#ifndef PAN_DRIVER_H
#define PAN_DRIVER_H

#include "pan.h"
#include <library/map.h>
#include <system/demo.h>
#include <library/memory.h>

typedef struct pan_driver_t
{
    object_t super;

    int    (*new)(struct pan_driver_t* self, const char* device, int sample_rate);
    int    (*destroy)(struct pan_driver_t* self);
    void   (*start)(struct pan_driver_t* self);
    void   (*stop)(struct pan_driver_t* self);
    int    (*get_samples_number)(struct pan_driver_t* self);
    int    (*get_sample_rate)(struct pan_driver_t* self);
    int    (*has_fd)(struct pan_driver_t* self);
    long int (*get_fd)(struct pan_driver_t* self);
    int    (*is_ready)(struct pan_driver_t* self);
    void   (*update)(struct pan_driver_t* self, sample_t* samples);
    double (*get_time)(struct pan_driver_t* self);
    int    (*configure_demo)(struct pan_driver_t* self, demo_t* demo);
} pan_driver_t;

CLASS_INHERIT(pan_driver, object);

map_t* get_pan_drivers();
void put_pan_driver(const char* name, pan_driver_t* driver);

/*
  an immutable, <non operating> driver
*/
pan_driver_t* null_pan_driver_instantiate();

#endif
