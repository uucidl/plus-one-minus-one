/* a10 190
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/receiver.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 190 */

#include <messaging/receiver.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_MESSAGING_RECEIVER);

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include <library/map_impl.h>

message_fifo_t *message_fifo_instantiate(message_fifo_t *x)
{
    return message_fifo_instantiate_super(x);
}

static message_fifo_t *get_fifo(receiver_t *self, atom_t a)
{
    map_value_t pfifo = map_get(&self->fifos, (unsigned long)a);
    message_fifo_t *fifo =
        map_value_is_there(pfifo) ? map_value_obtain(pfifo) : NULL;

    return fifo;
}

static message_fifo_t *add_fifo(receiver_t *self, atom_t a)
{
    message_fifo_t *fifo = get_fifo(self, a);

    TRACE("adding fifo: '%s'\n", atom_get_cstring_value(a));
    if (!fifo) {
        fifo = message_fifo_instantiate_toplevel(NULL);
        fifo->atom = a;
        fifo->end = NULL;
        map_put(&self->fifos, (unsigned long)a, fifo);
    }

    return fifo;
}

/*--- node iterator ---*/

typedef struct node_iterator_t {
    message_fifo_t *parent;
    message_node_t *node; /* current node */
} node_iterator_t;

static message_node_t *node_iterator_next(node_iterator_t *self)
{
    message_node_t *node = self->node;
    if (self->node) {
        if (self->node == self->parent->end)
            self->node = NULL;
        else
            self->node = self->node->next;
    }

    return node;
}

static message_node_t *node_iterator_prev(node_iterator_t *self)
{
    message_node_t *node = self->node;
    if (self->node) {
        if (self->node == self->parent->end->next)
            self->node = NULL;
        else
            self->node = self->node->prev;
    }

    return node;
}

static node_iterator_t *get_node_iterator(message_fifo_t *fifo,
                                          node_iterator_t *x)
{
    node_iterator_t *it = x ? x : malloc(sizeof(node_iterator_t));
    memset(it, 0, sizeof(node_iterator_t));

    it->parent = fifo;
    if (fifo->end)
        it->node = fifo->end->next;
    else
        it->node = NULL;

    return it;
}

static node_iterator_t *get_node_iterator_end(message_fifo_t *fifo,
                                              node_iterator_t *x)
{
    node_iterator_t *it = x ? x : malloc(sizeof(node_iterator_t));
    memset(it, 0, sizeof(node_iterator_t));

    it->parent = fifo;
    if (fifo->end)
        it->node = fifo->end;
    else
        it->node = NULL;

    return it;
}

/*--- receiver ---*/
static void receiver_eval(receiver_t *self, bytecode_stream_t *message,
                          context_t *context)
{
    self->dispatcher.dispatch(&self->dispatcher, context, message);
    bytecode_stream_retire(message);
    context_retire(context);
}

static void receiver_eval_pending(receiver_t *self, atom_t selector,
                                  context_t *context)
{
    message_fifo_t *fifo = get_fifo(self, selector);
    node_iterator_t it;

    if (fifo == NULL) {
        WARNING("fifo for selector '%s' inexistant.",
                 atom_get_cstring_value(selector));
        return;
    }

    TRACE("eval_pending for fifo: '%s'", atom_get_cstring_value(fifo->atom));
    if (fifo->end == NULL) {
        return;
    }

    if (get_node_iterator_end(fifo, &it)) {
        message_node_t *current;
        while ((current = node_iterator_prev(&it))) {
            TRACE("current: %f", current->context->ms);
            if (!current->dispatched_p) {
                if (current->context->is_in(current->context, context)) {
                    self->eval(self, current->bytecodes, current->context);
                    current->dispatched_p = 1;
                } else {
                    break;
                }
            }
        }
    }
}

static void receiver_eval_all_pending(receiver_t *self, context_t *context)
{
    map_iterator_t it;
    map_value_t pfifo;

    map_get_iterator(&self->fifos, &it);
    while (map_value_is_there(pfifo = map_iterator_next(&it))) {
        message_fifo_t *fifo = map_value_obtain(pfifo);
        self->eval_pending(self, fifo->atom, context);
    }
    map_iterator_destroy(&it);
    map_iterator_retire(&it);
}

static message_node_t *receiver_peek_pending(receiver_t *self, atom_t selector,
                                             context_t *context)
{
    message_fifo_t *fifo = get_fifo(self, selector);
    node_iterator_t it;
    message_node_t *head = NULL;

    if (fifo == NULL) {
        WARNING("fifo for selector '%s' inexistant.",
                 atom_get_cstring_value(selector));
        return NULL;
    } else if (fifo->end == NULL) {
        return NULL;
    }

    if (get_node_iterator_end(fifo, &it)) {
        message_node_t *current;
        while ((current = node_iterator_prev(&it))) {
            TRACE("ms: %f.", current->context->ms);
            if (!current->dispatched_p &&
                current->context->is_in(current->context, context)) {
                head = current;
                break;
            }
        }
    }

    return head;
}

static message_node_t *receiver_peek_any_pending(receiver_t *self,
                                                 context_t *context)
{
    map_iterator_t it;
    message_node_t *head = NULL;
    map_value_t pfifo;

    /* find first occurence of next event in any fifo */
    map_get_iterator(&self->fifos, &it);
    while (map_value_is_there(pfifo = map_iterator_next(&it))) {
        message_fifo_t *fifo = map_value_obtain(pfifo);
        message_node_t *node = self->peek_pending(self, fifo->atom, context);
        if (head == NULL) {
            head = node;
        } else if (node && node->context->ms < head->context->ms) {
            head = node;
        }
    }
    map_iterator_destroy(&it);
    map_iterator_retire(&it);

    return head;
}

static void receiver_receive(receiver_t *self, bytecode_stream_t *message,
                             context_t *context)
{
    atom_t selector;
    message_fifo_t *fifo;
    message_node_t *node;
    node_iterator_t it;

    if (!message || !message->end) {
        WARNING("message === NULL");
        return;
    }

    selector = message->end->code.verb;
    fifo = get_fifo(self, selector);
    node = calloc(sizeof(message_node_t), 1);

    if (!fifo)
        fifo = add_fifo(self, selector);

    if (fifo->closed_p) {
        /* destroy the message and the context objects */
        bytecode_stream_retire(message);
        context_retire(context);

        return;
    }

    /*
       insert message in the right position

       messages are ordered by decreasing context from beginning to end.
    */
    if (get_node_iterator(fifo, &it)) {
        message_node_t *current = NULL;
        message_node_t *last = NULL;

        TRACE("adding at: %f in fifo: %s", context->ms,
               atom_get_cstring_value(fifo->atom));

        while ((current = node_iterator_next(&it)) &&
               context->is_in(context, current->context)) {
            /* advance until context is not anymore in current context */
            TRACE("current at: %f", current->context->ms);
            last = current;
        }

        if (last == NULL) {
            if (fifo->end == NULL) {
                TRACE("append to empty fifo");
                node->next = node;
                node->prev = node;
                fifo->end = node;
            } else {
                if (context->is_in(context, current->context)) {
                    TRACE("insert after current at %f", current->context->ms);
                    node->prev = current;
                    node->next = current->next;
                    current->next = node;
                    node->next->prev = node;
                } else {
                    TRACE("insert before current at %f", current->context->ms);
                    node->next = current;
                    node->prev = current->prev;
                    current->prev = node;
                    node->prev->next = node;
                }
                if (current == fifo->end)
                    fifo->end = node;
            }
        } else {
            if (context->is_in(context, last->context)) {
                TRACE("insert after last at %f", last->context->ms);
                node->prev = last;
                node->next = last->next;
                last->next = node;
                node->next->prev = node;
            } else {
                TRACE("insert before last at %f", last->context->ms);
                node->next = last;
                node->prev = last->prev;
                last->prev = node;
                node->prev->next = node;
            }
            if (last == fifo->end)
                fifo->end = node;
        }
    }
    node->bytecodes = message;
    node->context = context;
    node->dispatched_p = 0;

    if (node->context->dispatch_now_p) {
        self->eval(self, node->bytecodes, node->context);
        node->dispatched_p = 1;
    }
}

static void receiver_receive_for_self(receiver_t *self,
                                      bytecode_stream_t *message,
                                      context_t *context)
{
    /* overrides destination object */
    context->object = self;
    receiver_receive(self, message, context);
}

static void receiver_open(receiver_t *self, atom_t selector)
{
    message_fifo_t *fifo = get_fifo(self, selector);
    if (!fifo) {
        fifo = add_fifo(self, selector);
    }
    fifo->closed_p = 0;
}

static void receiver_close(receiver_t *self, atom_t selector)
{
    message_fifo_t *fifo = get_fifo(self, selector);
    if (fifo) {
        fifo->closed_p = 1;
    }
}

static void receiver_set_definition(struct receiver_t *self, atom_t a,
                                    bytecode_stream_t *args, recp_f f)
{
    ((definitions_t *)&self->dispatcher.definitions)
        ->set_callback((definitions_t *)&self->dispatcher.definitions, a, args,
                       f);

    add_fifo(self, a);
}

receiver_t *receiver_instantiate(receiver_t *x)
{
    receiver_t *r = receiver_instantiate_super(x);

    map_instantiate_toplevel(&r->fifos);

    dispatcher_instantiate_toplevel(&r->dispatcher);

    r->eval = receiver_eval;
    r->eval_pending = receiver_eval_pending;
    r->eval_all_pending = receiver_eval_all_pending;
    r->peek_pending = receiver_peek_pending;
    r->peek_any_pending = receiver_peek_any_pending;
    r->receive = receiver_receive;
    r->open = receiver_open;
    r->close = receiver_close;
    r->set_definition = receiver_set_definition;

    return r;
}

receiver_t *self_receiver_instantiate(receiver_t *x)
{
    receiver_t *r = receiver_instantiate(x);

    r->receive = receiver_receive_for_self;

    return r;
}
