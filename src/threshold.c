
/**
 * @file  threshold.c
 * @brief Implementation for thresholding related activities
 */

#include <stdint.h>
#include <stdbool.h>
#include <cv.h>

#include "labelling.h"

#include "threshold.h"


#define KOKI_RGB_SUM(frame, x, y)					\
	( KOKI_IPLIMAGE_ELEM(frame, x, y, 0) +				\
	  KOKI_IPLIMAGE_ELEM(frame, x, y, 1) +				\
	  KOKI_IPLIMAGE_ELEM(frame, x, y, 2) )				\

#define KOKI_THRESHOLD_LOWER_BOUND  60
#define KOKI_THRESHOLD_UPPER_BOUND 160
#define KOKI_THRESHOLD_INCREMENT     1

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

	return KOKI_RGB_SUM(frame, x, y) > threshold * 3;

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



/**
 * @brief performs thresholding with a given threshold and generates averages
 *        of the greyscale values of each pixel in each class (i.e. black or
 *        white)
 *
 * @param frame      the \c IplImage to threshold
 * @param threshold  the threshold to apply (in range \c 0-255)
 * @param avg_white  a pointer to where the white average should be stored
 * @param avg_black  a pointer to where the black average should be stored
 */
static void classify_and_average(IplImage *frame, uint16_t threshold,
				 uint16_t *avg_white, uint16_t *avg_black)
{

	uint16_t num_white, num_black;
	uint32_t sum_white, sum_black;
	uint16_t threshold_x_3, rgb;
	CvSize size;

	assert(frame != NULL);
	assert(threshold >= 0 && threshold <= 255);

	sum_white = 0;
	sum_black = 0;
	num_white = 0;
	num_black = 0;

	threshold_x_3 = threshold * 3;

	size = cvGetSize(frame);

	for (uint16_t y=0; y<size.height; y++){
		for (uint16_t x=0; x<size.width; x++){

			rgb = KOKI_RGB_SUM(frame, x, y);

			if (rgb >= threshold_x_3){
				sum_white += rgb;
				num_white++;
			} else {
				sum_black += rgb;
				num_black++;
			}

		}//for
	}//for

	*avg_white = 255;
	*avg_black = 0;

	if (num_white != 0)
		*avg_white = (sum_white / 3) / num_white;

	if (num_black != 0)
		*avg_black = (sum_black / 3) / num_black;

}



/**
 * @brief performs a linear search for the threshold that is equdistant to
 *        the average thresholded white level and the average thresholded
 *        black level as a way of finding a good threshold
 *
 * Starting at some initial threshold, the greyscale value of the pixels that
 * are thresholded white are averaged, and the same for the thresholded black
 * pixels.  The aim of this algorithm is to find the threshold that sits in
 * the middle of the two averages (computed using that threshold).
 *
 * The test threshold sweeps from some minimum threshold, up to some maximum
 * threshold.  For each threshold, both the average white value and the average
 * black value are calculated.  The algorithm stops when threshold is
 * approximately equidistant to both averages, i.e. the threshold is
 * approximately equal to the average of the average white and average black
 * values.  The search stops when:
 *
 * /code threshold >= (avg_white + avg_black)/2 \end_code
 *
 * @param frame  the \c IplImage to threshold
 * @return       the threshold, in the range \c 0-255
 */
uint16_t koki_threshold_auto(IplImage *frame)
{

	uint16_t avg_black, avg_white, threshold;

	assert(frame != NULL);

	avg_black = avg_white = 256;
	threshold = KOKI_THRESHOLD_LOWER_BOUND - KOKI_THRESHOLD_INCREMENT;

	while (threshold < (avg_black + avg_white)/2 &&
	       threshold < KOKI_THRESHOLD_UPPER_BOUND){

		threshold += KOKI_THRESHOLD_INCREMENT;
		classify_and_average(frame, threshold, &avg_white, &avg_black);

	}//while

	return threshold;

}
