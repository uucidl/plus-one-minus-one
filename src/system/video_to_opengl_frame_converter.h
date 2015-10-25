/* a10 829
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file
 *('src/system/video_to_opengl_frame_converter.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 829 */

#ifndef KNOS_DEMOS_SYSTEM_VIDEO_TO_OPENGL_FRAME_CONVERTER_H
#define KNOS_DEMOS_SYSTEM_VIDEO_TO_OPENGL_FRAME_CONVERTER_H

#include <system/frame_converter.h>

#include <third-party/opengl/gl.h>

typedef enum { BILINEAR_FILTER, POINT_FILTER } pixel_filter_type_t;

typedef struct video_to_opengl_frame_converter_t {
    frame_converter_t super;

    void (*activate_filter)(struct video_to_opengl_frame_converter_t *self,
                            pixel_filter_type_t filter);

    int width;
    int height;
    int pitch;

    int activate_bilinear_filter_p;

    uint32_t *framebuffer; /* framebuffer texture */
    uint32_t *buffer;      /* temporary buffer */
    GLuint texture_id;

    int tex_width;
    int tex_height;
    float tex_w; /* floating point coordinate */
    float tex_h; /* idem */
} video_to_opengl_frame_converter_t;

CLASS_INHERIT(video_to_opengl_frame_converter, frame_converter);

#endif
