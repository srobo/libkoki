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
#include "integral-image.h"

#define KOKI_RGB_SUM(frame, x, y)					\
	( KOKI_IPLIMAGE_ELEM(frame, x, y, 0) +				\
	  KOKI_IPLIMAGE_ELEM(frame, x, y, 1) +				\
	  KOKI_IPLIMAGE_ELEM(frame, x, y, 2) )				\

#define KOKI_THRESHOLD_LOWER_BOUND  60
#define KOKI_THRESHOLD_UPPER_BOUND 160
#define KOKI_THRESHOLD_INCREMENT     1



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

	assert(frame != NULL && frame->nChannels == 1);
	assert(threshold >= 0 && threshold <= 255);

	uint8_t v;
	IplImage *ret = cvCreateImage(cvSize(frame->width, frame->height),
				      IPL_DEPTH_8U, 1);

	for (uint16_t y=0; y<frame->height; y++){
		for (uint16_t x=0; x<frame->width; x++){

			v = KOKI_IPLIMAGE_GS_ELEM(frame, x, y);
			KOKI_IPLIMAGE_GS_ELEM(ret, x, y) = v > threshold ? 255 : 0;

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
	CvSize size;
	uint8_t val;

	assert(frame != NULL);
	assert(threshold >= 0 && threshold <= 255);

	sum_white = 0;
	sum_black = 0;
	num_white = 0;
	num_black = 0;

	size = cvGetSize(frame);

	for (uint16_t y=0; y<size.height; y++){
		for (uint16_t x=0; x<size.width; x++){

			val = KOKI_IPLIMAGE_GS_ELEM(frame, x, y);

			if (val >= threshold){
				sum_white += val;
				num_white++;
			} else {
				sum_black += val;
				num_black++;
			}

		}//for
	}//for

	*avg_white = 255;
	*avg_black = 0;

	if (num_white != 0)
		*avg_white = sum_white / num_white;

	if (num_black != 0)
		*avg_black = sum_black / num_black;

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
 * @brief adaptively threshold the given pixel
 *
 * @param frame		the frame to threshold
 * @param iimg		the integral image for the frame
 * @param roi		the region to use for adaptive thresholding
 * @param x		the x-coordinate of the pixel to threshold
 * @param y		the y-coordinate of the pixel to threshold
 * @param c       	the constant to subtract from mean to use as the threshold
 *
 * @return true if the pixel exceeds the local threshold.
 */
bool koki_threshold_adaptive_pixel( const IplImage *frame,
				    const koki_integral_image_t *iimg,
				    const CvRect *roi,
				    uint16_t x, uint16_t y, int16_t c )
{
	uint16_t w, h;
	uint32_t sum;
	uint32_t cmp;

	w = roi->width;
	h = roi->height;

	/* calculate threshold */
	sum = koki_integral_image_sum( iimg, roi );

	/* The following is a rearranged version of
	      threshold = sum / (w*h);
	      if( KOKI_IPLIMAGE_GS_ELEM(frame, x, y) > (threshold-c) ) ...
	   This is re-arranged to avoid division. */

	cmp = KOKI_IPLIMAGE_GS_ELEM(frame, x, y) + c;
	cmp *= w * h;

	/* apply threshold */
	if( cmp > sum )
		return true;

	return false;
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
 * @param iimg    the integral image of the frame being thresholded
 * @param output  the 1-channel image to output the thresholded image to
 * @param x       the X co-ordinate of current pixel (probably the centre of the roi)
 * @param y       the Y co-ordinate of current pixel (probably the centre of the roi)
 * @param roi     the \c CvRect describing the regoin we're interested in
 * @param c       the constant to subtract from mean to use as the threshold
 */
static void threshold_window_mean(IplImage *frame,
				  koki_integral_image_t *iimg,
				  IplImage *output,
				  uint16_t x, uint16_t y, CvRect roi, int16_t c)
{
	uint8_t grey = 0;

	if( koki_threshold_adaptive_pixel( frame, iimg, &roi, x, y, c ) )
		grey = 255;

	KOKI_IPLIMAGE_GS_ELEM(output, x, y) = grey;

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
 * @param output  the 1-channel image to output the thresholded image to
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
			data[win_y*w + win_x] = KOKI_IPLIMAGE_GS_ELEM(frame, roi.x + win_x,
								      roi.y + win_y);

	/* sort and find threshold */
	qsort(data, data_len, sizeof(int), thresh_cmp);

	threshold = data_len % 2 != 0
		? data[data_len/2]                           /* odd  */
		: (data[data_len/2-1] + data[data_len/2])/2; /* even */


	/* apply threshold */
	grey = KOKI_IPLIMAGE_GS_ELEM(frame, x, y) > threshold - c
		? 255
		: 0;

	KOKI_IPLIMAGE_GS_ELEM(output, x, y) = grey;

}



/**
 * @brief constructs a window to look at, and calls the required thresholding
 *        function as specified by method
 *
 * @param frame        the image the threshold
 * @param iimg         the integral image of frame
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
static void threshold_window(IplImage *frame,
			     koki_integral_image_t *iimg,
			     IplImage *output,
			     uint16_t x, uint16_t y, uint16_t window_size,
			     int16_t c, uint8_t method)
{
	CvRect roi;

	koki_threshold_adaptive_calc_window( frame, &roi,
					     window_size, x, y );

	/* threshold */
	if (method == KOKI_ADAPTIVE_MEAN)
		threshold_window_mean(frame, iimg, output, x, y, roi, c);

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
	koki_integral_image_t *iimg = NULL;

	assert(frame != NULL && frame->nChannels == 1);

	/* create the integral image to accelerate window summation */
	iimg = koki_integral_image_new( frame, true );

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

			threshold_window(frame, iimg, output, x, y,
					 window_size, c, method);

		}//for
	}//for

	koki_integral_image_free( iimg );

	return output;

}

void koki_threshold_adaptive_calc_window( const IplImage *frame,
					  CvRect *win,
					  uint16_t window_size,
					  uint16_t x, uint16_t y )
{
	uint16_t width, height;
	assert(window_size % 2 == 1);

	width = frame->width;
	height = frame->height;
	assert( x < width);
	assert( y < height);

	/* identify the window - x */
	if (x >= window_size / 2 &&
	    x < (width-1) - window_size / 2){

		/* normal case, away from frame edges */
		win->x      = x - window_size/2;
		win->width  = window_size;

	} else {

		/* we're at the edge, limit roi accordingly */
		win->width = window_size / 2 + 1;
		win->x = x < window_size / 2
			? 0
			: (width-1) - window_size / 2;

	}

	/* identify the window - y */
	if (y >= window_size / 2 &&
	    y < (height-1) - window_size / 2){

		/* normal case, away from frame edges */
		win->y      = y - window_size/2;
		win->height = window_size;

	} else {

		/* we're at the edge, limit roi accordingly */
		win->height = window_size / 2 + 1;
		win->y = y < window_size / 2
			? 0
			: (height-1) - window_size / 2;

	}
}
