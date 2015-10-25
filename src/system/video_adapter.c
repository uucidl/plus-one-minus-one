/* a10 483
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/video_adapter.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 483 */

#include "video_adapter.h"

#include <system/opengl_effect.h>
#include <system/opengl_to_video_frame_converter.h>
#include <system/demo.h>

#include <string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_VIDEO_ADAPTER);

static int video_adapter_destroy(effect_t *zelf)
{
    video_adapter_t *self = (video_adapter_t *)zelf;
    if (self->fc) {
        self->fc->destroy(self->fc);
        object_retire(frame_converter_to_object(self->fc));
        self->fc = NULL;
    }

    if (self->frame) {
        a_free(self->frame);
        self->frame = NULL;
    }

    if (self->own_p && self->effect) {
        self->effect->destroy(self->effect);
        object_retire(effect_to_object(self->effect));
    }

    return 1;
}

static int video_adapter_plug_effect(video_adapter_t *self, effect_t *e,
                                     int own_p)
{
    int status;
    atom_t video_frame_atom = dictionary_get_instance()->get_atom(
        dictionary_get_instance(), OPENGL_FRAME_TYPE_NAME);
    atom_t frame_type = e->get_frame_type(e);

    if (video_frame_atom == frame_type) {
        INFO1("all is fine and dandy.");
        self->fc =
            &opengl_to_video_frame_converter_instantiate_toplevel(NULL)->super;
        self->fc->new (self->fc);

        /*
          we are doing it like this. yes. taking the width and height from
          demo_get_instance.
        */
        {
            frame_converter_frame_signature_t signature;
            argb32_video_frame_signature_t video_signature;

            frame_converter_frame_signature_instantiate_toplevel(&signature);
            argb32_video_frame_signature_instantiate_toplevel(&video_signature);

            video_signature.width = demo_get_instance()->video_width;
            video_signature.height = demo_get_instance()->video_height;
            video_signature.pitch = video_signature.width;

            /*
              allocate a frame
            */
            self->nbytes = sizeof(uint32_t) * video_signature.pitch *
                           video_signature.height;
            self->frame = a_malloc(self->nbytes);

            signature.input_signature = 0;
            signature.output_signature = &video_signature.super;
            self->fc->set_frame_signature(self->fc, &signature);

            e->set_frame_signature(e, &video_signature.super);

            argb32_video_frame_signature_retire(&video_signature);
            frame_converter_frame_signature_retire(&signature);
        }
        self->effect = e;
        self->own_p = !!own_p;

        status = 1;
    } else {
        ERROR2("Unknown frame type: '%s'\n",
               atom_get_cstring_value(frame_type));
        status = 0;
    }

    return status;
}

static void video_adapter_computes(effect_t *zelf, void *content, double ms)
{
    video_adapter_t *self = (video_adapter_t *)zelf;
    frame_converter_frame_t converter_frame;

    memset(self->frame, 0x0, self->nbytes);
    self->effect->computes(self->effect, self->frame, ms);

    converter_frame.input_frame = self->frame;
    converter_frame.output_frame = content;

    self->fc->computes(self->fc, &converter_frame, ms);
}

video_adapter_t *video_adapter_instantiate(video_adapter_t *x)
{
    video_adapter_t *oa = video_adapter_instantiate_super(x);

    oa->plug_effect = video_adapter_plug_effect;
    oa->super.super.computes = video_adapter_computes;
    oa->super.super.destroy = video_adapter_destroy;

    return oa;
}
