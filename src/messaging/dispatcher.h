/* a10 882
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/dispatcher.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 882 */



#ifndef KNOS_MESSAGING_DISPATCHER_H
#define KNOS_MESSAGING_DISPATCHER_H

#include <scripting/bytecode_stream.h>
#include <library/memory.h>

#ifdef USE_BUILTIN_APPLY
#include <messaging/definitions.h>
#else
#include <messaging/libffi_definitions.h>
#endif

typedef struct dispatcher_t
{
    object_t super;
    void (*dispatch)(struct dispatcher_t* self, 
		     context_t* context, 
		     bytecode_stream_t* message);

#ifdef USE_BUILTIN_APPLY
    definitions_t definitions;
#else
    libffi_definitions_t definitions;
#endif
} dispatcher_t;

CLASS_INHERIT(dispatcher, object);

#endif
