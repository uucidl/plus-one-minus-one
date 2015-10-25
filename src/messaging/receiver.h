/* a10 389
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/messaging/receiver.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 389 */

#ifndef KNOS_MESSAGING_RECEIVER_H
#define KNOS_MESSAGING_RECEIVER_H

#include <messaging/definitions.h>
#include <messaging/dispatcher.h>
#include <messaging/context.h>
#include <scripting/bytecode_stream.h>
#include <library/map.h>
#include <library/memory.h>

typedef struct message_node_t {
    bytecode_stream_t *bytecodes;
    context_t *context;
    int dispatched_p;
    struct message_node_t *next;
    struct message_node_t *prev;
} message_node_t;

typedef struct message_fifo_t {
    object_t super;
    atom_t atom;
    message_node_t *end;
    int closed_p;
} message_fifo_t;

CLASS_INHERIT(message_fifo, object)

typedef struct receiver_t {
    object_t super;
    dispatcher_t dispatcher;

    map_t fifos; /* mapping between atoms and message_fifo_t* */

    /* add and sets the definition for this atom, and create
       message fifo queue */
    void (*set_definition)(struct receiver_t *self, atom_t a,
                           bytecode_stream_t *args, recp_f f);

    /* evaluate one message */
    void (*eval)(struct receiver_t *self, bytecode_stream_t *message,
                 context_t *context);

    /* evaluate all messages received for the given selector in the given
     * context */
    void (*eval_pending)(struct receiver_t *self, atom_t selector,
                         context_t *context);

    /* evaluate all the messages received for any selector, in the given context
     */
    void (*eval_all_pending)(struct receiver_t *self, context_t *context);

    /* returns the first non-dispatched message node in context;
       if none, returns NULL */
    message_node_t *(*peek_pending)(struct receiver_t *self, atom_t selector,
                                    context_t *context);

    /* returns the first non-dispatched message node in context;
       if none, returns NULL */
    message_node_t *(*peek_any_pending)(struct receiver_t *self,
                                        context_t *context);

    /* send a message to this receiver with the given context.
       the objects passed to it via this method are considered
       owned by the receiver, thus should not be allocated on the stack,
       or manipulated, or only allocated once

       the message is dispatched immediatly if (context->dispatch_now_p)
    */
    void (*receive)(struct receiver_t *self, bytecode_stream_t *message,
                    context_t *context);

    /* close or open a given message queue */
    /* open allocates the fifo if not already done */
    void (*open)(struct receiver_t *self, atom_t selector);
    void (*close)(struct receiver_t *self, atom_t selector);
} receiver_t;

CLASS_INHERIT(receiver, object);

/* a special receiver that owns all received messages.
   -context is automatically  associated with self-
*/
receiver_t *self_receiver_instantiate(receiver_t *x);

#endif
