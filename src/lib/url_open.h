/* a10 536
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/url_open.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 536 */



#ifndef KNOS_DEMOS_LIB_URL_OPEN_H
#define KNOS_DEMOS_LIB_URL_OPEN_H

#include <library/stream.h>
#include <lib/url.h>

/* opens an url, (protocol:scheme) 
   it uses the resource_loaders api.
*/
stream_t* knos_demos_lib_url_open(const char* url, const char* mode);
stream_t* knos_demos_lib_url_open2(const url_t* url, const char* mode);

static inline 
stream_t* url_open (const char* url, const char* mode) {
    return knos_demos_lib_url_open (url, mode);
}

static inline 
stream_t* url_open2 (const url_t* url, const char* mode) {
    return knos_demos_lib_url_open2 (url, mode);
}

#endif
