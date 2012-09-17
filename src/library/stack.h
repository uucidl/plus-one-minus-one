/* a10 767
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stack.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 767 */



#ifndef KNOS_LIBRARY_STACK_H
#define KNOS_LIBRARY_STACK_H

#include <library/memory.h>

#define STACK_ROPE_ELEM_N (128 / sizeof(void*))

typedef struct stack_rope_t
{
  object_t             super;
  void*                elems[STACK_ROPE_ELEM_N];
  unsigned int         next_n;
  struct stack_rope_t* next;
} stack_rope_t;

CLASS_INHERIT(stack_rope, object);

typedef struct kn_stack_t
{
    object_t      super;
    stack_rope_t* end;
    
    int   (*new)(struct kn_stack_t* self);
    int   (*destroy)(struct kn_stack_t* self);
    void  (*push)(struct kn_stack_t* self, void* obj);
    void* (*pop)(struct kn_stack_t* self);
    void* (*top)(struct kn_stack_t* self);
    void  (*dup)(struct kn_stack_t* self);
    void  (*xchg)(struct kn_stack_t* self);
    unsigned long (*count)(struct kn_stack_t* self);
} kn_stack_t;

CLASS_INHERIT(kn_stack, object)

#endif
