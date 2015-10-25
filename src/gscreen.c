/* a10 696
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/gscreen.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 696 */

#include "gscreen.h"

#include <libc/math.h>
#include <libc/stdlib.h>
#include "neq.xbm"
#include <audio/wavetable.h>
#include <audio/wrapper.h>

static float gauss_gen(unsigned int d2, unsigned int of)
{
    static wrapper_t wrap = {WAVETABLE_BITS, {2048.0}};
    wavetable_t g = wavetable_get_gaussian();

    wrapper_set(&wrap, (double)WAVETABLE_SIZE / 2 * (1.0 + (double)d2 / of));

    return wavetable_get_linear(g, &wrap);
}

static int gscreen_new(effect_t *self)
{
    gscreen_t *scr = (gscreen_t *)self;

    // load image
    scr->w = pix_width;
    scr->h = pix_height;
    scr->amp = (unsigned char **)malloc(scr->h * sizeof(unsigned char *));
    {
        int i;
        for (i = 0; i < scr->h; i++) {
            int j;
            *(scr->amp + i) = (unsigned char *)malloc(scr->w);
            for (j = 0; j < scr->w; j++) {
                unsigned char c = pix_bits[(i * scr->w + j) / 8];
                c >>= ((i * scr->w + j) % 8);
                *(*(scr->amp + i) + j) = (c & 0x1) ? 255 : 0;
            }
        }
    }

    // init particles
    scr->ps = ps_instantiate_toplevel(NULL);
    scr->ps->new (scr->ps, scr->w * scr->h, 32, gauss_gen, 255, 255, 255);

    return 1;
}

static void set_frame_size(video_effect_t *v, int width, int height, int pitch)
{
    gscreen_t *scr = (gscreen_t *)v;

    scr->super.width = width;
    scr->super.height = height;
    scr->super.pitch = pitch;

    // adjust parameters
    scr->winc = width / scr->w;
    scr->hinc = height / scr->h;
    scr->s = 0.75 * (scr->winc > scr->hinc ? scr->winc : scr->hinc);
}

static int gscreen_destroy(effect_t *self)
{
    gscreen_t *x = (gscreen_t *)self;

    x->ps->destroy(x->ps);
    object_retire(ps_to_object(x->ps));
    x->ps = NULL;

    return 1;
}

static void gscreen_computes(effect_t *self, void *content, double ms)
{
    gscreen_t *scr = (gscreen_t *)self;
    int32_t *__restrict__ pixels = content;

    static double xr = 8.0;
    static double yr = 0.8;
    static double cr = .9;
    static double csr = 255.0;
    static int f = 0;
    int i, j;
    int x = 0;
    int y = 0;
    int p = 0;

    if (!(++f % 1000)) {
        xr = 8.0;
        yr = 0.8;
        cr = .9;
        csr = 255.0;
    }

    for (j = 0; j < scr->h; j++) {
        for (i = 0; i < scr->w; i++) {
            int xd = 0;
            int yd = 0;
            int c = *(*(scr->amp + j) + i);
            c *= (1.0 - cr) + cr * (rand() - RAND_MAX / 2) / RAND_MAX;
            c += csr * rand() / RAND_MAX;
            if (c < 0) {
                scr->ps->particles[p].amp = 0.0;
                scr->ps->particles[p].size = 0.0; // disable
            } else {
                if (c > 255) {
                    c = 255;
                }
                xd = scr->s * xr * (rand() - RAND_MAX / 2) / RAND_MAX;
                yd = scr->s * yr * (rand() - RAND_MAX / 2) / RAND_MAX;

                // update particle
                scr->ps->particles[p].x = x + xd;
                scr->ps->particles[p].y = y + yd;
                scr->ps->particles[p].size = scr->s * 3 * c / 255.0;
                scr->ps->particles[p].size_phase =
                    scr->ps->opacity_r / scr->ps->particles[p].size;
                scr->ps->particles[p].amp = c / 255.0;
            }

            x += scr->winc;
            p++;
        }
        x = 0;
        y += scr->hinc;
    }

    xr *= .99;
    yr -= .00008;
    cr *= .999;
    csr *= .92;

    scr->ps->render(scr->ps, pixels, scr->super.width, scr->super.height,
                    scr->super.pitch);
}

gscreen_t *gscreen_instantiate(gscreen_t *x)
{
    gscreen_t *s = gscreen_instantiate_super(x);

    s->super.super.new = gscreen_new;
    s->super.super.destroy = gscreen_destroy;
    s->super.super.computes = gscreen_computes;
    s->super.set_frame_size = set_frame_size;

    return s;
}
