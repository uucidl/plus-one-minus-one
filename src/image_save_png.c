/* a10 920
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_save_png.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 920 */

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_IMAGE_SAVE_PNG);

#include "image_save_png.h"
#include <png.h>

void image_save_png(image_t *image, FILE *fd)
{
    png_structp png_write_ptr =
        png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr;

    if (!png_write_ptr) {
        ERROR1("png_write_ptr was null");
        return;
    }

    info_ptr = png_create_info_struct(png_write_ptr);
    if (!info_ptr) {
        ERROR1("info_ptr was null");
        png_destroy_write_struct(&png_write_ptr, (png_infopp)NULL);
        return;
    }

    if (setjmp(png_jmpbuf(png_write_ptr))) {
        ERROR1("encoding error.");
        png_destroy_write_struct(&png_write_ptr, &info_ptr);
        return;
    }

    png_init_io(png_write_ptr, fd);

    png_set_filter(png_write_ptr, 0, PNG_FILTER_NONE);

    png_set_IHDR(png_write_ptr, info_ptr, image->width, image->height, 8,
                 PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    // png_set_PLTE(png_write_ptr, info_ptr, NULL, 0);
    // png_set_gAMA(png_write_ptr, info_ptr, 1.2);
    png_set_sRGB(png_write_ptr, info_ptr, PNG_sRGB_INTENT_PERCEPTUAL);

    png_write_info(png_write_ptr, info_ptr);

    png_set_filler(png_write_ptr, 0, PNG_FILLER_AFTER);

    png_set_bgr(png_write_ptr);

    // png_set_compression_level(png_write_ptr, Z_BEST_SPEED);

    {
        int i;
        for (i = 0; i < image->height; i++) {
            png_bytep row_pointer =
                (png_bytep)(image->pixels + i * image->pitch);

            png_write_row(png_write_ptr, row_pointer);
        }
    }

    png_write_end(png_write_ptr, info_ptr);

    png_destroy_write_struct(&png_write_ptr, &info_ptr);
}
