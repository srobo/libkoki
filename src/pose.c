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
 * @file  pose.c
 * @brief Implementation of position estimation in 3D space
 */

#include <cv.h>
#include <stdint.h>

#include "points.h"
#include "camera.h"
#include "marker.h"

#include "pose.h"


/**
 * @brief given 4 co-planar 2D image points of a square (e.g. a marker), the
 * width of a side of said square, and camera's parameters (for focal length),
 * this function calculates the 3D co-ordinates of the 4 vertices
 *
 * Based on the method detailed in:
 *   Y. Hung., et.al. "Passive Ranging to Known Planar Point Sets", 1985
 *
 * In short, if one casts rays through the 4 image points (on the image plane,
 * i.e. where the X and Y values are image co-ordinates and the Z value is the
 * focal length) from the camera's centre, each ray must pass through the
 * corresponding 3D point.  \c k0 through \c k3 are the scale factors necessary
 * to scale the vector from the camera to the corresponding point on the image
 * plane to the 3D world point.  By considering 1 of the scale factors as being
 * defined by the other 3, we can calculate that point's scale factor and
 * therefore the others too.
 *
 * The world point is simply the ray vector from the camera to the point on
 * the image plane, scaled by its corresponding \c k.
 *
 * @param img           the 2D image points of the 4 vertices of the square
 * @param world         the destination 3D points to store the world
 *                      co-ordinates in
 * @param marker_width  the width, in meters, of the square.
 * @param params        the camera parameters
 */
void koki_pose_estimate_arrays(koki_point2Df_t img[4], koki_point3Df_t world[4],
			       float marker_width, koki_camera_params_t *params)
{

	CvMat *A, *A_inv, *b, *k_out;
	float focal_length, k0_over_k3, tmp;
	float k[4];

	/* average the X and Y focal lengths
	   (they are approx. the same, anyway) */
	focal_length = (params->focal_length.x + params->focal_length.y) / 2;

	A = cvCreateMat(3, 3, CV_64FC1);
	A_inv = cvCreateMat(3, 3, CV_64FC1);
	b = cvCreateMat(3, 1, CV_64FC1);
	k_out = cvCreateMat(3, 1, CV_64FC1);

	/* solve some linear equations */

	/* setup A */
	cvmSet(A, 0, 0, -img[0].x);
	cvmSet(A, 0, 1,  img[1].x);
	cvmSet(A, 0, 2,  img[2].x);

	cvmSet(A, 1, 0, -img[0].y);
	cvmSet(A, 1, 1,  img[1].y);
	cvmSet(A, 1, 2,  img[2].y);

	cvmSet(A, 2, 0, -focal_length);
	cvmSet(A, 2, 1,  focal_length);
	cvmSet(A, 2, 2,  focal_length);

	/* setup b */
	cvmSet(b, 0, 0, img[3].x);
	cvmSet(b, 1, 0, img[3].y);
	cvmSet(b, 2, 0, focal_length);


	/* invert A */
	cvInvert(A, A_inv, CV_LU);

	/* solve */
	cvMatMulAdd(A_inv, b, NULL, k_out);


	k0_over_k3 = cvmGet(k_out, 0, 0);

	/* calculate k3 */
	tmp = sqrt( pow(-k0_over_k3 * img[0].x - img[3].x, 2) +
		    pow(-k0_over_k3 * img[0].y - img[3].y, 2) +
		    pow(-k0_over_k3 * focal_length - focal_length, 2));

	k[3] = fabs(marker_width / tmp);

	/* use k3 to calculate the others */
	for (uint8_t i=0; i<3; i++)
		k[i] = fabs(cvmGet(k_out, i, 0)) * k[3];

	/* do pose estimation calculation */
	for (uint8_t i=0; i<4; i++){

		world[i].x = img[i].x * k[i];
		world[i].y = img[i].y * k[i];
		world[i].z = focal_length * k[i];

	}//for


	/* clean up */
	cvReleaseMat(&A);
	cvReleaseMat(&A_inv);
	cvReleaseMat(&b);
	cvReleaseMat(&k_out);

}



/**
 * @brief estimates the 3D position in space of the marker's vertices
 *
 * @param marker        the marker to estimate the position of
 * @param marker_width  the width, in meters, of the marker
 * @param params        the camera params
 */
void koki_pose_estimate(koki_marker_t *marker, float marker_width,
			koki_camera_params_t *params)
{

	koki_point2Df_t image[4];
	koki_point3Df_t world[4];
	koki_point3Df_t centre = {0, 0, 0};

	assert(marker != NULL);
	assert(marker_width > 0);
	assert(params != NULL);

	/* prepare array: use co-ordinates with origin being
	   the principal point */
	for (uint8_t i=0; i<4; i++){

		image[i].x = marker->vertices[i].image.x -
			params->principal_point.x;

		image[i].y = params->principal_point.y -
			marker->vertices[i].image.y;

	}//for

	/* perform estimation */
	koki_pose_estimate_arrays(image, world, marker_width, params);

	/* copy results and calc centre point */
	for (uint8_t i=0; i<4; i++){
		marker->vertices[i].world = world[i];
		centre.x += world[i].x;
		centre.y += world[i].y;
		centre.z += world[i].z;
	}

	marker->centre.world.x = centre.x/4;
	marker->centre.world.y = centre.y/4;
	marker->centre.world.z = centre.z/4;

	/* calc straight line distance to centre */
	marker->distance = sqrt(pow(marker->centre.world.x, 2) +
				pow(marker->centre.world.y, 2) +
				pow(marker->centre.world.z, 2));

}
