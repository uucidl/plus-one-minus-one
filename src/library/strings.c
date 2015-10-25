/* a10 179
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/strings.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 179 */

#include <library/strings.h>

#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_STRINGS);

/*
  implementation of strings using a linked list.

  should probably use a pool for allocating and freeing nodes instead of
  plain malloc/calloc.

  strings only support const char* atm to avoid c string allocation/deallocation
  issues. this will have to be tackled later, either by reference counting, real
  gc or an adhoc scheme.
*/

#include <libc/strings.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>

static int string_new(string_t *self, const char *src)
{
    self->end_node = calloc(sizeof(string_node_t), 1);

    if (!src)
        return 0;

    self->end_node->data = src;
    self->end_node->head = 0;
    self->end_node->n = strlen(src);
    self->end_node->next = self->end_node;
    self->end_node->prev = self->end_node;

    return 1;
}

static int string_destroy(string_t *self)
{
    if (self->end_node) {
        string_node_t *first = self->end_node->next;
        string_node_t *node = first;
        do {
            string_node_t *next = node->next;
            free(node);
            node = next;
        } while (node != first);
        self->end_node = NULL;
    }

    return 1;
}

static int string_release(object_t *zelf)
{
    string_t *self = (string_t *)zelf;
    return string_destroy(self);
}

string_t *string_clone_to(const string_t *self, string_t *dest)
{
    string_t *to = NULL;
    string_iterator_t it;

    if (self->get_iterator((string_t *)self, &it)) {
        to = it.clone_to(&it, dest);
    }

    return to;
}

static char *string_new_c_str(string_t *self)
{
    char *ret = NULL;
    /* first determine the length */
    unsigned int len = self->get_length(self);

    ret = calloc(1, len + 1);

    /* fill the c string */
    if (len) {
        string_iterator_t *it = self->get_iterator(self, NULL);
        unsigned int i = 0;
        const char *c;
        while ((c = it->next(it)) && i < len) {
            ret[i++] = *c;
        }
        free(it);
    }

    return ret;
}

static unsigned int string_get_length(string_t *self)
{
    string_node_t *first;
    string_node_t *node;
    unsigned int n = 0;

    if (self->end_node) {
        first = self->end_node->next;
        node = first;
        do {
            if (!node) {
                n = 0;
                break;
            }

            n += node->n;
            node = node->next;
        } while (node != first);
    }

    return n;
}

static void string_node_prepend(string_t *self, string_node_t *node)
{
    string_node_t *first;
    if (self->end_node == NULL) {
        first = node;
        self->end_node = node;
    } else {
        first = self->end_node->next;
        self->end_node->next = node;
    }
    node->prev = self->end_node;
    node->next = first;
}

static void string_node_append(string_t *self, string_node_t *node)
{
    string_node_t *first;
    if (self->end_node == NULL) {
        first = node;
        node->prev = node;
    } else {
        first = self->end_node->next;
        self->end_node->next = node;
        node->prev = self->end_node;
    }
    node->next = first;
    self->end_node = node;
}

static void string_prepend(string_t *self, const char *str)
{
    string_node_t *node = calloc(sizeof(string_node_t), 1);
    node->data = str;
    node->head = 0;
    node->n = strlen(str);

    string_node_prepend(self, node);
}

static void string_append(string_t *self, const char *str)
{
    string_node_t *node = calloc(sizeof(string_node_t), 1);
    node->data = str;
    node->head = 0;
    node->n = strlen(str);

    string_node_append(self, node);
}

static const char *iterator_next_character(string_iterator_t *self)
{
    const char *c;

    if (self->index >= self->node->n) {
        /* need to change nodes */
        if (self->node == self->parent->end_node) {
            /* this was the last one */
            return NULL;
        } else {
            self->node = self->node->next;
            self->index = 0;
            if (!self->node->n)
                DEBUG1("node length was zero.");
        }
    }

    c = self->node->data + self->node->head + self->index;
    self->index++;

    return c;
}

static const char *iterator_previous_character(string_iterator_t *self)
{
    const char *c;

    if (self->index == 0) {
        /* need to change nodes */
        if (self->node == self->parent->end_node->next) {
            /* this was the first one */
            return NULL;
        } else {
            self->node = self->node->prev;
            if (!self->node->n)
                DEBUG1("node length was zero.");
            else
                self->index = self->node->n - 1;
        }
    }

    c = self->node->data + self->node->head + self->index;
    self->index--;

    return c;
}

/*
  deletion means creating 2 nodes out of one to jump
  over the deleted area. of course one should be able
  to handle a total deletion of one node.
*/
static void iterator_delete(string_iterator_t *self, unsigned int n)
{
    string_node_t temp = *self->node; // a temp copy
    unsigned int remaining = temp.n - self->index;

    if (n > 0) {
        if (n < remaining) {
            /* intra node cut.. means we need to create an additional node */
            temp.head += self->index + n;
            temp.n = remaining - n;
            if (!self->index) {
                // this node can be trashed, since we are cutting at point 0
                *self->node = temp;
                self->index = 0; /* useless but more clear (moved to new first
                                    position) */
            } else {
                string_node_t *new_node = calloc(1, sizeof(string_node_t));
                self->node->n = self->index;
                *new_node = temp;
                self->node->next = new_node;
                new_node->prev = self->node;

                /* now jump after the deletion point */
                self->index = 0;
                self->node = new_node;
            }
        } else {
            /* must cut this node and use-up the next  */
            if (!self->index) {
                /* the whole node is destroyed,
                   we must find the previous one */
                string_node_t *to_destroy = self->node;
                string_node_t *pred = self->node->prev;
                if (pred == self->parent->end_node)
                    self->parent->end_node = self->node->next;
                pred->next = self->node->next;
                self->node->next->prev = pred;
                free(to_destroy);
            } else {
                temp.n = temp.head + self->index;
                *self->node = temp;
                n -= remaining;
                self->node = self->node->next;
                self->index = 0;
                if (n > 0 && self->node) {
                    self->delete (self, n);
                }
            }
        }
    } else {
        WARNING1("tried to delete 0 char.");
    }
}

static string_t *iterator_split(string_iterator_t *self, string_t *x)
{
    string_node_t *first = self->parent->end_node->next;
    string_node_t *new_end_node = NULL;
    string_node_t *first_node_after_point = NULL;

    /* right-end part of the string */
    string_t *remaining = string_instantiate_toplevel(x);

    /* node truncation */
    if (!self->index) { /* beginning of node */
        string_node_t *prev = self->node->prev;
        first_node_after_point = self->node;

        if (prev == NULL)
            WARNING1("couldn't find previous node. data structure broken?");
        else {
            new_end_node = prev;
            if (new_end_node == self->node) {
                new_end_node = NULL;
            }
        }
    } else {
        /* allocate a new node to cut this node in 2 parts if cut is inside */
        if (self->index < self->node->n - 1) {
            first_node_after_point = calloc(sizeof(string_node_t), 1);
            *first_node_after_point = *self->node;
            first_node_after_point->head += self->index;
            first_node_after_point->n -= self->index;
        } else {
            if (self->node->next != self->node)
                first_node_after_point = self->node->next;
            else
                first_node_after_point = NULL;
        }
        self->node->n = self->index;
        new_end_node = self->node;
    }

    /* create 'remaining' string */
    {
        string_node_t *node = first_node_after_point;
        while (node) {
            string_node_t *next =
                (node == self->parent->end_node) ? NULL : node->next;
            if (next == new_end_node)
                next = NULL;

            TRACE3("appending: %s, [%d]", node->data + node->head, node->n);
            string_node_append(remaining, node);
            node = next;
        }
    }

    /* correct new end point of self */
    if (new_end_node) {
        new_end_node->next = first;
        first->prev = new_end_node;
    }
    self->parent->end_node = new_end_node;

    return remaining;
}

/* truncate at point */
static void iterator_truncate(string_iterator_t *self)
{
    string_t *remaining = self->split(self, NULL);

    remaining->destroy(remaining);
    free(remaining);
}

static void iterator_insert(string_iterator_t *self, const char *str)
{
    unsigned int len = 0;
    if (str && (len = strlen(str))) {
        string_node_t *new_node = calloc(sizeof(string_node_t), 1);
        new_node->data = str;
        new_node->head = 0;
        new_node->n = len;

        if (!self->index) {
            /* if insertion at beginning, find previous node */
            string_node_t *pred = self->node->prev;

            new_node->next = self->node;
            self->node->prev = new_node;

            if (pred->next == self->parent->end_node)
                self->parent->end_node = new_node;
            pred->next = new_node;
            new_node->prev = pred;

        } else {
            /* we're cutting the current node in the middle, and creating
               a followup one */
            if (self->index == self->node->n) {
                /* actually it's really the end of the node, so just an append
                   is needed */
                DEBUG1("self->index == self->node->n");
            } else {
                string_node_t *second_part = calloc(sizeof(string_node_t), 1);
                *second_part = *self->node;
                second_part->head += self->index;
                second_part->n -= self->index;
                new_node->next = second_part;
                second_part->prev = new_node;
                self->node->n = self->index;
                self->node->next = new_node;
                new_node->prev = self->node;
            }
        }
    } else {
        WARNING1("tried to insert null or empty string.");
    }
}

string_t *iterator_clone_to_after_point(string_iterator_t *self, string_t *dest)
{
    string_t *to = string_instantiate_toplevel(dest);

    {
        string_node_t *node = self->node; /* start node */
        string_node_t *to_node_first = calloc(sizeof(string_node_t), 1);
        string_node_t *to_node = to_node_first;

        do {
            if (!node) {
                ERROR1("Should not happen!");
                return NULL;
            }

            to_node->data = node->data;
            if (node == self->node) {
                to_node->head = node->head + self->index;
                to_node->n = node->n - self->index;
            } else {
                to_node->head = node->head;
                to_node->n = node->n;
            }
            node = node->next;
            if (node != self->node) {
                to_node->next = calloc(sizeof(string_node_t), 1);
                to_node->next->prev = to_node;
                to_node = to_node->next;
            } else {
                to_node->next = to_node_first;
                to_node_first->prev = to_node;
                to->end_node = to_node;
            }
        } while (node != self->node);
    }

    return to;
}

static string_iterator_t *string_get_iterator(string_t *self,
                                              string_iterator_t *x)
{
    string_iterator_t *it = NULL;

    if (self->end_node) {
        it = x ? x : malloc(sizeof(string_iterator_t));
        memset(it, 0, sizeof(string_iterator_t));

        it->parent = self;
        it->node = self->end_node->next;
        it->next = iterator_next_character;
        it->prev = iterator_previous_character;
        it->delete = iterator_delete;
        it->split = iterator_split;
        it->truncate = iterator_truncate;
        it->insert = iterator_insert;
        it->clone_to = iterator_clone_to_after_point;
    }

    return it;
}

static string_t *partition_next(substring_generator_t *self)
{
    string_t *string = NULL; /* return string */
    string_iterator_t it;
    const char *cp = NULL;
    int quit_p = 0;

    if (self->node &&
        self->parent->get_iterator((string_t *)self->parent, &it)) {
        string_iterator_t it2;
        it.node = self->node;
        it.index = self->index;
        string = it.clone_to(&it, NULL);

        if (string->get_iterator(string, &it2)) {
            int iter = 0;

            /* go through characters starting from current pos */
            while (!quit_p && (cp = it.next(&it))) {
                int no_advance_p = 0;
                if (*cp == self->delimiter) {
                    if (iter == 0) {
                        it2.delete(&it2, 1);
                        no_advance_p = 1;
                    } else {
                        /* found delimiter, cut destination iterator */
                        it2.truncate(&it2);
                        quit_p = 1;
                    }
                }
                /* update substring generator */
                self->node = it.node;
                self->index = it.index;
                /* next destination character (to prepare the truncation point)
                 */
                if (!no_advance_p)
                    it2.next(&it2);
                iter++;
            }
            if (!cp) {
                /* this was the end */
                self->node = NULL;
            }
        }
    }

    return string;
}

static substring_generator_t *
string_get_substring_generator(const string_t *self, char d,
                               substring_generator_t *x)
{
    substring_generator_t *sgen = NULL;
    if (self->end_node) {
        sgen = x ? x : malloc(sizeof(substring_generator_t));
        memset(sgen, 0, sizeof(substring_generator_t));

        sgen->parent = self;
        sgen->node = self->end_node->next;
        sgen->next = partition_next;
        sgen->index = 0;
        sgen->delimiter = d;
    }

    return sgen;
}

string_t *string_instantiate(string_t *x)
{
    string_t *str = string_instantiate_super(x);

    str->new = string_new;
    str->destroy = string_destroy;

    object_set_release_callback(string_to_object(str), string_release);

    str->clone_to = string_clone_to;
    str->new_c_str = string_new_c_str;
    str->get_length = string_get_length;
    str->get_iterator = string_get_iterator;
    str->get_substring_generator = string_get_substring_generator;
    str->append = string_append;
    str->prepend = string_prepend;

    return str;
}
