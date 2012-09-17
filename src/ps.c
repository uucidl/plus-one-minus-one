/* a10 229
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/ps.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 229 */




/*
  particle system code
*/

#include <libc/stdlib.h>
#include "ps.h"
#include <lib/pixel.h>
#include <lib/fpu.h>

particle_t* particle_instantiate (particle_t* x) {
    return particle_instantiate_super (x);
}

static
int ps_new(ps_t* self, int max_particles, int max_radius,
	   float (*gen)(unsigned int d2, unsigned int of),
	   int r, int g, int b)
{
    /* create lookup table for pixel values */
    self->eamp_lu = calloc(256, sizeof(int32_t));
    { int i; for(i = 0; i < 256; i++) {
	self->eamp_lu[i] = (b*i>>8) | (g*i>>8)<<8 | (r*i>>8)<<16;
    } }

    /* create opacity levels */
    self->opacity_r = max_radius;
    self->opacity_n = 2 * max_radius * max_radius;
    self->opacity   = calloc(sizeof(float), self->opacity_n);

    { int i; for(i=0; i<self->opacity_n; i++) {
	self->opacity[i] = gen(i, self->opacity_n);
    } }

    self->lookup = calloc(sizeof(int*), self->opacity_r);
    { int i; for(i=0; i<self->opacity_r; i++) {
	self->lookup[i] = calloc(sizeof(int), self->opacity_r);
	{ int j; for(j=0; j<self->opacity_r; j++) {
	    self->lookup[i][j] = 255 * self->opacity[i*i + j*j];
	} }
    } }

    self->particles_n = max_particles;
    self->particles = calloc(sizeof(particle_t), self->particles_n);
    { int i; for(i=0; i<self->particles_n; i++) {
	particle_instantiate_toplevel (&self->particles[i]);
    } }

    return 1;
}

static
int ps_destroy(ps_t* self)
{
    free(self->eamp_lu);
    free(self->opacity);

    { int i; for(i=0; i<self->opacity_r; i++) {
	free(self->lookup[i]);
    } }
    free(self->lookup);

    { int i; for(i=0; i<self->particles_n; i++) {
      object_retire(particle_to_object(&self->particles[i]));
    } }
    free(self->particles);

    return 1;
}

static
void dumb_draw(ps_t* self, int32_t* pixels,
	       unsigned int width, unsigned int height, unsigned int pitch)
{
    int p;
    int16_t fpu_status = fpu_setround();

    for(p=0; p<self->particles_n; p++) {
	int32_t* pixeld;

	if(self->particles[p].size <= 0.0 ||
	   self->particles[p].x <= self->particles[p].size ||
	   self->particles[p].y <= self->particles[p].size ||
	   self->particles[p].x >= (width - self->particles[p].size) ||
	   self->particles[p].y >= (height- self->particles[p].size) ||
	   self->particles[p].amp <= 0.0)
	    continue;
	else {
	    const int fx = float_to_int(self->particles[p].x);
	    const int fy = float_to_int(self->particles[p].y);
	    const float dec = self->particles[p].size_phase;
	    const unsigned int size_i = float_to_int(self->particles[p].size);
	    signed int i;
	    signed int j = size_i;
	    float ii; // phase in lookup table
	    float jj = self->opacity_r - 1.0 + dec;
	    int offset_line = j*pitch;

	    pixeld = pixels + fy*pitch + fx;
	    while(j--) {
		jj -= dec;
		/* FIXME: */
		if(jj < 0.0) jj = 0.0;
		offset_line -= pitch;

		ii = self->opacity_r - 1.0 + dec;
		i = size_i;
		while(--i) {
		    int32_t* pixel;
		    int      amp;

		    ii -= dec;
		    amp = float_to_int(self->particles[p].amp*self->lookup[float_to_int(ii)][float_to_int(jj)]);
		    if(amp <= 0)
			continue;
		    else if(amp >= 255)
			amp = 0xffffff;
		    else
			amp = self->eamp_lu[amp]; /* expands amp into 32bit */

		    pixel = pixeld+offset_line+i;
		    *pixel = pixel_add_saturate(amp, *pixel);

		    pixel[-2*i] = pixel_add_saturate(amp, pixel[-2*i]);

		    if(j!=0) {
			pixel[-2*offset_line] =
			    pixel_add_saturate(amp, pixel[-2*offset_line]);
			pixel[-2*offset_line-2*i] =
			    pixel_add_saturate(amp, pixel[-2*offset_line-2*i]);
		    }
		}
		/* i == 0 */
		{
		    int32_t* pixel;
		    int      amp;

		    /* FIXME:
		    if(ii < 0.0) ii = 0.0;
		    */
		    amp = float_to_int(self->particles[p].amp*self->lookup[0][float_to_int(jj)]);
		    if(amp <= 0)
			continue;
		    else if(amp >= 255)
			amp = 0xffffff;
		    else
			amp = self->eamp_lu[amp];

		    pixel = pixeld+offset_line+i;
		    *pixel = pixel_add_saturate(*pixel, amp);

		    if(j!=0) {
			pixel[-2*offset_line] =
			    pixel_add_saturate(pixel[-2*offset_line], amp);
		    }
		}
	    }
	}
    }

    (void) fpu_restore (fpu_status);
}

ps_t* ps_instantiate(ps_t* x)
{
    ps_t* ps = ps_instantiate_super (x);

    ps->new =     ps_new;
    ps->destroy = ps_destroy;
    ps->render =  dumb_draw;

    return (ps_t*) ps;
}
