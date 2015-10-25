/* a10 283
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/bytecode_stream.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 283 */

#include "bytecode_stream.h"

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_SCRIPTING_BYTECODE_STREAM);

#include "dictionary.h"
#include "atom.h"

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <library/memory_impl.h>

bytecode_node_t *bytecode_node_instantiate(bytecode_node_t *x)
{
    return bytecode_node_instantiate_super(x);
}

static bytecode_t *i_next(bytecode_iterator_t *self)
{
    bytecode_t *b = NULL;

    if (self->current) {
        b = memory_allocate(sizeof(bytecode_t));
        memcpy(b, &self->current->code, sizeof(bytecode_t));

        if (self->current != self->parent->end) {
            self->current = self->current->next;
        } else {
            self->current = NULL;
        }
    }

    return b;
}

static bytecode_t *i_prev(bytecode_iterator_t *self)
{
    bytecode_t *b = NULL;

    if (self->current) {
        b = memory_allocate(sizeof(bytecode_t));
        memcpy(b, &self->current->code, sizeof(bytecode_t));

        if (self->current != self->parent->end->next) {
            self->current = self->current->prev;
        } else {
            self->current = NULL;
        }
    }

    return b;
}

bytecode_iterator_t *bytecode_iterator_instantiate(bytecode_iterator_t *x)
{
    bytecode_iterator_t *i = bytecode_iterator_instantiate_super(x);

    i->next = i_next;
    i->prev = i_prev;

    return i;
}

int bytecode_stream_new(struct bytecode_stream_t *self)
{
    self->end = NULL;

    return 1;
}

static int bytecode_stream_release(object_t *zelf)
{
    bytecode_stream_t *self = (bytecode_stream_t *)zelf;
    bytecode_node_t *to_free;
    bytecode_node_t *current;
    int ret_p;

    if (!self->end) {
        ret_p = 1;
    } else {
        to_free = self->end;
        current = to_free->next;

        do {
            to_free = current;
            current = current->next;
            object_retire(&to_free->super);
        } while (to_free != self->end);

        self->end = NULL;
        ret_p = 1;
    }

    return ret_p;
}

static int bytecode_stream_destroy(struct bytecode_stream_t *self)
{
    ERROR3("DEPRECATED, don't call this method anymore. caller is: %x - %x",
           __builtin_return_address(0), __builtin_return_address(1));
    return bytecode_stream_release(bytecode_stream_to_object(self));
}

static bytecode_stream_t *bytecode_stream_copy(bytecode_stream_t *self,
                                               bytecode_stream_t *src)
{
    object_t *zelf = bytecode_stream_to_object(self);

    if (!zelf->release(zelf))
        return NULL;
    else {
        bytecode_iterator_t it;

        if (src->get_iterator(src, &it)) {
            bytecode_t *code;

            while ((code = it.next(&it))) {
                self->push(self, code);
            }
        }

        return self;
    }
}

static unsigned int bytecode_stream_get_count(bytecode_stream_t *self)
{
    bytecode_iterator_t iterator;
    unsigned int i = 0;

    self->get_iterator(self, &iterator);

    while (iterator.next(&iterator) != NULL) {
        i++;
    }

    object_retire(bytecode_iterator_to_object(&iterator));

    return i;
}

static bytecode_stream_t *bytecode_stream_append_node(bytecode_stream_t *self,
                                                      bytecode_node_t *node)
{
    bytecode_node_t *end = self->end;

    if (node == NULL) {
        WARNING1("tried to append null node.");
        return self;
    }

    if (end == NULL) {
        node->next = node;
        node->prev = node;
        self->end = node;
    } else {
        node->prev = end;
        node->next = end->next;
        end->next->prev = node;
        end->next = node;
        self->end = node;
    }

    return self;
}

static bytecode_stream_t *bytecode_stream_prepend_node(bytecode_stream_t *self,
                                                       bytecode_node_t *node)
{
    bytecode_node_t *end = self->end;

    if (node == NULL) {
        WARNING1("tried to append null node.");
        return self;
    }

    if (end == NULL) {
        node->next = node;
        node->prev = node;
        self->end = node;
    } else {
        node->prev = end;
        node->next = end->next;
        end->next->prev = node;
        end->next = node;
    }

    return self;
}

static bytecode_iterator_t *
bytecode_stream_get_iterator(bytecode_stream_t *self, bytecode_iterator_t *x)
{
    bytecode_iterator_t *i = NULL;

    i = bytecode_iterator_instantiate_toplevel(x);

    i->parent = self;
    if (self->end)
        i->current = self->end->next; // position to first one
    else
        i->current = NULL;

    return i;
}

static bytecode_iterator_t *
bytecode_stream_get_iterator_end(bytecode_stream_t *self,
                                 bytecode_iterator_t *x)
{
    bytecode_iterator_t *i = self->get_iterator(self, x);

    if (i)
        i->current = self->end;

    return i;
}

static bytecode_node_t *allocate_atom_node(atom_t a)
{
    bytecode_node_t *node = bytecode_node_instantiate_toplevel(NULL);
    dictionary_t *dict = dictionary_get_instance();

    node->code.verb = a;
    node->code.adverb = dict->get_atom(dict, "atom");

    return node;
}

static bytecode_stream_t *bytecode_stream_append_atom(bytecode_stream_t *self,
                                                      atom_t a)
{
    return self->append(self, allocate_atom_node(a));
}

static bytecode_stream_t *bytecode_stream_prepend_atom(bytecode_stream_t *self,
                                                       atom_t a)
{
    return self->prepend(self, allocate_atom_node(a));
}

static bytecode_node_t *allocate_integer_node(int i)
{
    bytecode_node_t *node = bytecode_node_instantiate_toplevel(NULL);
    dictionary_t *dict = dictionary_get_instance();

    node->code.verb = atom_new_integer(i);
    node->code.adverb = dict->get_atom(dict, "integer");

    return node;
}

static bytecode_stream_t *
bytecode_stream_append_integer(bytecode_stream_t *self, int i)
{
    return self->append(self, allocate_integer_node(i));
}

static bytecode_stream_t *
bytecode_stream_prepend_integer(bytecode_stream_t *self, int i)
{
    return self->prepend(self, allocate_integer_node(i));
}

static bytecode_node_t *allocate_float_node(float f)
{
    bytecode_node_t *node = bytecode_node_instantiate_toplevel(NULL);
    dictionary_t *dict = dictionary_get_instance();

    node->code.verb = atom_new_integer(f);
    node->code.adverb = dict->get_atom(dict, "float");

    return node;
}

bytecode_stream_t *bytecode_stream_append_float(bytecode_stream_t *self,
                                                float f)
{
    return self->append(self, allocate_float_node(f));
}

bytecode_stream_t *bytecode_stream_prepend_float(bytecode_stream_t *self,
                                                 float f)
{
    return self->prepend(self, allocate_float_node(f));
}

static bytecode_t *bytecode_stream_pop(bytecode_stream_t *self)
{
    bytecode_node_t *node = self->end;

    if (node != NULL) {
        bytecode_node_t *first = node->next;
        bytecode_node_t *end_1 = node->prev;
        bytecode_t *code = memory_allocate(sizeof(bytecode_t));

        if (first == self->end || end_1 == self->end) {
            self->end = NULL;
        } else {
            first->prev = end_1;
            end_1->next = first;

            self->end = end_1;
        }

        memcpy(code, &node->code, sizeof(bytecode_t));
        object_retire(&node->super);

        return code;
    } else {
        return NULL;
    }
}

static void bytecode_stream_push(bytecode_stream_t *self, bytecode_t *code)
{
    bytecode_node_t *node = bytecode_node_instantiate_toplevel(NULL);

    memcpy(&node->code, code, sizeof(bytecode_t));
    memory_free(code);

    self->append(self, node);
}

bytecode_stream_t *bytecode_stream_instantiate(bytecode_stream_t *x)
{
    bytecode_stream_t *s = bytecode_stream_instantiate_super(x);

    object_set_release_callback(bytecode_stream_to_object(s),
                                bytecode_stream_release);

    s->new = bytecode_stream_new;
    s->destroy = bytecode_stream_destroy;
    s->copy = bytecode_stream_copy;

    s->get_count = bytecode_stream_get_count;

    s->append = bytecode_stream_append_node;
    s->append_atom = bytecode_stream_append_atom;
    s->append_integer = bytecode_stream_append_integer;
    s->append_float = bytecode_stream_append_float;

    s->prepend = bytecode_stream_prepend_node;
    s->prepend_atom = bytecode_stream_prepend_atom;
    s->prepend_integer = bytecode_stream_prepend_integer;
    s->prepend_float = bytecode_stream_prepend_float;

    s->push = bytecode_stream_push;
    s->pop = bytecode_stream_pop;

    s->get_iterator = bytecode_stream_get_iterator;
    s->get_iterator_end = bytecode_stream_get_iterator_end;

    return s;
}
