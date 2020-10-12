/* a10 48
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/event_listener.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 48 */

#include "event_listener.h"
#include <library/map_impl.h>
#include <scripting/dictionary.h>

#include <logging.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_DEMOS_SYSTEM_EVENT_LISTENER);

static void event_listener_accept(event_listener_t *self, const event_t *event)
{
    /* iterate through all the childs,
       trying to match the signature of
       each child with the input event.
       if matching, forward it to listener */
    {
        map_iterator_t it;
        map_value_t e;

        self->childs.get_iterator(&self->childs, &it);
        while (map_value_is_there(e = map_iterator_next(&it))) {
            filtered_listener_t *l = map_value_obtain(e);
            if (l && l->signature.match_event_p(&l->signature, event)) {
                l->listener->accept(l->listener, event);
            }
        }
        map_iterator_destroy(&it);
        map_iterator_retire(&it);
    }
}

void event_listener_add_child(struct event_listener_t *self,
                              filtered_listener_t *listener)
{
    if (listener == NULL) {
        ERROR("listener is null.");
    } else {
        self->childs.push(&self->childs, listener);
    }
}

void event_listener_remove_child(struct event_listener_t *self,
                                 filtered_listener_t *listener)
{
    map_iterator_t it;
    unsigned long index = 0L;
    int found_p = 0;
    map_value_t e = NULL_MAP_VALUE;

    self->childs.get_iterator(&self->childs, &it);
    while (map_value_is_there(e = map_iterator_next(&it))) {
        if (map_value_obtain(e) == listener) {
            index = map_iterator_get_index(&it);
            found_p = 1;
            break;
        }
    }
    map_iterator_destroy(&it);
    map_iterator_retire(&it);

    if (found_p)
        self->childs.delete(&self->childs, index);
}

event_listener_t *event_listener_instantiate(event_listener_t *x)
{
    event_listener_t *el = OBJECT_INSTANTIATE(event_listener, x);

    map_instantiate_toplevel(&el->childs);

    el->accept = event_listener_accept;
    el->add_child = event_listener_add_child;
    el->remove_child = event_listener_remove_child;

    return el;
}

static void
filtered_listener_set_signature_from_cstring(filtered_listener_t *self,
                                             const char *s)
{
    event_new_from_cstring(&self->signature.super, s);
}

filtered_listener_t *filtered_listener_instantiate(filtered_listener_t *x)
{
    filtered_listener_t *fl = OBJECT_INSTANTIATE(filtered_listener, x);

    signature_instantiate(&fl->signature);

    fl->set_signature_from_cstring =
        filtered_listener_set_signature_from_cstring;

    return fl;
}
