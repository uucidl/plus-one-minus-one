/* a10 262
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 262 */




#include <library/map.h>

#include <libc/string.h>
#include <library/map_impl.h>

map_t* map_instantiate(map_t* x)
{
    map_t* m = map_instantiate_super (x);

    map_new(m);

    m->put	    = map_put;
    m->get	    = map_get;
    m->get_count    = map_get_count;
    m->get_by_count = map_get_by_count;
    m->delete	    = map_delete;
    m->push	    = map_push;
    m->pop	    = map_pop;
    m->destroy	    = map_destroy;
    m->get_iterator = map_get_iterator;

    return m;
}

map_iterator_t* map_iterator_instantiate(map_iterator_t* x)
{
    map_iterator_t* it = map_iterator_instantiate_super (x);

    return it;
}

void* map_value_obtain (map_value_t value) {
    return inline_map_value_obtain (value);
}

int map_value_is_there (map_value_t value) {
    return inline_map_value_is_there (value);
}
