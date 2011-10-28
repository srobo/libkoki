/* Copyright 2011 Robert Spanton

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
#ifndef __KOKI_INTEGRAL_IMAGE_H_
#define __KOKI_INTEGRAL_IMAGE_H_

/**
 * @file  integral-image.h
 * @brief Header for integral image management and operations
 */
#include <stdbool.h>
#include <stdint.h>
#include <cv.h>

/**
 * @brief An integral image
 *
 * Integral images are useful for summing areas of pixels within an image.
 * This structure contains information about an integral image, including
 * the image itself.  In particular, this structure supports an integral
 * image being constructed gradually.
 *
 * Pixels in the integral image should be accessed using the 
 * \c koki_integral_image_pixel macro.
 */
typedef struct {
	uint32_t *data;
	uint16_t w, h;		/* The width and height of the
				 * integral image */
	const IplImage *src; /* The IplImage that this integral image represents */

	/* The pixel to the SE of the last completed pixel of the II */
	uint16_t complete_x,
		complete_y;

	/* The sum row */
	uint32_t *sum;

} koki_integral_image_t;


#define koki_integral_image_pixel( img, x, y ) \
	( (img)->data[ ((img)->w * (y)) + (x) ] )

koki_integral_image_t* koki_integral_image_new( const IplImage *src,
						bool complete_now );

void koki_integral_image_free( koki_integral_image_t *ii );

void koki_integral_image_advance( koki_integral_image_t *ii,
				  uint16_t x, uint16_t y );

uint32_t koki_integral_image_sum( const koki_integral_image_t *ii,
				  const CvRect *region );

#endif	/* __KOKI_INTEGRAL_IMAGE_H_ */
