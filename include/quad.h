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

void koki_quad_free(koki_quad_t *quad);

void koki_quad_draw_on_frame(IplImage *frame, koki_quad_t *quad);

#endif /* _KOKI_QUAD_H_ */
