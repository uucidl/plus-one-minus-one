/* a10 716
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/freetype/ft_renderer.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 716 */

#ifndef FT_RENDERER_H
#define FT_RENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <image.h>
#include <library/stream.h>
#include <library/strings.h>
#include <scripting/atom.h>

#include "outline.h"

#define MAX_FACES 16

typedef struct ft_renderer_t {
    object_t super;

    int (*new)(struct ft_renderer_t *self);
    int (*destroy)(struct ft_renderer_t *self);

    /*
       loads the font pointed by filename,
       returns 0 if failed, or id of loaded font
    */
    atom_t (*load_font)(struct ft_renderer_t *self, stream_t *stream);

    /* make one font active. if not loaded, tries to use another */
    void (*use_font)(struct ft_renderer_t *self, atom_t id);

    /* draw a string to the passed image */
    void (*draw_string)(struct ft_renderer_t *self, image_t *image,
                        string_t *string);

    /* compute the necessary size for displaying this string, and return it
     * inside the image_t* structure */
    void (*compute_string_extent)(struct ft_renderer_t *self,
                                  image_t *destination, string_t *string);

    void (*draw_cstring)(struct ft_renderer_t *self, image_t *image,
                         const char *string);

    void (*draw_outline)(struct ft_renderer_t *self, image_t *image,
                         outline_t *outline);
    /*
      draw an outline, taking pixels from a source image, using (x0, y0)
      as its origin.
    */
    void (*draw_outlined_image)(struct ft_renderer_t *self,
                                image_t *destination, outline_t *outline,
                                image_t *source, int x0, int y0);

    /* you must set the following parameters before drawing */
    int pen_x;
    int pen_y;
    uint32_t color;

    /* otherwise, interpolate linearly over the destination bitmap */
    int must_overwrite_p;

    FT_Face faces[MAX_FACES];
    int faces_next_index;
    /* starts at 1. 0 means no font selected */
    unsigned int current_font_id;
    unsigned int font_height;

    /*
      a callback responsible for drawing spans emanating from freetype's
      anti-aliased rasterizer.

      Parameters are:
        y: the scanline's y-coordinate.
        count: the number of spans to draw on this scanline.
        spans: a table of `count' spans to draw on the scanline.
        user: self!
    */
    FT_SpanFunc draw_span_callback;

} ft_renderer_t;

CLASS_INHERIT(ft_renderer, object)

FT_Library ft_renderer_get_library();

#endif
