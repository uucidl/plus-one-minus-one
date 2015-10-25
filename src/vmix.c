/* a10 72
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vmix.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 72 */

#include "vmix.h"
#include <libc/math.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <system/effects.h>
#include <system/main.h>

#include <lib/chance.h>
#include <lib/pixel.h>
#include <scripting/compile.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_VMIX);

static atom_t crossfade_atom = 0;
static atom_t mutilate_atom = 0;

static void vmix_set_frame_size(video_effect_t *v, int width, int height,
                                int pitch)
{
    vmix_t *vmix = (vmix_t *)v;

    if (vmix->super.width != width || vmix->super.width != height ||
        vmix->super.pitch != pitch) {
        vmix->super.width = width;
        vmix->super.height = height;
        vmix->super.pitch = pitch;

        if (vmix->left)
            free(vmix->left);
        vmix->left = calloc(width * height, sizeof(int32_t));
        if (vmix->left_effect)
            vmix->left_effect->set_frame_size(vmix->left_effect, width, height,
                                              width);

        if (vmix->right)
            free(vmix->right);
        vmix->right = calloc(width * height, sizeof(int32_t));
        if (vmix->right_effect)
            vmix->right_effect->set_frame_size(vmix->right_effect, width,
                                               height, width);
    }
}

static void vmix_set_mix_type_cb(vmix_t *self, atom_t type)
{
    enum vmix_type t;
    if (type == crossfade_atom)
        t = CROSSFADE;
    else if (type == mutilate_atom)
        t = MUTILATE;
    else {
        ERROR2("Unsupported mix type: '%s'", atom_get_cstring_value(type));
        return;
    }

    self->set_mix_type(self, t);
}

static void vmix_set_mix_type(vmix_t *self, enum vmix_type type)
{
    self->type = type;
}

static void vmix_set_mix(vmix_t *self, float factor)
{
    if (factor < 0.0f)
        factor = 0.0f;
    if (factor > 1.0f)
        factor = 1.0f;

    self->mix = factor;
}

static void vmix_set_left_effect(vmix_t *self, video_effect_t *e)
{
    self->left_effect = e;
    if (e)
        e->set_frame_size(e, self->super.width, self->super.height,
                          self->super.width);
}

static void vmix_set_right_effect(vmix_t *self, video_effect_t *e)
{
    self->right_effect = e;
    if (e)
        e->set_frame_size(e, self->super.width, self->super.height,
                          self->super.width);
}

static void vmix_set_left_offset_ms(vmix_t *self, double ms)
{
    self->left_ms = ms;
}

static void vmix_set_right_offset_ms(vmix_t *self, double ms)
{
    self->right_ms = ms;
}

#if defined(P1M1_UNUSED)
static inline void blit_amp_add(vmix_t *self, int32_t *__restrict__ pixelsd,
                                int32_t *__restrict__ pixelss, double amp)
{
    int amount = self->super.width * self->super.height;
    while (amount--) {
        double r = (*pixelss >> 16) & 0xff;
        double g = (*pixelss >> 8) & 0xff;
        double b = (*pixelss) & 0xff;
        *pixelsd +=
            ((int)(amp * r) << 16) | ((int)(amp * g) << 8) | (int)(amp * b);
        pixelss++;
        pixelsd++;
    }
}

static inline void blit_mix(vmix_t *self, int32_t *__restrict__ pixelsd,
                            int32_t *__restrict__ pixelsl,
                            int32_t *__restrict__ pixelsr, double amp)
{
    int amount = self->super.width * self->super.height;
    while (amount--) {
        float r = amp * ((*pixelsl >> 16) & 0xff);
        float g = amp * ((*pixelsl >> 8) & 0xff);
        float b = amp * ((*pixelsl) & 0xff);
        r += (1.0 - amp) * ((*pixelsr >> 16) & 0xff);
        g += (1.0 - amp) * ((*pixelsr >> 8) & 0xff);
        b += (1.0 - amp) * ((*pixelsr) & 0xff);

        *pixelsd = ((int)(r) << 16) | ((int)(g) << 8) | (int)(b);
        pixelsl++;
        pixelsr++;
        pixelsd++;
    }
}
#endif

static inline void blit_mix(vmix_t *self, int32_t *__restrict__ pixelsd,
                            int32_t *__restrict__ pixelsl,
                            int32_t *__restrict__ pixelsr, float amp)
{
    int alpha = amp * 256;
    int h = self->super.height;

    while (h--) {
        int i;

        for (i = 0; i < self->super.width; i++) {
            pixelsd[i] = pixel_lerp(alpha, pixelsr[i], pixelsl[i]);
        }
        pixelsd += self->super.pitch;
        pixelsr += self->super.width;
        pixelsl += self->super.width;
    }
}

static void vmix_computes(effect_t *self, void *content, double ms)
{
    vmix_t *vmix = (vmix_t *)self;

    const int width = vmix->super.width;
    const int height = vmix->super.height;
    const int pitch = vmix->super.pitch;
    int32_t *__restrict__ pixelsd = content;
    int32_t *__restrict__ pixelsl = vmix->left;
    int32_t *__restrict__ pixelsr = vmix->right;

    if (!pixelsl || !pixelsr)
        return;
    if (vmix->mix >= 1.0f) {
        if (vmix->left_effect)
            ((effect_t *)vmix->left_effect)
                ->computes((effect_t *)vmix->left_effect, pixelsl,
                           ms - vmix->left_ms);
        {
            int h = height;
            for (h = 0; h < height; h++)
                memcpy(pixelsd + h * pitch, pixelsl + h * width,
                       width * sizeof(int32_t));
        }
    } else if (vmix->mix <= 0.0f) {
        if (vmix->right_effect)
            ((effect_t *)vmix->right_effect)
                ->computes((effect_t *)vmix->right_effect, pixelsr,
                           ms - vmix->right_ms);
        {
            int h = height;
            for (h = 0; h < height; h++)
                memcpy(pixelsd + h * pitch, pixelsr + h * width,
                       width * sizeof(int32_t));
        }
    } else {
        /* then do it */
        if (vmix->left_effect)
            ((effect_t *)vmix->left_effect)
                ->computes((effect_t *)vmix->left_effect, pixelsl,
                           ms - vmix->left_ms);
        if (vmix->right_effect)
            ((effect_t *)vmix->right_effect)
                ->computes((effect_t *)vmix->right_effect, pixelsr,
                           ms - vmix->right_ms);

        if (vmix->type == CROSSFADE) {
            blit_mix(vmix, pixelsd, pixelsl, pixelsr, vmix->mix);
        } else if (vmix->type == MUTILATE) {
            int h = height;
            float hwater = vmix->mix;
            while (h--) {
                float l = unirandf();
                float hw; /* shaped high water */
                // float phase = 0;
                hw = hwater; /* uniform distribution */
                // hw = (0.5+0.5*cos(M_PI_2*h/height+M_PI_2*phase))*hwater; /*
                // sinus? */
                // hw = h*(hwater/height); /* linear */

                if (l <= hw)
                    memcpy(pixelsd, pixelsl, width * sizeof(int32_t));
                else
                    memcpy(pixelsd, pixelsr, width * sizeof(int32_t));
                pixelsd += pitch;
                pixelsl += width;
                pixelsr += width;
            }
        }
    }
}

static int vmix_new(effect_t *self)
{
    vmix_t *vmix = (vmix_t *)self;

    vmix->type = MUTILATE;
    vmix->mix = 1.0f; /* full left */

    vmix->left = NULL; /* will alloc in set_frame_size */
    vmix->left_effect = NULL;
    vmix->left_ms = 0.0;
    vmix->right = NULL; /* will alloc in set_frame_size */
    vmix->right_effect = NULL;
    vmix->left_ms = 0.0;

    return 1;
}

static int vmix_destroy(effect_t *self)
{
    vmix_t *vmix = (vmix_t *)self;
    if (vmix->left) {
        free(vmix->left);
        vmix->left = NULL;
    }

    if (vmix->right) {
        free(vmix->right);
        vmix->right = NULL;
    }

    return 1;
}

#include <scripting/dictionary.h>

vmix_t *vmix_instantiate(vmix_t *x)
{
    vmix_t *vm = vmix_instantiate_super(x);

    effect_register_instance("vmix", &vm->super.super);

    {
        dictionary_t *dict = dictionary_get_instance();
        crossfade_atom = dict->new_atom(dict, "crossfade");
        mutilate_atom = dict->new_atom(dict, "mutilate");
    }

    vm->super.super.new = vmix_new;
    vm->super.super.destroy = vmix_destroy;
    vm->super.super.computes = vmix_computes;
    vm->super.set_frame_size = vmix_set_frame_size;
    vm->set_mix = vmix_set_mix;
    vm->set_mix_type = vmix_set_mix_type;

    {
        receiver_t *r =
            router_get_receiver(effect_get_router(&vm->super.super));
        dictionary_t *d = dictionary_get_instance();

        r->set_definition(r, d->new_atom(d, "set-mix"),
                          compile_cstring("integer", NULL),
                          (recp_f)vm->set_mix);
        r->set_definition(r, d->new_atom(d, "set-mix-type"),
                          compile_cstring("atom", NULL),
                          (recp_f)vmix_set_mix_type_cb);
    }

    vm->set_left_effect = vmix_set_left_effect;
    vm->set_right_effect = vmix_set_right_effect;
    vm->set_left_offset_ms = vmix_set_left_offset_ms;
    vm->set_right_offset_ms = vmix_set_right_offset_ms;

    return vm;
}
