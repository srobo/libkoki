/* Copyright 2011 Chris Kirkham, Robert Spanton

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
 * @file  marker.c
 * @brief Implementation of marker related things
 */

#include <stdint.h>
#include <stdbool.h>
#include <cv.h>
#include <glib.h>

#include "quad.h"
#include "code_grid.h"
#include "unwarp.h"
#include "threshold.h"
#include "camera.h"
#include "labelling.h"
#include "contour.h"
#include "pose.h"
#include "rotation.h"
#include "bearing.h"
#include "debug.h"

#include "marker.h"


/**
 * @brief creates a marker, copying data from a quad, and returns a pointer
 *        to said marker
 *
 * @param quad  the quad the transfer data from
 * @return      a pointer to the new marker
 */
koki_marker_t* koki_marker_new(koki_quad_t *quad)
{

	koki_marker_t *marker;
	float sum[2] = {0, 0};

	marker = malloc(sizeof(koki_marker_t));
	assert(marker != NULL);

	/* copy quad vertex values over */
	for (uint8_t i=0; i<4; i++){
		marker->vertices[i].image = quad->vertices[i];
		sum[0] += quad->vertices[i].x;
		sum[1] += quad->vertices[i].y;
	}

	/* calculate image centre */
	marker->centre.image.x = sum[0]/4;
	marker->centre.image.y = sum[1]/4;

	/* ensure the top-left vertex is the first */
	if (marker->vertices[0].image.y > marker->vertices[1].image.y){

		/* first vertex is top right, rotate indices so that
		   vertices[0] is the top left */

		koki_point2Df_t tmp;

		tmp = marker->vertices[0].image;
		marker->vertices[0].image = marker->vertices[1].image;
		marker->vertices[1].image = marker->vertices[2].image;
		marker->vertices[2].image = marker->vertices[3].image;
		marker->vertices[3].image = tmp;

	}

	marker->rotation_offset = 0;

	/* zero the rotations */
	marker->rotation.x = 0;
	marker->rotation.y = 0;
	marker->rotation.z = 0;

	return marker;

}



/**
 * @brief frees an allocated marker
 *
 * @param marker  the marker to free
 */
void koki_marker_free(koki_marker_t *marker)
{

	free(marker);

}



/**
 * @brief recovers the code from a marker, if possible
 *
 * @param koki    the libkoki context
 * @param marker  the marker to try and get the code for
 * @param frame   the original image, used to extract the marker's pixels from
 * @return        TRUE if a good code is found, indicating the marker structure
 *                has been changed to reflect this; FALSE if no success
 */
bool koki_marker_recover_code( koki_t* koki, koki_marker_t *marker, IplImage *frame )
{

	IplImage *unwarped;
	IplImage *res;
	koki_grid_t grid;
	float rotation;
	int16_t code;

	assert(marker != NULL);
	assert(frame != NULL && frame->nChannels == 1);

	/* unwarp */
	unwarped = koki_unwarp_marker( koki, marker, frame, 100 );

	/* can we continue? */
	if (unwarped == NULL)
		return FALSE;

	koki_log( koki, "unwarped marker\n", unwarped );

	/* Adaptively threshold the marker */
	res = koki_threshold_adaptive( unwarped, 21, 3, KOKI_ADAPTIVE_MEAN );
	koki_log( koki, "unwarped and thresholded marker\n", res );

	/* Resulting image is already b&w, so a threshold of 127 will do */
	koki_grid_from_image(res, 127, &grid);

	/* recover code */
	code = koki_code_recover_from_grid(&grid, &rotation);

	if (code < 0){ /* code not recovered */
		koki_log( koki, "Failed to recover code from unwarped marker -- discarding\n", NULL );

		cvReleaseImage(&unwarped);
		cvReleaseImage(&res);

		return FALSE;
	}

	/* add code to the marker */
	marker->code = koki_code_translation(code);

	/* add rotation info to the marker */
	marker->rotation_offset = rotation;

	/* clean up */
	cvReleaseImage(&unwarped);
	cvReleaseImage(&res);

	return TRUE;

}

/**
 * @brief Find the markers in the given frame.  This function can
 *        take the physical size of the markers as a constant, or a
 *        pointer to a function that returns the size of a given
 *        marker.
 *
 * @param koki              the libkoki context
 * @param frame             the input image
 * @param fp                a pointer to a function that returns the size of
 *                          the marker of the given number in metres.  If
 *                          NULL, marker_width will be used.
 * @param marker_width      the marker size to use if fp is NULL, in
 *                          metres.
 * @param params            the camera params for the camera at \c
 *                          frame's resolution
 * @return a \c GptrArray* containing all of the found markers
 */
static GPtrArray* find_markers( koki_t *koki,
				IplImage *frame,
			        float (*fp)(int),
			        float marker_width,
			        koki_camera_params_t *params )
{
	koki_labelled_image_t *labelled_image;
	GSList *contour;
	koki_quad_t *quad;
	koki_marker_t *marker;
	GPtrArray *markers = NULL;
	IplImage *contours = NULL, *disc_contours = NULL;

	assert(frame != NULL && frame->nChannels == 1);

	koki_log( koki, "find_markers() input image\n", frame );

	/* labelling */
	labelled_image = koki_label_adaptive( koki, frame, 11, 5 );

	if (labelled_image == NULL)
		return NULL;

	if (koki_is_logging(koki) ) {
		/* Create images of contours and discarded contours */
		contours = cvCreateImage( cvSize( frame->width, frame->height ),
					  IPL_DEPTH_8U, 3 );

		disc_contours = cvCreateImage( cvSize( frame->width, frame->height ),
					       IPL_DEPTH_8U, 3 );

		/* Set both to be black */
		cvSetZero( contours );
		cvSetZero( disc_contours );
	}

	/* init markers array */
	markers = g_ptr_array_new();

	/* loop though all regions */
	for (label_t i=0; i<labelled_image->clips->len; i++){

		/* make sure it's big enough, etc... */
		if (!koki_label_useable(labelled_image, i))
			continue;

		/* get contour */
		contour = koki_contour_find(labelled_image, i);

		/* find vertices */
		quad = koki_quad_find_vertices(contour);

		if (quad == NULL){
			if( disc_contours != NULL )
				koki_contour_draw( disc_contours, contour );

			koki_contour_free(contour);
			continue;
		}

		if( contours != NULL )
			koki_contour_draw( contours, contour );

		/* refine vertices */
		koki_quad_refine_vertices(quad);

		/* create a base marker */
		marker = koki_marker_new(quad);
		assert(marker != NULL);

		/* recover code */
		if (koki_marker_recover_code(koki, marker, frame)){
			float size;
			assert(marker != NULL);

			if( fp == NULL )
				size = marker_width;
			else
				size = fp(marker->code);

			koki_pose_estimate(marker, size, params);
			koki_rotation_estimate(marker);
			koki_bearing_estimate(marker);

			/* append the marker to the output array */
			g_ptr_array_add(markers, marker);

		} else {

			/* not a useful marker, free it */
			koki_marker_free(marker);

		}

		/* cleanup */
		koki_contour_free(contour);
		koki_quad_free(quad);

	}//for

	/* clean up */
	koki_labelled_image_free(labelled_image);

	if( contours != NULL ) {
		koki_log( koki, "Contours", contours );
		cvReleaseImage( &contours );
	}

	if( disc_contours != NULL ) {
		koki_log( koki, "Discarded Contours", disc_contours );
		cvReleaseImage( &disc_contours );
	}

	return markers;
}

/**
 * @brief a higher-level function that does everything necessary to return
 *        an array of markers that thare in the given frame
 *
 * Note that with this function, one can only have a single marker size.
 *
 * @param koki    the libkoki context
 * @param frame         the input image
 * @param marker_width  the width, in metres, of the marker(s) in the image
 * @param params        the camera params for the camera at \c frame's
 *                      resolution
 * @return              a \c GptrArray* containing all of the found markers
 */
GPtrArray* koki_find_markers( koki_t *koki,
			      IplImage *frame,
			      float marker_width,
			      koki_camera_params_t *params )
{
	return find_markers( koki, frame, NULL, marker_width, params );
}

/**
 * @brief a higher-level function that does everything necessary to return
 *        an array of markers that thare in the given frame, with a user-
 *        specified function for determining the marker width based on the
 *        code
 *
 * The function pointer, \c fp, should point to a function that takes as
 * argument a marker code, and returns a float representing the width of
 * a marker with said code.
 *
 * @param koki    the libkoki context
 * @param frame   the input image
 * @param fp      the function pointer
 * @param params  the camera params for the camera at \c frame's resolution
 * @return        a \c GptrArray* containing all of the found markers
 */
GPtrArray* koki_find_markers_fp( koki_t *koki,
				 IplImage *frame,
				 float (*fp)(int),
				 koki_camera_params_t *params )
{
	return find_markers( koki, frame, fp, 0, params );
}

/**
 * @brief frees all the markers pointed to from the array, then frees the
 *        array itself
 *
 * @param markers  the markers pointer array to free
 */
void koki_markers_free(GPtrArray *markers)
{

	if (markers == NULL)
		return;

	for (int i=0; i<markers->len; i++){

		koki_marker_t *marker;
		marker = g_ptr_array_index(markers, i);
		koki_marker_free(marker);

	}

	g_ptr_array_free(markers, TRUE);

}
