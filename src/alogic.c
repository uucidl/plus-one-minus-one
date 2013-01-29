/* a10 823
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/alogic.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 823 */




#include "alogic.h"

#include <libc/stdlib.h>
#include <libc/math.h>

#include <system/main.h>
#include <system/pan.h>

static
int alogic_new(effect_t* self)
{
    alogic_t* alogic = (alogic_t*) self;

    _8pu_init(&alogic->state, 127);

    chamberlin_2p_instantiate_toplevel (&alogic->filter);
    alogic->filter.super.super.new((effect_t*) &alogic->filter);

    alogic->state.ops[0].opcode = ADD;
    alogic->state.ops[0].parameter = 1;
    alogic->state.ops[1].opcode = XOR;
    alogic->state.ops[1].parameter = 0x99;
    alogic->state.ops[2].opcode = AND;
    alogic->state.ops[2].parameter = 0x80;

    alogic->i = 0;
#ifdef USE_CG
    alogic->and_code = malloc(1024); /* code gen section */
    alogic->xor_code = malloc(1024); /* code gen section */

    /* lets precompile */
    if(alogic->state.ops[1].opcode == XOR) {
	_8pu_specialize(&alogic->state, alogic->xor_code);
	alogic->state.ops[1].opcode = AND;
	_8pu_specialize(&alogic->state, alogic->and_code);
	alogic->state.ops[1].opcode = XOR;
	alogic->nexter = (gen_t) alogic->xor_code;
    } else {
	_8pu_specialize(&alogic->state, alogic->and_code);
	alogic->state.ops[1].opcode = XOR;
	_8pu_specialize(&alogic->state, alogic->xor_code);
	alogic->state.ops[1].opcode = AND;
	alogic->nexter = (gen_t) alogic->and_code;
    }
#endif

    return 1;
}

static
int alogic_destroy (effect_t* zelf)
{
    alogic_t* self = (alogic_t*) zelf;
    self->filter.super.super.destroy (&self->filter.super.super);
    chamberlin_2p_retire (&self->filter);

    return 1;
}

static
void alogic_set_area_parameters(audio_effect_t* ae,
			   int sample_rate, int frame_number, int frame_size)
{
    alogic_t* self = (alogic_t*) ae;

    ae->sample_rate  = sample_rate;
    ae->frame_number = frame_number;
    ae->frame_size   = frame_size;

    self->filter.super.set_area_parameters((audio_effect_t*) &self->filter,
					   ae->sample_rate,
					   ae->frame_number, ae->frame_size);
    self->filter.set_cutoff(&self->filter, 8000.0);
    self->filter.set_resonance(&self->filter, 0.22);

    self->block = calloc(sizeof(unsigned char),
			 ae->sample_rate/363.0);
}

#define AMP(a) (sample_t)((a-127)/127.0f)

static void alogic_computes_area(audio_effect_t* self, audio_area_t* area, double ms) __attribute__((unused));

static
void alogic_computes_area(audio_effect_t* self, audio_area_t* area, double ms)
{
    alogic_t* alogic = (alogic_t*) self;
    sample_t* __restrict__ samples = area->samples + area->head*alogic->super.frame_size;
    int i = 0;
    int n = area->frame_number;

    alogic->nz = alogic->super.sample_rate/363.0;
    alogic->ny = alogic->super.sample_rate/2909.0;

    alogic->state.ops[1].parameter = ((int) ms % 8000) * 256 / 8000;
    alogic->state.ops[2].parameter = (-cos(ms / 20000)/2 + 0.5) * 256;
    while(n--) {
	sample_t val;
	_8pu_next(&alogic->state);
	val = AMP(alogic->state.accum);
	samples[i] = val;
	samples[i+1] = val;
	if(!(alogic->i % alogic->nz)) {
	    if(alogic->state.ops[1].opcode == XOR)
		alogic->state.ops[1].opcode = AND;
	    else
		alogic->state.ops[1].opcode = XOR;
	}
	if(!(alogic->i % alogic->ny)) {
	    alogic->state.ops[0].parameter++;
	}
	alogic->i++;
	i += alogic->super.frame_size;
    }
    alogic->filter.super.super.computes((effect_t*) &alogic->filter,
					area, ms);
}

#ifdef USE_CG
unsigned char n_code[1024];
gen_t nexter;

/*

  this is an alternative implementation, using more aggressive
  specialization. it seems slower than the second version.

*/
#if 0
static
void alogic_computes_area_cg(audio_effect_t* self, audio_area_t* area, double ms)
{
    alogic_t* alogic = (alogic_t*) self;
    sample_t* __restrict__ samples = area->samples + area->head*alogic->super.frame_size;
    int i = 0;
    int n = area->frame_number;

    alogic->nz = alogic->super.sample_rate/363.0;
    alogic->ny = alogic->super.sample_rate/2909.0;

    alogic->state.ops[1].parameter = ((int) ms % 8000) * 256 / 8000;
    alogic->state.ops[2].parameter = (-cos(ms / 20000)/2 + 0.5) * 256;

    nexter = _8pu_specialize_static(&alogic->state, n_code);

    while(n--) {
	sample_t val;
	nexter();
	val = AMP(alogic->state.accum);
	samples[i] = val;
	samples[i+1] = val;
	if(!(alogic->i % alogic->nz)) {
	    if(alogic->state.ops[1].opcode == XOR)
		alogic->state.ops[1].opcode = AND;
	    else
		alogic->state.ops[1].opcode = XOR;
	}
	if(!(alogic->i % alogic->ny)) {
	    alogic->state.ops[0].parameter++;
	}
	nexter = _8pu_specialize_static(&alogic->state, n_code);

	alogic->i++;
	i += alogic->super.frame_size;
    }
    alogic->filter.super.super.computes((effect_t*) &alogic->filter, area, ms);
}
#endif

/* using non static parameter specialization */
static
void alogic_computes_area_cg_2(audio_effect_t* self, audio_area_t* area, double ms)
{
    alogic_t* alogic = (alogic_t*) self;
    sample_t* __restrict__ samples = area->samples + area->head*alogic->super.frame_size;
    int i = 0;
    int n = area->frame_number;

    alogic->nz = alogic->super.sample_rate/363.0;
    alogic->ny = alogic->super.sample_rate/2909.0;

    alogic->state.ops[1].parameter = ((int) ms % 8000) * 256 / 8000;
    alogic->state.ops[2].parameter = (-cos(ms / 20000)/2 + 0.5) * 256;


    while(n--) {
	sample_t val;
	alogic->nexter();
	val = AMP(alogic->state.accum);
	samples[i] = val;
	samples[i+1] = val;
	if(!(alogic->i % alogic->nz)) {
	    if(alogic->state.ops[1].opcode == XOR) {
		alogic->state.ops[1].opcode = AND;
		alogic->nexter = (gen_t) alogic->and_code;
	    } else {
		alogic->state.ops[1].opcode = XOR;
		alogic->nexter = (gen_t) alogic->xor_code;
	    }
	}
	if(!(alogic->i % alogic->ny)) {
	    alogic->state.ops[0].parameter++;
	}

	alogic->i++;
	i += alogic->super.frame_size;
    }
    alogic->filter.super.super.computes((effect_t*) &alogic->filter, area, ms);
}
#endif

alogic_t* alogic_instantiate(alogic_t* x)
{
    alogic_t* alogic = alogic_instantiate_super (x);

    alogic->super.super.new = alogic_new;
    alogic->super.super.destroy = alogic_destroy;
#ifdef USE_CG
    alogic->super.computes_area = alogic_computes_area_cg_2;
#else
    alogic->super.computes_area = alogic_computes_area;
#endif
    alogic->super.set_area_parameters = alogic_set_area_parameters;

    return alogic;
}
