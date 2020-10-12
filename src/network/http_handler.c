/* a10 555
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/http_handler.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 555 */

#include <network/http_handler.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_NETWORK_HTTP_HANDLER);

#include <libc/stdlib.h>
#include <libc/string.h>

static void set_state(http_handler_t *self, http_state_t nu)
{
    if (pthread_mutex_lock(&self->state_mutex))
        ERROR("state_mutex lock");
    self->state = nu;
    if (pthread_mutex_unlock(&self->state_mutex))
        ERROR("state_mutex unlock");
}

static http_state_t get_state(http_handler_t *self)
{
    http_state_t state;
    if (pthread_mutex_lock(&self->state_mutex))
        ERROR("state_mutex lock");
    state = self->state;
    if (pthread_mutex_unlock(&self->state_mutex))
        ERROR("state_mutex unlock");

    return state;
}

static void hh_new(http_handler_t *self, http_response_t *response)
{
    pthread_mutex_lock(&self->callback_mutex);
    set_state(self, NONE);
    self->response = response;

    pthread_mutex_unlock(&self->callback_mutex);
}

static void prepare_request(http_handler_t *self, const char *format,
                            url_t *url)
{
    url_instantiate_toplevel(&self->url);
    self->url.copy(&self->url, url);
    self->request_index = 0;

    memset(self->response, 0, sizeof(http_response_t));
    pthread_mutex_init(&self->response->response_mutex, NULL);
    pthread_cond_init(&self->response->response_complete, NULL);

    if (self->url.query) {
        char temp[strlen(self->url.path) + 1 + strlen(self->url.query) + 1];
        sprintf(temp, "%s?%s", self->url.path, self->url.query);
        self->request =
            malloc(strlen(format) + strlen(temp) + strlen(self->url.server));
        sprintf(self->request, format, temp, self->url.server);
    } else {
        self->request = malloc(strlen(format) + strlen(self->url.path) +
                               strlen(self->url.server));
        sprintf(self->request, format, self->url.path, self->url.server);
    }

    self->request_len = strlen(self->request);
}

static void hh_prepare_head_request(http_handler_t *self, url_t *url)
{
    const char *format = "HEAD %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: +1-1 "
                         "(http://neq.tpolm.com/)\r\n"
                         //"Connection: close\r\n"
                         "\r\n";

    pthread_mutex_lock(&self->callback_mutex);
    prepare_request(self, format, url);

    self->request_type = HEAD;
    set_state(self, REQUEST);

    TRACE("%s", self->request);
    pthread_mutex_unlock(&self->callback_mutex);
}

static void hh_prepare_get_request(http_handler_t *self, url_t *url)
{
    const char *format = "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: +1-1 "
                         "(http://neq.tpolm.com/)\r\n"
                         //"Connection: close\r\n"
                         "\r\n";

    pthread_mutex_lock(&self->callback_mutex);
    prepare_request(self, format, url);

    self->request_type = GET;
    set_state(self, REQUEST);

    TRACE("%s", self->request);
    pthread_mutex_unlock(&self->callback_mutex);
}

static int hh_on_write(transport_callbacks_t *zelf, channel_t *c, char *buffer,
                       int n)
{
    http_handler_t *self = (http_handler_t *)zelf;
    int ret = 0;

    pthread_mutex_lock(&self->callback_mutex);
    if (get_state(self) == REQUEST) {
        int remaining = self->request_len - self->request_index;
        int todo = n < remaining ? n : remaining;

        memcpy(buffer, self->request + self->request_index, todo);
        self->request_index += todo;
        if (self->request_index >= self->request_len) {
            free(self->request);
            self->request = NULL;
            set_state(self, STATUS_LINE);
            self->st_status = ST_WEIRD;
            self->st_next_status = ST_VERSION;
        }

        ret = todo;
    }

    pthread_mutex_unlock(&self->callback_mutex);

    return ret;
}

static int hh_st_parse_block(http_handler_t *self, char *buffer, int n)
{
    int i;
    int just_in_p;
    int done_p = 0;

    for (i = 0; i < n && !done_p; i++) {
        switch (buffer[i]) {
        case '\r':
            self->st_next_status = ST_CR;
            break;
        case '\n':
            if (self->st_status == ST_CR)
                self->st_next_status = ST_CRLF;
            else
                self->st_next_status = ST_WEIRD;
            break;
        case ' ':
            if (self->st_status == ST_VERSION)
                self->st_next_status = ST_SP1;
            else if (self->st_status == ST_STATUS)
                self->st_next_status = ST_SP2;
            else
                self->st_next_status = ST_WEIRD;
            break;
        default:
            if (self->st_status == ST_SP1)
                self->st_next_status = ST_STATUS;
            else if (self->st_status == ST_SP2)
                self->st_next_status = ST_REASON;
        }

        just_in_p = (self->st_next_status != self->st_status);

        if (self->st_next_status == ST_CRLF) {
            // the end, quit
            done_p = 1;
        } else if (just_in_p && self->st_status == ST_STATUS) {
            // leaving ST_STATUS
            self->st_code[self->st_index] = '\0';

            sscanf(self->st_code, "%u", &self->response->status_code);

            set_state(self, HEADER);
            self->hdr_status = HDR_WEIRD;
            self->hdr_next_status = HDR_FIELD;
        } else if (self->st_next_status == ST_STATUS) {
            if (just_in_p) {
                self->st_index = 0;
            }
            if (self->st_index < 3)
                self->st_code[self->st_index++] = buffer[i];
        }

        self->st_status = self->st_next_status;
    }

    return i;
}

static int hh_hdr_parse_block(http_handler_t *self, char *buffer, int n)
{
    int i;
    int just_in_p;
    int done_p = 0;

    for (i = 0; i < n && !done_p; i++) {
        switch (buffer[i]) {
        case '\r':
            if (self->hdr_status == HDR_CRLF)
                self->hdr_next_status = HDR_CRLFCR;
            else
                self->hdr_next_status = HDR_CR;
            break;
        case '\n':
            if (self->hdr_status == HDR_CRLFCR)
                self->hdr_next_status = HDR_CRLFCRLF;
            else if (self->hdr_status == HDR_CR)
                self->hdr_next_status = HDR_CRLF;
            else
                self->hdr_next_status = HDR_WEIRD;
            break;
        case ':':
            if (self->hdr_status == HDR_FIELD)
                self->hdr_next_status = HDR_COLON;
            else if (self->hdr_status != HDR_VALUE)
                self->hdr_next_status = HDR_WEIRD;
            break;
        case ' ':
            if (self->hdr_status == HDR_COLON)
                break;
        default:
            if (self->hdr_status == HDR_COLON)
                self->hdr_next_status = HDR_VALUE;
            else if (self->hdr_status != HDR_VALUE)
                self->hdr_next_status = HDR_FIELD;
        }

        just_in_p = (self->hdr_next_status != self->hdr_status);

        if (just_in_p) {
            if (self->hdr_status == HDR_FIELD)
                self->hdr_field[self->hdr_index] = '\0';
            else if (self->hdr_status == HDR_VALUE)
                self->hdr_value[self->hdr_index] = '\0';
        }

        if (self->hdr_next_status == HDR_CRLFCRLF) {
            // the empty line at the end, quit
            self->bdy_data = NULL;
            self->bdy_size = 0;
            self->bdy_index = 0;

            if (self->request_type == HEAD) {
                pthread_mutex_lock(&self->response->response_mutex);
                set_state(self, DONE);
                pthread_cond_signal(&self->response->response_complete);
                pthread_mutex_unlock(&self->response->response_mutex);
            } else if (self->request_type == GET) {
                if (self->response->chunked_p) {
                    set_state(self, CHUNKED_BODY);
                    self->chnk_status = BDY_CHUNK_WEIRD;
                    self->chnk_next_status = BDY_CHUNK_SIZE;
                    self->chnk_size_index = 0;
                } else
                    set_state(self, BODY);
            }

            done_p = 1;
        } else if (self->hdr_next_status == HDR_FIELD) {
            if (just_in_p) {
                self->hdr_index = 0;
            }

            if (self->hdr_index < 254) {
                self->hdr_field[self->hdr_index++] = buffer[i];
            }
        } else if (self->hdr_next_status == HDR_VALUE) {
            if (just_in_p) {
                self->hdr_index = 0;
            }

            if (self->hdr_index < 254) {
                self->hdr_value[self->hdr_index++] = buffer[i];
            }
        }

        if (just_in_p && self->hdr_status == HDR_VALUE) {
            // leaving value ...

            if (!strcmp(self->hdr_field, "Content-Type")) {
                self->response->content_type = strdup(self->hdr_value);
            } else if (!strcmp(self->hdr_field, "Location")) {
                self->response->location = strdup(self->hdr_value);
            } else if (!strcmp(self->hdr_field, "Content-Length")) {
                sscanf(self->hdr_value, "%u", &self->response->content_length);
            } else if (!strcmp(self->hdr_field, "Transfer-Encoding") &&
                       !strcmp(self->hdr_value, "chunked")) {
                self->response->chunked_p = 1;
            } else if (!strcmp(self->hdr_field, "Connection") &&
                       !strcmp(self->hdr_value, "close")) {
                self->response->connection_close_p = 1;
            }
        }

        self->hdr_status = self->hdr_next_status;
    }

    return i;
}

/* identity decoding */
static int hh_bdy_decode_block(http_handler_t *self, char *buffer, int n)
{
    if (self->bdy_index == self->bdy_size) {
        self->bdy_size += n;
        self->bdy_data = realloc(self->bdy_data, self->bdy_size);
    }

    memcpy(self->bdy_data + self->bdy_index, buffer, n);
    self->bdy_index += n;

    if (self->response->content_length > 0 &&
        self->bdy_index == self->response->content_length) {
        // we know now that we got everything
        self->response->data = self->bdy_data;
        self->response->size = self->bdy_size;
        pthread_mutex_lock(&self->response->response_mutex);
        set_state(self, DONE);
        pthread_cond_signal(&self->response->response_complete);
        pthread_mutex_unlock(&self->response->response_mutex);
    }

    return n;
}

static int hh_bdy_decode_chunked_block(http_handler_t *self, char *buffer,
                                       int n)
{
    int done = 0;
    char *readptr = buffer;

    while (n) {
        if (self->chnk_status == BDY_CHUNK_DATA) {
            int remaining = self->chunk_size - self->chunk_size_index;
            int todo = n < remaining ? n : remaining;

            if (self->chunk_size && !remaining)
                self->chnk_status = BDY_CHUNK_DATA_END;
            else if (todo) {
                if (self->bdy_index == self->bdy_size) {
                    self->bdy_size += todo;
                    self->bdy_data = realloc(self->bdy_data, self->bdy_size);
                }

                memcpy(self->bdy_data + self->bdy_index, readptr, todo);
                self->bdy_index += todo;
                self->chunk_size_index += todo;
                done += todo;
                n -= todo;
                readptr += todo;
            }
        }

        if (self->chnk_status == BDY_CHUNK_TRAILER) {
            int done_p = 0;
            int i;

            /* parse away header like trailer */
            for (i = 0; i < n && !done_p; i++) {
                switch (readptr[i]) {
                case '\r':
                    if (self->chnk_trlr_status != TRLR_TRLR)
                        self->chnk_trlr_status = TRLR_CRLFCR;
                    else
                        self->chnk_trlr_status = TRLR_CR;
                    break;
                case '\n':
                    if (self->chnk_trlr_status == TRLR_CRLFCR)
                        self->chnk_trlr_status = TRLR_CRLFCRLF;
                    else
                        self->chnk_trlr_status = TRLR_CRLF;
                    break;
                default:
                    self->chnk_trlr_status = TRLR_TRLR;
                }

                if (self->chnk_trlr_status == TRLR_CRLFCRLF) {
                    pthread_mutex_lock(&self->response->response_mutex);
                    set_state(self, DONE);
                    pthread_cond_signal(&self->response->response_complete);
                    pthread_mutex_unlock(&self->response->response_mutex);
                    done_p = 1;
                }
            }
            done += i;
            n -= i;
            readptr += i;

            if (done_p)
                return done;
        }

        if (self->chnk_status != BDY_CHUNK_DATA) {
            // else!
            int i;
            int just_in_p;
            int done_p = 0;

            for (i = 0; i < n && !done_p; i++) {
                switch (readptr[i]) {
                case '\r':
                    if (self->chnk_status == BDY_CHUNK_CRLF) {
                        if (self->chunk_size > 0)
                            self->chnk_next_status = BDY_CHUNK_DATA;
                        else
                            self->chnk_next_status = BDY_CHUNK_TRAILER;
                    } else if (self->chnk_status == BDY_CHUNK_SIZE ||
                               self->chnk_status == BDY_CHUNK_EXT)
                        self->chnk_next_status = BDY_CHUNK_CR;
                    else if (self->chnk_status == BDY_CHUNK_DATA_END)
                        self->chnk_next_status = BDY_CHUNK_DATA_CR;
                    else
                        self->chnk_next_status = BDY_CHUNK_WEIRD;
                    break;
                case '\n':
                    if (self->chnk_status == BDY_CHUNK_CRLF) {
                        if (self->chunk_size > 0)
                            self->chnk_next_status = BDY_CHUNK_DATA;
                        else
                            self->chnk_next_status = BDY_CHUNK_TRAILER;
                    } else if (self->chnk_status == BDY_CHUNK_CR)
                        self->chnk_next_status = BDY_CHUNK_CRLF;
                    else if (self->chnk_status == BDY_CHUNK_DATA_CR)
                        self->chnk_next_status = BDY_CHUNK_DATA_CRLF;
                    else
                        self->chnk_next_status = BDY_CHUNK_WEIRD;
                    break;
                case ' ':
                    if (self->chnk_status == BDY_CHUNK_CRLF) {
                        if (self->chunk_size > 0)
                            self->chnk_next_status = BDY_CHUNK_DATA;
                        else
                            self->chnk_next_status = BDY_CHUNK_TRAILER;
                    } else if (self->chnk_status == BDY_CHUNK_SIZE) {
                        self->chnk_next_status = BDY_CHUNK_EXT;
                        break;
                    }
                default:
                    if (self->chnk_status == BDY_CHUNK_CRLF) {
                        if (self->chunk_size > 0)
                            self->chnk_next_status = BDY_CHUNK_DATA;
                        else
                            self->chnk_next_status = BDY_CHUNK_TRAILER;
                    } else if (self->chnk_status == BDY_CHUNK_DATA_CRLF)
                        self->chnk_next_status = BDY_CHUNK_SIZE;
                }

                just_in_p = (self->chnk_status != self->chnk_next_status);

                if (just_in_p && self->chnk_next_status == BDY_CHUNK_CR) {
                    // leaving chunk size
                    self->chnk_size[self->chnk_size_index] = '\0';
                    sscanf(self->chnk_size, "%x", &self->chunk_size);
                }

                if (self->chnk_next_status == BDY_CHUNK_SIZE) {
                    if (just_in_p) {
                        self->chnk_size_index = 0;
                    }
                    if (self->chnk_size_index < 30)
                        self->chnk_size[self->chnk_size_index++] = readptr[i];
                }

                if (just_in_p && self->chnk_next_status == BDY_CHUNK_DATA) {
                    // entering data region
                    self->chunk_size_index = 0;
                    i--;
                    done_p = 1;
                }

                if (just_in_p && self->chnk_next_status == BDY_CHUNK_TRAILER) {
                    self->chnk_trlr_status = TRLR;
                    self->response->data = self->bdy_data;
                    self->response->size = self->bdy_size;
                    i--;
                    done_p = 1;
                }

                self->chnk_status = self->chnk_next_status;
            }

            done += i;
            n -= i;
            readptr += i;
        }
    }

    return done;
}

static void hh_on_read(transport_callbacks_t *zelf, channel_t *c, char *buffer,
                       int n)
{
    http_handler_t *self = (http_handler_t *)zelf;

    pthread_mutex_lock(&self->callback_mutex);
    if (n && get_state(self) == STATUS_LINE) {
        int done = self->st_parse_block(self, buffer, n);
        buffer += done;
        n -= done;
    }
    if (n && get_state(self) == HEADER) {
        int done = self->hdr_parse_block(self, buffer, n);
        buffer += done;
        n -= done;
    }
    if (n) {
        if (get_state(self) == BODY) {
            int done = self->bdy_decode_block(self, buffer, n);
            buffer += done;
            n -= done;
        } else if (get_state(self) == CHUNKED_BODY) {
            int done = self->bdy_decode_chunked_block(self, buffer, n);
            buffer += done;
            n -= done;
        }
    }
    if (n > 0)
        DEBUG("remaining: %d", n);
    pthread_mutex_unlock(&self->callback_mutex);
}

static void hh_on_close(transport_callbacks_t *zelf, channel_t *chan)
{
    http_handler_t *self = (http_handler_t *)zelf;

    pthread_mutex_lock(&self->callback_mutex);
    if (get_state(self) != DONE) {
        if (!self->response->connection_close_p && chan->stream.broken_p) {
            self->response->broken_p = 1;
            DEBUG("connection broken. (state: %d)", get_state(self));
            if (get_state(self) == BODY) {
                DEBUG("%d / %d", self->bdy_size,
                       self->response->content_length);
            }
        } else if (!self->response->data && self->bdy_size > 0) {
            // response data hasn't been updated although bdy_size > 0,
            // means the decoder was blind and didn't have content_length
            self->response->data = self->bdy_data;
            self->response->size = self->bdy_size;
        }
        pthread_mutex_lock(&self->response->response_mutex);
        set_state(self, DONE);
        pthread_cond_signal(&self->response->response_complete);
        pthread_mutex_unlock(&self->response->response_mutex);
    }
    pthread_mutex_unlock(&self->callback_mutex);
}

http_handler_t *http_handler_instantiate(http_handler_t *x)
{
    http_handler_t *h = http_handler_instantiate_super(x);

    pthread_mutex_init(&h->state_mutex, NULL);
    pthread_mutex_init(&h->callback_mutex, NULL); /* global lock */

    h->new = hh_new;
    h->prepare_head_request = hh_prepare_head_request;
    h->prepare_get_request = hh_prepare_get_request;
    h->get_state = get_state;
    h->st_parse_block = hh_st_parse_block;
    h->hdr_parse_block = hh_hdr_parse_block;
    h->bdy_decode_block = hh_bdy_decode_block;
    h->bdy_decode_chunked_block = hh_bdy_decode_chunked_block;

    h->super.write = hh_on_write;
    h->super.read = hh_on_read;
    h->super.close = hh_on_close;

    return h;
}
