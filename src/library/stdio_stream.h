/* a10 407
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/stdio_stream.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 407 */

#ifndef KNOS_LIBRARY_STDIO_STREAM_H
#define KNOS_LIBRARY_STDIO_STREAM_H

#include <library/stream.h>

typedef struct stdio_stream_t {
    stream_t super;
    FILE *fd;
    char *original_path;
    char *original_mode;
} stdio_stream_t;

CLASS_INHERIT(stdio_stream, stream)

stdio_stream_t *stdio_stream_instantiate(stdio_stream_t *x);
stdio_stream_t *stdio_open(stdio_stream_t *x, const char *path,
                           const char *mode);

stdio_stream_t *stdio_stream_get_stdout_instance();
stdio_stream_t *stdio_stream_get_stdin_instance();
stdio_stream_t *stdio_stream_get_stderr_instance();

#endif
