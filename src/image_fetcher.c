/* a10 30
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_fetcher.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 30 */

#include "image_fetcher.h"

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_IMAGE_FETCHER);

#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/ctype.h>

#include <library/vector_impl.h>

static void add_url(url_fetcher_t *self, url_t *url)
{
    url_t *url_slot = add_element(&self->urls);
    url_instantiate_toplevel(url_slot);

    url_slot->copy(url_slot, url);

    if (url->path && strlen(url->path) &&
        url->path[strlen(url->path) - 1] == '/') {
        // if directory listing (prolly apache style), remove query part
        // from cached url. (we don't want to fetch the same listing many
        // times for the different sorts.
        url_slot->query = NULL;
    }
}

static int url_not_seen(url_fetcher_t *self, url_t *url)
{
    iterator_t i;
    url_t *url_slot;
    char *query = NULL;

    if (url->path && strlen(url->path) &&
        url->path[strlen(url->path) - 1] == '/') {
        // if directory listing (prolly apache style), remove query part
        // from cached url. (we don't want to fetch the same listing many
        // times for the different sorts.
        query = url->query;
        url->query = NULL;
    }

    get_iterator(&self->urls, &i);
    while ((url_slot = iterator_next(&i))) {
        if (url_slot->equals(url_slot, url))
            break;
    }

    if (query)
        url->query = query;

    return !url_slot;
}

static void add_collected_url(url_fetcher_t *self, char *url)
{
    char **string_slot;

    pthread_mutex_lock(&self->collected_urls_mutex);
    string_slot = add_element(&self->collected_urls);

    *string_slot = url;
    pthread_mutex_unlock(&self->collected_urls_mutex);
}

int images_google_com_callback(url_fetcher_t *self, http_stream_t *url_found)
{
    int ret_p = 0;
    int n;

    if (url_found) {
        n = strlen(url_found->url.path);

        if (n > 3 && !strcmp(url_found->url.path + n - 3, "jpg")) {
            /*url_found->stat(url_found);
              if(url_found->status == 200)
            */
        } else if ((strstr(url_found->url.path, "/images") ||
                    strstr(url_found->url.path, "/img")) &&
                   (!url_found->url.query ||
                    !strstr(url_found->url.query, "imgsafe=on")) &&
                   url_not_seen(self, &url_found->url)) {
            if (url_found->url.query &&
                strstr(url_found->url.query, "imgurl=")) {
                char *imgurl;
                char *imgurl_begin =
                    strstr(url_found->url.query, "imgurl=") + strlen("imgurl=");
                char *imgurl_end;

                imgurl_end = strstr(imgurl_begin, "&");
                if (!imgurl_end) {
                    imgurl_end = imgurl_begin + strlen(imgurl_begin);
                }
                if (!strncmp(imgurl_end - 3, "jpg", 3)) {
                    imgurl = calloc(imgurl_end - imgurl_begin + 1, 1);
                    strncpy(imgurl, imgurl_begin, imgurl_end - imgurl_begin);

                    add_collected_url(self, imgurl);
                }
            } else if (self->depth < self->guard) {
                ret_p = 1;
            }
        }
    }

    return ret_p;
}

int http_directory_callback(url_fetcher_t *self, http_stream_t *url_found)
{
    int ret_p = 0;

    if (url_found) {
        url_found->stat(url_found);
        if (url_found->status == 200) {
            if (url_found->content_type &&
                (strstr(url_found->content_type, "image/jpeg") ||
                 strstr(url_found->content_type, "image/png"))) {
                add_collected_url(
                    self, url_found->url.create_string(&url_found->url));
            } else if (!strstr(url_found->url.path, "..") &&
                       url_not_seen(self, &url_found->url) &&
                       url_found->url.is_subdir_of(&url_found->url,
                                                   &self->from_url->url)) {
                if (url_found->content_type &&
                    strstr(url_found->content_type, "text/html"))
                    ret_p = 1;
            }
        }
    } else {
        DEBUG("stream passed is null");
    }

    return ret_p;
}

static void parse_links(url_fetcher_t *state, http_stream_t *stream, int depth)
{
    if (depth < state->guard && stream && stream->status == 200 &&
        stream->content_type && strstr(stream->content_type, "text/html")) {
        stream_t *s;
        enum {
            TAG_IN,
            TAG_NOTA,
            IN_A,
            TAG_A,
            H,
            HR,
            HRE,
            HREF,
            EQUALS,
            LINK,
            QUOTE,
            TAG_OUT
        } status = TAG_OUT,
          new_status = TAG_OUT;
        int index = 0;
        char link_url[256];
        char character;
        int just_in_p = 1;

        s = &stream->super.super;

        memset(link_url, 0, 256);

        add_url(state, &stream->url);

        TRACE("parsing... '%s%s?%s'\n", stream->url.server, stream->url.path,
               stream->url.query);

        while (stream_get_callbacks(s)->read(&character, 1, 1, s)) {
            switch (tolower((int)character)) {
            case '<':
                new_status = TAG_IN;
                break;
            case 'a':
                if (status == TAG_IN)
                    new_status = IN_A;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
                break;
            case 'h':
                if (status == TAG_A)
                    new_status = H;
                else if (status == TAG_IN)
                    new_status = TAG_NOTA;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
                break;
            case 'r':
                if (status == H)
                    new_status = HR;
                else if (status == TAG_IN)
                    new_status = TAG_NOTA;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
                break;
            case 'e':
                if (status == HR)
                    new_status = HRE;
                else if (status == TAG_IN)
                    new_status = TAG_NOTA;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
                break;
            case 'f':
                if (status == HRE)
                    new_status = HREF;
                else if (status == TAG_IN)
                    new_status = TAG_NOTA;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
                break;
            case '=':
                if (status == HREF)
                    new_status = EQUALS;
                else if (status == TAG_IN)
                    new_status = TAG_NOTA;
                break;
            case '\0' ... ' ':
                if (status == IN_A)
                    new_status = TAG_A;
                else if (status == EQUALS)
                    new_status = LINK;
                else if (status == LINK)
                    new_status = TAG_A;
                break;
            case '>':
                if (status == QUOTE)
                    new_status = LINK;
                else
                    new_status = TAG_OUT;
                break;
            case '"':
                if (status == EQUALS) {
                    new_status = QUOTE;
                } else if (status == QUOTE || status == LINK) {
                    new_status = TAG_A;
                }
                break;
            default:
                if (status == TAG_IN)
                    new_status = TAG_NOTA;
                else if (status == EQUALS || status == QUOTE)
                    new_status = LINK;
            }

            just_in_p = (new_status != status);
            if (just_in_p) {
                if (status == LINK) {
                    if (state->callback) {
                        http_stream_t *stream2 =
                            http_stream_instantiate_toplevel(NULL);

                        link_url[index] = '\0';

                        TRACE("url found: %s", link_url);

                        stream2->url.new2(&stream2->url, &stream->url,
                                          link_url);

                        state->depth = depth + 1;
                        state->from_url = stream;

                        if (state->callback(state, stream2)) {
                            http_stream_t *effective_stream =
                                stream2->open(stream2);
                            if (!effective_stream) {
                                WARNING("couldn't open %s", link_url);
                            } else {
                                parse_links(state, effective_stream, depth + 1);
                                stream_get_callbacks(
                                    &effective_stream->super.super)
                                    ->close(&effective_stream->super.super);
                                http_stream_retire(effective_stream);
                                if (effective_stream != stream2) {
                                    http_stream_retire(stream2);
                                }
                            }
                        } else {
                            http_stream_retire(stream2);
                        }
                    }
                }
            }

            /* copy link url data */
            if (new_status == LINK) {
                if (just_in_p) {
                    index = 0;
                }

                if (index < 254) {
                    link_url[index] = character;
                    index++;
                }
            }

            status = new_status;
        }
    }
}

url_fetcher_t *url_fetcher_instantiate(url_fetcher_t *x)
{
    url_fetcher_t *u = url_fetcher_instantiate_super(x);

    vector_new(&u->urls, url_t);

    pthread_mutex_init(&u->collected_urls_mutex, NULL);
    vector_new(&u->collected_urls, char *);

    u->parse_links = parse_links;

    u->guard = 2;

    return u;
}
