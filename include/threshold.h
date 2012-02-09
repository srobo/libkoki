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


IplImage* koki_threshold_frame(IplImage *frame, uint16_t threshold);

uint16_t koki_threshold_global(IplImage *frame);

IplImage* koki_threshold_adaptive(IplImage *frame, uint16_t window_size,
				  int16_t c, uint8_t method);

bool koki_threshold_adaptive_pixel( const IplImage *frame,
				    const koki_integral_image_t *iimg,
				    const CvRect *roi,
				    uint16_t x, uint16_t y, int16_t c );

void koki_threshold_adaptive_calc_window( const IplImage *frame,
					  CvRect *win,
					  uint16_t width,
					  uint16_t x, uint16_t y );

#endif /* _KOKI_THRESHOLD_H_ */
