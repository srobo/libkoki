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

/**
 * @file  code_grid.c
 * @brief Implementation for code grid handling
 */

#include <stdint.h>
#include <cv.h>
#include <stdio.h>

#include "labelling.h"
#include "crc12.h"

#include "code_grid.h"

#include "code_table.h" /* int fwd_code_table[256]... */


#define ROT_000(grid, gw, bw, x, y)		\
	(grid->data[bw+y][bw+x].val)

#define ROT_090(grid, gw, bw, x, y)		\
	(grid->data[bw+x][bw+(gw-1)-y].val)

#define ROT_180(grid, gw, bw, x, y)			\
	(grid->data[bw+(gw-1)-y][bw+(gw-1)-x].val)

#define ROT_270(grid, gw, bw, x, y)		\
	(grid->data[bw+(gw-1)-x][bw+y].val)

#define MARKER_NUM(code) (code & 0xFF)
#define MARKER_CRC(code) ((code >> 8) & 0xFFF)


/**
 * @brief sets all fields in a grid to \c 0
 *
 * @param grid  the grid to zero
 */
static void zero_grid(koki_grid_t *grid)
{

	for (uint8_t i=0; i<KOKI_MARKER_GRID_WIDTH; i++){
		for (uint8_t j=0; j<KOKI_MARKER_GRID_WIDTH; j++){

			grid->data[i][j].sum = 0;
			grid->data[i][j].num_pixels = 0;
			grid->data[i][j].val = 0;

		}//for
	}//for

}



/**
 * @brief converts an \c IplImage of an unwarped marker into a square
 *        thresholded grid
 *
 * @brief unwarped_image  the square, unwarped image
 * @brief threshold       the threshold in the range \c 0-255 to apply
 * @brief grid            the grid to output to
 */
void koki_grid_from_image(IplImage *unwarped_frame, uint16_t threshold,
			     koki_grid_t *grid)
{

	uint8_t cell_pixel_width;
	uint16_t x, y, v;
	uint16_t avg;

	/* ensure the image is square and that it can be chunked into
	   a grid without any remainder */
	assert(unwarped_frame != NULL && unwarped_frame->nChannels == 1);
	assert(unwarped_frame->width == unwarped_frame->height
	       && unwarped_frame->width % KOKI_MARKER_GRID_WIDTH == 0);

	cell_pixel_width = unwarped_frame->width / KOKI_MARKER_GRID_WIDTH;

	/* check threshold */
	assert(threshold <= 255 && threshold >= 0);

	/* clear the grid */
	zero_grid(grid);

	for (uint8_t row=0; row<KOKI_MARKER_GRID_WIDTH; row++){
		for (uint8_t col=0; col<KOKI_MARKER_GRID_WIDTH; col++){

			for (uint8_t i=0; i<cell_pixel_width; i++){
				for (uint8_t j=0; j<cell_pixel_width; j++){

					x = col * cell_pixel_width + i;
					y = row * cell_pixel_width + j;

					v = KOKI_IPLIMAGE_GS_ELEM(unwarped_frame, x, y);

					grid->data[row][col].sum += v;
					grid->data[row][col].num_pixels++;

				}//for j
			}//for i

			/* threshold the cell */
			avg = grid->data[row][col].sum / grid->data[row][col].num_pixels;
			grid->data[row][col].val = avg > threshold ? 1 : 0;

		}//for col
	}//for row

}



/**
 * @brief prints a grid to stdout
 *
 * @param the grid
 */
void koki_grid_print(koki_grid_t *grid)
{

	printf("+                      +\n");

	for (uint8_t i=0; i<KOKI_MARKER_GRID_WIDTH; i++){

		printf("  ");

		for (uint8_t j=0; j<KOKI_MARKER_GRID_WIDTH; j++){

			printf(grid->data[i][j].val == 0 ? "# " : "  ");

		}

		printf("\n");
	}

	printf("+                      +\n");

}



/**
 * @brief creates a new \c IplImage of just the code section of the possible
 *        marker (i.e. the unwarped marker, sans the border)
 *
 * This is useful for auto-thresholding, for example.
 *
 * @param unwarped_frame  the square, unwarped frame of the possible marker
 * @return                the code portion of the input image
 */
IplImage *koki_code_sub_image(IplImage *unwarped_frame)
{

	IplImage *sub = NULL;
	CvRect rect;
	float width_proportion, centre_offset;

	assert(unwarped_frame != NULL);
	assert(unwarped_frame->width == unwarped_frame->height);

	width_proportion = (float)KOKI_CODE_GRID_WIDTH / KOKI_MARKER_GRID_WIDTH;
	centre_offset = (float)(KOKI_MARKER_GRID_WIDTH - KOKI_CODE_GRID_WIDTH) /
		2 / KOKI_MARKER_GRID_WIDTH;

	rect.width = unwarped_frame->width * width_proportion;
	rect.height = unwarped_frame->height * width_proportion;
	rect.x = unwarped_frame->width * centre_offset;
	rect.y = unwarped_frame->height * centre_offset;

	sub = cvCreateImage(cvSize(rect.width, rect.height),
			    unwarped_frame->depth,
			    unwarped_frame->nChannels);

	assert(sub != NULL);

	cvSetImageROI(unwarped_frame, rect);

	cvCopy(unwarped_frame, sub, NULL);

	cvResetImageROI(unwarped_frame);

	return sub;

}



/**
 * @brief recoveres sequences of blocks/chunks from the grid in all 4 possible
 *        rotations so that the code can be recovered
 *
 * For a single code, there are 5 blocks.  The top left of the marker
 * \c (0, 0) contains block 0 bit 0, \c (1, 0) contains block 1 bit 0, etc...
 * \c (5, 0) would contain block 0 bit 1, \c (0, 1) would contain block 1 bit
 * 1, etc...  This line-by-line continues until the last bit \c (5, 5) which
 * is left unused.
 *
 * This info is extracted for all 4 possible rotations of the grid, using the
 * macros defined above.
 *
 * @param grid   the grid to extract the rotations from
 * @param codes  the array to store the results in
 */
static void code_rotations(koki_grid_t *grid, uint8_t codes[4][5])
{

	uint8_t block_no, block_index;
	uint8_t grid_width   = KOKI_CODE_GRID_WIDTH;
	uint8_t border_width = (KOKI_MARKER_GRID_WIDTH -
				KOKI_CODE_GRID_WIDTH) / 2;

	/* zero code chunks */
	for (uint8_t i=0; i<4; i++)
		for (uint8_t j=0; j<5; j++)
			codes[i][j] = 0;

	/* extract chunks */
	for (uint8_t y=0; y<KOKI_CODE_GRID_WIDTH; y++){
		for (uint8_t x=0; x<KOKI_CODE_GRID_WIDTH; x++){

			/* because only 35 bits are used, skip the last one */
			if (y == KOKI_CODE_GRID_WIDTH-1 &&
			    x == KOKI_CODE_GRID_WIDTH-1)
				continue;

			/* work out which block it's from and the block's
			   bit offset */
			block_no    = (y * KOKI_CODE_GRID_WIDTH + x) % 5;
			block_index = (y * KOKI_CODE_GRID_WIDTH + x) / 5;

			/* rotate 000 */
			codes[0][block_no] |= ROT_000(grid, grid_width,
						      border_width,
						      x, y) << block_index;

			/* rotate 090 */
			codes[1][block_no] |= ROT_090(grid, grid_width,
						      border_width,
						      x, y) << block_index;

			/* rotate 180 */
			codes[2][block_no] |= ROT_180(grid, grid_width,
						      border_width,
						      x, y) << block_index;

			/* rotate 270 */
			codes[3][block_no] |= ROT_270(grid, grid_width,
						      border_width,
						      x, y) << block_index;

		}//for
	}//for

	/* invert the bits since black represents a
	   set bit in the marker */
	for (uint8_t i=0; i<4; i++)
		for (uint8_t j=0; j<5; j++)
			codes[i][j] = ~codes[i][j] & 0x7F;

}



/**
 * @brief computes the syndrome of the received chunk/block
 *
 * A syndrome of \c 0 means no errors, anything else means there were
 * errors which could *perhaps* be correct.  The syndrome is the 1-based
 * index for the bit to flip to possible correct the received block.
 *
 * @param r  the recieved block, as a 7x1 column matrix
 * @return   the syndrome
 */
static uint8_t hamming_syndrome(CvMat *r)
{

	uint8_t syndrome = 0;
	CvMat *z = cvCreateMat(3, 1, CV_32FC1);
	CvMat *H = cvCreateMat(3, 7, CV_32FC1);

	/* fill H */
	cvmSet(H, 0, 0, 1);
	cvmSet(H, 0, 1, 0);
	cvmSet(H, 0, 2, 1);
	cvmSet(H, 0, 3, 0);
	cvmSet(H, 0, 4, 1);
	cvmSet(H, 0, 5, 0);
	cvmSet(H, 0, 6, 1);

	cvmSet(H, 1, 0, 0);
	cvmSet(H, 1, 1, 1);
	cvmSet(H, 1, 2, 1);
	cvmSet(H, 1, 3, 0);
	cvmSet(H, 1, 4, 0);
	cvmSet(H, 1, 5, 1);
	cvmSet(H, 1, 6, 1);

	cvmSet(H, 2, 0, 0);
	cvmSet(H, 2, 1, 0);
	cvmSet(H, 2, 2, 0);
	cvmSet(H, 2, 3, 1);
	cvmSet(H, 2, 4, 1);
	cvmSet(H, 2, 5, 1);
	cvmSet(H, 2, 6, 1);

	/* calculate syndrome */
	cvMatMulAdd(H, r, NULL, z);

	/* get syndrome from bits in maxtrix */
	syndrome |= (((uint8_t)cvmGet(z, 0, 0)) & 0x1) << 0;
	syndrome |= (((uint8_t)cvmGet(z, 1, 0)) & 0x1) << 1;
	syndrome |= (((uint8_t)cvmGet(z, 2, 0)) & 0x1) << 2;

	/* clean up */
	cvReleaseMat(&z);
	cvReleaseMat(&H);

	return syndrome;

}



/**
 * @brief given a syndrome and the received block (as a 7x1 column matrix),
 *        this function corrects (flips) the erroneous bit is there is one
 *
 * @param syndrome  the syndrome, as output by \c hamming_syndrome()
 * @param r         the recieved matrix to possibly modify
 */
static void hamming_correct(uint8_t syndrome, CvMat *r)
{

	if (syndrome == 0 || syndrome > 7)
		return;

	/* might be wrong, correct and hope for the
	   best -- that's all we can do */

	/* invert the bit */
	cvmSet(r, syndrome-1, 0, ~((uint8_t)cvmGet(r, syndrome-1, 0)) & 0x1);

}



/**
 * @brief decodes, i.e. extracts the original data, from the received block
 *
 * libkoki uses Hamming(7,4), the implemetation seen here is based on the
 * Hamming(7,4) Wikipedia page:
 *
 *   http://en.wikipedia.org/wiki/Hamming(7,4)
 *
 * @param block  the block with the received data in it (7 bits)
 * @return       the decoded data nibble (4 bits) in a \c uint8_t
 */
static uint8_t hamming_decode(uint8_t block)
{

	uint8_t syndrome, data = 0;
	CvMat *r     = cvCreateMat(7, 1, CV_32FC1);
	CvMat *R_mat = cvCreateMat(4, 7, CV_32FC1);
	CvMat *pr    = cvCreateMat(4, 1, CV_32FC1);

	/* fill R */
	assert(R_mat != NULL);
	cvmSet(R_mat, 0, 0, 0);
	cvmSet(R_mat, 0, 1, 0);
	cvmSet(R_mat, 0, 2, 1);
	cvmSet(R_mat, 0, 3, 0);
	cvmSet(R_mat, 0, 4, 0);
	cvmSet(R_mat, 0, 5, 0);
	cvmSet(R_mat, 0, 6, 0);

	cvmSet(R_mat, 1, 0, 0);
	cvmSet(R_mat, 1, 1, 0);
	cvmSet(R_mat, 1, 2, 0);
	cvmSet(R_mat, 1, 3, 0);
	cvmSet(R_mat, 1, 4, 1);
	cvmSet(R_mat, 1, 5, 0);
	cvmSet(R_mat, 1, 6, 0);

	cvmSet(R_mat, 2, 0, 0);
	cvmSet(R_mat, 2, 1, 0);
	cvmSet(R_mat, 2, 2, 0);
	cvmSet(R_mat, 2, 3, 0);
	cvmSet(R_mat, 2, 4, 0);
	cvmSet(R_mat, 2, 5, 1);
	cvmSet(R_mat, 2, 6, 0);

	cvmSet(R_mat, 3, 0, 0);
	cvmSet(R_mat, 3, 1, 0);
	cvmSet(R_mat, 3, 2, 0);
	cvmSet(R_mat, 3, 3, 0);
	cvmSet(R_mat, 3, 4, 0);
	cvmSet(R_mat, 3, 5, 0);
	cvmSet(R_mat, 3, 6, 1);

	/* fill r with bits from block */
	for (uint8_t i=0; i<7; i++)
		cvmSet(r, i, 0, (block >> i) & 0x1);

	/* correct any errors -- this will break the code more if it's
	   got too many errors, but that doesn't matter */
	syndrome = hamming_syndrome(r);
	hamming_correct(syndrome, r);

	/* get data out */
	cvMatMulAdd(R_mat, r, NULL, pr);

	/* get bits from matrix */
	for (uint8_t i=0; i<4; i++)
		data |= (((uint8_t)cvmGet(pr, i, 0)) & 0x1) << i;

	/* clean up */
	cvReleaseMat(&r);
	cvReleaseMat(&R_mat);
	cvReleaseMat(&pr);

	return data;

}



/**
 * @brief discoveres if the 12-bit CRC of \c num is \c crc
 *
 * @param num  the marker number
 * @param crc  the received CRC
 * @return     \c TRUE if \c CRC12(num) equals \c crc, \c FALSE otherwise
 */
static bool crc_check(uint8_t num, uint16_t crc)
{

	/* note the 'num+1'. koki_crc12(0) == 0, which would be
	   bad bacause that'd be very common in an image. Adding
	   one alleviates this issue.  The same has been done in
	   the marker generation scripts. */
	return koki_crc12(num+1) == crc;


}



/**
 * @brief recovers the code, if there is one, from the given grid
 *
 * @param grid             the populated input grid
 * @param rotation_offset  a pointer to a \c float in which a multiple of 90
 *                         degrees will be stored, representing the number
 *                         of times the grid had to be rotated to make it 'fit'
 * @return                 the code, if the is one, \c -1 otherwise
 */
int16_t koki_code_recover_from_grid(koki_grid_t *grid, float *rotation_offset)
{

	uint8_t codes[4][5];
	uint32_t data[4];
	uint8_t marker_num;
	uint16_t marker_crc;

	assert(grid != NULL);

	/* get rotations */
	code_rotations(grid, codes);

	/* get data from chunks */
	for (uint8_t i=0; i<4; i++){

		data[i] = 0;
		for (int j=0; j<5; j++)
			data[i] |= (hamming_decode(codes[i][j]) & 0xF) << (j*4);

	}//for


	/* check CRCs and discover the marker's code */
	for (uint8_t i=0; i<4; i++){

		marker_num = MARKER_NUM(data[i]);
		marker_crc = MARKER_CRC(data[i]);

		/* is it valid? If so, return */
		if (crc_check(marker_num, marker_crc)){

			if (rotation_offset != NULL)
				*rotation_offset = 90.0 * i;

			return marker_num;

		}

	} //for

	/* no good code, return -1 to indicate this */
	return -1;

}



/**
 * @brief translates between from marker code space to user code space
 *
 * Some codes have a low hamming distance and have been ignored for
 * use in user code space.  This function performs the necessary lookup
 * to find the user code values.
 *
 * @param code  the code observed by thr camera
 * @return      the code the user expects to see
 */
int16_t koki_code_translation(int code)
{

	if (code == -1)
		return -1;

	return fwd_code_table[code];

}
