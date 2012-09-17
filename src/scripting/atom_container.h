/* a10 944
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/atom_container.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 944 */



#ifndef KNOS_SCRIPTING_ATOM_CONTAINER_H
#define KNOS_SCRIPTING_ATOM_CONTAINER_H

/* internal representation of an atom, only accessible through the atom.h
   interface */

#include <libc/stdint.h>
#include <library/memory.h>

typedef struct atom_container_t
{
    object_t super;

    intptr_t id;
    const char* name;
} atom_container_t;

CLASS_INHERIT(atom_container, object)

#endif
