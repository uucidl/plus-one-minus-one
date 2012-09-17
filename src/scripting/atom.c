/* a10 424
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/atom.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 424 */




#include "atom.h"
#include "atom_container.h"

atom_container_t* atom_container_instantiate (atom_container_t* x)
{
    return atom_container_instantiate_super (x);
}

uint32_t atom_get_id(atom_t a)
{
    atom_container_t* ac = (atom_container_t*) a;

    if(ac)
	return ac->id;
    else
	return 0;
}

const char* atom_get_cstring_value(atom_t a)
{
    atom_container_t* ac = (atom_container_t*) a;

    if(ac)
	return ac->name;
    else
	return "#undefined";
}

atom_t atom_new_float(float f)
{
    union {
	atom_t a;
	float f;
    } alias;

    alias.f = f;

    return alias.a;
}

float atom_get_float_value(atom_t a)
{
    union {
	atom_t a;
	float f;
    } alias;

    alias.a = a;
    return alias.f;
}

atom_t atom_new_integer(int i)
{
    union {
	atom_t a;
	int i;
    } alias;

    alias.i = i;

    return alias.a;
}

int atom_get_integer_value(atom_t a)
{
    union {
	atom_t a;
	int i;
    } alias;

    alias.a = a;
    return alias.i;
}
