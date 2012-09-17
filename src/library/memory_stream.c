/* a10 888
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/memory_stream.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 888 */




#include <library/memory_stream.h>

#include <libc/stdlib.h>
#include <libc/string.h>

static
char* memory_get_as_memory_area(stream_t* self, int64_t* returned_size)
{
    memory_stream_t* m = (memory_stream_t*) self;

    *returned_size = m->size;
    return m->buffer;
}

static
void memory_free_memory_area(stream_t* self, char* buffer, int64_t returned_size)
{
    // do nothing, it's our internal buffer
}

static
size_t memory_read(void* data, size_t size, size_t nmemb, struct stream_t* self)
{
    memory_stream_t* m = (memory_stream_t*) self;
    size_t s = size * nmemb;

    if(m->pos + s > m->size) {
	if((size_t) m->pos == m->size)
	    return 0;
	s = m->size - m->pos;
    }

    memcpy(data, &m->buffer[m->pos], s);

    m->pos += s;
    if((size_t) m->pos > m->maxpos)
	m->maxpos = m->pos;

    return s;
}

static
int memory_seek(struct stream_t *self, int64_t offset, int whence)
{
    memory_stream_t* m = (memory_stream_t*) self;
    int64_t np;

    switch(whence)
    {
    case SEEK_SET:
	np = offset;
	break;

    case SEEK_CUR:
	np = m->pos + offset;
	break;

    case SEEK_END:
	np = m->size - offset;
	break;

    default:
	return -1;
    }

    if(np < 0 || (size_t) np > m->size)
	return -1;

    m->pos = np;

    return np;
}

static
int memory_close(struct stream_t *self)
{
    memory_stream_t* m = (memory_stream_t*) self;

    if(m->buffer_owned_p && m->buffer) {
	free(m->buffer);
	m->buffer = NULL;
    }

    return 0;
}

static
long memory_tell(struct stream_t *self)
{
    memory_stream_t* m = (memory_stream_t*) self;

    return m->pos;
}

static
int memory_error_p(struct stream_t* self)
{
    return 0;
}

static
void memory_clear_error(struct stream_t* self)
{
    // nothing
}

memory_stream_t* memory_stream_instantiate(memory_stream_t* x)
{
    memory_stream_t* m = memory_stream_instantiate_super (x);

    m->super.get_as_memory_area = memory_get_as_memory_area;
    m->super.free_memory_area = memory_free_memory_area;

    m->super.callbacks.read        = memory_read;
    m->super.callbacks.seek        = memory_seek;
    m->super.callbacks.close       = memory_close;
    m->super.callbacks.tell        = memory_tell;
    m->super.callbacks.error_p     = memory_error_p;
    m->super.callbacks.clear_error = memory_clear_error;

    return m;
}

memory_stream_t* memory_open(memory_stream_t* x,
			     const char* buffer, const int64_t size, const char* mode)
{
    int buffer_owned_p = (buffer == NULL);

    return memory_open2(x, buffer_owned_p, buffer, size, mode);
}

memory_stream_t* memory_open2(memory_stream_t* x, int buffer_owned_p,
			      const char* buffer, const int64_t size, const char* mode)
{
    memory_stream_t* m = memory_stream_instantiate_toplevel(x);

    m->buffer_owned_p = buffer_owned_p;
    if(buffer == NULL) {
	if(size) {
	    m->buffer = (char *) malloc(size);
	    m->buffer[0] = '\0';
	} else
	    m->buffer = NULL;
	m->buffer_owned_p = 1;
    } else {
	m->buffer = (char*) buffer;
    }

    m->size = size;

    if (mode[0] == 'w')
	m->buffer[0] = '\0';

    m->maxpos = size;

    if (mode[0] == 'a')
	m->pos = m->maxpos;
    else
	m->pos = 0;

    return m;
}
