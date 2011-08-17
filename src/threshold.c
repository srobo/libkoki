
/**
 * @file  threshold.c
 * @brief Implementation for thresholding related activities
 */

#include <stdint.h>
#include <stdbool.h>
#include <cv.h>

#include "labelling.h"

#include "threshold.h"


/**
 * @brief thresholds a specific pixel with the provided threshold
 *
 * @param frame      the image with the pixel to threshold
 * @param x          the pixel's X co-ordinate
 * @param y          the pixel's Y co-ordinate
 * @param threshold  the threshold (in the range \c 0-255) to apply
 * @return           \c TRUE (1) if it's above the threshold, \c FALSE (0)
 *                   otherwise
 */
bool koki_threshold_rgb_pixel(IplImage *frame, uint16_t x, uint16_t y,
			      uint16_t threshold)
{

	return (KOKI_IPLIMAGE_ELEM(frame, x, y, 0) +
		KOKI_IPLIMAGE_ELEM(frame, x, y, 1) +
		KOKI_IPLIMAGE_ELEM(frame, x, y, 2))
		> threshold * 3;

}



/**
 * @brief thresholds an entire \c IplImage, returning it as a new \c IplImage
 *
 * @param frame      the image to threshold
 * @param threshold  the threshold to apply (in the range \c 0-255)
 * @return           a pointer to a new \c IplImage containing the thresholded
 *                   image
 */
IplImage* koki_threshold_frame(IplImage *frame, uint16_t threshold)
{

	assert(frame != NULL);
	assert(threshold >= 0 && threshold <= 255);

	IplImage *ret = cvCreateImage(cvSize(frame->width, frame->height),
				      IPL_DEPTH_8U, 3);

	for (uint16_t y=0; y<frame->height; y++){
		for (uint16_t x=0; x<frame->width; x++){

			uint8_t grey =
				koki_threshold_rgb_pixel(frame, x, y,
							 threshold)
				? 255
				: 0;

			KOKI_IPLIMAGE_ELEM(ret, x, y, 0) = grey;
			KOKI_IPLIMAGE_ELEM(ret, x, y, 1) = grey;
			KOKI_IPLIMAGE_ELEM(ret, x, y, 2) = grey;

		}//for
	}//for

	return ret;

}
