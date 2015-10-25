/* a10 241
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/definitions.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 241 */

#include <messaging/definitions.h>

#include <libc/stdlib.h>

#include <library/map_impl.h>

static desc_t *add_desc(definitions_t *self)
{
    return calloc(sizeof(desc_t), 1);
}

static desc_t *definitions_get_desc(definitions_t *self, atom_t a)
{
    map_value_t pdesc = map_get(&self->definitions, atom_get_id(a));
    desc_t *desc = map_value_is_there(pdesc) ? map_value_obtain(pdesc) : NULL;

    return desc;
}

/*
  only handles the number of arguments, but not the types
*/
static void definitions_set_callback(definitions_t *self, atom_t a,
                                     bytecode_stream_t *args, recp_f f)
{
    desc_t *desc = self->get_desc(self, a);

    if (!desc) {
        desc = add_desc(self);
        map_put(&self->definitions, atom_get_id(a), desc);
    }

    desc->atom = a;
    desc->arg_number = args ? args->get_count(args) : 0;
    desc->function = f;
}

definitions_t *definitions_instantiate(definitions_t *x)
{
    definitions_t *d = definitions_instantiate_super(x);

    map_instantiate_toplevel(&d->definitions);

    d->set_callback = definitions_set_callback;
    d->get_desc = definitions_get_desc;

    return d;
}
