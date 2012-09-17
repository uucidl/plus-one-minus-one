/* a10 710
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map_stl_impl.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 710 */



#ifndef KNOS_LIBRARY_MAP_STL_IMPL_H

/*
  C++ part of the stl implementation of the map_t type.
*/
#ifdef __cplusplus
extern "C" {
#endif

#define delete kn_delete
#include <library/map.h>
#undef delete
    
    int map_new (map_t* self);
    void map_put (map_t* self, unsigned long index, void* value);
    map_value_t map_get (map_t* self, unsigned long index);
    unsigned long map_get_count (map_t* self);
    map_value_t map_get_by_count(map_t* self, unsigned long index);
    int map_delete(map_t* self, unsigned long index);
    void map_push (map_t* self, void* value);
    int map_pop (map_t* self, void** ret); 
    int map_destroy(map_t* self);
    map_iterator_t* map_get_iterator(map_t* self, map_iterator_t* iterator);
    map_iterator_t* map_get_iterator_first(map_t* self, map_iterator_t* iterator);
    map_iterator_t* map_get_iterator_last(map_t* self, map_iterator_t* iterator);
    int map_iterator_destroy (map_iterator_t* self);
    map_value_t map_iterator_next(map_iterator_t* self);
    map_value_t map_iterator_prev(map_iterator_t* self);
    unsigned long map_iterator_get_index(map_iterator_t* self);
    
    static inline
    void* inline_map_value_obtain (map_value_t value) {
	return value.value;
    }

    static inline
    int inline_map_value_is_there (map_value_t value) {
	return value.is_there_p;
    }

#ifdef __cplusplus
}
#endif

#endif
