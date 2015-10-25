/* a10 964
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/http_stream.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 964 */

#ifndef KNOS_LIBRARY_HTTP_STREAM_H
#define KNOS_LIBRARY_HTTP_STREAM_H

#include <library/memory_stream.h>
#include <lib/url.h>

typedef struct http_stream_t {
    memory_stream_t super;
    url_t url;

    struct http_stream_t *(*open)(struct http_stream_t *self);
    struct http_stream_t *(*stat)(struct http_stream_t *self);

    char *content_type;
    unsigned int status;
    unsigned int content_length;
    int connection_close_p;

    int chunked_p;
    int redirect_p;
    unsigned int redirected; // redirection count
    int broken_pipe_p;
} http_stream_t;

CLASS_INHERIT(http_stream, memory_stream);

#endif
