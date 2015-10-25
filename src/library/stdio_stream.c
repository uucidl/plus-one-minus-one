/* a10 836
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stdio_stream.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 836 */

#include <library/stdio_stream.h>

#include <libc/stdlib.h>
#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_STDIO_STREAM);

static char *stdio_get_as_memory_area(stream_t *self, int64_t *returned_size)
{
    stdio_stream_t *zelf = (stdio_stream_t *)self;
    int status = 1;
    int64_t size, n;
    char *buffer = NULL;
    FILE *file = zelf->fd;

    /* find size of file */
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    if (!size)
        status = 0;
    else {
        fseek(file, 0, SEEK_SET);
        buffer = calloc(size, 1);

        n = size;
        if (fread(buffer, n, 1, file) <= 0) {
            free(buffer);
            status = 0;
        }
    }

    *returned_size = size;

    if (status)
        return buffer;
    else
        return NULL;
}

static void stdio_free_memory_area(stream_t *self, char *buffer,
                                   int64_t returned_size)
{
    free(buffer);
}

static size_t stdio_read(void *data, size_t size, size_t nmemb, stream_t *self)
{
    return fread(data, size, nmemb, ((stdio_stream_t *)self)->fd);
}

static size_t stdio_write(const void *data, size_t size, size_t nmemb,
                          stream_t *self)
{
    return fwrite(data, size, nmemb, ((stdio_stream_t *)self)->fd);
}

static int stdio_seek(stream_t *self, int64_t offset, int whence)
{
    return fseek(((stdio_stream_t *)self)->fd, offset, whence);
}

static int stdio_close(stream_t *self)
{
    int ret = 0;

    if (!self->closed_p) {
        stdio_stream_t *s = (stdio_stream_t *)self;

        self->closed_p = 1;

        ret = fclose(s->fd);

        s->fd = 0;

        if (s->original_path) {
            free(s->original_path);
            s->original_path = NULL;
        }

        if (s->original_mode) {
            free(s->original_mode);
            s->original_mode = NULL;
        }
    }

    return ret;
}

static long stdio_tell(stream_t *self)
{
    return ftell(((stdio_stream_t *)self)->fd);
}

static int stdio_error_p(stream_t *self)
{
    return ferror(((stdio_stream_t *)self)->fd);
}

static void stdio_clear_error(stream_t *self)
{
    clearerr(((stdio_stream_t *)self)->fd);
}

static int stdio_get_length(stream_t *self)
{
    stream_callbacks_t *cb = stream_get_callbacks(self);
    int origin = cb->tell(self);
    int total_bytes = 0;

    cb->seek(self, 0, SEEK_END);
    total_bytes = cb->tell(self);
    cb->seek(self, origin, SEEK_SET);

    return total_bytes;
}

static stream_t *stdio_stream_fork(stream_t *zelf)
{
    stdio_stream_t *self = (stdio_stream_t *)zelf;

    return &stdio_open(NULL, self->original_path, self->original_mode)->super;
}

stdio_stream_t *stdio_stream_instantiate(stdio_stream_t *x)
{
    stdio_stream_t *s = stdio_stream_instantiate_super(x);

    s->super.get_as_memory_area = stdio_get_as_memory_area;
    s->super.free_memory_area = stdio_free_memory_area;
    s->super.fork = stdio_stream_fork;

    s->super.callbacks.read = stdio_read;
    s->super.callbacks.write = stdio_write;
    s->super.callbacks.seek = stdio_seek;
    s->super.callbacks.close = stdio_close;
    s->super.callbacks.tell = stdio_tell;
    s->super.callbacks.error_p = stdio_error_p;
    s->super.callbacks.clear_error = stdio_clear_error;
    s->super.callbacks.get_length = stdio_get_length;

    return s;
}

stdio_stream_t *stdio_open(stdio_stream_t *x, const char *path,
                           const char *mode)
{
    stdio_stream_t *s = stdio_stream_instantiate(x);

    s->fd = fopen(path, mode);
    if (!s->fd) {
        free(s);
        WARNING2("file '%s' not found", path);
        return NULL;
    } else {
        s->original_path = strdup(path);
        s->original_mode = strdup(mode);
        return s;
    }
}

static stdio_stream_t *stdio_stream_from_fd(stdio_stream_t *x, FILE *fd)
{
    stdio_stream_t *stream = stdio_stream_instantiate(x);

    stream->fd = fd;

    stream->original_path = strdup("-");

    return stream;
}

static stdio_stream_t *stdout_stream = 0;
static stdio_stream_t *stdin_stream = 0;
static stdio_stream_t *stderr_stream = 0;

stdio_stream_t *stdio_stream_get_stdout_instance()
{
    if (!stdout_stream) {
        stdout_stream = stdio_stream_from_fd(NULL, stdout);

        stdout_stream->original_mode = strdup("wb");
    }

    return stdout_stream;
}

stdio_stream_t *stdio_stream_get_stdin_instance()
{
    if (!stdin_stream) {
        stdin_stream = stdio_stream_from_fd(NULL, stdin);

        stdin_stream->original_mode = strdup("wb");
    }

    return stdin_stream;
}

stdio_stream_t *stdio_stream_get_stderr_instance()
{
    if (!stderr_stream) {
        stderr_stream = stdio_stream_from_fd(NULL, stderr);

        stderr_stream->original_mode = strdup("rb");
    }

    return stderr_stream;
}
