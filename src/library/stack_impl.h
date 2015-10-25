/* a10 616
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stack_impl.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 616 */

#include <library/stack.h>
#include <library/stack_rope_impl.h>

static inline void stack_push(kn_stack_t *self, void *obj)
{
    if (self->end == NULL) {
        self->end = stack_rope_instantiate_toplevel(NULL);
        self->end->next = self->end;
    }

    {
        stack_rope_t *first = self->end->next;
        stack_rope_t *r = self->end;

        if (stack_rope_is_full(r)) {
            r = stack_rope_instantiate_toplevel(NULL);
            r->next = first;
            self->end->next = r;
            self->end = r;
        }

        stack_rope_push(r, obj);
    }
}

static inline void *stack_pop(kn_stack_t *self)
{
    stack_rope_t *r = self->end;
    void *obj = NULL;

    if (r != NULL) {
        obj = stack_rope_pop(r);

        if (stack_rope_is_empty(r)) {
            stack_rope_t *first = r->next;
            stack_rope_t *prev;

            for (prev = r; prev->next != r; prev = prev->next)
                ;

            if (prev != r) {
                prev->next = first;
            } else {
                self->end = NULL;
            }

            stack_rope_retire(r);
        }
    }

    return obj;
}

static inline void *stack_top(kn_stack_t *self)
{
    return self->end ? stack_rope_top(self->end) : NULL;
}

static inline unsigned long stack_count(kn_stack_t *self)
{
    unsigned long count = 0L;

    if (self->end != NULL) {
        stack_rope_t *r;
        for (r = self->end->next; r != self->end; r = r->next) {
            count += r->next_n;
        }
        count += self->end->next_n;
    }

    return count;
}
