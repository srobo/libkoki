
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
	float scale;
	koki_marker_rotation_t output;

	assert(points != NULL);

	output.z = 0; /* TODO: z currently unused */

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
	scale = sqrt(pow(cvmGet(n, 0, 0), 2) +
		     pow(cvmGet(n, 1, 0), 2) +
		     pow(cvmGet(n, 2, 0), 2));

	for (uint8_t i=0; i<3; i++)
		cvmSet(n, i, 0, cvmGet(n, i, 0) / scale);


	/* rotation about X --> atan2(n_y, n_z) */
	output.x = atan2(cvmGet(n, 1, 0), cvmGet(n, 2, 0));

	/* rotation about Y --> atan2(n_x, n_z) */
	output.y = atan2(cvmGet(n, 0, 0), cvmGet(n, 2, 0));

	/* rotation about Z */
	// TODO: implement


	/* clean up */
	cvReleaseMat(&a);
	cvReleaseMat(&b);
	cvReleaseMat(&n);

	/* convert to degrees */
	output.x *= 180.0 / M_PI;
	output.y *= 180.0 / M_PI;
	output.z *= 180.0 / M_PI;

	output.x = 180.0 - output.x;
	output.y = 180.0 - output.y;

	/* put in range: -180 < angle <= 180 */
	output.x -= output.x >= 180.0 ? 360.0 : 0;
	output.y -= output.y >= 180.0 ? 360.0 : 0;

	/* invert Y so that +ve rotations are looking towards the +ve
	   end of the axis from (0, 0, 0), and rotating anti-clockwise */
	output.y = -output.y;

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

	marker->rotation.z += rotation.z;
	marker->rotation.z -= marker->rotation.z >= 360 ? 360 : 0;

}
