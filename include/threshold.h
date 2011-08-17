#ifndef _KOKI_THRESHOLD_H_
#define _KOKI_THRESHOLD_H_

/**
 * @file  threshold.h
 * @brief Header file for thresholding related activities
 */

#include <stdint.h>
#include <cv.h>

bool koki_threshold_rgb_pixel(IplImage *frame, uint16_t x, uint16_t y,
			      uint16_t threshold);

IplImage* koki_threshold_frame(IplImage *frame, uint16_t threshold);

#endif /* _KOKI_THRESHOLD_H_ */
