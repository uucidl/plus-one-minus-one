/* a10 4
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map_impl.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 4 */

/*
  inline implementation
*/

#ifndef KNOS_LIBRARY_CSTR_MAP_IMPL_H
#define KNOS_LIBRARY_CSTR_MAP_IMPL_H

#include <library/cstr_map.h>

#if defined(CSTR_MAP_JUDY_IMPL)
#include <library/cstr_map_judy_impl.h>
#elif defined(CSTR_MAP_GLIB_IMPL)
#include <library/cstr_map_glib_impl.h>
#elif defined(CSTR_MAP_STL_IMPL)
#include <library/cstr_map_stl_impl.h>
#else
#error missing implementation
#endif

#endif
