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

/**
 * @file  rotation.c
 * @brief Implementation for discovering marker rotation
 */

#include <stdint.h>
#include <cv.h>
#include <assert.h>
#include <math.h>

#include "points.h"
#include "marker.h"

#include "rotation.h"

#ifndef M_PI
#define M_PI 3.14159265
#endif


/**
 * @brief given 4 planar 3D points with the mean of the points at (0, 0, 0),
 *        i.e. rotations about the centre, calculate the rotation about each
 *        of the 3 axes
 *
 * To calcuate rotation about the X and Y axes, the normal to the 4 planar
 * points is constructed using the formula:
 *
 * \code  n = (p2 - p1) x (p3 - p1)  \endcode
 *
 * Since we know (0, 0, 0) is in the plane, and that all points are defined
 * relative to it, we can take \c p1 as (0, 0, 0) and simplify the above:
 *
 * \code  n = p2 X p3  \endcode
 *
 * In the code, \c p2 is referred to as \c a, and \c p3 as \c b.
 *
 * @param points  the array of points, centred about some centre (i.e. the
 *                mean of the 4 points) to estimate the rotation of
 * @return        an estimate of the rotation of the points, as a
 *                koki_marker_rotation_t
 */
koki_marker_rotation_t koki_rotation_estimate_array(koki_point3Df_t points[4])
{

	CvMat *a;
	CvMat *b;
	CvMat *n;
	CvMat *R;
	float len;
	float cos_x, cos_y, sin_x, sin_y;
	koki_marker_rotation_t output;

	assert(points != NULL);

	/* calculate normal */
	a = cvCreateMat(3, 1, CV_64FC1);
	b = cvCreateMat(3, 1, CV_64FC1);
	n = cvCreateMat(3, 1, CV_64FC1);

	/* fill a */
	cvmSet(a, 0, 0, points[0].x);
	cvmSet(a, 1, 0, points[0].y);
	cvmSet(a, 2, 0, points[0].z);

	/* fill b */
	cvmSet(b, 0, 0, points[1].x);
	cvmSet(b, 1, 0, points[1].y);
	cvmSet(b, 2, 0, points[1].z);


	/* calculate normal -- Note that this assumes the centre of
	   the points is \c (0, 0, 0) */
	cvCrossProduct(a, b, n);

	/* normalise -- unit vector */
	len = sqrt(pow(cvmGet(n, 0, 0), 2) +
		   pow(cvmGet(n, 1, 0), 2) +
		   pow(cvmGet(n, 2, 0), 2));

	for (uint8_t i=0; i<3; i++)
		cvmSet(n, i, 0, cvmGet(n, i, 0) / len);


	/* rotation about Y --> atan2(n_x, n_z) */
	output.y = atan2(cvmGet(n, 0, 0), cvmGet(n, 2, 0));

	/* rotation about X --> atan2(n_y, len) */
	output.x = asin(cvmGet(n, 1, 0) / 1);


	/* clean up (mat a and b will be used later) */
	cvReleaseMat(&n);


	/* re-jiggle the numbers to be between +/- 180 degrees (M_PI radians) */
	output.y = M_PI - output.y;

	/* put in range: -180 < angle <= 180 (but in radians) */
	output.x -= output.x >= M_PI ? 2 * M_PI : 0;
	output.y -= output.y >= M_PI ? 2 * M_PI : 0;

	/* invert Y so that +ve rotations are looking towards the +ve
	   end of the axis from (0, 0, 0), and rotating anti-clockwise */
	output.y = -output.y;


	/* rotation about Z -- unrotate about X and Y as calculated, then
	   calculate Z rotation from there */

	R = cvCreateMat(3, 3, CV_64FC1); /* a rotation matrix about the X
					    and Y axes */

	sin_x = sin(-output.x);
	sin_y = sin(-output.y);
	cos_x = cos(-output.x);
	cos_y = cos(-output.y);

	/* fill R */
	cvmSet(R, 0, 0, cos_y);
	cvmSet(R, 0, 1, 0);
	cvmSet(R, 0, 2, sin_y);

	cvmSet(R, 1, 0, -sin_x * -sin_y);
	cvmSet(R, 1, 1, cos_x);
	cvmSet(R, 1, 2, -sin_x * cos_y);

	cvmSet(R, 2, 0, -sin_y * cos_x);
	cvmSet(R, 2, 1, sin_x);
	cvmSet(R, 2, 2, cos_x * cos_y);

	/* fill a -- the point in between the first 2 vertices,
	   i.e. the centre point of the top edge */
	cvmSet(a, 0, 0, (points[0].x + points[1].x) / 2);
	cvmSet(a, 1, 0, (points[0].y + points[1].y) / 2);
	cvmSet(a, 2, 0, (points[0].z + points[1].z) / 2);

	/* unrotate about X and Y */
	cvMatMulAdd(R, a, NULL, b);

	output.z = atan2(cvmGet(b, 0, 0), cvmGet(b, 1, 0));

	/* convert to degrees */
	output.x *= 180.0 / M_PI;
	output.y *= 180.0 / M_PI;
	output.z *= 180.0 / M_PI;

	/* clean up */
	cvReleaseMat(&a);
	cvReleaseMat(&b);
	cvReleaseMat(&R);

	return output;

}



/**
 * @brief estimates the rotation of the marker in 3D space about the 3 axes
 *        and stores the result in the given marker
 *
 * This function calls koki_rotation_estimat_array() after constructing
 * suitable arrays.
 *
 * @param marker  the marker that contains the vertices to estimate the
 *                rotation of
 */
void koki_rotation_estimate(koki_marker_t *marker)
{

	koki_point3Df_t points[4];
	koki_marker_rotation_t rotation;

	assert(marker != NULL);

	/* create (0, 0, 0) centred points array */
	for (uint8_t i=0; i<4; i++){
		points[i].x = marker->vertices[i].world.x -
			marker->centre.world.x;
		points[i].y = marker->vertices[i].world.y -
			marker->centre.world.y;
		points[i].z = marker->vertices[i].world.z -
			marker->centre.world.z;
	}

	/* estimate rotation */
	rotation = koki_rotation_estimate_array(points);

	/* add to marker rotation and normalise */
	marker->rotation.x += rotation.x;
	marker->rotation.x -= marker->rotation.x >= 360 ? 360 : 0;

	marker->rotation.y += rotation.y;
	marker->rotation.y -= marker->rotation.y >= 360 ? 360 : 0;

	/* add code rotation offset */
	marker->rotation.z += rotation.z + marker->rotation_offset;
	marker->rotation.z -= marker->rotation.z >= 360 ? 360 : 0;

	/* put in range -180 < angle <= 180 */
	if (marker->rotation.z > 180.0)
		marker->rotation.z = -(360.0 - marker->rotation.z);

	/* negate so +ve rotation is anti-clockwise looking from (0, 0, 0)
	   towards +ve Z (in the distance) */
	marker->rotation.z = -marker->rotation.z;

}
