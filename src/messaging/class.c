/* a10 308
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/class.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 308 */

#include <messaging/class.h>

#include <library/map_impl.h>
#include <messaging/router.h>
#include <scripting/dictionary.h>

#include <system/demo.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_MESSAGING_CLASS);

static atom_t class_attach_instance(class_t *self, receiver_t *receiver)
{
    atom_t id = 0;

    if (receiver) {
        unsigned int i = self->next_instance++;
        dictionary_t *dict = dictionary_get_instance();

        id = dict->new_atom_from_integer(dict, i);

        self->router->set_child(self->router, id, receiver);
        TRACE3("added instance '%s' of class '%s'", atom_get_cstring_value(id),
               atom_get_cstring_value(self->class));
    }

    return id;
}

static map_t classes; /* where the class_t are stored */
static int init_p = 1;

class_t *init_class(const char *name)
{
    class_t *class = NULL;
    dictionary_t *dict = dictionary_get_instance();

    if (name) {
        atom_t class_atom = 0;

        if (init_p) {
            map_instantiate_toplevel(&classes);
            init_p = 0;
        }

        class_atom = dict->new_atom(dict, name);

        /* let's see if the class already exists */
        {
            map_value_t cp = map_get(&classes, (unsigned long)class_atom);
            class = map_value_is_there(cp) ? map_value_obtain(cp) : NULL;
        }
        if (!class) {
            class = calloc(sizeof(class_t), 1);
            class->class = class_atom;
            class->router = wildcard_router_instantiate_toplevel(NULL);
            class->attach_instance = class_attach_instance;
            class->next_instance = 0;
            map_put(&classes, (unsigned long)class_atom, class);

            /* attach to demo */
            demo_get_instance()->router.set_child(&demo_get_instance()->router,
                                                  class->class,
                                                  &class->router->super);
        }
    }

    return class;
}
