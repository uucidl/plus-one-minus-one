/* a10 382
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/url.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 382 */

#ifndef KNOS_DEMOS_LIB_URL_H
#define KNOS_DEMOS_LIB_URL_H

#include <library/memory.h>

typedef struct url_t {
    object_t super;
    int (*new)(struct url_t *self, const char *url);
    int (*new2)(struct url_t *self, struct url_t *base_url,
                const char *relative_path);
    int (*new3)(struct url_t *self, const char *protocol, const char *server,
                int port, const char *path, const char *query);
    int (*new4)(struct url_t *self, const char *protocol, const char *url);
    struct url_t *(*copy)(struct url_t *self, struct url_t *src);
    int (*destroy)(struct url_t *self);

    int (*equals)(struct url_t *self, struct url_t *other);
    int (*is_subdir_of)(struct url_t *self, struct url_t *other);
    char *(*create_string)(struct url_t *self);

    char *protocol;
    char *server;
    int port;
    char *path;
    char *query;
} url_t;

CLASS_INHERIT(url, object)

#endif
