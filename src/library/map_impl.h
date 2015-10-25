/* a10 909
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map_impl.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 909 */

/*
  inlines for map
*/

#ifndef KNOS_LIBRARY_MAP_IMPL_H
#define KNOS_LIBRARY_MAP_IMPL_H

#include <library/map.h>

#if defined(MAP_JUDY_IMPL)
#include <library/map_judy_impl.h>
#elif defined(MAP_GLIB_IMPL)
#include <library/map_glib_impl.h>
#elif defined(MAP_STL_IMPL)
#include <library/map_stl_impl.h>
#else
#error implement the map_t!
#endif

#endif
