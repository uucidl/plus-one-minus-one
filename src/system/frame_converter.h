/* a10 635
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/frame_converter.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 635 */



#ifndef KNOS_DEMOS_SYSTEM_FRAME_CONVERTER_H
#define KNOS_DEMOS_SYSTEM_FRAME_CONVERTER_H

#define FRAME_CONVERTER_FRAME_TYPE_NAME "frame_converter_frame"

#include <library/memory.h>
#include <scripting/atom.h>

typedef struct frame_converter_frame_t
{
    void *output_frame;
    void *input_frame;
} frame_converter_frame_t;

typedef struct frame_converter_frame_signature_t
{
    object_t super;
    object_t* output_signature;
    object_t* input_signature;
} frame_converter_frame_signature_t;

CLASS_INHERIT (frame_converter_frame_signature, object);

/*
  a frame converter will try to convert between two different kinds of frame
*/
typedef struct frame_converter_t
{
    object_t super;

    int (*new) (struct frame_converter_t* self);
    int (*destroy) (struct frame_converter_t* self);
    void (*set_frame_signature) (struct frame_converter_t* self,
				 frame_converter_frame_signature_t* signature);
    atom_t (*get_frame_type) (struct frame_converter_t* self);
    void (*computes) (struct frame_converter_t* self,
		      void* content, double ms);
} frame_converter_t;

CLASS_INHERIT(frame_converter, object);

#endif
