
/**
 * @file  threshold.c
 * @brief Implementation for thresholding related activities
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <cv.h>
#include <highgui.h>

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
uint16_t koki_threshold_global(IplImage *frame)
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



/**
 * @brief sets \c output(x,y) to the thresholded value of \c frame in the region of
 *        interest specified by \c roi, using the mean as the base threshold
 *
 * For this method, the pixels in the region of interest are mean averaged and then
 * a threshold of of \c mean-c is applied to the frame, and stored in output. The
 * \c mean-c allows the image to look less cluttered. Small values, 3-7 say, are
 * just enough to prevent tiny 'features' being noticed.
 *
 * @param frame   the frame being thresholded
 * @param output  the RGB image to output the thresholded image to
 * @param x       the X co-ordinate of current pixel (probably the centre of the roi)
 * @param y       the Y co-ordinate of current pixel (probably the centre of the roi)
 * @param roi     the \c CvRect describing the regoin we're interested in
 * @param c       the constant to subtract from mean to use as the threshold
 */
static void threshold_window_mean(IplImage *frame, IplImage *output,
				  uint16_t x, uint16_t y, CvRect roi, int16_t c)
{

	uint16_t w, h, sum, threshold, grey;

	w = roi.width;
	h = roi.height;

	/* calculate threshold */
	sum = 0;
	for (uint16_t win_y = 0; win_y < h; win_y++)
		for (uint16_t win_x = 0; win_x < w; win_x++)
			sum += KOKI_RGB_SUM(frame, roi.x + win_x, roi.y + win_y);

	threshold = sum / (w * h);

	/* apply threshold */
	grey = KOKI_RGB_SUM(frame, x, y) > threshold - (c * 3)
		? 255
		: 0;

	KOKI_IPLIMAGE_ELEM(output, x, y, 0) = grey;
	KOKI_IPLIMAGE_ELEM(output, x, y, 1) = grey;
	KOKI_IPLIMAGE_ELEM(output, x, y, 2) = grey;

}



/**
 * @brief a comparison function for use with qsort() to sort pixel values in a median
 *        window
 *
 * @param a  an \c int* to an element
 * @param b  an \c int* to an element
 * @return   a positive number if a > b, 0 if they are the same, or a negative number
 *           if a < b
 */
static int thresh_cmp(const void *a, const void *b)
{

	return *((const int *)a) - *((const int *)b);

}



/**
 * @brief sets \c output(x,y) to the thresholded value of \c frame in the region of
 *        interest specified by \c roi, using the median as the base threshold
 *
 * For this method, the pixels in the region of interest are median averaged and then
 * a threshold of of \c median-c is applied to the frame, and stored in output. The
 * \c median-c allows the image to look less cluttered. Small values, 3-7 say, are
 * just enough to prevent tiny 'features' being noticed.
 *
 * @param frame   the frame being thresholded
 * @param output  the RGB image to output the thresholded image to
 * @param x       the X co-ordinate of current pixel (probably the centre of the roi)
 * @param y       the Y co-ordinate of current pixel (probably the centre of the roi)
 * @param roi     the \c CvRect describing the regoin we're interested in
 * @param c       the constant to subtract from median to use as the threshold
 */

static void threshold_window_median(IplImage *frame, IplImage *output,
				    uint16_t x, uint16_t y, CvRect roi, int16_t c)
{

	uint16_t w, h, threshold, grey, data_len;

	w = roi.width;
	h = roi.height;
	data_len = w * h;

	int data[data_len];

	/* calculate threshold */
	for (uint16_t win_y = 0; win_y < h; win_y++)
		for (uint16_t win_x = 0; win_x < w; win_x++)
			data[win_y*w + win_x] = KOKI_RGB_SUM(frame, roi.x + win_x, roi.y + win_y);

	/* sort and find threshold */
	qsort(data, data_len, sizeof(int), thresh_cmp);

	threshold = data_len % 2 != 0
		? data[data_len/2]                           /* odd  */
		: (data[data_len/2-1] + data[data_len/2])/2; /* even */


	/* apply threshold */
	grey = KOKI_RGB_SUM(frame, x, y) > threshold - (c * 3)
		? 255
		: 0;

	KOKI_IPLIMAGE_ELEM(output, x, y, 0) = grey;
	KOKI_IPLIMAGE_ELEM(output, x, y, 1) = grey;
	KOKI_IPLIMAGE_ELEM(output, x, y, 2) = grey;

}



/**
 * @brief constructs a window to look at, and calls the required thresholding
 *        function as specified by method
 *
 * @param frame        the image the threshold
 * @param output       the image to write the thresholded values to
 * @param x            the current X co-ordinate of interest
 * @param y            the current Y co-ordinate of interest
 * @param window_size  the window size to use. NOTE: window size must be an odd
 *                     number so that \c (x,y) can be in the centre of the window
 * @param c            the constant to subtract from the threshold before it's
 *                     applied to de-clutter the thresholded image
 * @param method       the thresholding method to use, one of { KOKI_ADAPTIVE_MEAN,
 *                     KOKI_ADAPTIVE_MEDIAN }
 */
static void threshold_window(IplImage *frame, IplImage *output,
			     uint16_t x, uint16_t y, uint16_t window_size,
			     int16_t c, uint8_t method)
{

	CvRect roi;
	uint16_t width, height;

	width = frame->width;
	height = frame->height;
	assert(x >= 0 && x < width);
	assert(y >= 0 && y < height);

	/* identify the window - x */
	if (x >= window_size / 2 &&
	    x < (width-1) - window_size / 2){

		/* normal case, away from frame edges */
		roi.x      = x - window_size/2;
		roi.width  = window_size;

	} else {

		/* we're at the edge, limit roi accordingly */
		roi.width = window_size / 2 + 1;
		roi.x = x < window_size / 2
			? 0
			: (width-1) - window_size / 2;

	}

	/* identify the window - y */
	if (y >= window_size / 2 &&
	    y < (height-1) - window_size / 2){

		/* normal case, away from frame edges */
		roi.y      = y - window_size/2;
		roi.height = window_size;

	} else {

		/* we're at the edge, limit roi accordingly */
		roi.height = window_size / 2 + 1;
		roi.y = y < window_size / 2
			? 0
			: (height-1) - window_size / 2;

	}

	/* threshold */
	if (method == KOKI_ADAPTIVE_MEAN)
		threshold_window_mean(frame, output, x, y, roi, c);

	else if (method == KOKI_ADAPTIVE_MEDIAN)
		threshold_window_median(frame, output, x, y, roi, c);

	else
		assert(FALSE);

}



/**
 * @brief thresholds an image in a localised, adaptive way, allowing large
 *        illumination variations to exist in the source image and still be
 *        able to thresholded well
 *
 * This method is good at finding the borders of the markers, but should not be
 * used to recover the codes, for example. Because of its adaptive nature, the
 * centre of the large black areas can end up thresholded white because of a
 * lack of variation in brightness.
 *
 * @param frame        the frame to threshold
 * @param window_size  the size of the window to use (must be odd, small is
 *                     faster and less susceptible to illumination variations)
 * @param c            a constant to subtract from the threshold before it's
 *                     applied to de-clutter the output somewhat. Good values
 *                     are small, perhaps no greater than 10.
 * @param method       the method to use when thresholding a window, choose from
 *                     { KOKI_ADAPTIVE_MEAN, KOKI_ADAPTIVE_MEDIAN }
 */
IplImage* koki_threshold_adaptive(IplImage *frame, uint16_t window_size,
				  int16_t c, uint8_t method)
{

	IplImage *output = NULL;

	assert(frame != NULL);
	assert(window_size % 2 == 1);

	/* create output image */
	output = cvCreateImage(cvGetSize(frame),
			       frame->depth,
			       frame->nChannels);

	assert(output != NULL);

	if (method == 0) /* default */
		method = KOKI_ADAPTIVE_MEAN;


	/* threshold the image */
	for (uint16_t y=0; y<frame->height; y++){
		for (uint16_t x=0; x<frame->width; x++){

			threshold_window(frame, output, x, y,
					 window_size, c, method);

		}//for
	}//for

	return output;

}
