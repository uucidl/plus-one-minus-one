/* a10 806
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/memory_stream.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 806 */



#ifndef KNOS_LIBRARY_MEMORY_STREAM_H
#define KNOS_LIBRARY_MEMORY_STREAM_H

#include <library/stream.h>

typedef struct memory_stream_t
{
    stream_t super;

    char *buffer;
    int buffer_owned_p;
    size_t size;
    int64_t pos;
    size_t maxpos;
} memory_stream_t;

CLASS_INHERIT(memory_stream, stream);

/*
  if buffer is not null, it is not considered owned by the stream, and
  thus not freed when it is closed.
 */
memory_stream_t* memory_open(memory_stream_t* x, 
			     const char* buffer, const int64_t size, const char* mode);

/*
  stream_own_buffer_p : whether the stream frees the buffer when closed.
*/
memory_stream_t* memory_open2(memory_stream_t* x, int stream_own_buffer_p,
			      const char* buffer, const int64_t size, const char* mode);

#endif
