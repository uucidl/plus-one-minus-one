/* a10 46
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map_stl_impl.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 46 */

#ifndef KNOS_LIBRARY_CSTR_MAP_STL_IMPL_H
#define KNOS_LIBRARY_CSTR_MAP_STL_IMPL_H

#ifndef CSTR_MAP_STL_IMPL
#define CSTR_MAP_STL_IMPL
#endif

#include <library/cstr_map.h>

int cstr_map_new(cstr_map_t *self);
int cstr_map_destroy(cstr_map_t *self);
void cstr_map_put(cstr_map_t *self, const char *key, void *value);
cstr_map_value_t cstr_map_get(cstr_map_t *self, const char *key);
int cstr_map_delete(cstr_map_t *self, const char *key);
cstr_map_iterator_t *cstr_map_get_iterator(cstr_map_t *self,
                                           cstr_map_iterator_t *iterator);
int cstr_map_iterator_destroy(cstr_map_iterator_t *self);
cstr_map_value_t cstr_map_iterator_next(cstr_map_iterator_t *self);
const char *cstr_map_iterator_get_key(cstr_map_iterator_t *self);
int cstr_map_value_is_there(cstr_map_stl_value_t value);
void *cstr_map_value_obtain(cstr_map_stl_value_t value);

#endif
