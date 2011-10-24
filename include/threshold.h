#ifndef _KOKI_THRESHOLD_H_
#define _KOKI_THRESHOLD_H_

/**
 * @file  threshold.h
 * @brief Header file for thresholding related activities
 */
#include <stdbool.h>
#include <stdint.h>
#include <cv.h>

#include "integral-image.h"

#define KOKI_ADAPTIVE_MEAN   1
#define KOKI_ADAPTIVE_MEDIAN 2


bool koki_threshold_rgb_pixel(IplImage *frame, uint16_t x, uint16_t y,
			      uint16_t threshold);

IplImage* koki_threshold_frame(IplImage *frame, uint16_t threshold);

uint16_t koki_threshold_global(IplImage *frame);

IplImage* koki_threshold_adaptive(IplImage *frame, uint16_t window_size,
				  int16_t c, uint8_t method);

bool koki_threshold_adaptive_pixel( const IplImage *frame,
				    const koki_integral_image_t *iimg,
				    const CvRect *roi,
				    uint16_t x, uint16_t y, int16_t c );

#endif /* _KOKI_THRESHOLD_H_ */
