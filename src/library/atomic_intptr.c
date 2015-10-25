/* a10 353
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/atomic_intptr.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 353 */

#include <library/atomic_intptr.h>

atomic_intptr_t *atomic_intptr_instantiate(atomic_intptr_t *x)
{
    atomic_intptr_t *atomic = atomic_intptr_instantiate_super(x);

    atomic_intptr_impl_instantiate(&x->impl);

    return atomic;
}
