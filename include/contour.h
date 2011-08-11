#ifndef _KOKI_CONTOUR_H_
#define _KOKI_CONTOUR_H_

/**
 * @file  contour.h
 * @brief Header file for extracting contours from a labelled image
 */

#include <stdint.h>
#include <glib.h>

#include "labelling.h"

GSList* koki_contour_find(koki_labelled_image_t *labelled_image,
			       uint16_t region);

void koki_contour_free(GSList *contour);

void koki_contour_draw_on_frame(IplImage *frame, GSList *contour);

#endif /* _KOKI_CONTOUR_H_ */
