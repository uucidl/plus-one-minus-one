/* a10 422
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/vector_impl.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 422 */

#include <library/vector.h>

#include <libc/stdlib.h>
#include <libc/string.h> /* for memset */

static inline unsigned int get_element_count(vector_t *v)
{
    return v->next_element;
}

/* returns pointer to location of element */
static inline void *get_element(vector_t *v, unsigned int i)
{
    if (i < v->next_element) {
        unsigned int real_i = i * v->element_size;
        return (void *)&v->buffer[real_i];
    } else {
        return NULL;
    }
}

/* returns pointer to location of next element */
static inline void *add_elements(vector_t *v, unsigned int count)
{
    void *element;

    unsigned int target_count = v->next_element + count;

    if (target_count && target_count >= v->element_n) {
        unsigned int old_n = v->element_n;
        v->element_n += (1 + target_count / v->chunk_size) * v->chunk_size;

        v->buffer = realloc(v->buffer, v->element_n * v->element_size);
        memset(v->buffer + old_n * v->element_size, 0,
               (v->element_n - old_n) * v->element_size);
    }

    element = &v->buffer[v->next_element * v->element_size];

    v->next_element += count;

    return element;
}

/* returns pointer to location of new element */
static inline void *add_element(vector_t *v) { return add_elements(v, 1); }

typedef struct iterator_t {
    vector_t *v;
    unsigned int i;
    unsigned int n;
} iterator_t;

/* sets up an iterator */
static inline void get_iterator(vector_t *v, iterator_t *i)
{
    i->v = v;
    i->i = 0;
    i->n = v->next_element * v->element_size;
}

static inline void *iterator_next(iterator_t *it)
{
    void *ret = NULL;

    if (it->i < it->n) {
        ret = &it->v->buffer[it->i];
        it->i += it->v->element_size;
    }

    return ret;
}
