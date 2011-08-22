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
	uint16_t sum;        /**< the sum of \c (r+g+b) for all pixels in
                                  the cell */
	uint16_t num_pixels; /**< the number of pixels for the cell */
	uint8_t  val;        /**< the thresholded value, \c 0 or \c 1, for
                                  threshold black and threshold white,
                                  respectively */
} koki_cell_t;


void koki_grid_from_IplImage(IplImage *unwarped_frame, float threshold,
			     koki_cell_t grid[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH]);

void koki_grid_print(koki_cell_t grid[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH]);

IplImage *koki_code_sub_image(IplImage *unwarped_frame);

#endif /* _KOKI_CODE_GRID_H_ */
