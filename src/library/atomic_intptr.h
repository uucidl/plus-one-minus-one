/* a10 107
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/atomic_intptr.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 107 */

#ifndef KNOS_LIBRARY_ATOMIC_INTPTR_H
#define KNOS_LIBRARY_ATOMIC_INTPTR_H

#include <library/memory.h>
#include <libc/stdint.h>

/*
  An atomic integer or pointer value. Beware than on some
  architectures, the full bitrange is not preserved and only 24bit are
  preserved.  So make sure you are ok with that restriction, and can
  tweak the values a bit.

  All the operations defined herein except atomic_instantiate are
  guaranteed to be atomic in respect to their access to the actual
  value of the atomic_intptr_t object.
*/

#if defined(LINUX) || defined(MACOSX)

#if defined(i386) || defined(PENTIUM) || defined(PENTIUMPRO) || defined(K8)
#include <third-party/gpl/atomic-i386.h>
#elif defined(POWERPC)
#define __KERNEL__
#include <third-party/gpl/atomic-ppc.h>
#undef __KERNEL__
#endif

#endif

/*
  define platform dependant code.
 */
#ifdef LINUX

typedef atomic_t atomic_intptr_impl_t;

static inline void atomic_intptr_impl_instantiate(atomic_intptr_impl_t *x)
{
    atomic_t temp = ATOMIC_INIT(0);
    *x = temp;
}

static inline intptr_t atomic_intptr_impl_get(atomic_intptr_impl_t *x)
{
    return atomic_read(x);
}

static inline void atomic_intptr_impl_set(atomic_intptr_impl_t *x, intptr_t i)
{
    atomic_set(x, i);
}

static inline void atomic_intptr_impl_add(atomic_intptr_impl_t *x,
                                          intptr_t incr)
{
    atomic_add(incr, x);
}

static inline void atomic_intptr_impl_sub(atomic_intptr_impl_t *x,
                                          intptr_t decr)
{
    atomic_sub(decr, x);
}

#elif defined(WIN32)

#include <windows.h>

typedef LONG atomic_intptr_impl_t;

static inline void atomic_intptr_impl_instantiate(atomic_intptr_impl_t *x)
{
    // no op
}

static inline intptr_t atomic_intptr_impl_get(atomic_intptr_impl_t *x)
{
    return (*x);
}

static inline void atomic_intptr_impl_set(atomic_intptr_impl_t *x, intptr_t i)
{
    InterlockedExchange(x, i);
}

static inline void atomic_intptr_impl_add(atomic_intptr_impl_t *x,
                                          intptr_t incr)
{
    InterlockedExchangeAdd(x, incr);
}

static inline void atomic_intptr_impl_sub(atomic_intptr_impl_t *x,
                                          intptr_t decr)
{
    InterlockedExchangeAdd(x, -decr);
}

#elif defined(MACOSX)

typedef atomic_t atomic_intptr_impl_t;

static inline void atomic_intptr_impl_instantiate(atomic_intptr_impl_t *x)
{
    atomic_t temp = ATOMIC_INIT(0);
    *x = temp;
}

static inline intptr_t atomic_intptr_impl_get(atomic_intptr_impl_t *x)
{
    return atomic_read(x);
}

static inline void atomic_intptr_impl_set(atomic_intptr_impl_t *x, intptr_t i)
{
    atomic_set(x, i);
}

static inline void atomic_intptr_impl_add(atomic_intptr_impl_t *x,
                                          intptr_t incr)
{
    atomic_add(incr, x);
}

static inline void atomic_intptr_impl_sub(atomic_intptr_impl_t *x,
                                          intptr_t decr)
{
    atomic_sub(decr, x);
}

#elif PS2

#include <third-party/gpl/atomic-mips.h>

typedef atomic_t atomic_intptr_impl_t;

static inline void atomic_intptr_impl_instantiate(atomic_intptr_impl_t *x)
{
    atomic_t temp = ATOMIC_INIT(0);
    *x = temp;
}

static inline intptr_t atomic_intptr_impl_get(atomic_intptr_impl_t *x)
{
    return atomic_read(x);
}

static inline void atomic_intptr_impl_set(atomic_intptr_impl_t *x, intptr_t i)
{
    atomic_set(x, i);
}

static inline void atomic_intptr_impl_add(atomic_intptr_impl_t *x,
                                          intptr_t incr)
{
    atomic_add(incr, x);
}

static inline void atomic_intptr_impl_sub(atomic_intptr_impl_t *x,
                                          intptr_t decr)
{
    atomic_sub(decr, x);
}

#else
#error "Please implement atomic primitives for that architecture / processor"
#endif

typedef struct atomic_intptr_t {
    object_t super;
    atomic_intptr_impl_t impl;
} atomic_intptr_t;

CLASS_INHERIT(atomic_intptr, object);

static inline intptr_t atomic_intptr_get(atomic_intptr_t *x)
{
    return atomic_intptr_impl_get(&x->impl);
}

static inline void atomic_intptr_set(atomic_intptr_t *x, intptr_t i)
{
    atomic_intptr_impl_set(&x->impl, i);
}

static inline void atomic_intptr_add(atomic_intptr_t *x, intptr_t incr)
{
    atomic_intptr_impl_add(&x->impl, incr);
}

static inline void atomic_intptr_sub(atomic_intptr_t *x, intptr_t decr)
{
    atomic_intptr_impl_sub(&x->impl, decr);
}

#endif
