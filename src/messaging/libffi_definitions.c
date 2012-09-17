/* a10 711
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/libffi_definitions.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 711 */




#include <messaging/libffi_definitions.h>
#include <library/map_impl.h>

#include <scripting/dictionary.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_MESSAGING_LIBFFI_DEFINITIONS);

static dictionary_t* dict;
static atom_t atom_atom;
static atom_t atom_integer;
static atom_t atom_float;
static atom_t atom_double;

static
void initialize_atoms()
{
    dict = dictionary_get_instance();
    atom_atom    = dict->new_atom(dict, "atom");
    atom_integer = dict->new_atom(dict, "integer");
    atom_float   = dict->new_atom(dict, "float");
    atom_double  = dict->new_atom(dict, "double");
}

static
desc_t* add_desc(definitions_t* self)
{
    return calloc(sizeof(ffi_desc_t), 1);
}

static
void libffi_definitions_set_callback(definitions_t* self, 
				     atom_t a, bytecode_stream_t* args, recp_f f)
{
    ffi_desc_t* desc = (ffi_desc_t*) self->get_desc(self, a);
    ffi_type** atypes;

    if(!desc) {
	desc = (ffi_desc_t*) add_desc(self);
	map_put (&self->definitions, atom_get_id(a), desc);
    }

    /* standard interface */
    desc->desc.atom       = a;
    desc->desc.arg_number = args ? args->get_count(args) : 0;
    desc->desc.function   = f;

    desc->ffi_arg_number = desc->desc.arg_number;

    /* libffi side, set up args+2 arguments (last one is context) */
    desc->ffi_arg_number += 2; /* real arguments (self + arguments + context) */

    atypes = malloc(desc->ffi_arg_number*sizeof(ffi_type*));
    // self or context
    atypes[0]                        = &ffi_type_pointer;
    atypes[desc->ffi_arg_number - 1] = &ffi_type_pointer;

    if(args) {
	bytecode_iterator_t iterator;
	unsigned int i = 0;

	args->get_iterator(args, &iterator);

	for(i=1; i<desc->ffi_arg_number-1; i++) {
	    bytecode_t* b = iterator.next(&iterator);
	    if(b == NULL) {
		ERROR2("Unexpectedly exhausted argument list for selector: '%s'", atom_get_cstring_value(a));
	    } else {
		if(b->adverb != atom_atom) {
		    ERROR3("Argument bytecode #%d not an atom for selector: '%s'", i - 1, atom_get_cstring_value(a));
		} else {
		    const char* sel = atom_get_cstring_value(a);
		    if(b->verb == atom_atom) {
			atypes[i] = &ffi_type_pointer;
			TRACE3("%s#%d ffi_type_pointer", sel, i);
		    } else if(b->verb == atom_integer) {
			atypes[i] = &ffi_type_sint;
			TRACE3("%s#%d ffi_type_sint", sel, i);
		    } else if(b->verb == atom_float) {
			atypes[i] = &ffi_type_float;
			TRACE3("%s#%d ffi_type_float", sel, i);
		    } else if(b->verb == atom_double) {
			atypes[i] = &ffi_type_double;
			TRACE3("%s#%d ffi_type_double", sel, i);
		    } else {
			ERROR3("Argument type: '%s' unknown for selector: '%s'", atom_get_cstring_value(b->verb), sel);
			atypes[i] = &ffi_type_pointer;
		    }
		}
	    }
	}
	object_retire(bytecode_iterator_to_object(&iterator));
    }

    if(ffi_prep_cif
       (&desc->cif, FFI_DEFAULT_ABI, 
	desc->ffi_arg_number, &ffi_type_void, atypes) != FFI_OK) {
	ERROR2("ffi_prep_cif failure for selector: '%s'", 
	       atom_get_cstring_value(a));
    }
}

libffi_definitions_t* libffi_definitions_instantiate(libffi_definitions_t* x)
{
    libffi_definitions_t* d = libffi_definitions_instantiate_super (x);

    initialize_atoms();
    
    d->super.set_callback = libffi_definitions_set_callback;

    return d;
}
