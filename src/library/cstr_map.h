/* a10 203
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 203 */

/*
  a map object,
  mapping c strings _to_ void*

  here implemented using JudySL digital trees
*/
#ifndef KNOS_LIBRARY_CSTR_MAP_H
#define KNOS_LIBRARY_CSTR_MAP_H

#if defined(CSTR_MAP_JUDY_IMPL)
#include <library/cstr_map_judy.h>
typedef cstr_map_judy_impl_t cstr_map_impl_t;
typedef cstr_map_iterator_judy_impl_t cstr_map_iterator_impl_t;
typedef cstr_map_judy_value_t cstr_map_value_t;
#elif defined(CSTR_MAP_GLIB_IMPL)
#include <library/cstr_map_glib.h>
typedef cstr_map_glib_impl_t cstr_map_impl_t;
typedef cstr_map_iterator_glib_impl_t cstr_map_iterator_impl_t;
typedef cstr_map_glib_value_t cstr_map_value_t;
#elif defined(CSTR_MAP_STL_IMPL)
#include <library/cstr_map_stl.h>
typedef cstr_map_stl_impl_t cstr_map_impl_t;
typedef cstr_map_iterator_stl_impl_t cstr_map_iterator_impl_t;
typedef cstr_map_stl_value_t cstr_map_value_t;
#else
#error no implementation found for cstr_map_t
#endif

#include <library/memory.h>

typedef struct cstr_map_t {
    object_t super;

    cstr_map_impl_t impl;

    int (*new)(struct cstr_map_t *self);
    int (*destroy)(struct cstr_map_t *self);
    void (*put)(struct cstr_map_t *self, const char *key, void *value);
    cstr_map_value_t (*get)(struct cstr_map_t *self, const char *key);
    int (*delete)(struct cstr_map_t *self, const char *key);
    struct cstr_map_iterator_t *(*get_iterator)(
        struct cstr_map_t *self, struct cstr_map_iterator_t *old);
} cstr_map_t;

CLASS_INHERIT(cstr_map, object);

typedef struct cstr_map_iterator_t {
    object_t super;
    cstr_map_t *map;

    cstr_map_iterator_impl_t impl;

    int (*destroy)(struct cstr_map_iterator_t *self);
    cstr_map_value_t (*next)(struct cstr_map_iterator_t *self);
    const char *(*get_key)(struct cstr_map_iterator_t *self);

} cstr_map_iterator_t;

CLASS_INHERIT(cstr_map_iterator, object);

#endif
