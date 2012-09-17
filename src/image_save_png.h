/* a10 575
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_save_png.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 575 */



#ifndef KNOS_DEMOS_1_1_IMAGE_SAVE_PNG_H
#define KNOS_DEMOS_1_1_IMAGE_SAVE_PNG_H

#include "image.h"
#include <libc/stdio.h>

void image_save_png(image_t* x, FILE* fd);

#endif
