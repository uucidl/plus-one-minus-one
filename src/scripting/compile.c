/* a10 283
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/scripting/compile.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 283 */

#include "compile.h"

#include "dictionary.h"

#include "bytecode_stream.h"

#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/stdio.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_SCRIPTING_COMPILE);

#include <library/memory_impl.h>

static atom_t atom_atom = 0;
static atom_t integer_atom = 0;
static atom_t float_atom = 0;
static atom_t route_atom = 0;

/* parse an <adverb:verb> into a bytecode_t */
static bytecode_t *parse_definition(const string_t *string, bytecode_t *code)
{
    bytecode_t *b = code;
    dictionary_t *dict = dictionary_get_instance();
    bytecode_t *ret = NULL;

    /* find middle ':' and form bytecodes */
    string_iterator_t it;
    string_t temp;

    if (string->clone_to(string, &temp) && temp.get_iterator(&temp, &it)) {
        string_t *adverb = NULL;
        string_t *verb = NULL;
        const char *c;

        while ((c = it.next(&it)) && *c != ':') {
            /* do nothing */
        }

        if (c) {
            string_iterator_t it2;

            it.prev(&it); /* back to the colon */

            adverb = &temp;
            verb = it.split(&it, NULL);

            /*  delete ':' */
            if (verb->get_iterator(verb, &it2)) {
                it2.delete(&it2, 1);
            }
            TRACE("subs: '%s' '%s'", adverb->new_c_str(adverb),
                   verb->new_c_str(verb));
        } else {
            verb = &temp;
        }

        /* now form bytecode */
        if (adverb) {
            char *s = adverb->new_c_str(adverb);
            if (s) {
                b->adverb = dict->get_atom(dict, s);
                if (0 == b->adverb) {
                    b->adverb = dict->new_atom(dict, strdup(s));
                }
                free(s);
            }
            adverb->destroy(adverb);
        } else {
            b->adverb = atom_atom;
        }

        if (verb) {
            char *s = verb->new_c_str(verb);
            if (s && strlen(s)) {
                if (b->adverb == atom_atom) {
                    b->verb = dict->get_atom(dict, s);
                    if (0 == b->verb) {
                        b->verb = dict->new_atom(dict, strdup(s));
                    }
                    TRACE("atom: '%s'", atom_get_cstring_value(b->verb));
                } else if (b->adverb == integer_atom) {
                    int i = 0;
                    i = atoi(s);
                    TRACE("integer: '%d' (%s)", i, s);
                    b->verb = atom_new_integer(i);
                } else if (b->adverb == float_atom) {
                    float f;
                    f = atof(s);
                    TRACE("float: '%f' (%s)", f, s);
                    b->verb = atom_new_float(f);
                } else {
                    DEBUG("undefined adverb for '%s'", s);
                }
                ret = code;
            }
            if (s) {
                free(s);
            }
            verb->destroy(verb);
        }
    }

    return ret;
}

bytecode_stream_t *compile_string(const string_t *string, bytecode_stream_t *x)
{
    dictionary_t *dict = dictionary_get_instance();
    bytecode_stream_t *stream = NULL;
    substring_generator_t gen;

    if (!atom_atom)
        atom_atom = dict->new_atom(dict, "atom");
    if (!integer_atom)
        integer_atom = dict->new_atom(dict, "integer");
    if (!float_atom)
        float_atom = dict->new_atom(dict, "float");

    if (string->get_substring_generator(string, ' ', &gen)) {
        string_t *bytecode_s;
        stream = bytecode_stream_instantiate(x);

        while ((bytecode_s = gen.next(&gen))) {
            /* delete leading spaces */
            {
                string_iterator_t it;
                unsigned int n = 0;
                const char *c;

                if (bytecode_s->get_iterator(bytecode_s, &it)) {
                    while ((c = it.next(&it)) && *c == ' ') {
                        n++;
                    }
                    if (n > 0) {
                        /* now rewind */
                        bytecode_s->get_iterator(bytecode_s, &it);
                        it.delete(&it, n);
                    }

                    {
                        bytecode_node_t *node =
                            bytecode_node_instantiate_toplevel(NULL);
                        if (parse_definition(bytecode_s, &node->code)) {
                            stream->append(stream, node);
                        } else {
                            object_retire(bytecode_node_to_object(node));
                        }
                    }
                }
            }

            free(bytecode_s);
        }
    }

    return stream;
}

bytecode_stream_t *compile_cstring(const char *str, bytecode_stream_t *x)
{
    string_t string;
    bytecode_stream_t *ret;

    string_instantiate_toplevel(&string);
    string.new(&string, str);

    ret = compile_string(&string, x);

    string_retire(&string);

    return ret;
}

bytecode_stream_t *compile_osc_string(const string_t *osc_string,
                                      bytecode_stream_t *x)
{
    dictionary_t *dict = dictionary_get_instance();
    bytecode_stream_t *stream = NULL;
    substring_generator_t gen;

    if (!atom_atom)
        atom_atom = dict->new_atom(dict, "atom");
    if (!integer_atom)
        integer_atom = dict->new_atom(dict, "integer");
    if (!float_atom)
        float_atom = dict->new_atom(dict, "float");
    if (!route_atom)
        route_atom = dict->new_atom(dict, "route");

    if (osc_string->get_substring_generator(osc_string, ' ', &gen)) {
        string_t *s;

        stream = bytecode_stream_instantiate_toplevel(x);

        /* process address */
        if ((s = gen.next(&gen))) {
            substring_generator_t route_gen;
            if (s->get_substring_generator(s, '/', &route_gen)) {
                string_t *adr;
                while ((adr = route_gen.next(&route_gen))) {
                    char *cs = adr->new_c_str(adr);
                    atom_t atom = dict->get_atom(dict, cs);
                    if (0 == atom) {
                        atom = dict->new_atom(dict, strdup(cs));
                    }
                    TRACE("prepending atom '%s'",
                           atom_get_cstring_value(atom));
                    stream->prepend_atom(stream, route_atom);
                    stream->prepend_atom(stream, atom);
                    free(cs);
                }
            }
        }

        /* process arguments */
        while ((s = gen.next(&gen))) {
            bytecode_node_t *node = bytecode_node_instantiate_toplevel(NULL);
            parse_definition(s, &node->code);
            stream->prepend(stream, node);
        }
    }

    return stream;
}
