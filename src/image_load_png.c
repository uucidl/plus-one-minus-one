/* a10 296
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image_load_png.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 296 */

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_IMAGE_LOAD_PNG);

#include "image_load_png.h"
#include <png.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/endian.h>

static void stream_read_data(png_structp png_ptr, png_bytep data,
                             png_uint_32 length)
{
    stream_t *stream = png_get_io_ptr(png_ptr);

    if (stream_get_callbacks(stream)->read(data, length, 1, stream) < 1) {
        png_error(png_ptr, "couldn't read further");
    }
}

image_t *image_load_png(image_t *x, stream_t *stream)
{
    const int header_n = 8;
    unsigned char header[8];
    int is_png = 0;

    image_t *im = image_instantiate_toplevel(x);

    if (stream_get_callbacks(stream)->read(header, 1, header_n, stream) <
        (unsigned int)header_n) {
        if (!x)
            free(im);
        return NULL;
    }

    is_png = png_check_sig(header, header_n);
    if (!is_png) {
        WARNING("not a valid png file.");
        if (!x)
            free(im);
        return NULL;
    }

    {
        png_structp png_ptr;
        png_infop info_ptr;
        png_infop end_info;

        png_ptr =
            png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
        if (!png_ptr) {
            WARNING("can't init libpng\n");
            return im;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            png_destroy_read_struct(&png_ptr, NULL, NULL);
            return im;
        }

        end_info = png_create_info_struct(png_ptr);
        if (!end_info) {
            png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
            return im;
        }

        png_set_read_fn(png_ptr, stream, (png_rw_ptr)stream_read_data);
        // png_init_io(png_ptr, fd);
        png_set_sig_bytes(png_ptr, header_n); // jump the header

        png_read_info(png_ptr, info_ptr);

        {
            char *color_type = NULL;

            switch (png_get_color_type(png_ptr, info_ptr)) {
            case PNG_COLOR_TYPE_GRAY:
                color_type = "GRAY";
                png_set_gray_to_rgb(png_ptr);
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                color_type = "GRAY_ALPHA";
                png_set_gray_to_rgb(png_ptr);
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                break;
            case PNG_COLOR_TYPE_PALETTE:
                color_type = "PALETTE";
                png_set_palette_to_rgb(png_ptr);
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                break;
            case PNG_COLOR_TYPE_RGB:
                color_type = "RGB";
                png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
                break;
            case PNG_COLOR_TYPE_RGB_ALPHA:
                color_type = "RGB_ALPHA";
#if BYTE_ORDER == BIG_ENDIAN
                png_set_swap_alpha(png_ptr);
#endif
                break;
            }
#if (BYTE_ORDER == LITTLE_ENDIAN && defined(PIXEL_RGBA8888)) ||                \
    (BYTE_ORDER == BIG_ENDIAN && defined(PIXEL_BGRA8888))
            png_set_bgr(png_ptr);
#endif
            png_read_update_info(png_ptr, info_ptr);
            {
                int w = png_get_image_width(png_ptr, info_ptr);
                image_new(im, w, png_get_image_height(png_ptr, info_ptr), w);
            }
            int channels = png_get_channels(png_ptr, info_ptr);

            TRACE("image width: %d, height: %d", im->width, im->height);
            TRACE("channels: %d", channels);
            TRACE("color type: %s", color_type);

            {
                int i;
                png_bytep *row_pointers =
                    malloc(sizeof(png_bytep) * im->height);

                for (i = 0; i < im->height; i++)
                    row_pointers[i] = calloc(im->width * sizeof(uint32_t),
                                             sizeof(unsigned char));
                png_read_image(png_ptr, row_pointers);
                /* now convert and put in image */
                png_read_end(png_ptr, end_info);

                {
                    uint32_t *scanline = im->pixels;
                    for (i = 0; i < im->height; i++) {
                        uint32_t *row = (uint32_t *)row_pointers[i];
                        unsigned int n = im->width;
                        memcpy(scanline, row, sizeof(uint32_t) * n);
                        scanline += im->pitch;
                    }
                }

                for (i = 0; i < im->height; i++)
                    free(row_pointers[i]);
                free(row_pointers);
            }
        }
    }

    return im;
}
