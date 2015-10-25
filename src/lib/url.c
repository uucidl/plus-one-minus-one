/* a10 790
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/url.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 790 */

#include "url.h"

#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_LIB_URL);

#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>

static int url_open_with_protocol(url_t *self, const char *protocol,
                                  const char *url)
{
    char *resource = strdup(url);
    char *port;

    self->protocol = strdup(protocol);

    if (strcmp(self->protocol, "http") != 0) {
        self->server = NULL;
        self->port = 0;
        self->path = resource;
    } else {
        self->server = resource;

        {
            char *q = strstr(resource, "?");
            if (q) {
                *q++ = '\0';
                self->query = strdup(q);
            }
        }

        self->path = strstr(resource, "/");
        if (!self->path) {
            self->path = strdup("/");
        } else {
            char *old = self->path;
            self->path = strdup(old);
            *old = '\0';
        }

        if (self->protocol && strcmp("file", self->protocol)) {
            port = strstr(self->server, ":");
            if (port) {
                *port++ = '\0';
                sscanf(port, "%u", &self->port);
            } else {
                self->port = 80;
            }
        }
    }
    return 1;
}

static int url_new(url_t *self, const char *url)
{
    /* parts of an url */
    const char *resource;
    char *protocol;
    unsigned int owns_protocol_p = 0;
    int ret = 0;

    if (!url)
        return 0;

    /* jump over protocol */
    resource = strstr(url, "://");
    if (resource) {
        unsigned int len = resource - url;
        if (len) {
            protocol = malloc(len + 1);
            strncpy(protocol, url, len);
            protocol[len] = '\0';
            owns_protocol_p = 1;
        } else {
            protocol = "unknown";
        }
        resource = resource + 3;
    } else {
        protocol = "file";
        resource = url;
    }

    ret = url_open_with_protocol(self, protocol, resource);
    if (owns_protocol_p)
        free(protocol);

    return ret;
}

static int url_new2(url_t *self, url_t *base_url, const char *relative_path)
{
    int i;
    int is_absolute_p = 1;
    char *base_path = NULL;

    if (!base_url)
        return self->new (self, relative_path);

    if (base_url->protocol)
        self->protocol = strdup(base_url->protocol);
    if (base_url->path)
        base_path = strdup(base_url->path);
    if (base_url->server)
        self->server = strdup(base_url->server);
    self->port = base_url->port;

    if (!relative_path)
        return 1;

    /* !!! FIXME: test wether relative path isn't really at all */
    if (!strncasecmp(relative_path, "http://", strlen("http://"))) {
        return self->new (self, relative_path);
    }

    /* test if relative path is absolute */
    for (i = 0; relative_path[i]; i++) {
        if (is_absolute_p) {
            if (relative_path[i] == '/')
                break;
            else if (relative_path[i] != ' ') {
                is_absolute_p = 0;
                break;
            }
        }
    }

    if (is_absolute_p) {
        self->path = strdup(relative_path);
    } else {
        char *dir;
        if (!base_path) {
            WARNING1("base path was NULL. construction failed.");
            return 0;
        }

        /* do the concatenation thing */
        {
            int n = strlen(base_path);
            int len;
            char *absolute_path;

            dir = strrchr(base_path, '/');
            if (dir) {
                n = dir - base_path + 1;
            }

            len = n + strlen(relative_path) + 1;

            absolute_path = malloc(len);
            if (n > 0)
                strncpy(absolute_path, base_path, n);
            strncpy(absolute_path + n, relative_path, len - n);
            absolute_path[len - 1] = '\0';

            self->path = absolute_path;
        }
    }

    /* cut query part */
    {
        char *q = strstr(self->path, "?");
        if (q) {
            *q++ = '\0';
            self->query = strdup(q);
        }
    }

    return 1;
}

static int url_new3(url_t *self, const char *protocol, const char *server,
                    int port, const char *path, const char *query)
{
    self->protocol = protocol ? strdup(protocol) : NULL;
    self->server = server ? strdup(server) : NULL;
    self->port = port;
    self->path = path ? strdup(path) : NULL;
    self->query = query ? strdup(query) : NULL;

    return 1;
}

static int url_destroy(url_t *self)
{
    if (self->protocol) {
        free(self->protocol);
        self->protocol = NULL;
    }

    if (self->server) {
        free(self->server);
        self->server = NULL;
    }
    if (self->path) {
        free(self->path);
        self->path = NULL;
    }
    if (self->query) {
        free(self->query);
        self->query = NULL;
    }

    return 1;
}

static int url_equals(url_t *self, url_t *other)
{
    int ret_p = 1;

    if (other) {
        if (self->protocol != other->protocol)
            ret_p = ret_p && self->protocol && other->protocol &&
                    (strcasecmp(self->protocol, other->protocol) == 0);

        if (self->server != other->server)
            ret_p = ret_p && self->server && other->server &&
                    (strcasecmp(self->server, other->server) == 0);

        ret_p = ret_p && self->port == other->port;

        if (self->path != other->path)
            ret_p = ret_p && self->path && other->path &&
                    (strcasecmp(self->path, other->path) == 0);

        if (self->query != other->query)
            ret_p = ret_p && self->query && other->query &&
                    (strcasecmp(self->query, other->query) == 0);
    } else {
        ret_p = 0;
    }

    return ret_p;
}

/* very very dumb implementation */
static int url_is_subdir_of(url_t *self, url_t *other)
{
    int ret_p = 1;

    if (other) {
        if (self->protocol != other->protocol)
            ret_p = ret_p && self->protocol && other->protocol &&
                    (strcasecmp(self->protocol, other->protocol) == 0);

        if (self->server != other->server)
            ret_p = ret_p && self->server && other->server &&
                    (strcasecmp(self->server, other->server) == 0);

        ret_p = ret_p && self->port == other->port;

        if (self->path != other->path) {
            char *remaining = NULL;
            char *dir;
            char *other_dir;

            if (self->path && other->path) {
                if (ret_p && !strcasecmp(self->path, other->path)) {
                    // special case if we have the exact same path
                    return 1;
                }

                dir = strrchr(self->path, '/');
                if (dir == NULL)
                    ret_p = 0;
                else {
                    /* find dir component */
                    other_dir = strrchr(other->path, '/');
                    if (other_dir == NULL)
                        other_dir = strdup("/");
                    else {
                        int n = other_dir - other->path + 1;
                        other_dir = calloc(n, 1);
                        strncpy(other_dir, other->path, n - 1);
                    }

                    remaining = strstr(self->path, other_dir);

                    if (!remaining)
                        ret_p = 0;
                    else {
                        remaining += strlen(other_dir);

                        while (*remaining != '\0' &&
                               (*remaining == '/' || *remaining == '.'))
                            remaining++;

                        if (*remaining != '\0')
                            ret_p = ret_p && 1;
                        else
                            ret_p = 0;
                    }
                }
            } else {
                ret_p = ret_p && 1;
            }
        }
    } else {
        ret_p = 0;
    }

    return ret_p;
}

char *url_create_string(url_t *self)
{
    char *ret = NULL;
    int n = strlen("://");
    const char *protocol = self->protocol ? self->protocol : "file";
    const char *server = self->server ? self->server : "";
    n += strlen(protocol);
    n += strlen(server);

    if (self->server && self->port != 80) {
        int j = 1;
        int port = self->port;
        while (port /= 10) {
            j++;
        }

        n += strlen(":");
        n += j;
    }
    n += strlen(self->path);
    if (self->query) {
        n += strlen("?");
        n += strlen(self->query);
    }
    n += 1;

    ret = calloc(n, sizeof(char));
    {
        char *dest = ret;
        sprintf(dest, "%s://%s", protocol, server);
        dest = ret + strlen(ret);
        if (self->server && self->port != 80) {
            sprintf(dest, ":%u", self->port);
            dest = ret + strlen(ret);
        }
        if (!self->query)
            sprintf(dest, "%s", self->path);
        else
            sprintf(dest, "%s?%s", self->path, self->query);
    }

    return ret;
}

url_t *url_copy(url_t *zelf, url_t *src)
{
    url_t *self = zelf ? zelf : url_instantiate_toplevel(NULL);
    char *protocol = NULL;
    char *server = NULL;
    char *path = NULL;
    char *query = NULL;

    if (!src) {
        DEBUG1("src == NULL");
        return NULL;
    }

    if (src->protocol)
        protocol = strdup(src->protocol);

    if (src->server)
        server = strdup(src->server);

    if (self->protocol)
        free(self->protocol);
    self->protocol = protocol;

    if (self->server)
        free(self->server);
    self->server = server;

    self->port = src->port;

    if (src->path)
        path = strdup(src->path);

    if (self->path)
        free(self->path);

    self->path = path;

    if (src->query)
        query = strdup(src->query);

    if (self->query)
        free(self->query);

    self->query = query;

    return self;
}

url_t *url_instantiate(url_t *x)
{
    url_t *u = url_instantiate_super(x);

    u->new = url_new;
    u->new2 = url_new2;
    u->new3 = url_new3;
    u->new4 = url_open_with_protocol;
    u->destroy = url_destroy;
    u->equals = url_equals;
    u->copy = url_copy;
    u->is_subdir_of = url_is_subdir_of;
    u->create_string = url_create_string;

    return u;
}
