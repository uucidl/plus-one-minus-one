/* a10 73
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/sock_stream.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 73 */

#ifndef KNOS_LIBRARY_SOCK_STREAM_H
#define KNOS_LIBRARY_SOCK_STREAM_H

#include <library/stream.h>
#include <lib/url.h>

typedef struct sock_stream_t {
    stream_t super;

    struct sock_stream_t *(*open)(struct sock_stream_t *self);
    struct sock_stream_t *(*set_nonblocking)(struct sock_stream_t *self);
    struct sock_stream_t *(*connect)(struct sock_stream_t *self, url_t *url);
    int (*get_errno)(struct sock_stream_t *self);

    /* status */
    int error_p;
    int broken_p;

    /* socket */
    int socket;

    /* url */
    url_t url;
} sock_stream_t;

CLASS_INHERIT(sock_stream, stream);

#endif
