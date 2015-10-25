/* a10 397
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_load_png.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 397 */

#ifndef KNOS_DEMOS_1_1_IMAGE_LOAD_PNG_H
#define KNOS_DEMOS_1_1_IMAGE_LOAD_PNG_H

#include <image.h>
#include <library/stream.h>

image_t *image_load_png(image_t *x, stream_t *fd);

#endif
