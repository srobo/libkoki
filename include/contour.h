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
			       label_t region);

void koki_contour_free(GSList *contour);

void koki_contour_draw(IplImage *frame, GSList *contour);

#endif /* _KOKI_CONTOUR_H_ */
