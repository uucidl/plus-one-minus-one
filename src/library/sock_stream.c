/* a10 137
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/sock_stream.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 137 */

#include <library/sock_stream.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_SOCK_STREAM);

#include <library/sock.h>
#include <libc/stdlib.h>
#include <libc/string.h>

static sock_stream_t *sock_stream_open(sock_stream_t *self)
{
    self->socket = sock_open(PF_INET, SOCK_STREAM, 0);
    self->error_p = self->socket < 0; // overwrite
    self->url.destroy(&self->url);

    return self->error_p ? NULL : self;
}

static sock_stream_t *sock_stream_set_nonblocking(sock_stream_t *self)
{
    if (!self->error_p)
        sock_set_nonblocking(self->socket);

    return self->error_p ? NULL : self;
}

static int sock_stream_close(stream_t *zelf)
{
    sock_stream_t *self = (sock_stream_t *)zelf;

    self->error_p = sock_close(self->socket) < 0;
    self->url.destroy(&self->url);

    return self->error_p ? EOF : 0;
}

static int init_sockaddr(struct sockaddr_in *name, url_t *url)
{
    struct hostent *hostinfo;

    if (!url || !url->server) {
        DEBUG("url == NULL?");
        return -1;
    }

    memset(name, 0, sizeof(struct sockaddr_in));

    name->sin_family = AF_INET;
    name->sin_port = htons(url->port);

    hostinfo = gethostbyname(url->server);
    if (hostinfo == NULL) {
        ERROR("Unknown host %s.", url->server);
        return -1;
    }
    name->sin_addr = *(struct in_addr *)hostinfo->h_addr;

    return 1;
}

static sock_stream_t *sock_stream_connect(sock_stream_t *self, url_t *url)
{
    struct sockaddr_in server_addr;

    self->url.copy(&self->url, url);

    self->error_p =
        self->error_p || (init_sockaddr(&server_addr, &self->url) < 0);

    if (!self->error_p) {
        int err = sock_connect(self->socket, (struct sockaddr *)&server_addr,
                               sizeof(server_addr));
        if (err < 0) {
            int ii = sock_errno();

            if (ii != EISCONN && ii != EINPROGRESS && ii != EALREADY)
                self->error_p = 1;
        }
    }

    if (!self->error_p) {
        self->broken_p = 0;
    }

    return self->error_p ? NULL : self;
}

static size_t sock_stream_read(void *ptr, size_t size, size_t nmemb,
                               struct stream_t *zelf)
{
    sock_stream_t *self = (sock_stream_t *)zelf;
    int n = 0;

    if (!self->error_p) {
        n = sock_read(self->socket, ptr, size * nmemb);
        if (n < 0) {
            self->error_p = 1;
        }
    }

    if (self->error_p) {
        int err = sock_errno();
        if (err != EWOULDBLOCK && err != EAGAIN) {
            n = 0;
            self->broken_p = 1;
        } else {
            self->error_p = 0;
        }
    } else if (!n) {
        self->broken_p = 1;
    }

    n /= size;

    return n;
}

size_t sock_stream_write(const void *ptr, size_t size, size_t nmemb,
                         stream_t *zelf)
{
    sock_stream_t *self = (sock_stream_t *)zelf;
    int n = 0;

    if (!self->error_p) {
        n = sock_write(self->socket, ptr, size * nmemb);
        if (n < 0) {
            self->error_p = 1;
        }
    }

    /* let's assume it means automatically connection closed */
    if (self->error_p) {
        int err = sock_errno();
        self->error_p = 1;
        if (err != EWOULDBLOCK && err != EAGAIN) {
            n = 0;
            self->broken_p = 1;
        } else {
            self->error_p = 0;
        }
    } else if (!n) {
        self->broken_p = 1;
    }

    n /= size;

    return n;
}

int sock_stream_get_errno(sock_stream_t *self) { return errno; }

sock_stream_t *sock_stream_instantiate(sock_stream_t *x)
{
    sock_stream_t *s = sock_stream_instantiate_super(x);

    url_instantiate_toplevel(&s->url);

    s->open = sock_stream_open;
    s->set_nonblocking = sock_stream_set_nonblocking;
    s->super.callbacks.close = sock_stream_close;
    s->connect = sock_stream_connect;
    s->super.callbacks.read = sock_stream_read;
    s->super.callbacks.write = sock_stream_write;
    s->get_errno = sock_stream_get_errno;

    return s;
}
