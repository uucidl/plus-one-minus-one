/* a10 901
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/icefx.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 901 */

#include "icefx.h"

#include <libc/stdlib.h>
#include <libc/math.h>

#include <lib/chance.h>
#include <lib/pixel.h>
#include <system/main.h>

static void choose_center(icefx_t *icefx)
{
    unirand2df(&icefx->center.x, &icefx->center.y);

    video_effect_t *ve = (video_effect_t *)&icefx->super;
    int width = ve->width;
    int height = ve->height;

    icefx->center.x *= 140.0f / 320.f * width;
    icefx->center.x += 160.0f / 320.f * width;

    icefx->center.y *= 100.0f / 240.f * height;
    icefx->center.y += 120.0f / 240.f * height;

    /* maximum radius */
    icefx->rmax = 80 + 440.0 * exprand();
}

static int icefx_new(effect_t *e)
{
    icefx_t *icefx = (icefx_t *)e;
    int i;

    icefx->count = 156;
    icefx->ice = calloc(sizeof(ice_t), icefx->count);

    for (i = 0; i < icefx->count; i++) {
        ice_instantiate_toplevel(&icefx->ice[i]);
        icefx->ice[i].new(&icefx->ice[i]);
    }

    ft_renderer_instantiate_toplevel(&icefx->renderer);
    icefx->renderer.new(&icefx->renderer);

    icefx->alpha = 0;
    icefx->width = 320;
    icefx->height = 240;

    return 1;
}

static int icefx_destroy(effect_t *e)
{
    icefx_t *icefx = (icefx_t *)e;
    int i;

    for (i = 0; i < icefx->count; i++) {
        icefx->ice[i].destroy(&icefx->ice[i]);
        ice_retire(&icefx->ice[i]);
    }

    icefx->renderer.destroy(&icefx->renderer);
    ft_renderer_retire(&icefx->renderer);

    return 1;
}

static void icefx_throw(icefx_t *self, double now)
{
    int i;
    double var = 800.0 * (now) / demo_get_instance()->end_ms;
    double gamma = 0.45;

    if (now < 80000.0)
        gamma = 0.45 + (2.2 - 0.45) * (80000.0 - now) / 80000.0;

    choose_center(self);

    for (i = 0; i < self->count; i++) {
        double u, v;
        double r;
        double angle;

        unirand2d(&u, &v);
        r = exprand();

        angle = atan2(u, v);

        u *= r * self->rmax;
        u += self->center.x;

        v *= r * self->rmax;
        v += self->center.y;

        self->ice[i].destroy(&self->ice[i]);
        object_retire(ice_to_object(&self->ice[i]));
        ice_instantiate_toplevel(&self->ice[i]);
        self->ice[i].new(&self->ice[i]);
        self->ice[i].create(&self->ice[i], gamma, var + 400.0 * unirand(),
                            r * self->rmax + 75.0 * unirand());

        self->ice[i].x = u;
        self->ice[i].y = v;
        self->ice[i].scalev.x = r;
        self->ice[i].scalev.y = r;

        self->ice[i].angle = angle;
    }

    self->onset_ms = now;
}

static void icefx_computes(effect_t *e, void *content, double ms)
{
    icefx_t *icefx = (icefx_t *)e;
    video_effect_t *v = (video_effect_t *)e;

    image_t image;

    outline_t *outl;

    int i;

    image.pixels = content;
    image.width = v->width;
    image.height = v->height;
    image.pitch = v->pitch;

    for (i = 0; i < icefx->count; i++) {
        double f = 0.0;
        icefx->ice[i].ratio = (-cos(ms / 60000.0) + 1.0) / 2.0;
        icefx->ice[i].decal = ms / 240000.0 * M_PI_2;
        outl = icefx->ice[i].update(&icefx->ice[i], ms - icefx->onset_ms);

        icefx->renderer.pen_x = icefx->ice[i].x;
        icefx->renderer.pen_y = icefx->ice[i].y;

        if (ms > 80000.0 && ms <= 140000.0) {
            f = 0.04 * (60000.0 - (ms - 80000.0)) / 60000.0 *
                (ms - icefx->onset_ms) / icefx->ice[i].end_ms;
            icefx->renderer.pen_x += f * (icefx->ice[i].x - icefx->center.x);
            icefx->renderer.pen_y += f * (icefx->ice[i].y - icefx->center.y);
        }
        icefx->renderer.color =
            pixel_lerp(icefx->alpha, icefx->ice[i].color, 0x00000);

        icefx->renderer.draw_outline(&icefx->renderer, &image, outl);
    }
}

icefx_t *icefx_instantiate(icefx_t *x)
{
    icefx_t *icefx = icefx_instantiate_super(x);

    icefx->super.super.new = icefx_new;
    icefx->super.super.destroy = icefx_destroy;
    icefx->super.super.computes = icefx_computes;

    icefx->throw = icefx_throw;

    return icefx;
}
