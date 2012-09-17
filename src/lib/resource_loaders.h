/* a10 317
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/resource_loaders.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 317 */



#include <library/memory.h>

#include <libc/stdio.h>
#include <library/stream.h>

#define RLOADER_MAX_PROTOCOLS_N 32

typedef stream_t* (*fopen_function)(const char* path, const char* mode);

typedef struct rloader_t
{
    object_t super;

    fopen_function (*get_fopen)(struct rloader_t* self, 
				const char* protocol);
    void (*set_fopen)(struct rloader_t* self, 
		      const char* protocol, const fopen_function f);

    const char*          protocols[RLOADER_MAX_PROTOCOLS_N];
    fopen_function       functions[RLOADER_MAX_PROTOCOLS_N];
    unsigned int         f_max;
    unsigned int         f_next;
} rloader_t;

CLASS_INHERIT (rloader, object);

/* instantiate resource loaders. if x i NULL, returns a singleton */
rloader_t* rloader_instantiate(rloader_t* x);
