/* a10 889
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/atom.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 889 */



#ifndef KNOS_SCRIPTING_ATOM_H
#define KNOS_SCRIPTING_ATOM_H

/*
  atoms are an opaque type, referring to one persistent entity throughout
  the system.
*/

#include <libc/stdint.h>

typedef intptr_t atom_t;

uint32_t    atom_get_id(atom_t a);
const char* atom_get_cstring_value(atom_t a);

atom_t atom_new_float(float f);
float  atom_get_float_value(atom_t a);

atom_t atom_new_integer(int i);
int    atom_get_integer_value(atom_t a);

#endif
