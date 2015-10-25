/* a10 853
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map_stl.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 853 */

#ifndef KNOS_LIBRARY_MAP_STL_H
#define KNOS_LIBRARY_MAP_STL_H

/*
  C visible interface to the STL implementation of a map
*/

/*
  opaque pointer on the actual STL map and map iterator
*/
typedef void *map_stl_impl_t;
typedef void *map_iterator_stl_impl_t;
/* pointer to an iterator slot */
typedef struct {
    int is_there_p;
    void *value;
} map_stl_value_t;
#define NULL_MAP_VALUE                                                         \
    {                                                                          \
        0, NULL                                                                \
    }

#endif
