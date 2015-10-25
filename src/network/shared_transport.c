/* a10 158
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/shared_transport.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 158 */

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_NETWORK_SHARED_TRANSPORT);

#include <lib/chance.h>
#include <libc/pthread.h>
#include <library/time.h>
#include <library/vector_impl.h>
#include <network/channel.h>
#include <network/shared_transport.h>
#include <network/transport.h>

#include <libc/sys/time.h>
#include <libc/sys/types.h>
#include <libc/unistd.h>

#include <library/thread-helper.h>

/* channel cache
 *
 */

typedef struct cached_channel_t {
    channel_t channel;
    pthread_mutex_t channel_mutex;
} cached_channel_t;

static vector_t channel_cache; /* cached_socket_t* */
static int channel_cache_init_p = 0;
static pthread_mutex_t channel_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

static int channel_cache_init()
{
    if (pthread_mutex_lock(&channel_cache_mutex)) {
        ERROR1("error, lock channel cache");
    }

    if (!channel_cache_init_p) {
        unsigned int n = 12;
        vector_new(&channel_cache, cached_channel_t);

        /* allocate the channels now */
        while (n--) {
            cached_channel_t *cs = add_element(&channel_cache);

            if (pthread_mutex_init(&cs->channel_mutex, NULL)) {
                ERROR1("channel mutex init failed");
            }

            sock_stream_instantiate_toplevel(&cs->channel.stream);
            cs->channel.stream.open(&cs->channel.stream);
            cs->channel.stream.set_nonblocking(&cs->channel.stream);
        }

        channel_cache_init_p = 1;
    }

    if (pthread_mutex_unlock(&channel_cache_mutex)) {
        perror("unlock channel cache");
    }

    return 1;
}

static void channel_cache_cleanup() __attribute__((destructor));
static void channel_cache_cleanup()
{
    iterator_t it;
    cached_channel_t *ret;

    get_iterator(&channel_cache, &it);
    while ((ret = iterator_next(&it))) {
        stream_get_callbacks(&ret->channel.stream.super)
            ->close(&ret->channel.stream.super);
        pthread_mutex_destroy(&ret->channel_mutex);
        memset(&ret->channel, 0, sizeof(channel_t));
    }
}

/* get any possible channel */
static cached_channel_t *channel_cache_get_channel()
{
    cached_channel_t *ret = NULL;
    iterator_t it;
    int n = 0;

    get_iterator(&channel_cache, &it);
    while ((ret = iterator_next(&it))) {
        if (!pthread_mutex_trylock(&ret->channel_mutex)) {
            break;
        }
        n++;
    }

    if (!ret) {
        if (n)
            n--;
        n *= unirand();

        get_iterator(&channel_cache, &it);
        do {
            ret = iterator_next(&it);
        } while (n-- && ret);
        if (!ret)
            ERROR1("bug, couln't find a cached_channel to wait on");

        if (pthread_mutex_lock(&ret->channel_mutex)) {
            ERROR1("locking channel_mutex");
        }
    }

    stream_get_callbacks(&ret->channel.stream.super)
        ->close(&ret->channel.stream.super);
    ret->channel.stream.open(&ret->channel.stream);
    ret->channel.stream.set_nonblocking(&ret->channel.stream);

    return ret;
}

/* get a channel already attached to one url */
static cached_channel_t *channel_cache_get_channel_by_url(url_t *url)
{
    cached_channel_t *ret = NULL;
    iterator_t it;

    if (url && url->server) {
        get_iterator(&channel_cache, &it);

        while ((ret = iterator_next(&it))) {
            if (!pthread_mutex_trylock(&ret->channel_mutex)) {
                // lock success
                if (ret->channel.stream.url.server == NULL ||
                    (ret->channel.stream.url.port == url->port &&
                     !strcmp(ret->channel.stream.url.server, url->server))) {
                    // found a suitable one
                    break;
                } else {
                    // release and move on
                    pthread_mutex_unlock(&ret->channel_mutex);
                }
            }
        }
    }

    if (ret == NULL)
        ret = channel_cache_get_channel();

    return ret;
}

static void channel_cache_release_channel(cached_channel_t *cs)
{
    int err;

    if ((err = pthread_mutex_unlock(&cs->channel_mutex))) {
        ERROR1("error unlocking channel_mutex");
    }
}

/*

transport class

 */
typedef struct channel_entry_t {
    cached_channel_t *pair;
    transport_callbacks_t *callbacks;
    selector_t *selector; /* associated selector */
    struct channel_entry_t *next;
} channel_entry_t;

typedef struct entry_iterator_t {
    struct shared_transport_t *parent;
    channel_entry_t *current;
} entry_iterator_t;

typedef struct shared_transport_t {
    transport_t super;

    pthread_mutex_t entries_mutex;
    channel_entry_t *end; /* linked list of channel_entry_t */
} shared_transport_t;

CLASS_INHERIT(shared_transport, transport)

static void get_entry_iterator(shared_transport_t *self, entry_iterator_t *it)
{
    it->parent = self;
    it->current = self->end ? self->end->next : NULL;
}

static channel_entry_t *entry_iterator_next(entry_iterator_t *it)
{
    channel_entry_t *ret = it->current;
    if (it->current) {
        if (it->current == it->parent->end) {
            it->current = NULL;
        } else {
            it->current = it->current->next;
        }
    }

    return ret;
}

static void add_entry(shared_transport_t *self, channel_entry_t *e)
{
    if (!self->end) {
        e->next = e;
        self->end = e;
    } else {
        channel_entry_t *first = self->end->next;
        e->next = first;
        self->end->next = e;
    }
}

static void remove_entry(shared_transport_t *self, channel_entry_t *e)
{
    channel_entry_t *prev = self->end;
    channel_entry_t *current = NULL;
    entry_iterator_t it;

    if (!self->end) {
        WARNING1("empty. nothing to remove");
        return;
    }

    get_entry_iterator(self, &it);
    while ((current = entry_iterator_next(&it))) {
        if (current == e) {
            break;
        }
        prev = current;
    }

    if (current) {
        if (current == self->end) {
            prev->next = current->next;
            if (prev == self->end)
                self->end = NULL;
            else
                self->end = prev;
        } else {
            prev->next = current->next;
        }
    } else {
        WARNING1("entry not found.");
    }
}

#define BUFSIZE 4096

typedef struct tselector_t {
    selector_t super;
    pthread_mutex_t mutex;
    pthread_cond_t first_fd_registered;

    fd_set fds;
    channel_t *channels[FD_SETSIZE];
    transport_callbacks_t *cbs[FD_SETSIZE];
    double activated_ms[FD_SETSIZE]; /* last time of activity */
    int n;                           /* max fd number + 1 */
    char buffer[BUFSIZE];
} tselector_t;

CLASS_INHERIT2(private, tselector, selector)

static void ts_register_channel(selector_t *zelf, channel_t *channel,
                                transport_callbacks_t *cb)
{
    tselector_t *self = (tselector_t *)zelf;

    pthread_mutex_lock(&self->mutex);
    if (!FD_ISSET(channel->stream.socket, &self->fds)) {
        unsigned int i;

        if (self->n == 0) {
            /* this is the first one to be registered */
            pthread_cond_signal(&self->first_fd_registered);
        }

        if (channel->stream.socket >= self->n) {
            self->n = channel->stream.socket + 1;
        }

        FD_SET(channel->stream.socket, &self->fds);

        for (i = 0; i < FD_SETSIZE; i++) {
            if (!self->channels[i])
                break;
        }
        if (i == FD_SETSIZE)
            ERROR1("selector full.");
        else {
            self->channels[i] = channel;
            self->cbs[i] = cb;
            self->activated_ms[i] = get_milliseconds();
        }
    } else {
        WARNING1("socket was already set.");
    }
    pthread_mutex_unlock(&self->mutex);
}

/* function that assumes the lock on zelf->mutex to be taken */
static void internal_unregister_channel(selector_t *zelf, channel_t *channel)
{
    tselector_t *self = (tselector_t *)zelf;

    if (FD_ISSET(channel->stream.socket, &self->fds)) {
        unsigned int i;
        FD_CLR(channel->stream.socket, &self->fds);

        if (channel->stream.socket == self->n - 1) {
            int s;
            // this was the max, let's find the new one
            for (s = channel->stream.socket; s >= 0; s--) {
                if (FD_ISSET(s, &self->fds)) {
                    break;
                }
            }
            self->n = s + 1;
        }

        for (i = 0; i < FD_SETSIZE; i++) {
            if (self->channels[i] &&
                self->channels[i]->stream.socket == channel->stream.socket) {
                self->channels[i] = NULL;
                self->cbs[i] = NULL;
                self->activated_ms[i] = 0.0;
                break;
            }
        }
    }
}
static void ts_unregister_channel(selector_t *zelf, channel_t *channel)
{
    tselector_t *self = (tselector_t *)zelf;

    pthread_mutex_lock(&self->mutex);
    internal_unregister_channel(zelf, channel);
    pthread_mutex_unlock(&self->mutex);
}

static void ts_nop(selector_t *zelf) {}

typedef void *(*pthread_run_f)(void *);

static void *ts_run(selector_t *zelf)
{
    tselector_t *self = (tselector_t *)zelf;

    while (1) {
        int n = 0;
        struct timeval timeout;
        fd_set readfds;
        fd_set writefds;
        double t = get_milliseconds();
        int i;

        pthread_mutex_lock(&self->mutex);
        n = self->n;

        if (n == 0) {
            /* no active fds around anymore, let's not eat cpu time */
            TRACE1("waiting for new fd");
            pthread_cond_wait(&self->first_fd_registered, &self->mutex);
            TRACE1("done");
            n = self->n;
        }

        timeout.tv_sec = 0;
        timeout.tv_usec = 2000;

        /* recreate fd_sets */
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        for (i = 0; i < FD_SETSIZE; i++) {
            channel_t *c = self->channels[i];
            if (c) {
                FD_SET(c->stream.socket, &readfds);
                FD_SET(c->stream.socket, &writefds);
            }
        }
        pthread_mutex_unlock(&self->mutex);

        n = select(n, &readfds, &writefds, NULL, &timeout);
        if (n < 0) {
            perror("select");
        } else if (n) {
            pthread_mutex_lock(&self->mutex);
            for (i = 0; i < FD_SETSIZE; i++) {
                channel_t *c = self->channels[i];
                sock_stream_t *stream = &c->stream;
                transport_callbacks_t *const cbs = self->cbs[i];

                /* first check reads */
                if (c) {
                    if (FD_ISSET(stream->socket, &readfds)) {
                        // read from socket
                        int nbytes = 0;
                        nbytes = stream_get_callbacks(&stream->super)
                                     ->read(self->buffer, sizeof(char), BUFSIZE,
                                            &stream->super);

                        if (stream->broken_p) {
                            internal_unregister_channel(zelf, c);
                            if (cbs && cbs->close) {
                                cbs->close(cbs, c);
                            }
                        } else if (nbytes && cbs && cbs->read) {
                            // pass to callback
                            cbs->read(cbs, c, self->buffer, nbytes);
                            self->activated_ms[i] = t;
                        }
                    }

                    if (FD_ISSET(stream->socket, &writefds)) {
                        // pass to callback
                        if (cbs && cbs->write) {
                            int nbytes =
                                cbs->write(cbs, c, self->buffer, BUFSIZE);
                            // write to socket
                            if (nbytes) {
                                int n = 0;
                                n = stream_get_callbacks(&stream->super)
                                        ->write(self->buffer, 1, nbytes,
                                                &stream->super);

                                if (stream->broken_p) {
                                    internal_unregister_channel(zelf, c);
                                    if (cbs && cbs->close) {
                                        cbs->close(cbs, c);
                                    }
                                } else {
                                    self->activated_ms[i] = t;
                                }
                            }
                        }
                    }
                    /* check fd expirations */
                    if (FD_ISSET(stream->socket, &self->fds)) {
                        /* if not set, check expiration */
                        if (t - self->activated_ms[i] > 5000.0) {
                            TRACE3("%d (%s) slept for over 5sec",
                                   stream->socket, c->stream.url.server);
                            c->stream.broken_p = 1;
                            internal_unregister_channel(zelf, c);
                            if (cbs && cbs->close) {
                                cbs->close(cbs, c);
                            }
                        }
                    }
                }
            }
            pthread_mutex_unlock(&self->mutex);
        }
    }

    TRACE1("stopping selector");
    pthread_exit(NULL);
}

selector_t *selector_instantiate(selector_t *x)
{
    return selector_instantiate_super(x);
}

static tselector_t *tselector_instantiate(tselector_t *x)
{
    tselector_t *ts = tselector_instantiate_super(x);

    FD_ZERO(&ts->fds);

    ts->super.register_channel = ts_register_channel;
    ts->super.unregister_channel = ts_unregister_channel;
    ts->super.run = ts_nop;

    pthread_mutex_init(&ts->mutex, NULL);
    pthread_cond_init(&ts->first_fd_registered, NULL);

    {
        pthread_run_f ts_pthread_run = (pthread_run_f)ts_run;

        pthread_t id;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        if (kn_thread_create_minimum_priority_with_attributes(
                &id, &attr, ts_pthread_run, ts)) {
            ERROR1("error in pthread_create");
        }

        pthread_attr_destroy(&attr);
    }

    return ts;
}

static channel_t *transport_connect_to(transport_t *zelf, url_t *url,
                                       transport_callbacks_t *callbacks)
{
    shared_transport_t *self = (shared_transport_t *)zelf;
    channel_entry_t *ce;
    channel_t *channel;

    if (!callbacks)
        return NULL;

    {
        char *url_s = url->create_string(url);
        TRACE2("connect_to: %s", url_s);
        free(url_s);
    }

    ce = calloc(sizeof(channel_entry_t), 1);
    ce->pair = channel_cache_get_channel_by_url(url); /* locks */
    ce->callbacks = callbacks;

    channel = &ce->pair->channel;
    channel->stream.connect(&channel->stream, url);

    if (channel->stream.error_p) {
        stream_get_callbacks(&channel->stream.super)
            ->close(&channel->stream.super);
        channel->stream.open(&channel->stream);
        channel->stream.set_nonblocking(&channel->stream);

        channel_cache_release_channel(ce->pair);
        free(ce);
        channel = NULL;
    } else {
        pthread_mutex_lock(&self->entries_mutex);
        add_entry(self, ce);
        pthread_mutex_unlock(&self->entries_mutex);
    }

    return channel;
}

static void transport_release(transport_t *zelf, channel_t *channel)
{
    shared_transport_t *self = (shared_transport_t *)zelf;

    entry_iterator_t it;
    channel_entry_t *ce;

    // find channel
    pthread_mutex_lock(&self->entries_mutex);

    get_entry_iterator(self, &it);
    while ((ce = entry_iterator_next(&it))) {
        // pointer comparaison only
        if (ce->pair && &ce->pair->channel == channel) {
            sock_stream_t *stream = &ce->pair->channel.stream;
            if (stream->error_p || stream->broken_p) {
                /* closing and reopening socket */
                stream->super.callbacks.close(&stream->super);
                stream->open(stream);
                stream->set_nonblocking(stream);
            }
            // unregister from selector
            if (ce->selector)
                ce->selector->unregister_channel(ce->selector,
                                                 &ce->pair->channel);
            break;
        }
    }

    if (ce) {
        remove_entry(self, ce);
        if (ce->pair)
            channel_cache_release_channel(ce->pair);
        else
            DEBUG1("pair was null.");
        free(ce);
    } else {
        WARNING2("removing an entry that was not found? (socket: %d)",
                 channel->stream.socket);
    }

    pthread_mutex_unlock(&self->entries_mutex);
}

static void transport_run(transport_t *zelf, selector_t *selector)
{
    shared_transport_t *self = (shared_transport_t *)zelf;
    entry_iterator_t it;
    channel_entry_t *c;

    /* register opened channels to selector */
    pthread_mutex_lock(&self->entries_mutex);
    get_entry_iterator(self, &it);

    while ((c = entry_iterator_next(&it))) {
        if (!c->selector && c->pair && c->callbacks) {
            selector->register_channel(selector, &c->pair->channel,
                                       c->callbacks);
            c->selector = selector;
        }
    }
    pthread_mutex_unlock(&self->entries_mutex);

    /* run */
    selector->run(selector);
}

shared_transport_t *shared_transport_instantiate(shared_transport_t *x)
{
    shared_transport_t *s = OBJECT_INSTANTIATE(shared_transport, x);

    channel_cache_init();

    if (pthread_mutex_init(&s->entries_mutex, NULL)) {
        ERROR1("failed mutex_init_channel");
    }

    s->super.connect_to = transport_connect_to;
    s->super.release = transport_release;
    s->super.run = transport_run;
    s->super.finish = NULL;

    return s;
}

static shared_transport_t *_transport = NULL;
static pthread_mutex_t _transport_mutex = PTHREAD_MUTEX_INITIALIZER;

shared_transport_t *shared_transport_get_instance()
{
    pthread_mutex_lock(&_transport_mutex);
    if (!_transport) {
        _transport = shared_transport_instantiate_toplevel(NULL);
    }
    pthread_mutex_unlock(&_transport_mutex);

    return _transport;
}

static tselector_t *_selector = NULL;
static pthread_mutex_t _selector_mutex = PTHREAD_MUTEX_INITIALIZER;

selector_t *shared_selector_get_instance()
{
    pthread_mutex_lock(&_selector_mutex);
    if (!_selector) {
        _selector = tselector_instantiate_toplevel(NULL);
    }
    pthread_mutex_unlock(&_selector_mutex);

    return (selector_t *)_selector;
}
