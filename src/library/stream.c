/* a10 788
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stream.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 788 */




#include <library/stream.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_STREAM);

#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

static
char* stream_get_as_memory_area(stream_t* self, int64_t* returned_size)
{
    ERROR1("get_as_memory_area unsupported.");
    
    *returned_size = 0;
    return NULL;
}

static
void stream_free_memory_area(stream_t* self, char* buffer, int64_t returned_size)
{
    ERROR1("free_memory_area unsupported.");
}

static
int stream_get_length(stream_t* self)
{
    ERROR1("get_length not supported.");

    return -1;
}

static
stream_t* stream_fork (stream_t* self)
{
    ERROR1 ("fork not supported.");

    return NULL;
}

static
int stream_release (object_t* zelf) {
	stream_t* self = (stream_t*) zelf;
	
	stream_get_callbacks(self)->close (self);
	
	return 1;
}

extern 
void stream_copy (stream_t* dst, stream_t* src) {
	char buffer [256*1024];
	unsigned int n = 0;

	while ( (n = stream_get_callbacks(src)->read (buffer, 1, sizeof(buffer), src)) > 0) {

		unsigned int dn = n;
		unsigned int d = 0;
		while ( (d = stream_get_callbacks(dst)->write (buffer, 1, dn, dst)) > 0 && (dn = dn - d) > 0);
	}
}

stream_t* stream_instantiate(stream_t* x)
{
    stream_t* s = stream_instantiate_super (x);

    object_set_release_callback (stream_to_object(s), stream_release);

    s->get_as_memory_area = stream_get_as_memory_area;
    s->free_memory_area = stream_free_memory_area;
    stream_get_callbacks(s)->get_length = stream_get_length;

    s->fork = stream_fork;

    return s;
}

