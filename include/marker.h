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
#ifndef _KOKI_MARKER_H_
#define _KOKI_MARKER_H_

/**
 * @file  marker.h
 * @brief Header file for marker related things
 */

#include <stdint.h>
#include <stdbool.h>
#include <cv.h>
#include <glib.h>

#include "points.h"
#include "camera.h"
#include "context.h"

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
 * @brief a structure for representing a 3-dimensional bearing
 */
typedef struct {
	float x; /**< the angle about the x-axis */
	float y; /**< the angle about the y-axis */
	float z; /**< the angle about the z-axis */
} koki_bearing_t;


/**
 * @brief a structure detailing a single marker
 */
typedef struct {
	uint8_t code;                      /**< the marker number/code */
	koki_marker_vertex_t centre;       /**< the centre of the marker */
	koki_marker_vertex_t vertices[4];  /**< the 4 vertices of the marker */
	float rotation_offset;             /**< a multiple of 90 degrees,
					        indicating how many times the
						code grid has been rotated */
	koki_marker_rotation_t rotation;   /**< the rotation of the marker
					        about its centre point */
	koki_bearing_t bearing;            /**< the relative bearing to the
					        marker */
	float distance;                    /**< the straight line distance to
					        the centre of the marker */
} koki_marker_t;



koki_marker_t* koki_marker_new(koki_quad_t *quad);

void koki_marker_free(koki_marker_t *marker);

bool koki_marker_recover_code( koki_t* koki, koki_marker_t *marker, IplImage *frame );

GPtrArray* koki_find_markers( koki_t *koki,
			      IplImage *frame,
			      float marker_width,
			      koki_camera_params_t *params );

GPtrArray* koki_find_markers_fp( koki_t *koki,
				 IplImage *frame,
				 float (*fp)(int),
				 koki_camera_params_t *params );

void koki_markers_free(GPtrArray *markers);


#endif /* _KOKI_MARKER_H_ */
