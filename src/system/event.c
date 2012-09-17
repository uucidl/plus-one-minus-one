/* a10 227
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/event.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 227 */




#include "event.h"

#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/stdio.h> // sprintf

#include <scripting/dictionary.h>
#include <library/strings.h>
#include <library/stack.h>

#include <scripting/compile.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_EVENT);

int event_new (event_t* e, unsigned int size)
{
  e->hierarchy = calloc(sizeof(bytecode_t), size);
  e->size      = size;

  return !!e->hierarchy;
}

int event_new_from_bytecode_stream (event_t* e, bytecode_stream_t* stream) {
    int result = event_new (e, stream->get_count (stream));
    
    if (result) {
	bytecode_iterator_t it;
	if (stream->get_iterator (stream, &it)) {
	    bytecode_t* b;
	    int i = 0;
	    while ((b = it.next (&it)) && i < e->size) {
		memcpy (e->hierarchy + i, b, sizeof (bytecode_t));
		i++;
	    }
	}
    }

    return result;
}

int event_new_from_cstring (event_t* e, const char* string) {
    bytecode_stream_t* s = compile_cstring (string, NULL);

    int result = s != NULL;

    if (result) {
	result = event_new_from_bytecode_stream (e, s);

	bytecode_stream_retire (s);
    }

    return result;
}

int event_new_from_string (event_t* e, const string_t* string) {
    bytecode_stream_t* s = compile_string (string, NULL);

    int result = s != NULL;

    if (result) {
	result = event_new_from_bytecode_stream (e, s);

	bytecode_stream_retire (s);
    }

    return result;
}

int event_equals(const event_t* a, const event_t* b)
{
    if(a == b) {
	return 1;
    } else {
	if(!a || !b)
	    return 0;
	else {
	    if(a->size != b->size) 
		return 0;
	    else {
		return memcmp(a->hierarchy, b->hierarchy, a->size * sizeof(bytecode_t)) == 0;
	    }
	}
    }
}

int event_match_p (const event_t* self, const event_t* event) {
    int match_p = 1;
    int i;
    dictionary_t* dict = dictionary_get_instance();

    for(i=0; i < self->size; i++) {
	bytecode_t* b = &self->hierarchy[i];

	/*
	  FIXME: this only works when the * is at the end of the signature
	 */
	if (b->adverb == dict->get_atom (dict, "atom") &&
	    b->verb   == dict->get_atom (dict, "*")) {
	    break;
	}

	if(event->size <= i) {
	    match_p = 0;
	    break;
	}

	if(b->adverb == dict->get_atom(dict, "atom") && 
	   b->verb == dict->get_atom(dict, "?")) {
	    continue;
	} else if(b->adverb != event->hierarchy[i].adverb || 
		  b->verb != event->hierarchy[i].verb) {
	    match_p = 0;
	    break;
	}
    }

    return match_p;
}

event_t* event_copy(const event_t* self, event_t* to)
{
    if (self == NULL)
	return NULL;
    else {
	event_t* other = event_instantiate_toplevel (to);
	
	other->size      = self->size;
	other->hierarchy = calloc(sizeof(bytecode_t), other->size);
	
	memcpy(other->hierarchy, self->hierarchy, sizeof(bytecode_t) * other->size);

	return other;
    }
}

char* event_as_cstring(const event_t* self)
{
    string_t s;
    char* result = "";
    int i;
    dictionary_t* dict = dictionary_get_instance();
    const atom_t atom_atom = dict->get_atom(dict, "atom");
    const atom_t integer_atom = dict->get_atom(dict, "integer");
    kn_stack_t to_free;

    kn_stack_instantiate_toplevel (&to_free);
    to_free.new(&to_free);

    string_instantiate_toplevel(&s);
    
    for(i=0; i<self->size; i++) {
	if(self->hierarchy[i].adverb == atom_atom) {
	    s.append(&s, atom_get_cstring_value(self->hierarchy[i].verb));
	} else if(self->hierarchy[i].adverb == integer_atom) {
	    const int value = self->hierarchy[i].verb;
	    int temp = value;
	    int i = 2;
	    char* buffer;

	    while(temp >>= 4) i++;
	    
	    buffer = calloc(1, i);
	    sprintf(buffer, "%x", value);
	    s.append(&s, buffer);
	    to_free.push(&to_free, buffer);
	} else {
	    s.append(&s, "?");
	}
	if(i < self->size - 1)
	    s.append(&s, "-");
    }

    result = s.new_c_str(&s);

    s.destroy(&s);
    object_retire(string_to_object(&s));

    {
	void* ptr;
	while( (ptr = to_free.pop(&to_free)) ) {
	    free(ptr);
	}
    }

    to_free.destroy(&to_free);
    object_retire(kn_stack_to_object(&to_free));

    return result;
}

int event_destroy(event_t* e)
{
    e->size = 0;

    if(e->hierarchy) {
	free(e->hierarchy);
	e->hierarchy = NULL;
    }
    
    return 1;
}

static 
int event_release (object_t* o) {
	event_t* e = (event_t*) o;
	return event_destroy (e);
}

event_t* event_instantiate (event_t* x) {
	event_t* e = event_instantiate_super (x);

	object_set_release_callback (event_to_object (e), event_release);

	return e;
}

key_event_t* key_event_instantiate(key_event_t* x)
{
    dictionary_t* dict = dictionary_get_instance();
    key_event_t* ke = OBJECT_INSTANTIATE (key_event, x);
    event_instantiate (ke);

    /* install new atoms */
    dict->new_atom(dict, "key");
    dict->new_atom(dict, "up");
    dict->new_atom(dict, "down");
    /* must install key symbols too here ... */

    ke->size      = 4;
    ke->hierarchy = calloc(sizeof(bytecode_t), ke->size);

    ke->hierarchy[0].adverb = dict->get_atom(dict, "atom");
    ke->hierarchy[0].verb   = dict->get_atom(dict, "key");
    
    return ke;
}

mouse_event_t* mouse_event_instantiate(mouse_event_t* x)
{
    dictionary_t* dict = dictionary_get_instance();
    mouse_event_t* me = OBJECT_INSTANTIATE(mouse_event, x);
    event_instantiate (me);
    
    /* installs mouse event atoms */
    dict->new_atom(dict, "mouse");
    dict->new_atom(dict, "move");
    dict->new_atom(dict, "button");
    dict->new_atom(dict, "up");
    dict->new_atom(dict, "down");
    dict->new_atom(dict, "lmb");
    dict->new_atom(dict, "mmb");
    dict->new_atom(dict, "rmb");

    me->hierarchy = calloc(sizeof(bytecode_t), 4);
    me->size      = 4;

    return me;
}

mouse_event_t* mouse_move_event_instantiate(mouse_event_t* x)
{
    mouse_event_t* me = mouse_event_instantiate(x);

    dictionary_t* dict = dictionary_get_instance();
    
    me->hierarchy[0].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[0].verb   = dict->get_atom(dict, "mouse");
    me->hierarchy[1].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[1].verb   = dict->get_atom(dict, "move");
    
    return me;
}

mouse_event_t* mouse_button_event_instantiate(mouse_event_t* x)
{
    mouse_event_t* me = mouse_event_instantiate(x);

    dictionary_t* dict = dictionary_get_instance();
    
    me->hierarchy[0].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[0].verb   = dict->get_atom(dict, "mouse");
    me->hierarchy[1].adverb = dict->get_atom(dict, "atom");
    me->hierarchy[1].verb   = dict->get_atom(dict, "button");
    
    return me;
}

drop_event_t* drop_event_instantiate(drop_event_t* x)
{
  dictionary_t* dict = dictionary_get_instance();
  drop_event_t* de = OBJECT_INSTANTIATE (drop_event, x);
  event_instantiate (de);

  de->hierarchy = calloc(sizeof(bytecode_t), 3);
  de->size      = 3;

  de->hierarchy[0].adverb = dict->new_atom(dict, "atom");
  de->hierarchy[0].verb   = dict->new_atom(dict, "drop");
  de->hierarchy[1].adverb = dict->get_atom(dict, "atom");
  de->hierarchy[1].verb   = dict->new_atom(dict, "file");
  de->hierarchy[2].adverb = dict->new_atom(dict, "string");

  return de;
}

static
int signature_match_event_p(signature_t* self, const event_t* event)
{
    if(!self->super.size)
	DEBUG1("empty signature");

    return event_match_p (&self->super, event);
}

signature_t* signature_instantiate(signature_t* x)
{
    dictionary_t* dict = dictionary_get_instance();

    signature_t* s = signature_instantiate_super (x);

    s->match_event_p = signature_match_event_p;
    
    dict->new_atom(dict, "*");
    dict->new_atom(dict, "?");

    return s;
}
