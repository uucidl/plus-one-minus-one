/* a10 426
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 426 */




#include <library/cstr_map.h>
#include <library/cstr_map_impl.h>

cstr_map_t* cstr_map_instantiate(cstr_map_t* x)
{
    cstr_map_t* m = cstr_map_instantiate_super (x); 

    cstr_map_new(m);

    m->new	    = cstr_map_new;
    m->destroy	    = cstr_map_destroy;
    m->put	    = cstr_map_put;
    m->get	    = cstr_map_get;
    m->delete	    = cstr_map_delete;
    m->get_iterator = cstr_map_get_iterator;

    return m;
}

cstr_map_iterator_t* cstr_map_iterator_instantiate(cstr_map_iterator_t* x)
{
    cstr_map_iterator_t* it = cstr_map_iterator_instantiate_super (x);

    it->destroy = cstr_map_iterator_destroy;
    it->next    = cstr_map_iterator_next;
    it->get_key = cstr_map_iterator_get_key;

    return it;
}

