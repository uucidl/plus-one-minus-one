/* a10 322
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/projector.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 322 */

#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <system/effects.h>
#include "image.h"
#include <libc/stdio.h>
#include <library/stream.h>

typedef struct source_t {
    image_t source;
    int zoom_p;
    float zoom;

    /* auto center offsets */
    int x;
    int y;
} source_t;

typedef struct projector_t {
    video_effect_t super;
    int (*add_image)(struct projector_t *self, stream_t *fd);
    void (*set_current)(struct projector_t *self, int i);
    /* sets magnification for picture i.
       the zoom factor is 'normalized' so that
       z == 1.0 means full-screen picture.
    */
    void (*set_normalized_magnification)(struct projector_t *self, int i,
                                         float z);

    source_t *sources;
    int next_source;
    int sources_n;
    int current;

    int tile_p; /* wether or not to tile */
    int x, y;   /* position of up-left corner */
} projector_t;

CLASS_INHERIT(projector, video_effect)

#endif
