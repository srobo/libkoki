#ifndef _KOKI_CONTOUR_H_
#define _KOKI_CONTOUR_H_

#include <stdint.h>
#include <glib.h>

#include "labelling.h"

GSList* koki_contour_find(koki_labelled_image_t *labelled_image,
			       uint16_t region);

void koki_contour_free(GSList *contour);

#endif /* _KOKI_CONTOUR_H_ */
