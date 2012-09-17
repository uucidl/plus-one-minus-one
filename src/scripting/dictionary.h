/* a10 950
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/dictionary.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 950 */



#ifndef KNOS_SCRIPTING_DICTIONARY_H
#define KNOS_SCRIPTING_DICTIONARY_H

/*
  a dictionary establishes the mapping between atoms and their 
  string value

  NULL is the undefined atom
*/

#include "atom.h"
#include <library/cstr_map.h>
#include <library/memory.h>

typedef struct dictionary_t
{
    object_t super;

    atom_t (*new_atom)(struct dictionary_t* self, const char* name);
    /* create an atom converting from an integer to a decimal number string */
    atom_t (*new_atom_from_integer)(struct dictionary_t* self, const int i);
    atom_t (*get_atom)(struct dictionary_t* self, const char* name);

    /* mapping between id and atom */
    cstr_map_t atoms; /* key -> atom_t* */
} dictionary_t;

CLASS_INHERIT(dictionary, object);

dictionary_t* dictionary_get_instance();

#endif
