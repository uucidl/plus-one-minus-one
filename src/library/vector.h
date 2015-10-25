/* a10 589
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/vector.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 589 */

#ifndef KNOS_LIBRARY_VECTOR_H
#define KNOS_LIBRARY_VECTOR_H

#include <library/memory.h>

typedef struct vector_t {
    object_t super;

    char *buffer;

    unsigned int element_size;
    unsigned int next_element;
    unsigned int element_n;
    unsigned int chunk_size;
} vector_t;

CLASS_INHERIT(vector, object);

vector_t *vector_new_2(vector_t *x, unsigned int element_size,
                       unsigned int chunk_size);

#define vector_new(v, element) vector_new_2(v, sizeof(element), 1)

#endif
