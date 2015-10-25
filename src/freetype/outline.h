/* a10 334
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/freetype/outline.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 334 */

#ifndef OUTLINE_H
#define OUTLINE_H

#include <ft2build.h>
#include FT_OUTLINE_H

#include <vector.h>
#include <library/memory.h>

typedef struct outline_t {
    object_t super;
    FT_Outline ftoutline;

    FT_Matrix transform;

    int (*new)(struct outline_t *self);
    int (*destroy)(struct outline_t *self);

    /* copy src outline to self, possibly allocating */
    struct outline_t *(*copy)(struct outline_t *self, struct outline_t *src);

    /* move to a position, starts a new contour */
    struct outline_t *(*move_to)(struct outline_t *self, vector2d_t *to);

    /* creates a line */
    struct outline_t *(*line_to)(struct outline_t *self, vector2d_t *to);

    /* one control point, one destination */
    struct outline_t *(*conic_to)(struct outline_t *self, vector2d_t *control,
                                  vector2d_t *to);
    /* two control points, one destination */
    struct outline_t *(*cubic_to)(struct outline_t *self, vector2d_t *control1,
                                  vector2d_t *control2, vector2d_t *to);

    /* scale outline */
    struct outline_t *(*scale)(struct outline_t *self,
                               vector2d_t *scaling_factors);
    /* rotation */
    struct outline_t *(*rotate)(struct outline_t *self, double angle);
    /* translate */
    struct outline_t *(*translate)(struct outline_t *self,
                                   vector2d_t *displacement);

} outline_t;

CLASS_INHERIT(outline, object)

#endif
