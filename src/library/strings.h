/* a10 456
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/strings.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 456 */

#ifndef KNOS_LIBRARY_STRINGS_H
#define KNOS_LIBRARY_STRINGS_H

#include <library/memory.h>

/*
  a string_node_t defines a circular linked list of nodes containing chars
*/

typedef struct string_node_t {
    const char *data;  /* allocated area for the chars */
    unsigned int head; /* index of first character, data+head: real string */
    unsigned int n;    /* number of chars */

    struct string_node_t *next; /* next node     in string */
    struct string_node_t *prev; /* previous node in string */
} string_node_t;

typedef struct string_iterator_t {
    /* iteration */
    const char *(*next)(struct string_iterator_t *self);
    const char *(*prev)(struct string_iterator_t *self);

    /* modifications */
    /* delete current n chars. state of iterator undefined after delete
     * operation */
    void (*delete)(struct string_iterator_t *self, unsigned int n);
    /* truncate original string at point, and returns as x the right-end side */
    struct string_t *(*split)(struct string_iterator_t *self,
                              struct string_t *x);
    /* truncate string at point */
    void (*truncate)(struct string_iterator_t *self);
    /* insert a string at point */
    void (*insert)(struct string_iterator_t *self, const char *str);
    /* clone string following point */
    struct string_t *(*clone_to)(struct string_iterator_t *self,
                                 struct string_t *to);

    struct string_t *parent;
    string_node_t *node; /* current node */
    unsigned int index;  /* index in node */
} string_iterator_t;

typedef struct substring_generator_t {
    struct string_t *(*next)(struct substring_generator_t *self);

    const struct string_t *parent;
    string_node_t *node; /* current node */
    unsigned int index;  /* index in node */

    /* partition_generator_t */
    char delimiter;
} substring_generator_t;

/*
  a string is the last element of a circular list of
  string_node_t
*/

typedef struct string_t {
    object_t super;
    string_node_t *end_node;

    int (*new)(struct string_t *self, const char *src);
    /* deallocate all the nodes constituting this string */
    int (*destroy)(struct string_t *self);
    struct string_t *(*clone_to)(const struct string_t *self,
                                 struct string_t *to);
    string_iterator_t *(*get_iterator)(struct string_t *self,
                                       string_iterator_t *x);
    unsigned int (*get_length)(struct string_t *self);

    /* allocate a c string representing this string */
    char *(*new_c_str)(struct string_t *self);
    /* partition string in substrings separated by delimiter d */
    substring_generator_t *(*get_substring_generator)(
        const struct string_t *self, char d, substring_generator_t *x);

    /* append */
    void (*append)(struct string_t *self, const char *str);
    void (*prepend)(struct string_t *self, const char *str);
} string_t;

CLASS_INHERIT(string, object);

#endif
