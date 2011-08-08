#ifndef _KOKI_LABELING_H_
#define _KOKI_LABELING_H_

/**
 * @file  labeling.h
 * @brief Header file for thresholding and labeling an image
 */

#include <stdint.h>
#include <glib.h>

#include "points.h"

/**
 * @brief A structure for storing the co-ordinate extremes for a given labeled
 *        region and also the size of said region.

 * In screen co-ordinates, where \c (0, 0) is the top left, \c min will be
 * the top left corner of the bounding box, and \c max the bottom right. A
 * bounding box can be thought of as the smallest rectangle which completely
 * encompases a labeled region.
 */
typedef struct {
	koki_point2Di_t min;  /**< the top left corner co-ordinates */
	koki_point2Di_t max;  /**< the bottom right corner co-ordinates */
	uint16_t mass;        /**< the number of labeled pixels in the region */
} koki_clip_region_t;



/**
 * @brief A structure representing a labeled image 
 *
 * A labeled image is a representation of an input \c IplImage with separate
 * thresholded regions identified by differing label numbers. A label of
 * \c 0 is used for thresholded white values, with labels greater than \c 0
 * for black regions.
 *
 * \c data is the array of labels representing an \c IplImage. It can be
 * indexed as follows:
 *
 * @code
 *   uint16_t label = labeled_image.data[row * labeled_image.w + col];
 * @endcode
 *
 * Indexing the \c aliases \c GArray with \c (label_no-1) will give the
 * true label number for a givel label. When two labeled regions merge,
 * their labels are aliased to the lower of the two, i.e. \c min(l1, l2).
 *
 * The \c clips GArray should be indexed in the same way, i.e. with
 * \c (label_no-1).
 */
typedef struct {
	uint16_t *data;    /**< the array of labels, organised row after row */
	uint16_t w;        /**< the width of the labeled image */
	uint16_t h;        /**< the height of the labeled image */
	GArray *clips;     /**< a \c GArray* of \c koki_clip_region_t for the
                                clip regions of each label */
	GArray *aliases;   /**< a GArray* of \c uint16_t containing the final
                                label number (see above) */
} koki_labeled_image_t;


#endif /* _KOKI_LABELING_H_ */
