/* a10 220
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/http_stream.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 220 */

#include <library/http_stream.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_HTTP_STREAM);

#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>

#include <library/vector_impl.h>

#include <libc/pthread.h>
#include <network/transport.h>
#include <network/shared_transport.h>
#include <network/http_handler.h>
#include <library/time.h>

/*

  do should reuse connections, benefiting from http/1.1 keep alives
  for this we need to keep an array of servers -> sockets, if server
  not in this cache, establish connection. if yes, get that socket
  from the cache.
  issue operations. if error occurs,

*/

static inline void say() { TRACE1("had to retry"); }

static http_stream_t *http_stream_do(http_stream_t *self, http_method_t method)
{
    http_response_t response;
    transport_t *t = (transport_t *)shared_transport_get_instance();
    http_handler_t *h = http_handler_instantiate_toplevel(NULL);
    channel_t *c;

    {
        int n = 0;

        do {
            http_state_t state;
            double ms;

            h->new (h, &response);
            if ((c = t->connect_to(t, &self->url, &h->super))) {
                if (method == GET)
                    h->prepare_get_request(h, &self->url);
                else if (method == HEAD)
                    h->prepare_head_request(h, &self->url);

                ms = -1.0 * get_milliseconds();
                t->run(t, shared_selector_get_instance());

                /* wait for the thread to start and complete its work */
                pthread_mutex_lock(&response.response_mutex);
                state = h->get_state(h);
                if (state != DONE)
                    pthread_cond_wait(&response.response_complete,
                                      &response.response_mutex);
                state = h->get_state(h);
                if (state != DONE)
                    DEBUG2("state != DONE but == %d", state);
                ms += get_milliseconds();
                DEBUG2("stream fetched in %f", ms / 1000.0);

                t->release(t, c);
                pthread_mutex_unlock(&response.response_mutex);
            } else {
                break;
            }
        } while (response.broken_p && (say(), n++ < 5));
    }
    object_retire(http_handler_to_object(h));

    // return block
    {
        http_stream_t *ret = NULL;

        if (c && !c->stream.error_p && !response.broken_p) {
            /* copy response data to self */
            self->content_type = response.content_type;
            self->content_length = response.content_length;
            self->status = response.status_code;

            if (response.status_code / 100 == 3) {
                self->redirected++;
                self->redirect_p = 1;
                if (self->redirected < 5) {
                    self->url.destroy(&self->url);
                    self->url.new(&self->url, response.location);
                    ret = http_stream_do(self, method);
                }
            } else if (response.status_code == 200) {
                ret = self;
                if (response.size) {
                    memory_open(&self->super, response.data, response.size,
                                "rb");
                    self->super.buffer_owned_p = 1;
                } else if (method == GET) {
                    char *url_s = self->url.create_string(&self->url);
                    ERROR4("size of returned body was zero for get method. "
                           "location: %s, status: %d, content_length: %d\n",
                           url_s, self->status, self->content_length);
                    free(url_s);
                }
            } else {
                DEBUG2("status code was %d\n", response.status_code);
            }
        } else {
            char *url_s = self->url.create_string(&self->url);
            ERROR5("an error occured while downloading the resource: '%s'\n"
                   "c: %d / c->stream.error_p: %d / response.broken_p: %d",
                   url_s, !!c, c ? c->stream.error_p : -1,
                   c ? response.broken_p : -1);
            free(url_s);
        }

        return ret;
    }
}

static http_stream_t *http_stream_open(http_stream_t *self)
{
    char *s = NULL;
    TRACE2("Opening url: %s", s = self->url.create_string(&self->url));
    if (s) {
        free(s);
    }

    return http_stream_do(self, GET);
}

static http_stream_t *http_stream_stat(http_stream_t *self)
{
    return http_stream_do(self, HEAD);
}

http_stream_t *http_stream_instantiate(http_stream_t *x)
{
    http_stream_t *h = http_stream_instantiate_super(x);

    url_instantiate_toplevel(&h->url);

    h->open = http_stream_open;
    h->stat = http_stream_stat;

    h->status = 0;
    h->redirected = 0;
    h->content_length = 0;

    return h;
}
