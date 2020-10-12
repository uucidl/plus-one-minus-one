/* a10 326
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/progress.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 326 */

#include "progress.h"

#include "logging.h"
LOG_NEW_DEFAULT_CATEGORY(KNOS_AUDIO_PROGRESS);

#ifdef NDEBUG

#define PRECONDITIONS_ASSERT_CURRENT_ISNT_NOTHING(self)
#define PRECONDITIONS_ASSERT_STATE_ISNT_FINISHED(self)

#else

#define PRECONDITIONS_ASSERT_CURRENT_ISNT_NOTHING(self)                        \
    do {                                                                       \
        progress_t *_self = (self);                                            \
        if (MAYBE_IS_NOTHING(&_self->current_n)) {                             \
            ERROR("current_n was advanced even though it was nothing!");      \
        }                                                                      \
    } while (0);

#define PRECONDITIONS_ASSERT_STATE_ISNT_FINISHED(self)                         \
    do {                                                                       \
        progress_t *_self = (self);                                            \
        if (_self->get_state(_self) == FINISHED) {                             \
            ERROR("cannot change state of a FINISHED progress!");             \
        } else if (_self->get_state(_self) == ABORTED) {                       \
            ERROR("cannot change state of an ABORTED progress!");             \
        }                                                                      \
    } while (0);

#endif

static state_t progress_get_state(progress_t *self)
{
    return atomic_load(&self->status);
}

static maybe_size_t progress_get_max_n(progress_t *self)
{
    maybe_size_t result;

    MAYBE_INITIALIZE(&result);

    /*
      not atomic
     */
    if (!MAYBE_IS_NOTHING(&self->max_n)) {
        /*
           atomic
        */
        intptr_t value = atomic_load(MAYBE_REF_LVALUE(&self->max_n));
        MAYBE_SET_VALUE(&result, value);
    }

    return result;
}

static maybe_size_t progress_get_current_n(progress_t *self)
{
    maybe_size_t result;

    MAYBE_INITIALIZE(&result);

    /*
      not atomic
     */
    if (!MAYBE_IS_NOTHING(&self->current_n)) {
        /*
           atomic
        */
        intptr_t value = atomic_load(MAYBE_REF_LVALUE(&self->current_n));
        MAYBE_SET_VALUE(&result, value);
    }

    return result;
}

static void progress_set_state(progress_t *self, state_t st)
{
    PRECONDITIONS_ASSERT_STATE_ISNT_FINISHED(self);

    TRACE("setting state: %d\n", st);

    atomic_store(&self->status, st);
}

static void progress_set_max_n(progress_t *self, maybe_size_t n)
{
    int nothing_p = MAYBE_IS_NOTHING(&n);

    PRECONDITIONS_ASSERT_STATE_ISNT_FINISHED(self);

    if (!MAYBE_SET_IS_NOTHING(&self->max_n, nothing_p)) {
        atomic_store(MAYBE_REF_LVALUE(&self->max_n), MAYBE_GET_VALUE(&n));
    }
}

static void progress_set_current_n(progress_t *self, maybe_size_t n)
{
    int nothing_p = MAYBE_IS_NOTHING(&n);

    PRECONDITIONS_ASSERT_STATE_ISNT_FINISHED(self);

    if (!MAYBE_SET_IS_NOTHING(&self->current_n, nothing_p)) {
        atomic_store(MAYBE_REF_LVALUE(&self->current_n),
                          MAYBE_GET_VALUE(&n));
    }
}

static void progress_advance_current_n(progress_t *self, size_t by)
{
    atomic_fetch_add(MAYBE_REF_LVALUE(&self->current_n), by);
    PRECONDITIONS_ASSERT_CURRENT_ISNT_NOTHING(self);
}

progress_t *progress_instantiate(progress_t *x)
{
    progress_t *p = progress_instantiate_super(x);

    MAYBE_INITIALIZE(&p->current_n);
    MAYBE_INITIALIZE(&p->max_n);

    p->get_state = progress_get_state;
    p->get_max_n = progress_get_max_n;
    p->get_current_n = progress_get_current_n;
    p->set_state = progress_set_state;
    p->set_max_n = progress_set_max_n;
    p->set_current_n = progress_set_current_n;
    p->advance_current_n = progress_advance_current_n;

    return p;
}
