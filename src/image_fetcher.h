/* a10 371
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_fetcher.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 371 */



#ifndef KNOS_DEMOS_1_1_IMAGE_FETCHER_H
#define KNOS_DEMOS_1_1_IMAGE_FETCHER_H

#include <lib/url.h>
#include <library/http_stream.h>
#include <library/vector.h>
#include <libc/pthread.h>
#include <library/memory.h>

typedef struct url_fetcher_t
{
    object_t super;
    void (*parse_links)(struct url_fetcher_t* self, http_stream_t* stream, int depth);
    /* (not a method, but a callback set by the user
       returns true wether to parse link again */
    int (*callback)(struct url_fetcher_t* self, http_stream_t* url_found);

    http_stream_t* from_url;

    vector_t urls; /* url_t* */

    pthread_mutex_t collected_urls_mutex;
    vector_t collected_urls; /* char** */

    int depth;
    int guard; /* depth limit */
} url_fetcher_t;

CLASS_INHERIT(url_fetcher, object)

int images_google_com_callback(url_fetcher_t* self, http_stream_t* url_found);
int http_directory_callback(url_fetcher_t* self, http_stream_t* url_found);


#endif
