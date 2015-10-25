/* a10 763
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/ice.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 763 */

#include "ice.h"

#include <libc/stdlib.h>

#include <lib/chance.h>

static int init_p = 0;
static outline_t outline; /* the backup outline */

static void ice_create_outline(ice_t *self)
{
    outline_t *outl = outline_instantiate_toplevel(&outline);
    vector2d_t a, b, c, d;
    double h;
    double width = 17.0;

    a.x = -width;
    a.y = 0.0;
    b.x = width;
    b.y = 0.0;

    h = 1.0;

    c.x = -width * 2;
    c.y = h;
    d.x = width * 2;
    d.y = h;

    outl->new (outl);

    outl->move_to(outl, &a)->line_to(outl, &b)->line_to(outl, &d)->line_to(outl,
                                                                           &c);
}

static void ice_restore_outline(ice_t *self)
{
    outline_t *src = &outline;
    outline_t *dst = &self->super;

    dst->copy(dst, src);
}

static int ice_new(ice_t *self)
{

    self->super.new(&self->super);

    self->x = 60 + unirand() * 200;
    self->y = 50 + unirand() * 100;
    self->angle = 2 * unirand() * M_PI;

    if (!init_p)
        ice_create_outline(self);

    init_p = 1;

    return 1;
}

static int ice_destroy(ice_t *self) { return 1; }

static void ice_create(ice_t *self, double gamma, double end_ms, double h)
{
    double u, v, w, s;

    /* time */
    self->end_ms = end_ms;
    self->end_h = h;
    self->c = self->end_h / (self->end_ms * self->end_ms);

    /* color */
    self->color = 0x3d000000;

    unirand2d(&u, &v);

    {
        w = self->angle / (2 * M_PI);
    }

    s = pow(unirand(), gamma) / 2.0;
    u += 1.0;
    v += 1.0;
    u *= s;
    v *= s;
    w *= s;

    self->color |= ((int)(255 * u)) << 16;
    self->color |= ((int)(128 * w)) << 8;
    self->color |= ((int)(255 * v));

    ice_restore_outline(self);
}

static outline_t *ice_update(ice_t *self, double ms)
{
    outline_t *outl = &self->super;
    vector2d_t scalevv;
    double scale = ms * ms * self->c;

    if (ms < self->end_ms) {
        ice_restore_outline(self);

        scalevv.x = 2.0f * (1.0f + self->ratio * (unirandf() - 1.0f));
        scalevv.y = scale;

        outl->scale(outl, &scalevv)
            ->scale(outl, &self->scalev)
            ->rotate(outl, self->angle + self->decal * ms / self->end_ms);
    }

    return outl;
}

ice_t *ice_instantiate(ice_t *x)
{
    ice_t *ice = ice_instantiate_super(x);

    ice->new = ice_new;
    ice->destroy = ice_destroy;
    ice->create = ice_create;
    ice->update = ice_update;

    return ice;
}
