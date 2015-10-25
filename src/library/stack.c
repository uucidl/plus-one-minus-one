/* a10 731
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stack.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 731 */

#include <library/stack.h>
#include <library/stack_impl.h>

static int stack_new(kn_stack_t *self)
{
    /* so that self->end never be NULL */
    self->end = stack_rope_instantiate_toplevel(NULL);
    self->end->next = self->end;

    return 1;
}

static int stack_destroy(kn_stack_t *self)
{
    stack_rope_t *r, *old;
    r = self->end;

    if (r != NULL) {
        do {
            old = r;
            r = r->next;
            stack_rope_retire(old);
        } while (r != self->end);
    }

    self->end = NULL;

    return 1;
}

static void stack_ppush(kn_stack_t *self, void *obj) { stack_push(self, obj); }

static void *stack_ppop(kn_stack_t *self) { return stack_pop(self); }

static void *stack_ttop(kn_stack_t *self) { return stack_top(self); }

static void stack_dup(kn_stack_t *self) { stack_push(self, stack_top(self)); }

static void stack_xchg(kn_stack_t *self)
{
    void *a = stack_pop(self);
    void *b = stack_pop(self);
    stack_push(self, a);
    stack_push(self, b);
}

static unsigned long stack_ccount(kn_stack_t *self)
{
    return stack_count(self);
}

kn_stack_t *kn_stack_instantiate(kn_stack_t *x)
{
    kn_stack_t *s = kn_stack_instantiate_super(x);

    s->new = stack_new;
    s->destroy = stack_destroy;
    s->push = stack_ppush;
    s->pop = stack_ppop;
    s->top = stack_ttop;
    s->dup = stack_dup;
    s->xchg = stack_xchg;
    s->count = stack_ccount;

    return s;
}

stack_rope_t *stack_rope_instantiate(stack_rope_t *x)
{
    stack_rope_t *sr = stack_rope_instantiate_super(x);

    return sr;
}
