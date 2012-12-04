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
 * @file  unwarp.c
 * @brief Implementation of marker unwarping
 */

#include <stdint.h>
#include <cv.h>

#include "points.h"
#include "marker.h"

#include "unwarp.h"


/**
 * @brief identifies the clip region containing the marker's points as a
 *        preparation for unwarping
 *
 * @param marker  the marker in question
 * @return        a \c CvRect representing the clip region
 */
static CvRect get_clip_rectangle(koki_marker_t *marker)
{

	uint16_t min_x, max_x, min_y, max_y;
	CvRect rect;

	min_x = min_y = 0xFFFF;
	max_x = max_y = 0;

	for (uint8_t i=0; i<4; i++){

		if (marker->vertices[i].image.x < min_x)
			min_x = marker->vertices[i].image.x;

		if (marker->vertices[i].image.x > max_x)
			max_x = marker->vertices[i].image.x;

		if (marker->vertices[i].image.y < min_y)
			min_y = marker->vertices[i].image.y;

		if (marker->vertices[i].image.y > max_y)
			max_y = marker->vertices[i].image.y;

	}//for

	/* create CvRect */
	rect.x = min_x;
	rect.y = min_y;
	rect.width = max_x - min_x;
	rect.height = max_y - min_y;

	return rect;

}



/**
 * @brief returns an \c IplImage of the provided marker, unwarped
 *
 * @param marker          the marker to unwarp
 * @param frame           the original image to unwarp using
 * @param unwarped_width  the width, in pixels, of the unwarped square image
 * @return                an image of the marker unwarped
 */
IplImage* koki_unwarp_marker( koki_t* koki, koki_marker_t *marker, IplImage *frame,
			      uint16_t unwarped_width )
{

	CvRect clip_rect;
	CvPoint2D32f src[4], dst[4];
	CvMat *map_matrix;
	IplImage *ret;

	assert(marker != NULL);
	assert(frame != NULL);
	assert(unwarped_width > 0 && unwarped_width % 10 == 0);

	/* make sure we're within bounds */
	for (uint8_t i=0; i<4; i++){
		if (marker->vertices[i].image.x < 0 ||
		    marker->vertices[i].image.y < 0 ||
		    marker->vertices[i].image.x >= frame->width ||
		    marker->vertices[i].image.y >= frame->height){

			return NULL;

		}//if
	}//for

	/* create map mat */
	map_matrix = cvCreateMat(3, 3, CV_64FC1);

	/* get clip region */
	clip_rect = get_clip_rectangle(marker);

	/* ensure there is actually something to unwarp -- this
	   may happen with shapes that aren't actually quads */
	if (clip_rect.width == 0 || clip_rect.height == 0){
		cvReleaseMat(&map_matrix);
		return NULL;
	}

	/* use the clip rect as region of interest */
	cvSetImageROI(frame, clip_rect);
	koki_log( koki, "Warped marker", frame );

	/* set source array */
	for (uint8_t i=0; i<4; i++){
		src[i].x = (int)marker->vertices[i].image.x - clip_rect.x;
		src[i].y = (int)marker->vertices[i].image.y - clip_rect.y;
	}

	/* set destination array */
	dst[0].x = 0;
	dst[0].y = 0;
	dst[1].x = unwarped_width;
	dst[1].y = 0;
	dst[2].x = unwarped_width;
	dst[2].y = unwarped_width;
	dst[3].x = 0;
	dst[3].y = unwarped_width;

	/* calc image transform */
	cvGetPerspectiveTransform(src, dst, map_matrix);

	/* unwarp the marker */
	ret = cvCreateImage(cvSize(unwarped_width, unwarped_width),
			    frame->depth, frame->nChannels);

	cvWarpPerspective(frame, ret, map_matrix, CV_WARP_FILL_OUTLIERS,
			  cvScalarAll(0));

	/* clean up */
	cvResetImageROI(frame);
	cvReleaseMat(&map_matrix);

	return ret;

}
