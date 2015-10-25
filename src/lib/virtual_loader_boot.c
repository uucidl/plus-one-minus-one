/* a10 182
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/virtual_loader_boot.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 182 */

#include <lib/virtual_loader.h>

static void initializes_resource_virtual_loader() __attribute__((constructor));

static void initializes_resource_virtual_loader()
{
    virtual_loader_t *vl = virtual_loader_get_instance();

    url_t *cwd_url = url_instantiate_toplevel(NULL);
    cwd_url->new4(cwd_url, "file", "./");

    vl->add_local_search_path(vl, "resources", cwd_url);

    cwd_url->destroy(cwd_url);
    url_retire(cwd_url);
}
