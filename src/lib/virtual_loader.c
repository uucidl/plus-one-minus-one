/* a10 291
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/virtual_loader.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 291 */

#include "resource_loaders.h"
#include "virtual_loader.h"
#include <lib/url.h>
#include <lib/url_open.h>

#include <library/cstr_map_impl.h>
#include <library/map_impl.h>

#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_LIB_VIRTUAL_LOADER);

typedef struct virtual_path_node_t {
    enum { URL, PATH } type;
    union {
        url_t *url;
        map_t *path;
    } content;
} virtual_path_node_t;

static inline virtual_path_node_t *vp_instantiate()
{
    return calloc(sizeof(virtual_path_node_t), 1);
}

static inline void vp_tear_off(virtual_path_node_t *x)
{
    if (x->type == URL) {
        if (x->content.url) {
            x->content.url->destroy(x->content.url);
            object_retire(url_to_object(x->content.url));
        }
    } else if (x->type == PATH) {
        if (x->content.path) {
            x->content.path->destroy(x->content.path);
            object_retire(map_to_object(x->content.path));
        }
    }

    free(x);
}

typedef struct {
    local_path_iterator_t super;
    map_iterator_t *m;
    url_t *next;
} search_path_iterator_t;

CLASS_INHERIT(search_path_iterator, local_path_iterator);

static url_t *search_path_iterator_next(local_path_iterator_t *zelf)
{
    search_path_iterator_t *self = (search_path_iterator_t *)zelf;
    map_value_t uu = map_iterator_next(self->m);
    url_t *u = map_value_is_there(uu) ? map_value_obtain(uu) : NULL;

    if (u) {
        /* clone */
        u = u->copy(NULL, u);
    }

    if (self->next) {
        self->next->destroy(self->next);
        object_retire(url_to_object(self->next));
    }

    return self->next = u;
}

static int search_path_iterator_destroy(object_t *zelf)
{
    search_path_iterator_t *self = (search_path_iterator_t *)zelf;

    if (self->m) {
        map_iterator_destroy(self->m);
        map_iterator_retire(self->m);
        self->m = NULL;
    }
    if (self->next) {
        self->next->destroy(self->next);
        object_retire(url_to_object(self->next));
        self->next = NULL;
    }

    return 1;
}

search_path_iterator_t *search_path_iterator_new(map_t *m)
{
    search_path_iterator_t *it =
        search_path_iterator_instantiate_toplevel(NULL);

    it->m = map_get_iterator(m, NULL);

    return it;
}

local_path_iterator_t *local_path_iterator_instantiate(local_path_iterator_t *x)
{
    return local_path_iterator_instantiate_super(x);
}

search_path_iterator_t *
search_path_iterator_instantiate(search_path_iterator_t *x)
{
    search_path_iterator_t *it = search_path_iterator_instantiate_super(x);

    it->super.next = search_path_iterator_next;
    it->super.destroy = search_path_iterator_destroy;

    object_set_release_callback(search_path_iterator_to_object(it),
                                it->super.destroy);

    return it;
}

static stream_t *indirect_open(url_t *local_url, const char *path,
                               const char *mode)
{
    stream_t *ztrmmm;
    url_t tu;
    url_instantiate_toplevel(&tu);

    tu.copy(&tu, local_url);

    /* concatenate paths */
    {
        char *absolute_path = calloc(strlen(tu.path) + strlen(path) + 1, 1);
        strcat(absolute_path, tu.path);
        strcat(absolute_path, path);
        free(tu.path);
        tu.path = absolute_path;
    }

    if (tu.server && strlen(tu.server))
        TRACE6("url got: '%s://%s:%d%s' for path '%s'", tu.protocol, tu.server,
               tu.port, tu.path, path);
    else
        TRACE4("url got: '%s://%s' for path '%s'", tu.protocol, tu.path, path);

    ztrmmm = url_open2(&tu, mode);
    tu.destroy(&tu);
    url_retire(&tu);

    return ztrmmm;
}

static stream_t *virtual_url_open(const char *url, const char *mode)
{
    stream_t *strmmmmmmmeeeee = NULL;
    char *vhost = strdup(url);
    char *vpath = strchr(vhost, '/');
    if (vpath == NULL) {
        ERROR2("no path specified for virtual host '%s'", vhost);
    } else {
        *vpath++ = '\0';
    }

    if (!strlen(vhost)) {
        ERROR1("virtual host not specified.");
    } else {
        virtual_loader_t *v = virtual_loader_get_instance();
        local_path_iterator_t *it = v->get_local_search_path(v, vhost);
        if (it) {
            url_t *lu;
            while ((lu = it->next(it))) {
                strmmmmmmmeeeee = indirect_open(lu, vpath, mode);
                if (strmmmmmmmeeeee)
                    break;
            }
            local_path_iterator_retire(it);
        } else {
            url_t *lu = v->get_local_url(v, vhost);
            if (lu) {
                strmmmmmmmeeeee = indirect_open(lu, vpath, mode);
                lu->destroy(lu);
                object_retire(url_to_object(lu));
            } else {
                WARNING3("didn't find local url '%s' for host: '%s'", vpath,
                         vhost);
            }
        }
    }

    if (vhost)
        free(vhost);

    return strmmmmmmmeeeee;
}

static inline virtual_path_node_t *virtual_path_get(virtual_loader_t *self,
                                                    const char *vhost)
{
    cstr_map_value_t vv = cstr_map_get(&self->local_urls, vhost);
    return cstr_map_value_is_there(vv) ? cstr_map_value_obtain(vv) : NULL;
}

static url_t *virtual_loader_get_local_url(virtual_loader_t *self,
                                           const char *vhost)
{
    virtual_path_node_t *v = virtual_path_get(self, vhost);
    url_t *u;

    if (v == NULL || v->type != URL) {
        u = NULL;
    } else {
        /* clone url */
        if (v->content.url != NULL) {
            u = v->content.url->copy(NULL, v->content.url);
        } else {
            u = NULL;
        }
    }

    return u;
}

static void virtual_loader_set_local_url(virtual_loader_t *self,
                                         const char *vhost, url_t *url)
{
    virtual_path_node_t *v = virtual_path_get(self, vhost);
    if (url) {
        if (v == NULL)
            v = vp_instantiate();
        else if (v->type != URL) {
            vp_tear_off(v);
        }
        v->type = URL;
        /* clone url */
        v->content.url = url->copy(NULL, url);
    } else {
        if (v) {
            vp_tear_off(v);
        }
        v = NULL;
    }
    cstr_map_put(&self->local_urls, vhost, v);
}

static local_path_iterator_t *
virtual_loader_get_search_path(virtual_loader_t *self, const char *vhost)
{
    virtual_path_node_t *v = virtual_path_get(self, vhost);
    local_path_iterator_t *it = NULL;
    if (v && v->type == PATH) {
        it = &search_path_iterator_new(v->content.path)->super;
    }
    return it;
}

static void virtual_loader_add_search_path(virtual_loader_t *self,
                                           const char *vhost, url_t *url)
{
    virtual_path_node_t *v = virtual_path_get(self, vhost);
    if (url) {
        if (v == NULL || v->type != PATH) {
            if (v == NULL)
                v = vp_instantiate();
            else
                vp_tear_off(v);
            v->type = PATH;
            v->content.path = map_instantiate_toplevel(NULL);
        }
        /* clone url */
        TRACE5("vhost: %s now points to: %s %s %s", vhost, url->protocol,
               url->server, url->path);
        map_push(v->content.path, url->copy(NULL, url));
    }

    cstr_map_put(&self->local_urls, vhost, v);
}

virtual_loader_t *virtual_loader_instantiate(virtual_loader_t *x)
{
    virtual_loader_t *vt = virtual_loader_instantiate_super(x);

    cstr_map_instantiate_toplevel(&vt->local_urls);

    vt->get_local_url = virtual_loader_get_local_url;
    vt->set_local_url = virtual_loader_set_local_url;
    vt->get_local_search_path = virtual_loader_get_search_path;
    vt->add_local_search_path = virtual_loader_add_search_path;

    return vt;
}

static virtual_loader_t *loader = NULL;

virtual_loader_t *virtual_loader_get_instance()
{
    if (!loader) {
        loader = virtual_loader_instantiate_toplevel(NULL);
    }

    return loader;
}

void virtual_loader_init()
{
    rloader_t *r = rloader_instantiate_toplevel(NULL);
    r->set_fopen(r, "virtual", virtual_url_open);
}
