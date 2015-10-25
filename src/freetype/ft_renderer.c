/* a10 518
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/freetype/ft_renderer.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 518 */

#include "ft_renderer.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_FREETYPE_FT_RENDERER);

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H

//#include <freetype/internal/ftobjs.h>

#include <library/stream.h>
#include <libc/stdlib.h>
#include <libc/math.h>
#include <lib/url_open.h>

static FT_Library library = NULL;
static int init_p = 0;

#define FT_CLIP

static int ft_new(ft_renderer_t *self)
{
    self->font_height = 12;

    return 1;
}

static int ft_destroy(ft_renderer_t *self)
{
    while (self->faces_next_index--) {
        FT_Done_Face(self->faces[self->faces_next_index]);
    }
    self->current_font_id = 0;

    return 1;
}

static int ft_renderer_init()
{
    int error;
    int ret = 1;

    error = FT_Init_FreeType(&library);
    if (error) {
        ERROR1("couldn't initialize the freetype library.");
        ret = 0;
    }

    return ret;
}

static unsigned long io_read(FT_Stream ft_stream, unsigned long offset,
                             unsigned char *buffer, unsigned long count)
{
    stream_t *stream = ft_stream->descriptor.pointer;

    stream_get_callbacks(stream)->seek(stream, offset, SEEK_SET);
    if (count > 0) {
        return stream_get_callbacks(stream)
            ->read(buffer, sizeof(char), count, stream);
    } else
        return 0;
}

static void io_close(FT_Stream ft_stream)
{
    stream_t *stream = ft_stream->descriptor.pointer;

    stream_get_callbacks(stream)->close(stream);
}

static FT_Stream create_ft_stream(stream_t *stream)
{
    struct FT_StreamRec_ *rec = calloc(sizeof(struct FT_StreamRec_), 1);

    {
        /* find out size of stream */
        stream_get_callbacks(stream)->seek(stream, 0, SEEK_END);
        rec->size = stream_get_callbacks(stream)->tell(stream);
        stream_get_callbacks(stream)->seek(stream, 0, SEEK_SET);
    }
    rec->descriptor.pointer = stream;
    rec->pathname.pointer = "bah";
    rec->read = io_read;
    rec->close = io_close;
    rec->memory = NULL;

    return rec;
}

/*
  unused, so hidden behind an #ifdef
*/

#ifdef FT_LOAD_FONT
static atom_t ft_load_font(ft_renderer_t *self, const char *filename)
{
    int ret = 0;
    int error;

    if (self->faces_next_index < MAX_FACES) {
        error = FT_New_Face(library, filename, 0,
                            &self->faces[self->faces_next_index]);
        if (error) {
            ERROR1("couldn't open face.");
            ret = 0;
        } else {
            ret = self->faces_next_index + 1;
            self->faces_next_index++;
        }
    }

    return ret;
}
#endif

static atom_t ft_load_font_from_stream(ft_renderer_t *self, stream_t *stream)
{
    atom_t ret = 0;
    int error;

    if (!stream) {
        WARNING1("stream was null.");
        return 0;
    } else if (self->faces_next_index < MAX_FACES) {
        FT_Open_Args args = {FT_OPEN_STREAM, NULL, 0, NULL,
                             create_ft_stream(stream), 0, 0, NULL};

        error = FT_Open_Face(library, &args, 0,
                             &self->faces[self->faces_next_index]);
        if (error) {
            ERROR1("couldn't open face.");
            ret = 0;
        } else {
            error = FT_Select_Charmap(self->faces[self->faces_next_index],
                                      ft_encoding_unicode);
            if (error)
                WARNING1("couldn't set charmap to unicode.");

            ret = self->faces_next_index + 1;
            self->faces_next_index++;
        }
    }

    return ret;
}

static void ft_use_font(ft_renderer_t *self, atom_t id)
{
    if (id) {
        while (id > 0 && id - 1 >= self->faces_next_index) {
            id--;
        }

        self->current_font_id = id;
    }
}

#define COLOR(a)                                                               \
    (((a << 15) & 0xff0000) | ((a << 8) & 0x00ff00) | ((a >> 2) & 0xff))

#include <lib/pixel.h>

/* version using their aa outline renderer directly */

static void ft_draw_span(int y, int count, FT_Span const *spans, void *user)
{
    void **data;
    ft_renderer_t *self;
    image_t *image;
    uint32_t *scanline;
    uint32_t *start;

    data = user;
    self = (ft_renderer_t *)data[0];
    image = (image_t *)data[1];

    /* determine scanline */
    {
        int i_y = -y + self->pen_y;
#ifndef FT_CLIP
        if (i_y < 0 || i_y >= image->height) {
            return;
        }
#endif
        scanline = image->pixels + image->pitch * i_y;
    }

    /* draw spans */
    {
        int i;
        for (i = 0; i < count; i++) {
            const FT_Span *span = spans + i;
            int len = span->len;
            int i_x = span->x + self->pen_x;
            int j;

#ifndef FT_CLIP
            if (i_x >= image->width)
                continue;
            else if (i_x < 0) {
                len += i_x;
                i_x = 0;
            }

            if (len <= 0)
                continue;

            if (i_x + len >= image->width) {
                len = image->width - i_x - 1;
            }
#endif

            start = scanline + i_x;

            if (self->must_overwrite_p) {
                const unsigned int c =
                    (self->color & 0xffffff) | (span->coverage << 24);
                for (j = 0; j < len; j++) {
                    start[j] = c;
                }
            } else {
                const unsigned int alpha =
                    (((self->color & 0xff000000) >> 16) * span->coverage) >> 16;

                for (j = 0; j < len; j++) {
                    start[j] = pixel_lerp(alpha, start[j], self->color);
                }
            }
        }
    }
}

static void ft_compute_string_extent(ft_renderer_t *self, image_t *destination,
                                     string_t *string)
{
    FT_GlyphSlot slot;
    FT_Face face;

    destination->width = 0;
    destination->height = 0;
    destination->pitch = 0;

    if (!self->current_font_id)
        return;

    face = self->faces[self->current_font_id - 1];
    slot = face->glyph;

    int error;

    error = FT_Set_Pixel_Sizes(face,               /* handle to face object */
                               0,                  /* pixel_width */
                               self->font_height); /* pixel_height */
    if (error)
        ERROR1("couldn't set pixel size");

    string_iterator_t it;
    const char *cp;

    if (!string->get_iterator(string, &it))
        return;

    float x = 0.f;
    float y = self->font_height;

    while ((cp = it.next(&it))) {
        /* assume first unicode page */
        int glyph_index = FT_Get_Char_Index(face, 0x00ff & *cp);

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue; // ignore errors

        FT_Glyph glyph;

        error = FT_Get_Glyph(slot, &glyph);
        if (error)
            continue;

        FT_Done_Glyph(glyph);

        // increment pen position
        x += slot->advance.x >> 6;
        if (slot->advance.y > y) {
            y = slot->advance.y;
        }
    }

    destination->width = ceil(x);
    destination->height = ceil(y);
    destination->pitch = destination->width;
}

static void ft_draw_string_direct(ft_renderer_t *self, image_t *image,
                                  string_t *string)
{
    void *user_data[2];
    user_data[0] = self;
    user_data[1] = image;

    FT_GlyphSlot slot;
    FT_Face face;

    if (!self->current_font_id)
        return;

    face = self->faces[self->current_font_id - 1];
    slot = face->glyph;

    int error;

    error = FT_Set_Pixel_Sizes(face,               /* handle to face object */
                               0,                  /* pixel_width */
                               self->font_height); /* pixel_height */
    if (error)
        ERROR1("couldn't set pixel size");

    string_iterator_t it;
    const char *cp;

    if (!string->get_iterator(string, &it))
        return;

    while ((cp = it.next(&it))) {
        /* assume first unicode page */
        int glyph_index = FT_Get_Char_Index(face, 0x00ff & *cp);

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue; // ignore errors

        FT_Glyph glyph;

        error = FT_Get_Glyph(slot, &glyph);
        if (error)
            continue;

        if (glyph->format == ft_glyph_format_outline) {
            /* a vectorial font */
            FT_Outline outline = ((FT_OutlineGlyph)glyph)->outline;

            FT_Raster_Params params;
            params.target = NULL;
            params.flags = ft_raster_flag_aa | ft_raster_flag_direct;
            params.gray_spans = self->draw_span_callback;
            params.user = user_data;

#ifdef FT_CLIP
            {
                params.flags |= ft_raster_flag_clip;

                params.clip_box.xMin = (-self->pen_x);
                params.clip_box.xMax = (image->width - 1 - self->pen_x);
                params.clip_box.yMin = (-image->height + 1 + self->pen_y);
                params.clip_box.yMax = (self->pen_y);
            }
#endif

            error = FT_Outline_Render(library, &outline, &params);
            if (error)
                continue; // ignore errors
        } else {
            /* render bitmap normally */
            ERROR1("unsupported bitmap character.");
        }

        FT_Done_Glyph(glyph);

        // increment pen position
        self->pen_x += slot->advance.x >> 6;
        // self->pen_y += slot->advance.y >> 6;   // kinda useless for now..
    }
}

static void ft_draw_outline(ft_renderer_t *self, image_t *image,
                            outline_t *outline)
{
    FT_Raster_Params params;
    int error;
    void *user_data[2];

    user_data[0] = self;
    user_data[1] = image;

    params.target = NULL;
    params.source = &outline->ftoutline;
    params.flags = ft_raster_flag_aa | ft_raster_flag_direct;
    params.gray_spans = self->draw_span_callback;
    params.user = user_data;

#ifdef FT_CLIP
    {
        params.flags |= ft_raster_flag_clip;

        params.clip_box.xMin = (-self->pen_x);
        params.clip_box.xMax = (image->width - 1 - self->pen_x);
        params.clip_box.yMin = (-image->height + 1 + self->pen_y);
        params.clip_box.yMax = (self->pen_y);

        DEBUG5("clip_box: x: %d - %d, y: %d - %d\n", params.clip_box.xMin >> 6,
               params.clip_box.xMax >> 6, params.clip_box.yMin >> 6,
               params.clip_box.yMax >> 6);
    }
#endif

    error = FT_Outline_Render(library, &outline->ftoutline, &params);
}

struct oi_data_t {
    ft_renderer_t *self;
    image_t *destination;
    image_t *source;
    int x0;
    int y0;
};

static void oi_draw_span(int y, int count, FT_Span const *spans, void *user)
{
    struct oi_data_t *data;
    ft_renderer_t *self;
    image_t *image;
    uint32_t *scanline;
    uint32_t *start;
    image_t *src_image;
    uint32_t *src_scanline;
    uint32_t *src_start;

    data = user;
    self = (ft_renderer_t *)data->self;
    image = data->destination;
    src_image = data->source;

    /* determine destination scanline */
    {
        int i_y = -y + self->pen_y;
#ifndef FT_CLIP
        if (i_y < 0 || i_y >= image->height) {
            return;
        }
#endif
        scanline = image->pixels + image->pitch * i_y;
    }

    /* determine source scanline */
    {
        int i_y = -y + self->pen_y + data->y0;
        if (i_y < 0 || i_y >= src_image->height) {
            return;
        }
        src_scanline = src_image->pixels + src_image->pitch * i_y;
    }

    /* draw spans */
    {
        int i;
        for (i = 0; i < count; i++) {
            const FT_Span *span = spans + i;
            int len = span->len;

            /* destination start */
            {
                int i_x = span->x + self->pen_x;
#ifndef FT_CLIP
                if (i_x >= image->width)
                    continue;
                else if (i_x < 0) {
                    len += i_x;
                    i_x = 0;
                }

                if (len <= 0)
                    continue;

                if (i_x + len >= image->width) {
                    len = image->width - i_x - 1;
                }
#endif

                start = scanline + i_x;
            }

            /* source start */
            {
                int i_x = span->x + self->pen_x + data->x0;
                int i_len = len;
                if (i_x >= src_image->width)
                    continue;
                else if (i_x < 0) {
                    i_len += i_x;
                    /* correct starting position to account for clipping of the
                     * source image */
                    start -= i_x;
                    i_x = 0;
                }

                if (i_len <= 0)
                    continue;

                if (i_x + i_len >= src_image->width) {
                    i_len = src_image->width - i_x - 1;
                }

                if (i_len < len) {
                    len = i_len;
                }

                src_start = src_scanline + i_x;
            }

            {
                unsigned int alpha =
                    (((self->color & 0xff000000) >> 16) * span->coverage) >> 16;
                int j;

                for (j = 0; j < len; j++) {
                    start[j] = pixel_lerp(alpha, start[j], src_start[j]);
                }
            }
        }
    }
}

static void ft_draw_outlined_image(ft_renderer_t *self, image_t *image,
                                   outline_t *outline, image_t *source, int x0,
                                   int y0)
{
    FT_Raster_Params params;
    int error;
    struct oi_data_t user_data;
    ;

    user_data.self = self;
    user_data.destination = image;
    user_data.source = source;
    user_data.x0 = x0;
    user_data.y0 = y0;

    params.target = NULL;
    params.source = &outline->ftoutline;
    params.flags = ft_raster_flag_aa | ft_raster_flag_direct;
    params.gray_spans = oi_draw_span;
    params.user = &user_data;

#ifdef FT_CLIP
    {
        params.flags |= ft_raster_flag_clip;

        params.clip_box.xMin = (-self->pen_x);
        params.clip_box.xMax = (image->width - 1 - self->pen_x);
        params.clip_box.yMin = (-image->height + 1 + self->pen_y);
        params.clip_box.yMax = (self->pen_y);

        DEBUG5("clip_box: x: %d - %d, y: %d - %d\n", params.clip_box.xMin >> 6,
               params.clip_box.xMax >> 6, params.clip_box.yMin >> 6,
               params.clip_box.yMax >> 6);
    }
#endif

    error = FT_Outline_Render(library, &outline->ftoutline, &params);
}

static void ft_draw_cstring(ft_renderer_t *self, image_t *dest, const char *str)
{
    string_t string;

    string_instantiate_toplevel(&string);
    string.new(&string, str);

    self->draw_string(self, dest, &string);

    string.destroy(&string);
    string_retire(&string);
}

#ifdef FT_INDIRECT_DRAW

static void draw_blend_bitmap(image_t *dest, FT_Bitmap *src, int x, int y)
{
    int32_t *pdest = dest->pixels + x + y * dest->pitch;
    unsigned char *psrc = src->buffer;
    int h = src->rows;
    while (h--) {
        int w = src->width;
        int i;
        for (i = 0; i < w; i++) {
            pdest[i] = pixel_lerp(psrc[i], pdest[i], 0x80ff40);
        }
        psrc += src->pitch;
        pdest += dest->pitch;
    }
}

static void ft_draw_string(ft_renderer_t *self, image_t *image,
                           string_t *string)
{
    FT_GlyphSlot slot;
    int error;
    FT_Face face;
    string_iterator_t it;
    const char *cp;

    if (!self->current_font_id)
        return;

    face = self->faces[self->current_font_id - 1];
    slot = face->glyph;

    error = FT_Set_Pixel_Sizes(face, /* handle to face object            */
                               0,    /* pixel_width                      */
                               self->font_height); /* pixel_height */
    if (error)
        ERROR1("couldn't set pixel size");

    if (!string->get_iterator(string, &it))
        return;
    while ((cp = it.next(&it))) {
        /* assume first unicode page */
        error = FT_Load_Char(face, 0xff & *cp, FT_LOAD_RENDER);
        if (error)
            continue; // ignore errors

        draw_blend_bitmap(image, &slot->bitmap, self->pen_x + slot->bitmap_left,
                          self->pen_y - slot->bitmap_top);

        // increment pen position
        self->pen_x += slot->advance.x >> 6;
        self->pen_y += slot->advance.y >> 6; // kinda useless for now..
    }
}

#endif

ft_renderer_t *ft_renderer_instantiate(ft_renderer_t *x)
{
    ft_renderer_t *r = ft_renderer_instantiate_super(x);

    if (!init_p) {
        init_p = ft_renderer_init();
    }

    r->new = ft_new;
    r->destroy = ft_destroy;
    r->load_font = ft_load_font_from_stream;
    r->use_font = ft_use_font;
#ifdef FT_INDIRECT_DRAW
    r->draw_string = ft_draw_string;
#else
    r->draw_span_callback = ft_draw_span;
    r->draw_string = ft_draw_string_direct;
#endif
    r->compute_string_extent = ft_compute_string_extent;

    r->draw_cstring = ft_draw_cstring;
    r->draw_outline = ft_draw_outline;
    r->draw_outlined_image = ft_draw_outlined_image;

    /*
      load default system font
    */
    int font_id;
#ifdef MACOSX
    font_id = r->load_font(r, url_open("virtual://fonts/Arial", "rb"));
#else
    font_id = r->load_font(r, url_open("virtual://fonts/arial.ttf", "rb"));
#endif
    if (font_id == 0) {
        ERROR1("couldn't load default font.");
    } else {
        r->use_font(r, font_id);
    }

    if (!init_p)
        return NULL;
    else
        return r;
}

FT_Library ft_renderer_get_library() { return library; }
