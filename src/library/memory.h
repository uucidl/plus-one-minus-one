/* a10 420
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/memory.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 420 */



#ifndef KNOS_LIBRARY_MEMORY_H
#define KNOS_LIBRARY_MEMORY_H

#include <libc/stdlib.h>

#include <library/object_macros.h>

/*
  an object is a bounded, structured and managed area of memory
*/

typedef struct object_t
{
    /* magic number to mark allocated objects */
    unsigned short magic;

    /* the allocator for that particular object */
    struct allocator_t* allocator;

    /* optional method to release any memory allocated with this object or
       retire any subobjects. It will return !!1 on success. */
    int (*release) (struct object_t* o);

#ifndef NDEBUG
    const char* class_name;
    const char* source_file;
    int source_line;
#endif

    /* 
       result of sizeof, can be used to recopy the object (does work
       only if its flat of course.
    */
    int size;
} object_t;

/* 
   use to set up a callback that will be called once the object is retired.
*/
void object_set_release_callback (object_t* o, int (*f)(object_t*));

static inline object_t* object_to_object(object_t* x) {
    return x;
}

static inline object_t* object_instantiate (object_t* x) {
    return x;
}

#ifndef NDEBUG
object_t* object_allocate_debug(object_t* x, size_t size, const char* class_name, const char* filename, int line);
#endif
object_t* object_allocate(object_t* x, size_t size);
object_t* object_allocate_copy (object_t* from);
void object_retire(object_t* x);

typedef struct allocator_t
{
    object_t super;

    void* (*allocate)(struct allocator_t* self, size_t size);
    void  (*retire)(struct allocator_t* self, void* ptr);
} allocator_t;

CLASS_INHERIT(allocator, object)

allocator_t* libc_allocator_instantiate(allocator_t* x);
allocator_t* libc_get_allocator();

allocator_t* alibc_allocator_instantiate(allocator_t* x);
allocator_t* alibc_get_allocator();

allocator_t* noop_allocator_instantiate(allocator_t* x);
allocator_t* noop_get_allocator();

/*
  to enable control on the object allocators used during 
  this or that path of the code, a per-thread allocator stack
  is provided.
*/
void         push_allocator(allocator_t* a);
allocator_t* get_top_allocator();
allocator_t* pop_allocator();

#define MANAGE_MEMORY_BY(allocator) push_allocator(allocator);
#define END_MEMORY pop_allocator()
#define LIBC libc_get_allocator()
#define ALIGNED_LIBC alibc_get_allocator()

#endif
