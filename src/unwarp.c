
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
IplImage* koki_unwarp_marker(koki_marker_t *marker, IplImage *frame,
			     uint16_t unwarped_width)
{

	CvRect clip_rect;
	CvPoint2D32f src[4], dst[4];
	CvMat *map_matrix = cvCreateMat(3, 3, CV_64FC1);
	IplImage *ret;

	assert(marker != NULL);
	assert(frame != NULL);
	assert(unwarped_width > 0 && unwarped_width % 10 == 0);

	/* get clip region */
	clip_rect = get_clip_rectangle(marker);

	/* use the clip rect as region of interest */
	cvSetImageROI(frame, clip_rect);

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
