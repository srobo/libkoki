#ifndef _KOKI_MARKER_H_
#define _KOKI_MARKER_H_

/**
 * @file  marker.h
 * @brief Header file for marker related things
 */

#include <stdint.h>

#include "points.h"
#include "quad.h"


/**
 * @brief a structure representing a a single vertex, both in 2D and 3D space
 */
typedef struct {
	koki_point2Df_t image; /**< a point on the image plane */
	koki_point3Df_t world; /**< a point in 3D (world) space */
} koki_marker_vertex_t;


/**
 * @brief a structure for representing marker rotation about its centre
 */
typedef struct {
	float x;  /**< roatation about the X axis */
	float y;  /**< roatation about the Y axis */
	float z;  /**< roatation about the Z axis */
} koki_marker_rotation_t;


/**
 * @brief a structure detailing a single marker
 */
typedef struct {
	uint8_t code;                      /**< the marker number/code */
	koki_marker_vertex_t centre;       /**< the centre of the marker */
	koki_marker_vertex_t vertices[4];  /**< the 4 vertices of the marker */
	koki_marker_rotation_t rotation;   /**< the rotation of the marker
					        about its centre point */
} koki_marker_t;



koki_marker_t* koki_marker_new(koki_quad_t *quad);

void koki_marker_free(koki_marker_t *marker);


#endif /* _KOKI_MARKER_H_ */
