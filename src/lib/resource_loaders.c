/* a10 1
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/resource_loaders.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 1 */




#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_LIB_RESOURCE_LOADERS);

#include "resource_loaders.h"

/* singleton */
rloader_t resource_loaders;
int resource_loaders_init_p = 0;

#include <libc/string.h>
#include <library/stdio_stream.h>

static
unsigned int find_slot(const char** strs, const unsigned int next, 
		       const unsigned int max, const char* str)
{
    unsigned int i; 
    for(i=0; i<next; i++) {
	if(!strcasecmp(strs[i], str)) {
	    break;
	}
    }
    return i;
}

static 
fopen_function res_get_fopen(rloader_t* self, const char* protocol)
{
    unsigned int i = find_slot(self->protocols, self->f_next, self->f_max,
			       protocol);
    if(i < self->f_next)
	return self->functions[i];
    else
	return NULL;
}


static
void res_set_fopen(struct rloader_t* self, 
		   const char* protocol, const fopen_function f)
{
    unsigned int i = find_slot(self->protocols, self->f_next, 
			       self->f_max, protocol);
    if(i >= self->f_max) 
	WARNING1(__FILE__ ": reached static limit in set_fopen.");
    else {
	if(i == self->f_next)
	    self->f_next++;

	self->protocols[i] = strdup(protocol);
	self->functions[i] = f;
    }
}

stream_t* stdio_fopen(const char* path, const char* mode)
{
    return (stream_t*) stdio_open(NULL, path, mode);
}

rloader_t* rloader_instantiate(rloader_t* x)
{
    rloader_t* l;

    if(x == NULL)
	l = &resource_loaders;
    else
	l = x;

    if (l != &resource_loaders ||
       !resource_loaders_init_p) {
	l = rloader_instantiate_super (l);

	l->get_fopen = res_get_fopen;
	l->set_fopen = res_set_fopen;
	l->f_max = RLOADER_MAX_PROTOCOLS_N;
	l->f_next = 0;

	// add default file protocol
	l->set_fopen(l, "file", stdio_fopen);

	if(l == &resource_loaders)
	    resource_loaders_init_p = 1;
    }

    return l;
}
