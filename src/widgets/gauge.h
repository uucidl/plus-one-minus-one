/* a10 14
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/widgets/gauge.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 14 */



#ifndef KNOS_DEMOS_WIDGETS_GAUGE_H 
  #define KNOS_DEMOS_WIDGETS_GAUGE_H 

#include <image.h>
#include <library/memory.h>

typedef struct gauge_t
{
    object_t super;
    void (*render)(struct gauge_t* self, image_t* dest);

    void (*set_percent)(struct gauge_t* self, float percent);
    float percent; /* 0.0 1.0 */

    float x, y; /* top left origin */
    float w, h; /* width height */
} gauge_t;

CLASS_INHERIT(gauge, object)

typedef gauge_t vgauge_t;

CLASS_INHERIT(vgauge, object)

vgauge_t* vgauge_instantiate(vgauge_t* x);

typedef gauge_t hgauge_t;

CLASS_INHERIT(hgauge, object)

hgauge_t* hgauge_instantiate(hgauge_t* x);

typedef gauge_t rgauge_t;

CLASS_INHERIT(rgauge, object)

rgauge_t* rgauge_instantiate(rgauge_t* x);

#endif
