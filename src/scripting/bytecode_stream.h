/* a10 337
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/bytecode_stream.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 337 */



#ifndef KNOS_SCRIPTING_BYTECODE_STREAM_H
#define KNOS_SCRIPTING_BYTECODE_STREAM_H

#include "bytecode.h"
#include <library/memory.h>

typedef struct bytecode_iterator_t 
{
    object_t super;
    bytecode_t* (*next)(struct bytecode_iterator_t* self);
    bytecode_t* (*prev)(struct bytecode_iterator_t* self);

    struct bytecode_stream_t* parent;
    struct bytecode_node_t* current;
} bytecode_iterator_t;

CLASS_INHERIT(bytecode_iterator, object)

typedef struct bytecode_stream_t {
    object_t super;
    struct bytecode_node_t* end;
    
    int (*new)(struct bytecode_stream_t* self);
    /*
      deprecated, it is done automatically when retiring the bytecode_stream (use object_t::release)
    */
    int (*destroy)(struct bytecode_stream_t* self);

    struct bytecode_stream_t* (*copy)(struct bytecode_stream_t* self,
				      struct bytecode_stream_t* src);

    unsigned int (*get_count)(struct bytecode_stream_t* self);

    /* 
       Append the node passed to it to the end of the stream. The
       pointer is used as-is.
    */
    struct bytecode_stream_t* (*append)(struct bytecode_stream_t* self, 
					struct bytecode_node_t* n);
    struct bytecode_stream_t* (*append_atom)(struct bytecode_stream_t* self, 
					     atom_t a);
    struct bytecode_stream_t* (*append_integer)(struct bytecode_stream_t* self, 
						int i);
    struct bytecode_stream_t* (*append_float)(struct bytecode_stream_t* self, 
					      float f);

    /* 
       Prepend the node passed to it to the beginning of the stream. The
       pointer is used as-is.
    */
    struct bytecode_stream_t* (*prepend)(struct bytecode_stream_t* self, 
					 struct bytecode_node_t* n);
    struct bytecode_stream_t* (*prepend_atom)(struct bytecode_stream_t* self, 
					      atom_t a);
    struct bytecode_stream_t* (*prepend_integer)(struct bytecode_stream_t* self, 
						 int i);
    struct bytecode_stream_t* (*prepend_float)(struct bytecode_stream_t* self, 
					       float f);
    
    /*
      Pops the bytecode_t currently on top of the stack | end of the stream
    */
    bytecode_t* (*pop)(struct bytecode_stream_t* self);

    /*
      Pushes a bytecode_t on top of the stack | end of the stream.
      The data pointed to is copied.
    */
    void (*push)(struct bytecode_stream_t* self, bytecode_t* code);

    bytecode_iterator_t* (*get_iterator)(struct bytecode_stream_t* self, 
					 bytecode_iterator_t* x);
    /* returns an iterator starting from the end */
    bytecode_iterator_t* (*get_iterator_end)(struct bytecode_stream_t* self, 
					     bytecode_iterator_t* x);
} bytecode_stream_t;

CLASS_INHERIT(bytecode_stream, object)

typedef struct bytecode_node_t
{
    object_t super;
    bytecode_t code;
    struct bytecode_node_t* next;
    struct bytecode_node_t* prev;
} bytecode_node_t;

CLASS_INHERIT(bytecode_node, object);

#endif
