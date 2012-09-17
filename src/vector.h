/* a10 504
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/vector.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 504 */



#ifndef _VECTOR_H_
#define _VECTOR_H_

typedef float co_t;

typedef 
union {
    struct {
	co_t x;
	co_t y;
    };
    co_t v[2];
} vector2d_t;

typedef union { 
    struct {
	co_t x;
	co_t y;
	co_t z;
    };
    co_t v[3];
} vector3d_t;

/* vertice of a polygon. 
   x,y: screen coordinates
   u,v: texture coordinates
*/
typedef struct
{
    co_t x, y;
    co_t u, v;
} vertex_t;

//vector3d_t* vector3d_instantiate(vector3d_t* x);

#endif
