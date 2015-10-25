/* a10 399
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_effect.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 399 */

#include "video_effect.h"

#include <libc/stdlib.h>
#include <libc/string.h>

#include <log4c.h>
// LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_VIDEO_EFFECT);

argb32_video_frame_signature_t *
argb32_video_frame_signature_instantiate(argb32_video_frame_signature_t *x)
{
    return argb32_video_frame_signature_instantiate_super(x);
}

static atom_t frame_type = 0;

static int blank_new(effect_t *self) { return 1; }

static int blank_destroy(effect_t *self) { return 1; }

static void blank_computes(effect_t *self, void *content, double ms)
{
    video_effect_t *x = (video_effect_t *)self;
    int h = x->height;
    while (h--) {
        memset(content + h * x->pitch * sizeof(int32_t), 0x00,
               x->width * sizeof(int32_t));
    }
}

static void set_frame_size_v(video_effect_t *self, int width, int height,
                             int pitch)
{
    self->width = width;
    self->height = height;
    self->pitch = pitch;
}

static atom_t get_frame_type_v(effect_t *self) { return frame_type; }

static void set_frame_signature_v(effect_t *self, object_t *signature)
{
    video_effect_t *ve = (video_effect_t *)self;
    argb32_video_frame_signature_t *sig =
        (argb32_video_frame_signature_t *)signature;

    ve->set_frame_size(ve, sig->width, sig->height, sig->pitch);
}

video_effect_t *video_effect_instantiate(video_effect_t *x)
{
    video_effect_t *e = video_effect_instantiate_super(x);

    if (!frame_type) {
        frame_type = dictionary_get_instance()->new_atom(
            dictionary_get_instance(), VIDEO_EFFECT_FRAME_TYPE_NAME);
    }

    e->super.new = blank_new;
    e->super.destroy = blank_destroy;
    e->super.computes = blank_computes;
    e->super.set_frame_signature = set_frame_signature_v;
    e->super.get_frame_type = get_frame_type_v;

    e->set_frame_size = set_frame_size_v;

    return (video_effect_t *)e;
}

static void video_blank_computes(effect_t *self, void *content, double ms)
{
    video_blank_t *vb = (video_blank_t *)self;
    blank_computes(self, content, ms);

    if (vb->next) {
        effect_t *e = (effect_t *)vb->next;
        e->computes(e, content, ms);
    }
}

static void video_blank_set_frame_size(video_effect_t *self, int width,
                                       int height, int pitch)
{
    video_blank_t *vb = (video_blank_t *)self;
    set_frame_size_v(self, width, height, pitch);

    if (vb->next) {
        vb->next->set_frame_size(vb->next, width, height, pitch);
    }
}

static void video_blank_set_next(video_blank_t *self, video_effect_t *next)
{
    self->next = next;
    if (next) {
        self->next->set_frame_size(self->next, self->super.width,
                                   self->super.height, self->super.pitch);
    }
}

video_blank_t *video_blank_instantiate(video_blank_t *x)
{
    video_blank_t *vb = video_blank_instantiate_super(x);

    vb->super.super.computes = video_blank_computes;
    vb->super.set_frame_size = video_blank_set_frame_size;
    vb->set_next_effect = video_blank_set_next;

    return vb;
}
