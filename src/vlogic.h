/* a10 433
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vlogic.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 433 */

#ifndef VLOGIC_H
#define VLOGIC_H

/*
  "video logic"

  uses a small 8bit cpu equipped with standard logic and numeric
  functions to create video patterns
*/

#include <system/effects.h>
#include <generators/8bit-cpu.h>

typedef uint32_t (*palette_gen_f)(unsigned int i, unsigned int max);

typedef struct vlogic_t {
    video_effect_t super;

    /* sets period of multiplier i to scanline_size * multiplier */
    void (*set_multiplier)(struct vlogic_t *self, unsigned int i,
                           double multiplier);

    void (*generate_palette)(struct vlogic_t *self, palette_gen_f gen);

    _8pu_t state;
    block_gen_t compiled_generator; /* generator compiled from the cpu */
    unsigned char *code;            /* actual code */
    unsigned char *block;           /* data generated by the compiled_generator.
                                       it should be of size nz */
    /* periods */
    int nz;
    int ny; /* 0 < ny <= nz */

    /* palette */
    uint32_t palette[256];
} vlogic_t;

CLASS_INHERIT(vlogic, video_effect)

#endif
