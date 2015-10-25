/* a10 924
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/url_open.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 924 */

#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_LIB_URL_OPEN);

#include <libc/stdio.h>
#include <libc/string.h>
#include <lib/url_open.h>

#include "resource_loaders.h"

static rloader_t *loaders = NULL;

stream_t *knos_demos_lib_url_open(const char *url_s, const char *mode)
{
    if (!url_s)
        return NULL;
    else {
        stream_t *s;
        url_t url;

        url_instantiate_toplevel(&url);
        url.new(&url, url_s);

        s = url_open2(&url, mode);

        url.destroy(&url);
        url_retire(&url);

        return s;
    }
}

stream_t *knos_demos_lib_url_open2(const url_t *url, const char *mode)
{
    if (!url)
        return NULL;
    else {
        fopen_function f;

        if (!loaders)
            loaders = rloader_instantiate_toplevel(NULL);

        f = loaders->get_fopen(loaders, url->protocol);
        if (f == NULL) {
            ERROR2("protocol '%s' not supported.", url->protocol);
            return NULL;
        } else {
            char filepath[(url->server ? strlen(url->server) : 0) + 1 +
                          (url->path ? strlen(url->path) : 0)];
            sprintf(filepath, "%s%s", url->server ? url->server : "",
                    url->path ? url->path : "");
            return f(filepath, mode);
        }
    }
}
