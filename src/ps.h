/* a10 111
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/ps.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 111 */

#ifndef PS_H
#define PS_H

#include <libc/stdint.h>
#include <library/memory.h>

typedef struct {
    object_t super;
    float x;
    float y;
    float z;
    float amp;
    float size;
    /* private */
    float size_phase;
} particle_t;

CLASS_INHERIT(particle, object)

typedef struct ps_t {
    object_t super;
    /* rendering */
    int opacity_r;         /* max radius of opacity table */
    float *opacity;        /* opacity vs distance^2 */
    int opacity_n;         /* size of opacity table */
    unsigned int **lookup; /* particle sprite */

    /* particle system */
    particle_t *particles; /* particles */
    int particles_n;       /* number of particles */

    /*  a table of 256 expansions of alpha values
        eamp = amp | amp<<8 | amp<<16
        (controls the color at the same time)
    */
    int32_t *eamp_lu;

    /* methods */
    int (*new)(struct ps_t *self,
               int max_particles, /* max number of rendered particles */
               int max_radius,    /* max radius of those particles */
               /* generator for opacity */
               float (*gen)(unsigned int d2, unsigned int of), int r, int g,
               int b);
    int (*destroy)(struct ps_t *self);
    void (*render)(struct ps_t *self, int32_t *pixels, unsigned int width,
                   unsigned int height, unsigned int pitch);
} ps_t;

CLASS_INHERIT(ps, object)

#endif
