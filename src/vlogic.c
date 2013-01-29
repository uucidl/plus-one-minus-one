/* a10 910
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vlogic.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 910 */




#include "vlogic.h"

#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_1_1_VLOGIC);

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/math.h>
#include <libc/stdint.h>

#if defined(__i386__)
/* XXX: _8pu_specialize_block is missing
#define USE_CG 1
*/
#endif

static
int vlogic_new(effect_t* self)
{
    vlogic_t* v = (vlogic_t*) self;

    _8pu_init(&v->state, 127);

    v->state.ops[0].opcode = ADD;
    v->state.ops[0].parameter = 1;
    v->state.ops[1].opcode = XOR;
    v->state.ops[1].parameter = 0x99;
    v->state.ops[2].opcode = AND;
    v->state.ops[2].parameter = 0x80;

#ifdef USE_CG
    v->code = malloc(1024); /* code gen section */
#endif

    /* generate palette */
#define GREY(a) ((((a)<<16) & 0xff0000) | \
		(((a)<<8) & 0x00ff00) | \
		(((a)) & 0xff))

    { int i; for(i=0; i<256; i++) {
	v->palette[i] = GREY(255-i);
    } }

#undef GREY
    return 1;
}

static
void vlogic_set_frame_size(video_effect_t* self, int width, int height, int pitch)
{
    vlogic_t* v  = (vlogic_t*) self;
    self->width  = width;
    self->height = height;
    self->pitch  = pitch;

    v->set_multiplier(v, 0,   16);
    v->set_multiplier(v, 1, 0.25);
}


static
void vlogic_set_multiplier(vlogic_t* self, unsigned int i, double multiplier)
{
    switch(i) {
    case 0:
	{
	    int nnz = self->super.width * multiplier;
	    if(nnz > self->nz) {
		if(!self->block) {
		    self->block = calloc(sizeof(unsigned char), nnz);
		} else {
		    self->block = realloc(self->block, nnz);
		}
	    }
	    self->nz = nnz;
	}
	break;
    case 1:
	{
	    int nny = self->super.width * multiplier;
	    if(nny > self->nz) {
		nny = self->nz;
	    }
	    self->ny = nny;
	}
	break;
    default:
	ERROR1("invalid multiplier index");
    }
}

static
void vlogic_generate_palette(vlogic_t* self, palette_gen_f gen)
{
    if(gen) {
	unsigned int i;
	for(i=0; i<256; i++) {
	    self->palette[i] = gen(i, 256);
	}
    }
}

static inline
uint32_t palette_lookup(vlogic_t* self, unsigned char i)
{
    return self->palette[i];
}

#ifdef USE_CG
static
void vlogic_computes_cg(effect_t* self, void* content, double ms)
{
    vlogic_t* vlogic = (vlogic_t*) self;
    int32_t* __restrict__ pixels = content;
    int n = vlogic->super.width * vlogic->super.height;
    const int nz = vlogic->nz;
    const int ny = vlogic->ny;
    const int width = vlogic->super.width;
    const int pitch = vlogic->super.pitch;
    int x = 0; /* current x position */

    if(!ny || !nz)
	return;

    vlogic->state.ops[1].parameter = ((int) ms % 8000) * 256 / 8000;
    vlogic->state.ops[2].parameter = (-cos(ms / 20000)/2 + 0.5) * 256;

    vlogic->compiled_generator = _8pu_specialize_block(&vlogic->state,
						       vlogic->code);

    while(n) {
	if(n >= nz) {
	    int count = nz;
	    while(count) {
		if(count >= ny) {
		    int count2 = ny;
		    int i = 0;

		    vlogic->compiled_generator(vlogic->block, count2);
		    while(count2--) {
			pixels[x++] = palette_lookup(vlogic, vlogic->block[i++]);
			if(x == width) {
			    x = 0;
			    pixels += pitch;
			}
		    }
		    // update ny indexed var
		    vlogic->state.ops[0].parameter++;
		    vlogic->compiled_generator =
			_8pu_specialize_block(&vlogic->state, vlogic->code);
		    //
		    count -= ny;
		    n     -= ny;
		} else {
		    int i = 0;

		    vlogic->compiled_generator(vlogic->block, count);
		    while(count) {
			// do shit
			pixels[x++] = palette_lookup(vlogic, vlogic->block[i++]);
			if(x == width) {
			    x = 0;
			    pixels += pitch;
			}

			n--;
			count--;
		    }
		}
		// update nz indexed var
		if(vlogic->state.ops[1].opcode == XOR)
		    vlogic->state.ops[1].opcode = OR;
		else
		    vlogic->state.ops[1].opcode = XOR;
		vlogic->compiled_generator =
		    _8pu_specialize_block(&vlogic->state, vlogic->code);
	    }
	} else {
	    int i = 0;
	    vlogic->compiled_generator(vlogic->block, n);
	    while(n) {
		pixels[x++] = palette_lookup(vlogic, vlogic->block[i++]);
		if(x == width) {
		    x = 0;
		    pixels += pitch;
		}

		n--;
	    }
	}
    }
}
#endif

void vlogic_computes(effect_t* self, void* content, double ms)
{
    vlogic_t* vlogic = (vlogic_t*) self;
    int32_t* __restrict__ pixels = content;
    int n = vlogic->super.width * vlogic->super.height;
    const int nz = vlogic->nz;
    const int ny = vlogic->ny;
    const int width = vlogic->super.width;
    const int pitch = vlogic->super.pitch;
    int x = 0; /* current x position */

    if(!ny || !nz)
	return;

    vlogic->state.ops[1].parameter = ((int) ms % 8000) * 256 / 8000;
    vlogic->state.ops[2].parameter = (-cos(ms / 20000)/2 + 0.5) * 256;

    while(n > 0) {
	if(n >= nz) {
	    int count = nz;
	    while(count > 0) {
		if(count >= ny) {
		    int count2 = ny;
		    while(count2-- > 0) {
			// do shit
			_8pu_next(&vlogic->state);
			pixels[x++] = palette_lookup(vlogic, vlogic->state.accum);
			if(x == width) {
			    x = 0;
			    pixels += pitch;
			}
			n--;
		    }
		    // update ny indexed var
		    vlogic->state.ops[0].parameter++;

		    //
		    count -= ny;
		} else {
		    while(count-- > 0) {
			// do shit
			_8pu_next(&vlogic->state);
			pixels[x++] = palette_lookup(vlogic, vlogic->state.accum);
			if(x == width) {
			    x = 0;
			    pixels += pitch;
			}
			n--;
		    }
		}
		// update nz indexed var
		if(vlogic->state.ops[1].opcode == XOR)
		    vlogic->state.ops[1].opcode = ADD;
		else
		    vlogic->state.ops[1].opcode = XOR;
	    }
	} else {
	    while(n-- > 0) {
		// do shit
		_8pu_next(&vlogic->state);
		pixels[x++] = palette_lookup(vlogic, vlogic->state.accum);
		if(x == width) {
		    x = 0;
		    pixels += pitch;
		}
	    }
	}
    }
}

vlogic_t* vlogic_instantiate(vlogic_t* x)
{
    vlogic_t* v = vlogic_instantiate_super (x);

    v->super.super.new = vlogic_new;

#ifdef USE_CG
    v->super.super.computes = vlogic_computes_cg;
#else
    v->super.super.computes = vlogic_computes;
#endif

    v->super.set_frame_size = vlogic_set_frame_size;

    v->set_multiplier = vlogic_set_multiplier;
    v->generate_palette = vlogic_generate_palette;

    return v;
}
