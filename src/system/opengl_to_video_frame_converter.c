/* a10 429
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file
 *('src/system/opengl_to_video_frame_converter.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 429 */

#include "opengl_to_video_frame_converter.h"

#include <system/video_effect.h> // for frame definition
#include <third-party/opengl/gl.h>
#include <libc/endian.h>

static void opengl_to_video_frame_converter_set_frame_signature(
    frame_converter_t *zelf, frame_converter_frame_signature_t *signature)
{
    opengl_to_video_frame_converter_t *self =
        (opengl_to_video_frame_converter_t *)zelf;
    frame_converter_frame_signature_t *sig =
        (frame_converter_frame_signature_t *)signature;
    argb32_video_frame_signature_t *output_sig =
        (argb32_video_frame_signature_t *)sig->output_signature;
    self->width = output_sig->width;
    self->height = output_sig->height;
    self->pitch = output_sig->pitch;
}

void opengl_to_video_frame_converter_computes(frame_converter_t *zelf,
                                              void *content, double ms)
{
    opengl_to_video_frame_converter_t *self =
        (opengl_to_video_frame_converter_t *)zelf;
    frame_converter_frame_t *frame = (frame_converter_frame_t *)content;

    uint32_t *pixels = (uint32_t *)frame->output_frame;
    unsigned int j;

    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_INT_8_8_8_8;

#if (defined(PIXEL_RGBA8888) && BYTE_ORDER == LITTLE_ENDIAN) ||                \
    (defined(PIXEL_BGRA8888) && BYTE_ORDER == BIG_ENDIAN)
    type = GL_UNSIGNED_INT_8_8_8_8_REV;
#endif

    /*
    format = GL_BGRA;
    type = GL_UNSIGNED_BYTE;
    */

    if (self->pitch == self->width) {
        glReadPixels(0, 0, self->width, self->height, format, type, pixels);
        // image is reversed?
    } else {
        for (j = 0; j < self->height; j++) {
            glReadPixels(0, j, self->width, 1, format, type,
                         pixels + j * self->pitch);
        }
    }
}

opengl_to_video_frame_converter_t *opengl_to_video_frame_converter_instantiate(
    opengl_to_video_frame_converter_t *x)
{
    opengl_to_video_frame_converter_t *fc =
        opengl_to_video_frame_converter_instantiate_super(x);

    fc->super.set_frame_signature =
        opengl_to_video_frame_converter_set_frame_signature;
    fc->super.computes = opengl_to_video_frame_converter_computes;

    return fc;
}
