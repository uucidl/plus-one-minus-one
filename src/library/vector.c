/* a10 393
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/vector.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 393 */




#include <library/vector.h>

#include <libc/stdio.h>  /* for puts */

vector_t* vector_instantiate (vector_t* x) {
    vector_t* v = vector_instantiate_super (x);
    return v;
}

static inline
int vector_release (object_t* self) {
    vector_t* v = (vector_t*) self;

    if (v->buffer != NULL) {
	free (v->buffer);
	v->buffer = NULL;
    }

    return 1;
}

/* creates a vector */
vector_t* vector_new_2 (vector_t* x, 
			unsigned int element_size, unsigned int chunk_size)
{
    vector_t* v = vector_instantiate_toplevel (x);

    object_set_release_callback (vector_to_object (v), vector_release);

    if(!element_size || !chunk_size) {
	puts(__FILE__ ": element_size || chunk_size == 0");
	if(v != x)
	    free(v);
	
	return NULL;
    }

    v->element_size = element_size;
    v->chunk_size = chunk_size;

    return v;
}

