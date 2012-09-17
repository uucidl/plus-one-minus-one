/* a10 621
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/tap.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 621 */



#ifndef TAP_H
  #define TAP_H

/* 
   delay construction objects
*/

#include <system/effects.h>
#include <system/pan.h>

/*
  a buffer recording anything sent to it via the 'computes' method, with wraparound
*/

typedef struct tapin_t
{
  audio_effect_t super;

  void (*set_size)(struct tapin_t* self, unsigned int size);

  sample_t* line; /* delay line */
  unsigned int line_size; /* number of frames */
  unsigned int line_phase; /* current phase */
} tapin_t;

CLASS_INHERIT(tapin, audio_effect)

/* 
   an object reading from another tapin_t object.. setting 
   its initial or instantaneous phase via set_offset creates
   a standard delay effect,
   feeding the output back to the originating tapin_t creates
   the standard feedback delay effect.
*/

typedef struct tapout_t
{
  audio_effect_t super;

  void (*set_tapin)(struct tapout_t* self, tapin_t* tapin);
  void (*set_offset)(struct tapout_t* self, unsigned int offset);

  tapin_t* tapin;
  unsigned int out_phase;
} tapout_t;

CLASS_INHERIT(tapout, audio_effect)

#endif
