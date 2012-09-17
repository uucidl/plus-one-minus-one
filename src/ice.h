/* a10 104
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/ice.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 104 */



#ifndef ICE_H
  #define ICE_H

#include <freetype/outline.h>
#include <libc/stdint.h>

typedef struct ice_t
{
  outline_t super;

  unsigned int x;
  unsigned int y;
  uint32_t color;
  double   angle;
  double   decal; /* angle+decal is the destination angle */
  vector2d_t scalev;
  float    ratio; /* perturbance */

  /* quadratic growth -- 
     height = ms * ms * c
     when ms > end_ms, 
     height = end_h
  */
  double end_ms;
  double end_h;
  double c;

  int (*new)(struct ice_t* self);
  int (*destroy)(struct ice_t* self);

  /* initialize variables for the growth envelope */
  void (*create)(struct ice_t* self, double gamma, double end_ms, double h);

  /* returns an outline object representing 
     the outline at time 'ms' */
  struct outline_t* (*update)(struct ice_t* self, double ms);
} ice_t;

CLASS_INHERIT(ice, outline)

#endif
