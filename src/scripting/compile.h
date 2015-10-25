/* a10 246
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/compile.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 246 */

#ifndef KNOS_SCRIPTING_COMPILE_H
#define KNOS_SCRIPTING_COMPILE_H

#include "bytecode_stream.h"
#include <library/strings.h>

/*
  compile a string describing each bytecodes with the <adverb>:<verb> form
  by appending into the corresponding bytecode_stream_t
*/
bytecode_stream_t *compile_cstring(const char *string, bytecode_stream_t *x);
bytecode_stream_t *compile_string(const string_t *string, bytecode_stream_t *x);

/*
  compile an OSC message of the form "/address1/address2/address3 {arguments}"
  by appending into a proper bytecode_stream_t
    "{arguments} atom:address3 route atom:address2 route atom:address1 route"
*/
bytecode_stream_t *compile_osc_string(const string_t *string,
                                      bytecode_stream_t *x);

#endif
