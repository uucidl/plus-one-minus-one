/* a10 831
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/router.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 831 */




#include <messaging/router.h>

#include <log4c.h>
LOG_NEW_DEFAULT_SUBCATEGORY(ROUTER, KNOS_MESSAGING_RECEIVER);

#include <scripting/dictionary.h>

#include <libc/stdlib.h>
#include <libc/stdio.h>

#include <library/map_impl.h>

static atom_t route_atom = 0;
static atom_t star_atom = 0;

static
router_slot_t* get_slot(router_t* self, atom_t a)
{
    map_value_t pslot = map_get(&self->children, (unsigned long) a);
    router_slot_t* slot;

    slot = map_value_is_there (pslot) ? map_value_obtain (pslot) : NULL;

    return slot;
}

static
void router_set_child(router_t* self, atom_t name, receiver_t* child)
{
    router_slot_t* slot = get_slot(self, name);

    if(child == &self->super) {
	/* detect stupid errors, but not complicated loops */
	WARNING1("what the fuck are you trying to do.");
    }

    if(!slot) {
	slot = calloc(sizeof(router_slot_t), 1);
	map_put (&self->children, (unsigned long) name, slot);
	TRACE2("added slot for selector: '%s'", atom_get_cstring_value(name));
    }

    slot->name = name;
    slot->receiver = child;
}

static
int router_route(router_slot_t* slot, bytecode_stream_t* message, context_t* context)
{
    if(slot) {
	slot->receiver->receive(slot->receiver, message, context);
	return 1;
    } else {
	return 0;
    }
}

static
void router_receive(receiver_t* self,
		    bytecode_stream_t* message, context_t* context)
{
    router_t* router = (router_t*) self;
    bytecode_t* selector = message->pop(message);

    if(selector && selector->verb == route_atom) {
	bytecode_t* destination = message->pop(message);

	if(!destination) {
	    WARNING1("no destination provided.");
	    return;
	} else {
	    atom_t dest = destination->verb;
	    if(!router_route(get_slot(router, dest), message, context)) {
		WARNING2("route '%s' not defined.", 
			 atom_get_cstring_value(dest));
	    }
	}
    } else {
	message->push(message, selector);
	router->receive_backup(self, message, context);
    }
}

static
void wildcard_router_receive(receiver_t* self,
			     bytecode_stream_t* message, context_t* context)
{
    router_t* router = (router_t*) self;
    bytecode_t* selector = message->pop(message);

    if(selector && selector->verb == route_atom) {
	bytecode_t* destination = message->pop(message);

	if(!destination) {
	    WARNING1("no destination provided.");
	    return;
	} else {
	    atom_t dest = destination->verb;

	    TRACE2("routing towards selector '%s' requested.", atom_get_cstring_value(dest));
	    if(dest == star_atom) {
		map_iterator_t i; 
		map_value_t pslot = NULL_MAP_VALUE;
		
		map_get_iterator(&router->children, &i);
		
		while (map_value_is_there (pslot = map_iterator_next(&i))) {
		    router_slot_t* slot = map_value_obtain (pslot);
		    // clone each message
		    bytecode_stream_t* message_copy = bytecode_stream_instantiate_toplevel(NULL);
		    message_copy->copy(message_copy, message);

		    TRACE2("sending event to slot: '%s'", 
			   atom_get_cstring_value(slot->name));
		    if(!router_route(slot, message_copy, context)) {
			WARNING2("route '%s' not defined.", 
				 atom_get_cstring_value(slot->name));
		    }
		}
		map_iterator_destroy (&i);
		map_iterator_retire (&i);

		bytecode_stream_retire (message);
	    } else {
		if(!router_route(get_slot(router, dest), message, context)) {
		    WARNING2("route '%s' not defined.", 
			     atom_get_cstring_value(dest));
		}
	    }
	}
    } else {
	message->push(message, selector);
	router->receive_backup(self, message, context);
    }
}

router_t* router_instantiate(router_t* x)
{
    router_t* r = router_instantiate_super (x);

    /* uses the 'self' variant of receivers */
    self_receiver_instantiate(&r->super);

    map_instantiate_toplevel(&r->children);

    r->set_child = router_set_child;
    r->receive_backup = r->super.receive;
    r->super.receive = router_receive;

    {
	dictionary_t* dict = dictionary_get_instance();
	
	route_atom = dict->new_atom(dict, "route");
    }
    
    return r;
}

router_t* wildcard_router_instantiate(router_t* x)
{
    router_t* r = router_instantiate(x);

    r->super.receive = wildcard_router_receive;
    
    if(!star_atom) {
	dictionary_t* dict = dictionary_get_instance();
	star_atom = dict->new_atom(dict, "*");
    }

    return r;
}
