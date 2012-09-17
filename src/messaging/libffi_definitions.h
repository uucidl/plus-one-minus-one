/* a10 554
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/libffi_definitions.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 554 */



#ifndef KNOS_MESSAGING_LIBFFI_DEFINITIONS_H
#define KNOS_MESSAGING_LIBFFI_DEFINITIONS_H

#include <messaging/definitions.h>
#include <ffi/ffi.h>

typedef struct ffi_desc_t
{
    desc_t  desc;
    unsigned int ffi_arg_number;
    ffi_cif cif;
} ffi_desc_t;

typedef struct libffi_definitions_t {
    definitions_t super;
} libffi_definitions_t;

CLASS_INHERIT (libffi_definitions, definitions);

#endif
