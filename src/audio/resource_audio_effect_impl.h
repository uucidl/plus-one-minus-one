/* a10 336
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/audio/resource_audio_effect_impl.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 336 */



#ifndef KNOS_AUDIO_RESOURCE_AUDIO_EFFECT_IMPL_H
#define KNOS_AUDIO_RESOURCE_AUDIO_EFFECT_IMPL_H

/* reusable, private implementation for resource_audio_effect_t */

#include "resource_audio_effect.h"
#include <scripting/dictionary.h>
#include <scripting/bytecode_stream.h>

static inline
void resource_audio_effect_send_eos_message(resource_audio_effect_t* p, double ms) 
{
    if(p->eos_rcv) {
	bytecode_stream_t* stream = bytecode_stream_instantiate_toplevel(NULL);
	context_t* c              = context_instantiate_toplevel(NULL);
	dictionary_t* d           = dictionary_get_instance();

	if(p->eos_msg) 
	    stream->copy(stream, p->eos_msg);
	stream->prepend_atom(stream, d->get_atom(d, "end-of-stream"));
	
	c->object = p->eos_rcv;
	c->ms = ms;
	p->eos_rcv->receive(p->eos_rcv, stream, c);
    }
}

#endif
