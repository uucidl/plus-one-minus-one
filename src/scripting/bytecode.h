/* a10 966
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/bytecode.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 966 */



#ifndef KNOS_SCRIPTING_BYTECODE_H
#define KNOS_SCRIPTING_BYTECODE_H

#include "atom.h"

typedef struct
{
    atom_t verb; /* or value */
    atom_t adverb; /* or type */
} bytecode_t;

#endif
