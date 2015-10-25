/* a10 906
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/definitions.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 906 */

#ifndef KNOS_MESSAGING_DEFINITIONS_H
#define KNOS_MESSAGING_DEFINITIONS_H

#include <scripting/atom.h>

#include <messaging/context.h>

#include <library/map.h>
#include <library/memory.h>
#include <scripting/bytecode_stream.h>

typedef void (*recp_f)(context_t *self, ...);

typedef struct desc_t {
    atom_t atom;
    int arg_number;
    recp_f function;
} desc_t;

typedef struct definitions_t {
    object_t super;
    /*
      a definition is accepted in the form of a bytecode_stream of atoms
      identifying the types of
      the arguments. (accepted: atom:integer, atom:float, atom:atom)
     */
    void (*set_callback)(struct definitions_t *self, atom_t a,
                         bytecode_stream_t *args, recp_f f);
    desc_t *(*get_desc)(struct definitions_t *self, atom_t a);

    map_t definitions;
} definitions_t;

CLASS_INHERIT(definitions, object);

#endif
