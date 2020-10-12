/* a10 561
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/class.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 561 */

#ifndef KNOS_MESSAGING_CLASS_H
#define KNOS_MESSAGING_CLASS_H

#include <scripting/atom.h>

/*
  a class is an entity regrouping objects of similar kinds.
*/
typedef struct class_t {
    atom_t class;     /* this class' atom  */
    unsigned int next_instance; /* serial number -used to generate atoms */
} class_t;

class_t *init_class(const char *name);

#endif
