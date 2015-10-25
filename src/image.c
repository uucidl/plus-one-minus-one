/* a10 87
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/image.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 87 */

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/stdint.h>
#include <libc/string.h>

#include <lib/pixel.h>

#include "image.h"

image_t *image_instantiate(image_t *x) { return image_instantiate_super(x); }

int image_new(image_t *self, unsigned int width, unsigned int height,
              unsigned int pitch)
{
    self->width = width;
    self->height = height;
    self->pitch = pitch;

    self->pixels = a_calloc(pitch * sizeof(int32_t), height);

    return 1;
}

int image_destroy(image_t *self)
{
    self->width = self->height = self->pitch = 0;
    if (self->pixels) {
        a_free(self->pixels);
        self->pixels = NULL;
    }

    return 1;
}

#undef CLIP_TEST

void image_blit(image_t *self, int x, int y, uint32_t *pixels, int width,
                int height, int pitch)
{
    /* start in image */
    uint32_t *src;
    /* start in fb */
    uint32_t *dst;
    int h; /* number of scanlines to blit */
    int w; /* number of pixels in scanline to blit */

    src = (uint32_t *)self->pixels;
    dst = pixels;
    w = self->width;
    h = self->height;

    if (x <= 0) {
        src -= x;
        w += x;
    } else {
        dst += x;
        width -= x;
    }

    if (w > width)
        w = width;

    if (y <= 0) {
        src -= y * self->pitch;
        h += y;
    } else {
        dst += y * pitch;
        height -= y;
    }

    if (h > height)
        h = height;

    if (src == dst) {
        // easy.
    } else if (w == self->pitch && w == pitch) {
        memcpy(dst, src, h * w * 4);
    } else if (w > 0) {
        const int wwww = w * 4;
        for (; h > 0; h--) {
            memcpy(dst, src, wwww);
            dst += pitch;
            src += self->pitch;
        }
    }
}

/* linear interpolation using the alpha values */
void image_blit_blend(image_t *self, int x, int y, uint32_t *pixels, int width,
                      int height, int pitch)
{
    /* start in image */
    uint32_t *src;
    /* start in fb */
    uint32_t *dst;
    int h; /* number of scanlines to blit */
    int w; /* number of pixels in scanline to blit */

    src = (uint32_t *)self->pixels;
    dst = pixels;
    w = self->width;
    h = self->height;

    if (x <= 0) {
        src -= x;
        w += x;
    } else {
        dst += x;
        width -= x;
    }

    if (w > width)
        w = width;

    if (y <= 0) {
        src -= y * self->pitch;
        h += y;
    } else {
        dst += y * pitch;
        height -= y;
    }

    if (h > height)
        h = height;

    if (w > 0) {
        for (; h > 0; h--) {
            int i;
            for (i = 0; i < w; i++) {
                int alpha = 256 - (src[i] >> 24);
                dst[i] = pixel_lerp(alpha, src[i], dst[i]);
            }
            dst += pitch;
            src += self->pitch;
        }
    }
}

/*
  works only with zu, zv in the ]0;65536] range
*/

void image_blit_scale(image_t *self, int x, int y, float zu, float zv,
                      uint32_t *pixels, int width, int height, int pitch)
{
    /* start in image */
    uint32_t *src;
    /* start in fb */
    uint32_t *dst;
    int h;               /* number of scanlines to blit */
    int w;               /* number of pixels in scanline to blit */
    int du = 65536 / zu; /* 16.16 fixed point */
    int dv = 65536 / zv; /* 16.16 fixed point */

    src = (uint32_t *)self->pixels;
    dst = pixels;
    w = self->width * zu;
    h = self->height * zv;

    if (x <= 0) {
        src -= (x * du) >> 16;
        w += x;
    } else {
        dst += x;
        width -= x;
    }

    if (w > 0 && w > width)
        w = width;

    if (y <= 0) {
        src -= ((y * dv) >> 16) * self->pitch;
        h += y;
    } else {
        dst += y * pitch;
        height -= y;
    }

    if (h > 0 && h > height)
        h = height;

    if (w > 0) {
        unsigned int u, v; /* 16.16 fixed point */
        unsigned int pv = 1;
        for (v = 0; h > 0; h--, v += dv) {
            int i;
            if (pv != (v & 0xffff0000)) {
                uint32_t *im = src + (v >> 16) * self->pitch;
                for (u = 0, i = 0; i < w; i++, u += du) {
                    dst[i] = im[u >> 16];
                }
            } else {
                memcpy(dst, dst - pitch, w * 4);
            }
            dst += pitch;
            pv = v & 0xffff0000;
        }
    }
}

void image_blit_blend_scale(image_t *self, int x, int y, float zu, float zv,
                            uint32_t *pixels, int width, int height, int pitch)
{
    /* start in image */
    uint32_t *src;
    /* start in fb */
    uint32_t *dst;
    int h;               /* number of scanlines to blit */
    int w;               /* number of pixels in scanline to blit */
    int du = 65536 / zu; /* 16.16 fixed point */
    int dv = 65536 / zv; /* 16.16 fixed point */

    src = (uint32_t *)self->pixels;
    dst = pixels;
    w = self->width * zu;
    h = self->height * zv;

    if (x <= 0) {
        src -= (x * du) >> 16;
        w += x;
    } else {
        dst += x;
        width -= x;
    }

    if (w > 0 && w > width)
        w = width;

    if (y <= 0) {
        src -= ((y * dv) >> 16) * self->pitch;
        h += y;
    } else {
        dst += y * pitch;
        height -= y;
    }

    if (h > 0 && h > height)
        h = height;

    if (w > 0) {
        unsigned int u, v; /* 16.16 fixed point */
        for (v = 0; h > 0; h--, v += dv) {
            int i;
            uint32_t *im = src + (v >> 16) * self->pitch;
            for (u = 0, i = 0; i < w; i++, u += du) {
                int x = u >> 16;
                int alpha = 256 - (im[x] >> 24);
                dst[i] = pixel_lerp(alpha, im[x], dst[i]);
            }
            dst += pitch;
        }
    }
}

/* render the quad
   (x1, y1) . (x4, y4) .
   (x2, y2) . (x3, y3) .
   from image to a quad of width 'qwidth' and height 'qheight' on 'pixels'
*/

void image_render_quad(image_t *self, float x1, float y1, float x2, float y2,
                       float x3, float y3, float x4, float y4, uint32_t *pixels,
                       int width, int height, int pitch, int qwidth,
                       int qheight)
{
    float dxdown_left = (x2 - x1) / qheight;
    float dydown_left = (y2 - y1) / qheight;
    float dxdown_right = (x3 - x4) / qheight;
    float dydown_right = (y3 - y4) / qheight;
    uint32_t *scanline = pixels;

    /* segment for scanline */
    float xa = x1;
    float ya = y1;
    float xb = x4;
    float yb = y4;
    {
        int j;
        for (j = 0; j < qheight; j++) {
            /* deltas for scanline */
            float dxleft_up = (xb - xa) / qwidth;
            float dyleft_up = (yb - ya) / qheight;
            /* current point */
            float x = xa;
            float y = ya;
            int i;
            for (i = 0; i < qwidth; i++) {
#ifdef CLIP_TEST
                if (x > 0 && x < self->width && y > 0 && y < self->height) {
#endif
                    scanline[i] =
                        self->pixels[((int)y % self->height) * self->pitch +
                                     ((int)x % self->width)];
#ifdef CLIP_TEST
                }
#endif
                x += dxleft_up;
                y += dyleft_up;
            }

            xa += dxdown_left;
            ya += dydown_left;
            xb += dxdown_right;
            yb += dydown_right;
            scanline += pitch;
        }
    }
}

#undef CLIP_TEST
