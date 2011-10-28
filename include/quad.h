/* Copyright 2011 Chris Kirkham

   This file is part of libkoki

   libkoki is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   libkoki is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with libkoki.  If not, see <http://www.gnu.org/licenses/>. */
#ifndef _KOKI_QUAD_H_
#define _KOKI_QUAD_H_

/**
 * @file  quad.h
 * @brief Header file for discovering quadrilaterals in contour chains
 */

#include <glib.h>
#include <cv.h>

#include "points.h"

/**
 * @brief a structure containing the links contour chain links and their
 *        respective points, for convinience.
 */
typedef struct {
	koki_point2Df_t vertices[4]; /**< the points, in a clockwise ordering
					  starting at index 0 */
	GSList *links[4];            /**< the \c GSLists of type
					  \c koki_point2Di_t* which relate
				          to the \c vertices */
} koki_quad_t;



koki_quad_t* koki_quad_find_vertices(GSList *contour);

void koki_quad_refine_vertices(koki_quad_t *quad);

void koki_quad_free(koki_quad_t *quad);

void koki_quad_draw(IplImage *frame, koki_quad_t *quad);

#endif /* _KOKI_QUAD_H_ */
