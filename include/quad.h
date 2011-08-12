#ifndef _KOKI_QUAD_H_
#define _KOKI_QUAD_H_

/**
 * @file  quad.h
 * @brief Header file for discovering quadrilaterals in contour chains
 */

#include <glib.h>

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


#endif /* _KOKI_QUAD_H_ */
