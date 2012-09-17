/* a10 411
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/dictionary.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 411 */




#include "dictionary.h"
#include "atom_container.h"

#include <libc/stdlib.h>
#include <library/cstr_map_impl.h>
#include <libc/string.h>
#include <libc/stdio.h>
#include <library/memory.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_SCRIPTING_DICTIONARY);

static
atom_container_t* atom_container_new(const char* name)
{
    atom_container_t* c = OBJECT_INSTANTIATE (atom_container, NULL);

    c->id = (long int) c;
    c->name = name;

    return c;
}

static inline 
atom_container_t* get_atom(dictionary_t* self, const char* name, atom_container_t** pac)
{
    cstr_map_value_t aac;
    atom_t a = 0;
    
    aac = cstr_map_get(&self->atoms, name);
    a = (atom_t) (cstr_map_value_is_there (aac) ? 
		  cstr_map_value_obtain (aac) : NULL);

    if(pac)
	*pac = (atom_container_t*) a;

    return (atom_container_t*) a;
}

static
atom_t dictionary_new_atom(dictionary_t* self, const char* name)
{
    atom_container_t* ac = get_atom(self, name, NULL);

    if(ac == NULL) {
	ac = atom_container_new(name);
	
	cstr_map_put(&self->atoms, name, ac);
	TRACE3("new atom: '%s' (<%x>)", name, (long int) ac);
    }

    return (atom_t) ac;
}

static
atom_t dictionary_new_atom_from_integer(dictionary_t* self, const int i)
{
    atom_t atom     = 0;
    unsigned int ii = i;
    unsigned int n  = 1;
    
    while(ii /= 10) 
	n++;
    n++; /* final \0 */
    {
	char* id_s = malloc (n);

	sprintf(id_s, "%u", i);
	id_s[n-1] = '\0';
	
	atom = self->get_atom(self, id_s);
	if(!atom) {
	    atom = self->new_atom(self, strdup(id_s));
	}
	free (id_s);
    }
    
    return atom;
}

static
atom_t dictionary_get_atom(dictionary_t* self, const char* name)
{
    atom_t a = (atom_t) get_atom(self, name, NULL);
    if(a == 0)
	TRACE2("unknown atom: '%s'", name);
    return a;
}

static
int dictionary_release (object_t* zelf) {
	dictionary_t* self = (dictionary_t*) zelf;

	self->atoms.destroy (&self->atoms);
	
	cstr_map_retire (&self->atoms);

	return 1;
}

dictionary_t* dictionary_instantiate(dictionary_t* x)
{
    dictionary_t* d = dictionary_instantiate_super (x);

    cstr_map_instantiate_toplevel(&d->atoms);

    object_set_release_callback (dictionary_to_object (d), dictionary_release);

    d->new_atom = dictionary_new_atom;
    d->new_atom_from_integer = dictionary_new_atom_from_integer;
    d->get_atom = dictionary_get_atom;

    /* the default atoms */
    d->new_atom(d, "atom");
    d->new_atom(d, "atomheart");
    d->new_atom(d, "integer");
    d->new_atom(d, "float");
    
    return d;
}

static dictionary_t* d = 0;

dictionary_t* dictionary_get_instance()
{
    if(!d) {
	d = dictionary_instantiate_toplevel(NULL);
    }
    return d;
}
