/* a10 530
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/resource_audio_effect.h') with
 *a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 530 */

#ifndef KNOS_AUDIO_RESOURCE_AUDIO_EFFECT_H
#define KNOS_AUDIO_RESOURCE_AUDIO_EFFECT_H

#include <system/effects.h>
#include <messaging/receiver.h>
#include <scripting/bytecode_stream.h>
#include <library/stream.h>
#include <messaging/context.h>
#include <audio/progress.h>

/*
  an (abstract) effect dedicated to compute/generate the samples taken from a
  disk resource
*/
typedef struct resource_audio_effect_t {
    audio_effect_t super;

    /*
       this function is guaranteed not to be realtime safe. Don't call it inside
       computes!

       loading progress is written to the preallocated progress structure.
       (which if NULL is discarded)
    */
    int (*load_file)(struct resource_audio_effect_t *self, stream_t *file,
                     progress_t *progress);
    /* set looping mode if input is true. looping mode may be altered
       if the resource is not loopable */
    void (*set_looping_mode)(struct resource_audio_effect_t *self,
                             const atom_t loop_p, context_t *c);
    /* master volume, [0.0f, 1.0f] */
    void (*set_master_volume)(struct resource_audio_effect_t *self,
                              const float volume, context_t *c);
    /* total duration in ms of currently loaded stream, or 0.0 */
    double (*get_total_ms)(struct resource_audio_effect_t *self);
    /* time in ms of current position in stream */
    double (*get_next_expected_ms)(struct resource_audio_effect_t *self);

    /* set given receiver to receive the end-of-stream atom,
     which will be prepended to the given message */
    void (*set_end_of_stream_receiver)(struct resource_audio_effect_t *self,
                                       receiver_t *r,
                                       bytecode_stream_t *message);

    /*
      This function may or not be realtime safe, depending on the
      actual implementation. It tries to duplicate the
      resource_audio_effect into a new instance, referring to the same
      underlying resource. This 'child' will be destroyed *and*
      retired once we destroy its parent.
    */
    struct resource_audio_effect_t *(*fork)(
        struct resource_audio_effect_t *self);

    receiver_t *eos_rcv;
    bytecode_stream_t *eos_msg;
} resource_audio_effect_t;

CLASS_INHERIT(resource_audio_effect, audio_effect)

#endif
