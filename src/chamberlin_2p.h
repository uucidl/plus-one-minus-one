/* a10 181
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/chamberlin_2p.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 181 */



#ifndef _CHAMBERLIN_2P_H_
#define _CHAMBERLIN_2P_H_

/*
  from: http://www.smartelectronix.com/musicdsp/filters.php

  Type: 12db resonant low, high or bandpass
  References: Effect Deisgn Part 1, Jon Dattorro, J. Audio Eng. Soc., 
              Vol 45, No. 9, 1997 September
  Notes:
         Digital approximation of Chamberlin two-pole low pass. 
         Easy to calculate coefficients, easy to process algorithm.
*/

#include <system/effects.h>

typedef struct chamberlin_2p_t
{
    audio_effect_t super;
    
    void (*set_cutoff)(struct chamberlin_2p_t* self, double cutoff_hz);
    void (*set_resonance)(struct chamberlin_2p_t* self, double q);
    
    double f; /* cutoff parameter computed by set_cutoff */
    /* resonance/bandwidth [0 < q <= 1]  most res: q=1, less: q=0 */
    double q;
    
    /* the outputs and state variables */
    double low_l;
    double high_l;
    double band_l;
    double notch_l;

    double low_r;
    double high_r;
    double band_r;
    double notch_r;
} chamberlin_2p_t;

CLASS_INHERIT(chamberlin_2p, audio_effect)

#endif
