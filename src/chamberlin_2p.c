/* a10 77
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/chamberlin_2p.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 77 */




#include "chamberlin_2p.h"

#include <system/pan.h>
#include <libc/math.h>
#include <libc/stdlib.h>

static
int chamberlin_2p_new(effect_t* self)
{
    chamberlin_2p_t* filter = (chamberlin_2p_t*) self;

    filter->f	    = 2;
    filter->q	    = 0.5;
    filter->low_l   = 0;
    filter->high_l  = 0;
    filter->band_l  = 0;
    filter->notch_l = 0;
    filter->low_r   = 0;
    filter->high_r  = 0;
    filter->band_r  = 0;
    filter->notch_r = 0;

    return 1;
}

static
void c_set_cutoff(chamberlin_2p_t* self, double cutoff_hz)
{
    if(cutoff_hz < 0.0) 
	cutoff_hz = 0.0;

    self->f = 2*sin(M_PI*cutoff_hz / self->super.sample_rate);
}

static
void c_set_resonance(chamberlin_2p_t* self, double q)
{
    if(q < 0.0)
	q = 0.001;
    else if(q > 1.0)
	q = 1.0;
    
    self->q = q;
}

static
void chamberlin_2p_computes_area(audio_effect_t* self, audio_area_t* area, double ms)
{
    chamberlin_2p_t* filter = (chamberlin_2p_t*) self;
    sample_t* __restrict__ samples = area->samples + area->head*filter->super.frame_size;
    int n = area->frame_number;
    int i = 0;
    double scale = filter->q;

    while(n--) {
	double input;
	
	/* left */
	input		 = samples[i];
	filter->low_l   += filter->f * filter->band_l;
	filter->high_l	 = scale * input - filter->low_l - 
	    filter->q*filter->band_l;
	filter->band_l  += filter->f * filter->high_l;
	filter->notch_l	 = filter->high_l + filter->low_l;
	samples[i]	 = filter->low_l;

	/* right */
	input		 = samples[i+1];
	filter->low_r   += filter->f * filter->band_r;
	filter->high_r	 = scale * input - filter->low_r - 
	    filter->q*filter->band_r;
	filter->band_r  += filter->f * filter->high_r;
	filter->notch_r	 = filter->high_r + filter->low_r;
	samples[i+1]	 = filter->low_r;

	i += filter->super.frame_size;
    }
}

chamberlin_2p_t* chamberlin_2p_instantiate(chamberlin_2p_t* x)
{
    chamberlin_2p_t* filter = chamberlin_2p_instantiate_super (x);

    filter->super.super.new	= chamberlin_2p_new;
    filter->super.computes_area = chamberlin_2p_computes_area;
    
    filter->set_cutoff	  = c_set_cutoff;
    filter->set_resonance = c_set_resonance;
    
    return filter;
}
