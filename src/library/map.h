/* a10 301
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 301 */

/*
  a map object,
  mapping unsigned long _to_ void*

  here implemented using JudyL digital trees
*/

#ifndef KNOS_LIBRARY_MAP_H
#define KNOS_LIBRARY_MAP_H

#if defined(MAP_JUDY_IMPL)
#include "map_judy.h"

typedef map_judy_impl_t map_impl_t;
typedef map_iterator_judy_impl_t map_iterator_impl_t;
typedef map_judy_value_t map_value_t;

#elif defined(MAP_GLIB_IMPL)
#include "map_glib.h"

typedef map_glib_impl_t map_impl_t;
typedef map_iterator_glib_impl_t map_iterator_impl_t;
typedef map_glib_value_t map_value_t;

#elif defined(MAP_STL_IMPL)
#include "map_stl.h"

typedef map_stl_impl_t map_impl_t;
typedef map_iterator_stl_impl_t map_iterator_impl_t;
typedef map_stl_value_t map_value_t;

#else
#error no implementation found for map_t
#endif

#include <library/memory.h>

typedef struct map_t {
    object_t super;

    map_impl_t impl;

    void (*put)(struct map_t *self, unsigned long index, void *value);
    map_value_t (*get)(struct map_t *self, unsigned long index);
    unsigned long (*get_count)(struct map_t *self);
    map_value_t (*get_by_count)(struct map_t *self, unsigned long index);
    int (*delete)(struct map_t *self, unsigned long index);
    void (*push)(struct map_t *self, void *value);
    int (*pop)(struct map_t *self, void **ret);
    int (*destroy)(struct map_t *self);
    struct map_iterator_t *(*get_iterator)(struct map_t *self,
                                           struct map_iterator_t *x);
} map_t;

CLASS_INHERIT(map, object);

typedef struct map_iterator_t {
    object_t super;
    map_t *map;
    map_iterator_impl_t impl;
} map_iterator_t;

CLASS_INHERIT(map_iterator, object);

void *map_value_obtain(map_value_t value);
int map_value_is_there(map_value_t value);

#endif
