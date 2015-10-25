/* a10 42
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/http_loader.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 42 */

#include "http_loader.h"
#include "resource_loaders.h"

#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/ctype.h>
#include <library/http_stream.h>

#include "url.h"

/* mode is ignored
 * data is loaded in one shot from url, and a memory stream
 * is created on this area
 */
stream_t *http_url_open(const char *url, const char *mode)
{
    http_stream_t *stream;

    stream = http_stream_instantiate_toplevel(NULL);

    stream->url.new4(&stream->url, "http", url);
    return (stream_t *)stream->open(stream);
}

void http_loader_init()
{
    rloader_t *r = rloader_instantiate_toplevel(NULL);
    r->set_fopen(r, "http", http_url_open);
}

#if 0
/* test url */
int main() {
    url_t* base_url = url_instantiate_toplevel(NULL);
    url_t* new_url = url_instantiate_toplevel(NULL);

    base_url->new(base_url, "http://neq.tpolm.com/jpg/");
    new_url->new2(new_url, base_url, "hypocurtain.jpg");
    
}
#endif
