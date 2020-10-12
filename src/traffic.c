/* a10 136
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/traffic.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 136 */

#include <libc/pthread.h>
#include <libc/signal.h>
#include "image_fetcher.h"
#include <library/http_stream.h>
#include <library/vector_impl.h>
#include <lib/chance.h>
#include <lib/url_open.h>
#include <libc/string.h>
#include <lib/pixel.h>

#include <pthread.h>
#include <system/demo.h>
#include <library/memory.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_TRAFFIC);

#include <library/thread-helper.h>
#include <library/time.h>

#if KNOS_BUILD == KNOS_RELEASE
static const char *sources_url =
    "zip://virtual://resources/+1-1.dat#sources.text";
static const char *base_url = "zip://virtual://resources/+1-1.dat#";
#else
static const char *sources_url = "virtual://resources/dat/sources.text";
static const char *base_url = "virtual://resources/dat/";
#endif

static int freeze_datafiles_p = 0;
static int restore_from_frozen_datafiles_p = 0;
static int is_multithreaded_p = 1;

void traffic_freeze_p(int predicat)
{
    freeze_datafiles_p = !!predicat;
    restore_from_frozen_datafiles_p = !predicat;
}

void traffic_restore_p(int predicat, const char *restore_base_url)
{
    restore_from_frozen_datafiles_p = !!predicat;
    freeze_datafiles_p = !predicat;
    if (restore_base_url)
        base_url = restore_base_url;
}

typedef struct {
    object_t super;
    url_fetcher_t *fetcher;
    int spot;     /* for projector threads */
    char *source; /* for parsing threads */

    /* completion condition */
    pthread_mutex_t complete_mutex;
    pthread_cond_t complete_cond;
    int finished_p;
} thread_arg_t;

CLASS_INHERIT(thread_arg, object)

thread_arg_t *thread_arg_instantiate(thread_arg_t *x)
{
    thread_arg_t *arg = thread_arg_instantiate_super(x);

    pthread_mutex_init(&arg->complete_mutex, NULL);
    pthread_cond_init(&arg->complete_cond, NULL);

    return arg;
}

int add_to_projector(stream_t *stream);

void *fetch_image(void *rg)
{
    thread_arg_t *arg = rg;

#ifdef LINUX
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigaddset(&set, SIGCHLD);

    pthread_sigmask(SIG_BLOCK, &set, NULL);
#endif

    if (arg->fetcher) {
        unsigned int n;
        pthread_mutex_lock(&arg->fetcher->collected_urls_mutex);
        n = arg->fetcher->collected_urls.next_element;
        pthread_mutex_unlock(&arg->fetcher->collected_urls_mutex);

        if (n > 0) {
            int err = 0;
            int n_err = 0;
            stream_t *stream = NULL;

            do {
                int j;
                char *url;

                err = 0;

                j = unirand() * n;
                {
                    char **string_slot;

                    pthread_mutex_lock(&arg->fetcher->collected_urls_mutex);
                    string_slot = get_element(&arg->fetcher->collected_urls, j);
                    url = *string_slot;
                    /* do not choose that one again */
                    *string_slot = NULL;
                    pthread_mutex_unlock(&arg->fetcher->collected_urls_mutex);
                }

                if (!url) {
                    /* continue if the url was null */
                    err = 1;

                    continue;
                }

                TRACE("opening %d - %s", arg->spot, url);
                {
                    char *url_s;
                    if (0 == strncmp("http://", url, strlen("http://"))) {
                        url_s = url;
                    } else {
                        unsigned int len = strlen("http://") + strlen(url) + 1;
                        url_s = malloc(len);

                        sprintf(url_s, "http://%s", url);
                        url_s[len - 1] = '\0';
                    }

                    stream = url_open(url_s, "rb");

                    if (!stream) {
                        err = 1;
                        TRACE("couldn't fetch from the internet");
                    } else {
                        if (!add_to_projector(stream)) {
                            err = 1;
                            TRACE("couldn't add to projector");
                        } else if (freeze_datafiles_p) {
                            int n = arg->spot;
                            int i = 1;

                            while (n /= 10)
                                i++;
                            n = i + 1;
                            {
                                FILE *fd;
                                char *filename = malloc(n + 4);
                                sprintf(filename, "%d.jpg", arg->spot);
                                fd = fopen(filename, "wb");
                                free(filename);

                                if (fd) {
                                    int n = 0;
                                    char data[4096];

                                    stream_get_callbacks(stream)
                                        ->seek(stream, 0, SEEK_SET);

                                    while (
                                        (n = stream_get_callbacks(stream)->read(
                                             data, 1, 4096, stream))) {
                                        if (1 != fwrite(data, n, 1, fd)) {
                                            ERROR("couldn't write to %s",
                                                   filename);
                                        }
                                    }
                                    fclose(fd);
                                }
                            }
                        }
                        stream_get_callbacks(stream)->close(stream);
                        free(stream);
                    }
                    if (url != url_s)
                        free(url_s);
                }
                free(url);
            } while (err && n_err++ < 10);

            if (!err) {
                TRACE("done with %d", arg->spot);
            } else {
                TRACE("error with %d", arg->spot);
            }
        }
    } else {
        DEBUG("fetcher was null.");
    }

    free(arg);

    return (void *)0;
}

static void *fetch_google(void *rg)
{
    thread_arg_t *arg = rg;

    if (arg->fetcher) {
        /* the global fetcher is available, fine */
        http_stream_t *hstream = http_stream_instantiate_toplevel(NULL);
        url_fetcher_t *fetcher = url_fetcher_instantiate_toplevel(NULL);
        const char *cquery = "http://images.google.com/"
                             "images?q=%s+filetype:jpg&svnum=10&hl=en&imgsafe="
                             "off&start=0&sa=N&filter=0";

        if (!arg->source)
            DEBUG("source was null");
        else {
            char *query = malloc(strlen(arg->source) + strlen(cquery));

            fetcher->callback = images_google_com_callback;

            sprintf(query, cquery, arg->source);

            hstream->url.new(&hstream->url, query);

            TRACE("opening query: %s", query);

            if (hstream->open(hstream)) {
                fetcher->parse_links(fetcher, hstream, 0);
                stream_get_callbacks(&hstream->super.super)
                    ->close(&hstream->super.super);
            }
            free(query);
        }
        free(hstream);

        /* now adds collected urls to global fetcher */
        {
            char **string_slot;
            iterator_t it;

            get_iterator(&fetcher->collected_urls, &it);
            while ((string_slot = iterator_next(&it))) {
                char **dest_slot;
                pthread_mutex_lock(&arg->fetcher->collected_urls_mutex);
                dest_slot = add_element(&arg->fetcher->collected_urls);

                *dest_slot = *string_slot;
                pthread_mutex_unlock(&arg->fetcher->collected_urls_mutex);
            }
        }
        free(fetcher);
    } else {
        DEBUG("fetcher was null");
    }

    pthread_mutex_lock(&arg->complete_mutex);
    pthread_cond_broadcast(&arg->complete_cond);
    arg->finished_p = 1;
    pthread_mutex_unlock(&arg->complete_mutex);

    return (void *)0;
}

static void *fetch_directory(void *rg)
{
    thread_arg_t *arg = rg;

    if (arg->fetcher) {
        /* the global fetcher is available, fine */
        http_stream_t *hstream = http_stream_instantiate_toplevel(NULL);
        url_fetcher_t *fetcher = url_fetcher_instantiate_toplevel(NULL);

        fetcher->callback = http_directory_callback;

        if (!arg->source)
            DEBUG("source was null");
        else {
            hstream->url.new(&hstream->url, arg->source);
            hstream = hstream->open(hstream);

            if (hstream) {
                fetcher->parse_links(fetcher, hstream, 0);
                stream_get_callbacks(&hstream->super.super)
                    ->close(&hstream->super.super);
                free(hstream);
            }
        }

        /* now adds collected urls to global fetcher */
        {
            char **string_slot;
            iterator_t it;

            get_iterator(&fetcher->collected_urls, &it);
            while ((string_slot = iterator_next(&it))) {
                char **dest_slot;
                pthread_mutex_lock(&arg->fetcher->collected_urls_mutex);
                dest_slot = add_element(&arg->fetcher->collected_urls);

                *dest_slot = *string_slot;
                pthread_mutex_unlock(&arg->fetcher->collected_urls_mutex);
            }
        }

        free(fetcher);
    } else {
        DEBUG("fetcher was null");
    }

    pthread_mutex_lock(&arg->complete_mutex);
    pthread_cond_broadcast(&arg->complete_cond);
    arg->finished_p = 1;
    pthread_mutex_unlock(&arg->complete_mutex);

    return (void *)0;
}

static void *fetch_null(void *rg) { return 0; }

uint32_t *videobuffer;
unsigned int width = 0;
unsigned int height = 0;

void start_grabbing_images(int max_images)
{
    url_fetcher_t *fetcher = url_fetcher_instantiate_toplevel(NULL);
    // grab some random images
    int i;
    unsigned int n;
    stream_t *sources;

    if (restore_from_frozen_datafiles_p) {
        for (i = 0; i < max_images; i++) {
            int c = 1;
            n = i;
            while (n /= 10)
                c++;
            n = c + 1;
            {
                char *filename = malloc(strlen(base_url) + n + strlen(".jpg"));
                stream_t *stream;

                sprintf(filename, "%s%d.jpg", base_url, i);
                stream = url_open(filename, "rb");
                add_to_projector(stream);

                free(filename);
            }
        }

        return;
    }

    // parse the sources urls
    sources = url_open(sources_url, "rb");

    if (sources) {
        pthread_t *parsing_threads_ids = NULL;
        thread_arg_t **parsing_threads_args = NULL;
        int thread_count = 0;
        char buf;
        char type[32];
        char source[32];
        int index = 0;
        char *dest = &buf;
        enum { COMMENT, TYPE, SP, SOURCE, LF } new_status = TYPE, status = LF;
        int just_in_p = 0;

        while (stream_get_callbacks(sources)->read(dest, 1, 1, sources)) {
            switch (*dest) {
            case ' ':
                if (status != COMMENT)
                    new_status = SP;
                break;
            case '\n':
                new_status = LF;
                break;
            case '#':
                if (status == LF)
                    new_status = COMMENT;
                break;
            default:
                if (status == LF) {
                    new_status = TYPE;
                } else if (status == SP) {
                    new_status = SOURCE;
                }
            }
            just_in_p = (new_status != status);

            if (just_in_p) {
                if (new_status == TYPE) {
                    index = 0;
                    type[0] = buf;
                } else if (new_status == SOURCE) {
                    index = 0;
                    source[0] = buf;
                } else if (new_status == SP) {
                    type[index] = '\0';
                    dest = &buf;
                } else if (new_status == LF && status != COMMENT) {
                    thread_arg_t *arg;
                    source[index] = '\0';
                    /* starts thread */
                    parsing_threads_ids =
                        realloc(parsing_threads_ids,
                                sizeof(pthread_t) * (thread_count + 1));
                    parsing_threads_args =
                        realloc(parsing_threads_args,
                                sizeof(thread_arg_t *) * (thread_count + 1));
                    parsing_threads_ids[thread_count] = 0;

                    arg = thread_arg_instantiate_toplevel(NULL);
                    arg->fetcher = fetcher;
                    arg->source = strdup(source);

                    /* save address of argument */
                    parsing_threads_args[thread_count] = arg;

                    /* launch */
                    {
                        void *(*fetching_function)(void *) = NULL;
                        if (!strcmp(type, "images.google.com")) {
                            fetching_function = fetch_google;
                        } else if (!strcmp(type, "directory")) {
                            fetching_function = fetch_directory;
                        } else {
                            fetching_function = fetch_null;
                        }

                        if (is_multithreaded_p) {
                            if (kn_thread_create(
                                    &parsing_threads_ids[thread_count],
                                    fetching_function, arg)) {
                                ERROR("error in pthread_create");
                            } else {
                                thread_count++;
                            }
                        } else {
                            fetching_function(arg);
                        }
                    }

                    dest = &buf;
                }
            }

            status = new_status;
            if (status == TYPE) {
                if (index < 30) {
                    dest = &type[++index];
                } else {
                    dest = &buf;
                }
            } else if (status == SOURCE) {
                if (index < 30) {
                    dest = &source[++index];
                } else {
                    dest = &buf;
                }
            }
        }

        /* display something while waiting for the parsers to exit */
        width = demo_get_instance()->video_width;
        height = demo_get_instance()->video_height;
        videobuffer = calloc(sizeof(uint32_t), width * height);

        TRACE("waiting for %d threads.", thread_count);
        /* wait for parsers to return */
        if (thread_count > 0) {
            unsigned int *times = calloc(thread_count, sizeof(unsigned int));

            n = thread_count;
            while (n) {
                double p = 1000.0 / n;
                TRACE("threads still left: %d", n);
                for (i = 0; i < thread_count; i++) {
                    int status = 0;
                    struct timespec timeout;
                    double ms = get_unix_milliseconds() + p;
                    timeout.tv_sec = ms / 1000.0;
                    timeout.tv_nsec =
                        (ms - 1000.0 * timeout.tv_sec) * 1000000.0;

                    if (!parsing_threads_args[i])
                        continue;

                    TRACE("waiting for '%d'", parsing_threads_ids[i]);
                    {
                        int h_block = height / thread_count;
                        uint32_t *scanline = videobuffer + i * width * h_block;
                        while (h_block--) {
                            memset(scanline, grey(0x44),
                                   width * sizeof(uint32_t));
                            scanline += width;
                        }
                        demo_get_instance()->update(demo_get_instance(),
                                                    videobuffer);
                    }
                    pthread_mutex_lock(
                        &parsing_threads_args[i]->complete_mutex);
                    status =
                        parsing_threads_args[i]->finished_p ||
                        !pthread_cond_timedwait(
                            &parsing_threads_args[i]->complete_cond,
                            &parsing_threads_args[i]->complete_mutex, &timeout);
                    if (status) {
                        TRACE("got completion message");
                        pthread_mutex_unlock(
                            &parsing_threads_args[i]->complete_mutex);
                        pthread_join(parsing_threads_ids[i], NULL);
                        if (parsing_threads_args[i]->source)
                            free(parsing_threads_args[i]->source);
                        pthread_mutex_destroy(
                            &parsing_threads_args[i]->complete_mutex);
                        pthread_cond_destroy(
                            &parsing_threads_args[i]->complete_cond);

                        free(parsing_threads_args[i]);
                        parsing_threads_args[i] = NULL;
                        n--;
                        {
                            int h_block = height / thread_count;
                            uint32_t *scanline =
                                videobuffer + i * width * h_block;
                            unsigned char amp = 0x80;
                            if (times[i] >= 0x7f)
                                times[i] = 0x7f;
                            amp += times[i];
                            while (h_block--) {
                                memset(scanline, grey(amp),
                                       width * sizeof(uint32_t));
                                scanline += width;
                            }
                            demo_get_instance()->update(demo_get_instance(),
                                                        videobuffer);
                        }
                        TRACE("thread: %d completed.", parsing_threads_ids[i]);
                    } else {
                        TRACE("timedout");
                        pthread_mutex_unlock(
                            &parsing_threads_args[i]->complete_mutex);
                        times[i]++;
                        /* blackens stripe */
                        {
                            int h_block = height / thread_count;
                            uint32_t *scanline =
                                videobuffer + i * width * h_block;
                            while (h_block--) {
                                memset(scanline, grey(0),
                                       width * sizeof(uint32_t));
                                scanline += width;
                            }
                            demo_get_instance()->update(demo_get_instance(),
                                                        videobuffer);
                        }
                    }
                }
            }

            free(times);
        }

        free(parsing_threads_args);
        free(parsing_threads_ids);
    }

    pthread_mutex_lock(&fetcher->collected_urls_mutex);
    n = fetcher->collected_urls.next_element;
    pthread_mutex_unlock(&fetcher->collected_urls_mutex);

    if (n > 0) {
        TRACE("fetching %d images.", max_images);
        for (i = 0; i < max_images; i++) {
            thread_arg_t *arg;
            pthread_t id;

            arg = calloc(sizeof(thread_arg_t), 1);

            arg->spot = i;
            arg->fetcher = fetcher;

            if (kn_thread_create_background(&id, fetch_image, arg)) {
                ERROR("error in pthread_create");
            }
        }
    }
}
