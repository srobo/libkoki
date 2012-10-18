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
/* Copyright Robert Spanton 2011 */
/**
 * @file  integral-image.c
 * @brief Routines for creating and performing operations on integral images. 
 */
#include <stdlib.h>

#include "integral-image.h"
#include "labelling.h"

/* Within this file it's useful to have this macro available */
#define ii_pix( img, x, y ) koki_integral_image_pixel( img, x, y )

/**
 * @brief Create a new integral image
 *
 * @param src		the image to create the integral image from
 * @param complete_now	whether to calculate the integral image now
 *
 * @reurn the new integral image
 */
koki_integral_image_t* koki_integral_image_new( const IplImage *src,
						bool complete_now )
{
	koki_integral_image_t *ii;

	ii = malloc( sizeof(koki_integral_image_t) );
	assert( ii != NULL );

	ii->src = src;
	ii->w = src->width;
	ii->h = src->height;
	ii->data = malloc( sizeof(uint32_t) * ii->w * ii->h );
	assert( ii->data != NULL );

	ii->complete_x = 0;
	ii->complete_y = 0;

	ii->sum = calloc( ii->w, sizeof(uint32_t) );
	assert( ii->sum != NULL );

	if( complete_now )
		koki_integral_image_advance( ii, ii->w - 1, ii->h - 1 );

	return ii;
}

/**
 * @brief Free an integral image
 *
 * @param ii	the integral image to free
 */
void koki_integral_image_free( koki_integral_image_t *ii )
{
	free( ii->data );

	if( ii->sum != NULL )
		free( ii->sum );

	free( ii );
}

/**
 * @brief Calculate the integral image value for the given pixel
 *
 * @param ii	the integral image
 * @param x	the x-coordinate
 * @param y	the y-coordinate
 */
static void update_pixel( koki_integral_image_t *ii,
			  uint16_t x, uint16_t y )
{
	uint32_t v = 0;

	/* Note that we expect the source image to be greyscale */
	ii->sum[x] += KOKI_IPLIMAGE_GS_ELEM( ii->src, x, y );

	v = ii->sum[x];

	if( x > 0 )
		v += koki_integral_image_pixel( ii, x-1, y );

	koki_integral_image_pixel( ii, x, y ) = v;
}

/**
 * @brief calculate integral image values down to the given pixel
 *  
 * @param ii		the integral image
 * @param target_x 	the x-coordinate to calculate (inclusive)
 * @param target_y 	the y-coordinate to calculate (inclusive)
 */
void koki_integral_image_advance( koki_integral_image_t *ii,
				  uint16_t target_x, uint16_t target_y )
{
	uint16_t x, y;
	assert( target_x < ii->w );
	assert( target_y < ii->h );

	/* Advance in the x-direction, but not y first */
	for( x = ii->complete_x; x <= target_x; x++ )
		for( y=0; y < ii->complete_y; y++ )
			update_pixel( ii, x, y );
	ii->complete_x = target_x + 1;

	/* Now advance in the y-direction */
	for( x=0; x < ii->complete_x; x++ )
		for( y = ii->complete_y; y <= target_y; y++ )
			update_pixel( ii, x, y );
	ii->complete_y = target_y + 1;
}

/**
 * @brief calculate the sum of the the given area using an integral image
 *
 * @param ii		the integral image
 * @param region	the region to calculate the sum of
 *
 * @return the sum
 */
uint32_t koki_integral_image_sum( const koki_integral_image_t *ii,
				  const CvRect *region )
{
	uint32_t v;
	/* Coordinates of the south-east pixel of the region */
	const uint32_t se_x = region->x + region->width - 1;
	const uint32_t se_y = region->y + region->height - 1;

	assert( region->x < ii->complete_x );
	assert( region->y < ii->complete_y );

	/* SE corner */
	v = ii_pix( ii, se_x, se_y );

	if( region->x > 0 && region->y > 0 )
		/* NW of top left corner */
		v += ii_pix( ii, region->x - 1, region->y - 1 );

	if( region->x > 0 )
		/* E of bottom left corner */
		v -= ii_pix( ii, region->x - 1, se_y );

	if( region->y > 0 )
		/* N of top right corner */
		v -= ii_pix( ii, se_x, region->y - 1 );

	return v;
}
