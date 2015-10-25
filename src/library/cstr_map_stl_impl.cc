/* a10 26
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/cstr_map_stl_impl.cc') with
 *a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 26 */

extern "C" {
#define new kn_new
#define delete kn_delete
#include <library/cstr_map_stl_impl.h>
#undef new
#undef delete
}

#if defined(__GNUC__) && __GNUC__ <= 2
#include <hash_map>
#else
#include <ext/hash_map>
#endif

struct eqstr {
    bool operator()(const char *s1, const char *s2) const
    {
        return strcmp(s1, s2) == 0;
    }
};

#if defined(__GNUC__) && __GNUC__ >= 3
#define HASH_MAP_NS __gnu_cxx
#elif defined(__MSVC__)
#if _MSC_VER >= 1300
#define HASH_MAP_NS stdext
#else
#define HASH_MAP_NS std
#endif
#elif defined(__GNUC__) && __GNUC__ == 2
#define HASH_MAP_NS std
#else
#error please define the namespace for hash_map
#endif

typedef HASH_MAP_NS::hash_map<const char *, void *,
                              HASH_MAP_NS::hash<const char *>,
                              eqstr> our_hash_map_t;
typedef our_hash_map_t::iterator our_hash_map_iterator_t;
typedef struct iterator_state_t {
    our_hash_map_iterator_t iterator;
    const char *previous_key;
} iterator_state_t;

int cstr_map_new(cstr_map_t *self)
{
    self->impl = new our_hash_map_t();

    return 1;
}

int cstr_map_destroy(cstr_map_t *self)
{
    our_hash_map_t *map = (our_hash_map_t *)self->impl;
    if (map) {
        delete map;
        map = NULL;
    }

    return 1;
}

void cstr_map_put(cstr_map_t *self, const char *key, void *value)
{
    our_hash_map_t *map = (our_hash_map_t *)self->impl;
    (*map)[key] = value;
}

cstr_map_value_t cstr_map_get(cstr_map_t *self, const char *key)
{
    our_hash_map_t *map = (our_hash_map_t *)self->impl;
    our_hash_map_iterator_t it = map->find(key);
    cstr_map_value_t value = {0, 0};
    value.is_there_p = it != map->end();
    if (value.is_there_p)
        value.value = (*it).second;
    return value;
}

int cstr_map_delete(cstr_map_t *self, const char *key)
{
    our_hash_map_t *map = (our_hash_map_t *)self->impl;

    map->erase(key);

    return 1;
}

cstr_map_iterator_t *cstr_map_get_iterator(cstr_map_t *self,
                                           cstr_map_iterator_t *iterator)
{
    our_hash_map_t *map = (our_hash_map_t *)self->impl;
    cstr_map_iterator_t *it = cstr_map_iterator_instantiate_toplevel(iterator);
    it->map = self;
    iterator_state_t *state =
        (iterator_state_t *)calloc(sizeof(iterator_state_t), 1);
    state->iterator = map->begin();
    state->previous_key = NULL;
    it->impl = state;

    return it;
}

int cstr_map_iterator_destroy(cstr_map_iterator_t *self)
{
    free(self->impl);

    return 1;
}

cstr_map_value_t cstr_map_iterator_next(cstr_map_iterator_t *self)
{
    our_hash_map_t *map = (our_hash_map_t *)(self->map->impl);
    iterator_state_t *state = (iterator_state_t *)self->impl;
    cstr_map_value_t v;

    if (state->iterator != map->end()) {
        v.is_there_p = 1;
        v.value = (*state->iterator).second;
        state->previous_key = (*state->iterator).first;
        state->iterator++;
    } else {
        v.is_there_p = 0;
        v.value = NULL;
    }

    return v;
}
const char *cstr_map_iterator_get_key(cstr_map_iterator_t *self)
{
    iterator_state_t *state = (iterator_state_t *)self->impl;
    return state->previous_key;
}

int cstr_map_value_is_there(cstr_map_stl_value_t value)
{
    return value.is_there_p;
}

void *cstr_map_value_obtain(cstr_map_stl_value_t value) { return value.value; }
