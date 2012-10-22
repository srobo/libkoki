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
#ifndef _KOKI_LABELLING_H_
#define _KOKI_LABELLING_H_

/**
 * @file  labelling.h
 * @brief Header file for thresholding and labelling an image
 */

#include <stdint.h>
#include <stdbool.h>
#include <glib.h>
#include <cv.h>

#include "context.h"
#include "points.h"


#define R 0
#define G 1
#define B 2


/**
 * @brief a macro for getting or setting an R, G or B value of an \c IplImage
 *
 * @param img  the \c IplImage in question
 * @param x    the X co-ordinate
 * @param y    the Y co-ordinate
 * @param rgb  0, 1 or 2 for R, G or B, respectively
 * @return     the R, G or B value
 */
#define KOKI_IPLIMAGE_ELEM(img, x, y, rgb) \
	(((uint8_t*)((img)->imageData + (img)->widthStep*(y)))[(x)*3+rgb])

#define KOKI_IPLIMAGE_GS_ELEM(img, x, y) \
	(((uint8_t*)((img)->imageData + (img)->widthStep*(y)))[(x)])


/**
 * @brief a macro for getting the label in a labeled image at point (x, y)
 *
 * @param limg  the labeled image in question
 * @param x     the X co-ordinate
 * @param y     the Y co-ordinate
 * @return      the label at point (x, y)
 */
#define KOKI_LABELLED_IMAGE_LABEL(limg, x, y) \
	((limg)->data[(y+1) * ((limg)->w+2) + (x+1)])

/**
 * an enumeration for compass directions
 */
enum DIRECTION {N, NE, E, SE, S, SW, W, NW};



/**
 * @brief A structure for storing the co-ordinate extremes for a given labelled
 *        region and also the size of said region.

 * In screen co-ordinates, where \c (0, 0) is the top left, \c min will be
 * the top left corner of the bounding box, and \c max the bottom right. A
 * bounding box can be thought of as the smallest rectangle which completely
 * encompases a labelled region.
 */
typedef struct {
	koki_point2Di_t min;  /**< the top left corner co-ordinates */
	koki_point2Di_t max;  /**< the bottom right corner co-ordinates */
	uint16_t mass;        /**< the number of labelled pixels in the region */
} koki_clip_region_t;


/**
 * @brief A label number
 */
typedef uint16_t label_t;

/**
 * @brief The maximum label number
 */
#define KOKI_LABEL_MAX 0xffff

/**
 * @brief A structure representing a labelled image
 *
 * A labelled image is a representation of an input \c IplImage with separate
 * thresholded regions identified by differing label numbers. A label of
 * \c 0 is used for thresholded white values, with labels greater than \c 0
 * for black regions.
 *
 * \c data is the array of labels representing an \c IplImage. It can be
 * indexed as follows:
 *
 * @code
 *   label_t label = labelled_image.data[row * labelled_image.w + col];
 * @endcode
 *
 * Indexing the \c aliases \c GArray with \c (label_no-1) will give the
 * true label number for a givel label. When two labelled regions merge,
 * their labels are aliased to the lower of the two, i.e. \c min(l1, l2).
 *
 * The \c clips GArray should be indexed in the same way, i.e. with
 * \c (label_no-1).
 */
typedef struct {
	label_t *data;    /**< the array of labels, organised row after row */
	uint16_t w;        /**< the width of the labelled image */
	uint16_t h;        /**< the height of the labelled image */
	GArray *clips;     /**< a \c GArray* of \c koki_clip_region_t for the
                                clip regions of each label */
	GArray *aliases;   /**< a GArray* of \c label_t containing the final
                                label number (see above) */
} koki_labelled_image_t;


/* FUNCTION PROTOTYPES */

koki_labelled_image_t* koki_labelled_image_new(uint16_t w, uint16_t h);

void koki_labelled_image_free(koki_labelled_image_t *labelled_image);

koki_labelled_image_t* koki_label_image(IplImage *image, uint16_t threshold);

bool koki_label_useable(koki_labelled_image_t *labelled_image, label_t region);

IplImage* koki_labelled_image_to_image(koki_labelled_image_t *labelled_image);

label_t get_connected_label(koki_labelled_image_t *labelled_image,
				    uint16_t x, uint16_t y,
			     enum DIRECTION direction);

koki_labelled_image_t* koki_label_adaptive( koki_t *koki,
					    const IplImage *frame,
					    uint16_t window_size,
					    int16_t thresh_margin );

#endif /* _KOKI_LABELLING_H_ */
