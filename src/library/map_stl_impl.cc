/* a10 717
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/map_stl_impl.cc') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 717 */

/*
  C++ part of the stl implementation of the map_t type.
*/
#include <library/map_stl_impl.h>

#include <map>

typedef std::map<unsigned long, void *> our_map_t;

typedef struct iterator_state_t {
    our_map_t::iterator iterator;
    unsigned long previous_key;
    unsigned long index;
} iterator_state_t;

int map_new(map_t *self)
{
    self->impl = new our_map_t();

    return 1;
}

void map_put(map_t *self, unsigned long index, void *value)
{
    our_map_t *map = (our_map_t *)(self->impl);
    (*map)[index] = value;
}

map_value_t map_get(map_t *self, unsigned long index)
{
    our_map_t *map = (our_map_t *)(self->impl);
    our_map_t::iterator it = map->find(index);
    map_value_t v;

    v.is_there_p = it != map->end();
    v.value = v.is_there_p ? (*it).second : NULL;

    return v;
}

unsigned long map_get_count(map_t *self)
{
    our_map_t *map = (our_map_t *)(self->impl);

    return map->size();
}

map_value_t map_get_by_count(map_t *self, unsigned long index)
{
    our_map_t *map = (our_map_t *)(self->impl);
    unsigned long i = 0;
    our_map_t::iterator it;
    map_value_t v;

    for (it = map->begin(); i < index && it != map->end(); ++it) {
        i++;
    }

    v.is_there_p = it != map->end();
    v.value = v.is_there_p ? (*it).second : NULL;

    return v;
}

int map_delete(map_t *self, unsigned long index)
{
    our_map_t *map = (our_map_t *)(self->impl);

    return map->erase(index);
}

void map_push(map_t *self, void *value)
{
    our_map_t *map = (our_map_t *)(self->impl);
    if (map->empty()) {
        (*map)[0] = value;
    } else {
        unsigned long next_index = (*(--map->end())).first + 1;
        (*map)[next_index] = value;
    }
}

int map_pop(map_t *self, void **ret)
{
    our_map_t *map = (our_map_t *)(self->impl);

    if (map->empty()) {
        return 0;
    } else {
        our_map_t::iterator last_one = map->end();
        void *value;
        --last_one;

        if (last_one != map->end()) {
            value = (*last_one).second;
            map->erase(last_one);

            *ret = value;

            return 1;
        } else {
            return 0;
        }
    }
}

int map_destroy(map_t *self)
{
    our_map_t *map = (our_map_t *)(self->impl);

    delete map;

    return 1;
}

static map_iterator_t *__map_get_iterator(map_t *self, map_iterator_t *iterator)
{
    map_iterator_t *it = map_iterator_instantiate_toplevel(iterator);

    it->map = self;

    return it;
}

map_iterator_t *map_get_iterator(map_t *self, map_iterator_t *iterator)
{
    our_map_t *map = (our_map_t *)(self->impl);
    map_iterator_t *it = __map_get_iterator(self, iterator);

    iterator_state_t *state =
        (iterator_state_t *)calloc(sizeof(iterator_state_t), 1);
    state->iterator = map->begin();
    state->index = 0;
    it->impl = state;

    return it;
}

map_iterator_t *map_get_iterator_first(map_t *self, map_iterator_t *iterator)
{
    return map_get_iterator(self, iterator);
}

map_iterator_t *map_get_iterator_last(map_t *self, map_iterator_t *iterator)
{
    our_map_t *map = (our_map_t *)(self->impl);
    map_iterator_t *it = __map_get_iterator(self, iterator);

    iterator_state_t *state =
        (iterator_state_t *)calloc(sizeof(iterator_state_t), 1);
    state->iterator = map->end();
    state->index = map->size() - 1;
    it->impl = state;

    return it;
}

int map_iterator_destroy(map_iterator_t *self)
{
    iterator_state_t *state = (iterator_state_t *)self->impl;
    if (NULL != state) {
        free(state);
        state = NULL;
    }
    return 1;
}

map_value_t map_iterator_next(map_iterator_t *self)
{
    our_map_t *map = (our_map_t *)(self->map->impl);
    iterator_state_t *state = (iterator_state_t *)self->impl;
    map_value_t v;

    if (state->iterator != map->end()) {
        v.is_there_p = 1;
        v.value = (*state->iterator).second;
        state->previous_key = (*state->iterator).first;
        state->iterator++;
        state->index++;
    } else {
        v.is_there_p = 0;
        v.value = NULL;
    }

    return v;
}

map_value_t map_iterator_prev(map_iterator_t *self)
{
    our_map_t *map = (our_map_t *)(self->map->impl);
    iterator_state_t *state = (iterator_state_t *)self->impl;
    map_value_t v;

    if (!map->empty() && state->iterator != map->begin()) {
        state->iterator--;
        state->index--;
        v.is_there_p = 1;
        state->previous_key = (*state->iterator).first;
    } else {
        v.is_there_p = 0;
    }

    v.value = v.is_there_p ? (*state->iterator).second : NULL;

    return v;
}

unsigned long map_iterator_get_index(map_iterator_t *self)
{
    iterator_state_t *state = (iterator_state_t *)self->impl;
    return state->previous_key;
}
