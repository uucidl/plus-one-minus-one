/* a10 891
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vmix.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 891 */



#ifndef _VMIX_H_
#define _VMIX_H_

#include <libc/stdint.h>

enum vmix_type { CROSSFADE, MUTILATE };

typedef struct vmix_t
{
    video_effect_t super;
    
    void (*set_mix)(struct vmix_t* self, float factor);
    void (*set_mix_type)(struct vmix_t* self, enum vmix_type type);
    void (*set_left_effect)(struct vmix_t* self, video_effect_t* e);
    void (*set_right_effect)(struct vmix_t* self, video_effect_t* e);
    void (*set_left_offset_ms)(struct vmix_t* self, double ms);
    void (*set_right_offset_ms)(struct vmix_t* self, double ms);

    float mix; /* mix factor. 1.0f is left, 0.0f is right */
    enum vmix_type type;

    int32_t* __restrict__ left;  /* left buffer */
    int32_t* __restrict__ right; /* right buffer */
    video_effect_t* left_effect;
    video_effect_t* right_effect;
    double left_ms;
    double right_ms;
} vmix_t;

CLASS_INHERIT(vmix, video_effect)

#endif
