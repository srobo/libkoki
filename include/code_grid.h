/* Copyright 2011 Chris Kirkham

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
#ifndef _KOKI_CODE_GRID_H_
#define _KOKI_CODE_GRID_H_

/**
 * @file  code_grid.h
 * @brief Header file for code grid handling
 */

#include <stdint.h>
#include <cv.h>


#define KOKI_CODE_GRID_WIDTH 6
#define KOKI_MARKER_GRID_WIDTH 10


/**
 * @brief a structure for counting and averaging an image of a marker into
 *        a grid
 */
typedef struct {
	uint32_t sum;        /**< the sum of \c (r+g+b) for all pixels in
                                  the cell */
	uint16_t num_pixels; /**< the number of pixels for the cell */
	uint8_t  val;        /**< the thresholded value, \c 0 or \c 1, for
                                  threshold black and threshold white,
                                  respectively */
} koki_cell_t;


/**
 * @brief a marker representing a marker grid
 */
typedef struct {
	koki_cell_t data[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH];
	/**< the 2-dimensional array of data */
} koki_grid_t;



void koki_grid_from_image(IplImage *unwarped_frame, uint16_t threshold,
			     koki_grid_t *grid);

void koki_grid_print(koki_grid_t *grid);

IplImage *koki_code_sub_image(IplImage *unwarped_frame);

int16_t koki_code_recover_from_grid(koki_grid_t *grid, float *rotation_offset);

int16_t koki_code_translation(int code);

#endif /* _KOKI_CODE_GRID_H_ */
