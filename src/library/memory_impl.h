/* a10 423
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/memory_impl.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 423 */

#include <library/memory.h>

#include <log4c.h>

/*
  Returns the current allocator in context.
*/
static inline allocator_t *memory_allocator() { return get_top_allocator(); }

/*
  allocate size byte with the current allocator
*/
static inline void *memory_allocate(size_t size)
{
    allocator_t *a = get_top_allocator();

    void *ptr = a->allocate(a, size);

    if (ptr == NULL) {
        WARNING1("Not enough memory left!");
    }

    return ptr;
}

/*
  Free ptr allocated by the current allocator.
  It is advised to use "fat pointers" (pointers to object_t) instead of
  memory_free
  when the pointer is going to leave the current allocation context,
  (as defined by a
    MANAGE_MEMORY_BY(...) {
    ...
    } END_MEMORY;
  sequence) since fat pointers keep the knowledge of how to cleanup themselves.
*/
static inline void memory_free(void *ptr)
{
    allocator_t *a = get_top_allocator();
    a->retire(a, ptr);
}
