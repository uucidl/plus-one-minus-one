/* a10 970
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/audio_effect.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 970 */

#ifndef KNOS_DEMOS_SYSTEM_AUDIO_EFFECT_H
#define KNOS_DEMOS_SYSTEM_AUDIO_EFFECT_H

#include "effect.h"

typedef float sample_t;

/*
   describes a buffer where audio_effect_t(s) can make their
   computations.
*/

#define AUDIO_EFFECT_FRAME_TYPE_NAME "audio_area"

typedef struct audio_area_audio_frame_signature_t {
    object_t super;
    int sample_rate;      /* number of frames per second */
    int max_frame_number; /* maximum number of sampling frames in each effect
                             frame */
    int frame_size;       /* number of samples/channels in sampling frame */
} audio_area_audio_frame_signature_t;

CLASS_INHERIT(audio_area_audio_frame_signature, object);

typedef struct audio_area_t {
    sample_t *__restrict__ samples;
    int head; /* offset in frames where to start */
    int frame_number;
} audio_area_t;

typedef struct audio_effect_t {
    effect_t super;
    void (*set_area_parameters)(struct audio_effect_t *self, int sample_rate,
                                int frame_number, int frame_size);
    /*
       by default, audio_effect_t::effect::computes cuts the input area into as
       many
       areas necessary to handle each received events at their ideal time,
       calling
       this method for each area.

       overriding audio_effect_t::effect::computes removes this behaviour.
    */
    void (*computes_area)(struct audio_effect_t *self, audio_area_t *area,
                          double ms);

    int sample_rate;
    int frame_number;
    int frame_size; /* frame size in samples */
} audio_effect_t;

CLASS_INHERIT(audio_effect, effect)

/*
  some example audio_effect that first clears the area with silence, then calls
  the 'next effect'
*/
typedef struct audio_silence_t {
    audio_effect_t super;

    void (*set_next_effect)(struct audio_silence_t *self, audio_effect_t *ae);
    /* an auxiliary audio_effect that is called
       after the buffer silencing. useful in end of chain */
    audio_effect_t *next;
} audio_silence_t;

CLASS_INHERIT(audio_silence, audio_effect)

#endif
