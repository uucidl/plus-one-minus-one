/* a10 397
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/virtual_loader.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 397 */

#ifndef KNOS_DEMOS_LIB_VIRTUAL_LOADER_H
#define KNOS_DEMOS_LIB_VIRTUAL_LOADER_H

#include <lib/url.h>
#include <library/map.h>
#include <library/cstr_map.h>
#include <library/memory.h>

typedef struct local_path_iterator_t {
    object_t super;
    url_t *(*next)(struct local_path_iterator_t *self);
    int (*destroy)(object_t *self);
} local_path_iterator_t;

CLASS_INHERIT(local_path_iterator, object);

/*
  the virtual loader adds a 'virtual' protocol whose namespace is
  composed partially programatically. This primitive version only
  allows the creation of 'servers' which refer to a physical url or an
  ordered list of urls.

  example:
    virtual://path/INFOFILE

    will refer, in order, to the paths composed by the concatenation of the
    urls configured for the vhost 'path'
*/

typedef struct virtual_loader_t {
    object_t super;
    url_t *(*get_local_url)(struct virtual_loader_t *self, const char *vhost);
    void (*set_local_url)(struct virtual_loader_t *self, const char *vhost,
                          url_t *url);
    local_path_iterator_t *(*get_local_search_path)(
        struct virtual_loader_t *self, const char *vhost);
    void (*add_local_search_path)(struct virtual_loader_t *self,
                                  const char *vhost, url_t *path);

    cstr_map_t local_urls;
} virtual_loader_t;

CLASS_INHERIT(virtual_loader, object)

virtual_loader_t *virtual_loader_get_instance();

void virtual_loader_init() __attribute__((constructor));

#endif
