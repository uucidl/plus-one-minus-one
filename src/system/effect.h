/* a10 297
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/effect.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 297 */



#ifndef EFFECT_H
  #define EFFECT_H

/*

an effect
  generates or modify frames
  exposes parameters to the outside world
  requires an initialisation part

*/

typedef struct effect effect_t;

/* 
   running modes of an effect 
*/
typedef enum { ON, OFF } emode_t;

#include <messaging/class.h>
#include <messaging/router.h>
#include <scripting/dictionary.h>

struct effect {
    router_t super;    /* message router */
    class_t* class;
    atom_t   id;

    emode_t mode;  /* running mode */

    /* methods */
    int  (*new) (effect_t* this);
    int  (*destroy) (effect_t* this);
    void (*computes) (effect_t* this, void* frame, double ms);
    void (*set_mode) (effect_t* this, emode_t mode);

    /* compile and send a message immediatly to this object */
    void (*send_immediate) (effect_t* this, const char* message);
    void (*send_message) (effect_t* this, 
			  bytecode_stream_t* msg, context_t* c);

    /* returns the difference of time between the output (what is
       produced by computes) and the input (what is provided to
       computes) */
    double (*get_latency_ms) (effect_t* this);
    
    /*
      returns an atom identifying the type of frames this effect is expecting in 'computes'
      those types are agreed upon by the effects and the drivers.
    */
    atom_t (*get_frame_type) (effect_t* this);

    /*
      set the description of the next frames that are going to be passed to 'computes'
      the signature depends on the frame type supported.

      this is expected to be non realtime, and will be called as sparingly as possible.
    */
    void (*set_frame_signature) (effect_t* this, object_t* signature);

    /* backup pointer for set_mode */
    void (*swapped_computes) (effect_t* this, void* frame, double ms);
};

CLASS_INHERIT(effect, router)

/* accessors */

static inline
router_t* effect_get_router (effect_t* e) {
    return &e->super;
}


static inline
class_t* effect_get_class (effect_t* e) {
    return e->class;
}

void effect_register_instance (const char* class_name, effect_t* instance);

#endif
