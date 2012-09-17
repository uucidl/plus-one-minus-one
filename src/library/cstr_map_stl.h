/* a10 666
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map_stl.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 666 */



#ifndef KNOS_LIBRARY_CSTR_MAP_STL_H
#define KNOS_LIBRARY_CSTR_MAP_STL_H

/* 
   define implementation as void pointers since they are 
   C++ classes.
*/

typedef void* cstr_map_stl_impl_t;
typedef void* cstr_map_iterator_stl_impl_t;
typedef struct { int is_there_p; void* value; } cstr_map_stl_value_t;
#define NULL_CSTR_MAP_VALUE { 0, NULL }

#endif
