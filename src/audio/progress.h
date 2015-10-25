/* a10 428
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/progress.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 428 */

#ifndef KNOS_AUDIO_PROGRESS_H
#define KNOS_AUDIO_PROGRESS_H

#include <library/memory.h>
#include <library/maybe.h>
#include <libc/unistd.h> // for size_t

#include <library/atomic_intptr.h>

typedef MAYBE(atomic_intptr_t) maybe_counter_t;
typedef MAYBE(size_t) maybe_size_t;
typedef enum {
    STOPPED = 0,
    RUNNING = 1,
    WAITING = 2,
    FINISHED = 3,
    ABORTED = 4
} state_t;

/*

  A shared data structure that represents the status of one computation.

  After the state of progress is set to FINISHED or ABORTED, the progress object
  guarantees its content will not change in any way.
*/
typedef struct progress_t {
    object_t super;

    atomic_intptr_t status;
    maybe_counter_t max_n;
    maybe_counter_t current_n;

    state_t (*get_state)(struct progress_t *self);
    maybe_size_t (*get_max_n)(struct progress_t *self);
    maybe_size_t (*get_current_n)(struct progress_t *self);
    void (*set_state)(struct progress_t *self, state_t state);
    void (*set_max_n)(struct progress_t *self, maybe_size_t n);
    void (*set_current_n)(struct progress_t *self, maybe_size_t n);

    /* the method of choice to denote progress */
    void (*advance_current_n)(struct progress_t *self, size_t by);
} progress_t;

CLASS_INHERIT(progress, object);

#endif
