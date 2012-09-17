/* a10 157
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/dispatcher.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 157 */




#include <messaging/dispatcher.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_MESSAGING_DISPATCHER);

#include <scripting/dictionary.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>

static atom_t atom_atom = 0;

#ifdef USE_BUILTIN_APPLY

typedef union {
    char* arg_ptr;
    char arg_regs[sizeof (char*)];
} *arglist_t; /* argument frame */

/* evil wizardry ahead */
static
void gcc_dispatch(dispatcher_t* self,  
		  context_t* context, bytecode_stream_t* message)
{
    arglist_t argframe;
    int stack_argsize;
    int reg_argsize;
    void* ret;
    desc_t* d;
    bytecode_iterator_t it;
    bytecode_t* b;
    dictionary_t* dict = dictionary_get_instance();
    int n;

    if(!message || !message->end) {
	WARNING1("message === null");
	return;
    }

    stack_argsize = 0;
    reg_argsize = 8;

    argframe = (arglist_t) malloc(sizeof(char*) + reg_argsize);

    d = self->definitions.get_desc(&self->definitions, message->end->code.verb);
    if(!d) {
	if(message->end->code.adverb == atom_atom)
	    WARNING2("description not found for verb '%s'", 
		     atom_get_cstring_value(message->end->code.verb) );
	else 
	    WARNING2("description not found for bytecode '%x'",
		     (unsigned int) message->end->code.verb);
	return;
    }

    message->get_iterator(message, &it);
    it.current = message->end;

    it.prev(&it);

    stack_argsize = 4; // for instance
    n = 0;
    while((b = it.prev(&it)) && n < d->arg_number ) {
	stack_argsize += 4;
	n++;
    }
    stack_argsize+= 4; // for final context object

    
    if(n < d->arg_number) {
	WARNING1("insufficient argument number.");
	return;
    }

    if(stack_argsize)
	argframe->arg_ptr = malloc(stack_argsize);
    else
	argframe->arg_ptr = 0;

    message->get_iterator(message, &it);
    it.current = message->end;

    it.prev(&it);

    { 
	int i = 0;
	int* arg_int = (int*) argframe->arg_ptr;

	arg_int[n+1] = (int) context;
	while((b = it.prev(&it)) && i < n) {
	    arg_int[i+1] = atom_get_integer_value(b->verb);
	    i++;
	}
	arg_int[0] = (int) context->object;
    }

    ret = __builtin_apply(d->function, argframe, stack_argsize);
    if(argframe->arg_ptr)
	free(argframe->arg_ptr);
    free(argframe);
}

#else 

/* if not def USE_BUILTIN_APPLY, use libffi version of the dispatcher */
#include "libffi_definitions.h"

static
void libffi_dispatch(dispatcher_t* self,  
		     context_t* context, bytecode_stream_t* message)
{
    bytecode_iterator_t it;
    unsigned int n;
    desc_t* d;
    bytecode_t* b;
    atom_t selector = 0;

    if(!message || !message->end) {
	WARNING1("message === null");
	return;
    }

    selector = message->end->code.verb;

    d = ((definitions_t*) &self->definitions)->get_desc
	((definitions_t*) &self->definitions, selector);
    if(!d) {
	if(message->end->code.adverb == atom_atom)
	    WARNING2("description not found for verb '%s'", 
		     atom_get_cstring_value(message->end->code.verb) );
	else 
	    WARNING2("description not found for bytecode '%x'",
		     (unsigned int) selector);
	return;
    }
    
    /* setup arguments */
    {
	/* +1 for self (beginning), +1 for context (end),  */
	/* argument i is pointed to by arguments[i] */
	void* arguments[d->arg_number+2]; 
	intptr_t nil_value = 0;
	
	/* set default values to undefined atom (0) */
	{
	    int i;
	    for(i=0; i<d->arg_number+2; i++) {
		arguments[i] = &nil_value;
	    }
	}

	message->get_iterator(message, &it);

	/* iterate from the end, then pass selector */
	it.current = message->end;
	it.prev(&it);
    
	n = 0;
	arguments[n++] = (void*) &context->object;
	while((b = it.prev(&it)) && n < d->arg_number+1) {
	    arguments[n++] = &b->verb;
	}
	if(n != d->arg_number+1)
	    WARNING4("selector: '%s', not enough arguments. (found %d / %d) Calling anyway.", 
		     atom_get_cstring_value (selector), n-1, d->arg_number);

	arguments[d->arg_number+1] = (void*) &context;

	{
	  void* rvalue = malloc(8);
	  TRACE1("calling function.");
	  ffi_call(&((ffi_desc_t*)d)->cif, (void*) d->function, rvalue, (void**) &arguments);
	  free(rvalue);
	}
    }
}
#endif

dispatcher_t* dispatcher_instantiate(dispatcher_t* x)
{
    dispatcher_t* d = dispatcher_instantiate_super (x);
    
    if(!atom_atom) {
	dictionary_t* dict = dictionary_get_instance();
	atom_atom = dict->get_atom(dict, "atom");
    }

#ifdef USE_BUILTIN_APPLY
    definitions_instantiate_toplevel (&d->definitions);
    d->dispatch = gcc_dispatch;
#else
    libffi_definitions_instantiate_toplevel (&d->definitions);
    d->dispatch = libffi_dispatch;
#endif
    
    return d;
}
