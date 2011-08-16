
/**
 * @file  code_grid.c
 * @brief Implementation for code grid handling
 */

#include <stdint.h>
#include <cv.h>
#include <stdio.h>

#include "labelling.h"

#include "code_grid.h"


/**
 * @brief sets all fields in a grid to \c 0
 *
 * @param grid  the grid to zero
 */
static void zero_grid(koki_cell_t grid[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH])
{

	for (uint8_t i=0; i<KOKI_MARKER_GRID_WIDTH; i++){
		for (uint8_t j=0; j<KOKI_MARKER_GRID_WIDTH; j++){

			grid[i][j].sum = 0;
			grid[i][j].num_pixels = 0;
			grid[i][j].val = 0;

		}//for
	}//for

}



/**
 * @brief converts an \c IplImage of an unwarped marker into a square
 *        thresholded grid
 *
 * @brief unwarped_image  the square, unwarped image
 * @brief threshold       the threshold in the range \c 0-1 to apply
 * @brief grid            the grid of \c koki_cell_t to output to
 */
void koki_grid_from_IplImage(IplImage *unwarped_frame, float threshold,
			     koki_cell_t grid[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH])
{

	uint16_t thresh;
	uint8_t r, g, b;
	uint8_t cell_pixel_width;
	uint16_t x, y;
	uint16_t avg;

	/* ensure the image is square and that it can be chunked into
	   a grid without any remainder */
	assert(unwarped_frame->width == unwarped_frame->height
	       && unwarped_frame->width % KOKI_MARKER_GRID_WIDTH == 0);

	cell_pixel_width = unwarped_frame->width / KOKI_MARKER_GRID_WIDTH;

	/* calculate int threshold */
	assert(threshold <= 1.0 && threshold >= 0.0);
	thresh = threshold * 255 * 3;

	/* clear the grid */
	zero_grid(grid);

	for (uint8_t row=0; row<KOKI_MARKER_GRID_WIDTH; row++){
		for (uint8_t col=0; col<KOKI_MARKER_GRID_WIDTH; col++){

			for (uint8_t i=0; i<cell_pixel_width; i++){
				for (uint8_t j=0; j<cell_pixel_width; j++){

					x = col * cell_pixel_width + i;
					y = row * cell_pixel_width + j;

					r = KOKI_IPLIMAGE_ELEM(unwarped_frame,
							       x, y, 0);

					g = KOKI_IPLIMAGE_ELEM(unwarped_frame,
							       x, y, 1);

					b = KOKI_IPLIMAGE_ELEM(unwarped_frame,
							       x, y, 2);

					grid[row][col].sum += r + g + b;
					grid[row][col].num_pixels++;

				}//for j
			}//for i

			/* threshold the cell */
			avg = grid[row][col].sum / grid[row][col].num_pixels;
			grid[row][col].val = avg > thresh ? 1 : 0;

		}//for col
	}//for row

}



/**
 * @brief prints a grid to stdout
 *
 * @param the grid
 */
void koki_grid_print(koki_cell_t grid[KOKI_MARKER_GRID_WIDTH][KOKI_MARKER_GRID_WIDTH])
{

	printf("+                      +\n");

	for (uint8_t i=0; i<KOKI_MARKER_GRID_WIDTH; i++){

		printf("  ");

		for (uint8_t j=0; j<KOKI_MARKER_GRID_WIDTH; j++){

			printf(grid[i][j].val == 0 ? "# " : "  ");

		}

		printf("\n");
	}

	printf("+                      +\n");

}
