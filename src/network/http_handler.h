/* a10 670
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/network/http_handler.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 670 */

#ifndef KNOS_NETWORK_HTTP_HANDLER_H
#define KNOS_NETWORK_HTTP_HANDLER_H

/*
  basic http/1.1 handler.

  doesn't support things like request pipelining...
*/

#include <network/transport_callbacks.h>
#include <libc/pthread.h>
#include <library/memory.h>

typedef struct http_response_t {
    object_t super;
    int status_code;
    char *location;
    char *content_type;

    int content_length;
    int chunked_p;
    int connection_close_p;
    int broken_p;

    char *data;
    int size;

    pthread_mutex_t response_mutex;
    pthread_cond_t response_complete; /* signaled when the response arrived */
} http_response_t;

CLASS_INHERIT(http_response, object)

typedef enum {
    NONE,
    REQUEST,
    STATUS_LINE,
    HEADER,
    BODY,
    CHUNKED_BODY, /* exclusive */
    DONE
} http_state_t;

typedef enum { GET, HEAD } http_method_t;

typedef struct http_handler_t {
    transport_callbacks_t super;
    pthread_mutex_t callback_mutex;
    url_t url;
    http_state_t state;
    pthread_mutex_t state_mutex;
    http_response_t *response;

    /* provides response object */
    void (*new)(struct http_handler_t *self, http_response_t *response);

    /* send http requests */
    void (*prepare_head_request)(struct http_handler_t *self, url_t *url);
    void (*prepare_get_request)(struct http_handler_t *self, url_t *url);

    /* get state */
    http_state_t (*get_state)(struct http_handler_t *self);

    char *request;
    int request_index;
    int request_len;
    http_method_t request_type;

    /* parsers: */
    /* status line parser */
    enum {
        ST_VERSION,
        ST_SP1,
        ST_STATUS,
        ST_SP2,
        ST_REASON,
        ST_CR,
        ST_CRLF,
        ST_WEIRD
    } st_status,
        st_next_status;
    char st_code[4];
    int st_index;
    /* returns number of char processed */
    int (*st_parse_block)(struct http_handler_t *self, char *buffer, int n);

    /* header parser */
    enum {
        HDR_CR,
        HDR_CRLF,
        HDR_CRLFCR,
        HDR_CRLFCRLF,
        HDR_FIELD,
        HDR_COLON,
        HDR_VALUE,
        HDR_WEIRD
    } hdr_status,
        hdr_next_status;
    char hdr_field[256];
    char hdr_value[256];
    int hdr_index;
    int (*hdr_parse_block)(struct http_handler_t *self, char *buffer, int n);

    /* body decoder */
    char *bdy_data;
    int bdy_size;
    int bdy_index;
    int (*bdy_decode_block)(struct http_handler_t *self, char *buffer, int n);

    /* chunk decoder */
    enum {
        BDY_CHUNK_SIZE,
        BDY_CHUNK_EXT,
        BDY_CHUNK_CR,
        BDY_CHUNK_CRLF,
        BDY_CHUNK_DATA,
        BDY_CHUNK_DATA_END,
        BDY_CHUNK_DATA_CR,
        BDY_CHUNK_DATA_CRLF,
        BDY_CHUNK_TRAILER,
        BDY_CHUNK_WEIRD
    } chnk_status,
        chnk_next_status;
    /* for grabbing the chunk size string */
    char chnk_size[32];
    int chnk_size_index;
    /* for processing the chunk itself */
    int chunk_size;
    int chunk_size_index;
    int (*bdy_decode_chunked_block)(struct http_handler_t *self, char *buffer,
                                    int n);

    /* trailer decoder */
    enum {
        TRLR,
        TRLR_TRLR,
        TRLR_CR,
        TRLR_CRLF,
        TRLR_CRLFCR,
        TRLR_CRLFCRLF
    } chnk_trlr_status;
} http_handler_t;

CLASS_INHERIT(http_handler, transport_callbacks);

#endif
