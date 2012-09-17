/* a10 340
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/widgets/gauge.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 340 */




#include "gauge.h"
#include <lib/num.h>
#include <vector.h>
#include <generators/sincos.h>

#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/math.h>

/* returns 0 if area outside */
static
int clip(gauge_t* self, int width, int height,
	 int* l, int* u, int* w, int* h)
{
    if(self->x < width && self->y < height) {
	int x, y;
	if(self->x < 0)
	    *l = 0;
	else
	    *l = self->x;

	if(self->y < 0)
	    *u = 0;
	else
	    *u = self->y;

	x = self->x + self->w;
	y = self->y + self->h;

	if(x < 0)
	    return 0;
	else if(x > width) {
	    x = width;
	}

	if(y < 0)
	    return 0;
	else if(y > height) {
	    y = height;
	}

	*w = x - *l;
	*h = y - *u;

	return 1;
    } else
	return 0;
}

static
void gauge_set_percent(gauge_t* self, float percent)
{
    if(percent < 0.0)
	percent = 0.0;
    else if(percent > 1.0f)
	percent = 1.0f;

    self->percent = percent;
}

static
void hgauge_render(gauge_t* self, image_t* dest)
{
    int l, u; /* left, up borders */
    int w, h;

    if(clip(self, dest->width, dest->height, &l, &u, &w, &h)) {
	int w1 = self->w * self->percent;
	uint32_t* pixelsd = dest->pixels + l + u*dest->pitch;

	w1 = clamp(w1, 0, w);
	while(h--) {
	    memset(pixelsd,    0xff,     w1*sizeof(int32_t));
	    memset(pixelsd+w1, 0x00, (w-w1)*sizeof(int32_t));
	    pixelsd += dest->pitch;
	}
    }
}

static
void vgauge_render(gauge_t* self, image_t* dest)
{
    int l, u; /* left, up borders */
    int w, h;

    if(clip(self, dest->width, dest->height, &l, &u, &w, &h)) {
	int h1 = self->h * self->percent;
	uint32_t* pixelsd = dest->pixels + l + u*dest->pitch;
	int n;

	h1 = clamp(h1, 0, h);
	n = h-h1;
	while(n--) {
	    memset(pixelsd, 0x00, w*sizeof(int32_t));
	    pixelsd += dest->pitch;
	}

	n = h1;
	while(n--) {
	    memset(pixelsd, 0xff, w*sizeof(int32_t));
	    pixelsd += dest->pitch;
	}
    }
}

#define min(a, b) (a) < (b) ? (a) : (b)

static
void draw_arc(image_t* dest, int x0, int y0, int r, float angle)
{
    uint32_t* center = dest->pixels + x0 + y0*dest->pitch;

    if(angle <= M_PI_2) {
	uint32_t* line = center;
	sincos_state_t state;
	int left = 0;
	int right = 0;
	int n = angle*r;//r*(1.f-sin_a);
	sincos_init(&state, 2*M_PI*r);
	sincos_next(&state);
	while(n-- >= 0) {
	    int diff;
	    right = r * state.cos;
	    diff = right-left;
	    clamp(diff, 0, dest->width);
	    if(diff > 0)
		memset(line + (int)(-r*state.sin)*dest->pitch, 0xff, diff*sizeof(int32_t));
	    sincos_next(&state);
	}
	if(angle < M_PI_2) {
	    float dec = state.cos / state.sin;
	    float fright = right;
	    n = r*(state.sin);
	    line -= n*dest->pitch;
	    while(n-- > 0) {
		int diff;
		fright -= dec;
		right = fright;
		diff = right - left;
		clamp(diff, 0, dest->width);
		if(diff > 0)
		    memset(line, 0xff, diff*sizeof(int32_t));
		line += dest->pitch;
	    }
	}
    } else if(angle <= M_PI) {
	draw_arc(dest, x0, y0, r, M_PI_2-.001);
	angle -= M_PI_2;

	{
	    uint32_t* line = center;
	    sincos_state_t state;
	    float fleft = 0;
	    float inc;
	    int left = 0;
	    int right = 0;
	    int n = angle*r;

	    inc = cos(angle)/sin(angle);

	    sincos_init(&state, 2*M_PI*r);
	    sincos_next(&state);
	    while(n-- >= 0) {
		int diff;
		fleft += inc;
		left = fleft;
		right = r * state.sin;
		diff = right-left;
		clamp(diff, 0, dest->width);
		if(diff > 0)
		    memset(line + left + (int)(r*state.cos)*dest->pitch, 0xff, diff*sizeof(int32_t));
		sincos_next(&state);
	    }
	}
    } else if(angle <= (M_PI+M_PI_2)) {
	draw_arc(dest, x0, y0, r, M_PI-0.01);
	angle -= M_PI;

	{
	    uint32_t* line = center;
	    sincos_state_t state;
	    int left = 0;
	    int right = 0;
	    int n = angle*r;
	    sincos_init(&state, 2*M_PI*r);
	    sincos_next(&state);
	    while(n-- >= 0) {
		int diff;
		left = r * state.cos;
		diff = left-right;
		clamp(diff, 0, dest->width);
		if(diff > 0)
		    memset(line - left + (int)(r*state.sin)*dest->pitch, 0xff, diff*sizeof(int32_t));
		sincos_next(&state);
	    }

	    if(angle < M_PI_2) {
		float dec = state.cos / state.sin;
		float fleft = left;
		n = r*(state.sin);
		line += n*dest->pitch;
		while(n-- > 0) {
		    int diff;
		    fleft -= dec;
		    left = fleft;
		    diff = left - right;
		    clamp(diff, 0, dest->width);
		    if(diff > 0)
			memset(line - left, 0xff,
			       diff*sizeof(int32_t));
		    line -= dest->pitch;
		}
	    }
	}
    } else if(angle <= 2*M_PI) {
	draw_arc(dest, x0, y0, r, M_PI+M_PI_2-0.01);
	angle -= M_PI+M_PI_2;

	{
	    uint32_t* line = center;
	    sincos_state_t state;
	    float fright = 0;
	    float inc;
	    int left = 0;
	    int right = 0;
	    int n = angle*r;

	    inc = cos(angle)/sin(angle);

	    sincos_init(&state, 2*M_PI*r);
	    sincos_next(&state);
	    while(n-- >= 0) {
		int diff;
		fright += inc;
		right = fright;
		left = r * state.sin;
		diff = left - right;
		clamp(diff, 0, dest->width);
		if(diff > 0)
		    memset(line - left + (int)(-r*state.cos)*dest->pitch, 0xff, diff*sizeof(int32_t));
		sincos_next(&state);
	    }
	}
    } else {
	while(angle > 2*M_PI)
	    angle -= 2*M_PI;
	draw_arc(dest, x0, y0, r, angle);
    }

}

static
void rgauge_render(gauge_t* self, image_t* dest)
{
    int x0, y0, r;
    float angle = 2*M_PI*self->percent;
    x0 = self->x + self->w/2;
    y0 = self->y + self->h/2;
    r  = min(self->w, self->h);
    r /= 2;

    {
	vector3d_t o, b;
	//	line_t l = { &o, &b, 0x00ffffff };
	o.x = x0;
	o.y = y0;
	b.x = x0 + r * sin(angle);
	b.y = y0 - r * cos(angle);

	//	draw_line(dest->pixels, dest->width, dest->height, &l);
    }

    {
	sincos_state_t sstate;
	int n;
	uint32_t* center = dest->pixels + x0 + y0*dest->pitch;

	sincos_init(&sstate, 2*M_PI*r);
	sincos_next(&sstate);
	n = 2*M_PI*r;
	while(n--) {
	    center[(int)(r*sstate.cos) - (int)(r*sstate.sin)*dest->pitch] = 0x00ffffff;
	    sincos_next(&sstate);
	}

	draw_arc(dest, x0, y0, r, angle);

    }
}

gauge_t* gauge_instantiate(gauge_t* x)
{
    gauge_t* g = OBJECT_INSTANTIATE(gauge, x);

    g->set_percent = gauge_set_percent;

    return g;
}

vgauge_t* vgauge_instantiate(vgauge_t* x)
{
    vgauge_t* v = OBJECT_INSTANTIATE(vgauge, x);

    gauge_instantiate(v);

    v->render = vgauge_render;

    return v;
}

hgauge_t* hgauge_instantiate(hgauge_t* x)
{
    hgauge_t* h = OBJECT_INSTANTIATE(hgauge, x);

    gauge_instantiate(h);

    h->render = hgauge_render;

    return h;
}

rgauge_t* rgauge_instantiate(rgauge_t* x)
{
    rgauge_t* r = OBJECT_INSTANTIATE(rgauge, x);

    gauge_instantiate(r);

    r->render = rgauge_render;

    return r;
}
