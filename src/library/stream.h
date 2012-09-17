/* a10 114
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stream.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 114 */



#ifndef KNOS_LIBRARY_STREAM_H
#define KNOS_LIBRARY_STREAM_H

#include <libc/stdint.h>
#include <libc/stdio.h>

#include <library/memory.h>

struct stream_t;

typedef struct stream_callbacks_t 
{
    size_t (*read)  (void* data, size_t size, size_t nmemb, struct stream_t* self);
    size_t (*write) (const void* data, size_t size, size_t nmemb, struct stream_t* self);
    int    (*seek)  (struct stream_t *self, int64_t offset, int whence);
    int    (*close) (struct stream_t *self);
    long   (*tell)  (struct stream_t *self);
    int    (*error_p)(struct stream_t *self);
    void   (*clear_error)(struct stream_t* self);
    int    (*get_length)(struct stream_t* self);
} stream_callbacks_t;

typedef struct stream_t
{
    object_t super;
    stream_callbacks_t callbacks;

    char* (*get_as_memory_area)(struct stream_t* self, int64_t* returned_size);
    void (*free_memory_area)(struct stream_t* self, char* buffer, int64_t size);
    /*
      allocates a new stream object "cloning" the access to the current stream.
     */
    struct stream_t* (*fork)(struct stream_t* self);
    
    int closed_p;
} stream_t;

CLASS_INHERIT(stream, object);

/* 
   the stdio interface, only FILE* being replaced by stream_t* 
   (beware the order of the fread arguments)

   the callbacks here are trying to be compatible with vorbisfile.h' callbacks
*/

static inline
stream_callbacks_t* stream_get_callbacks(stream_t* s)
{
  return &s->callbacks;
}

/*
  copy the content of a stream in another one.
*/
extern void stream_copy (stream_t* dst, stream_t* src);

#endif
