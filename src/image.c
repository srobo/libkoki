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
 * @file  image.c
 * @brief Implementation for helpful image functions
 */

#include <cv.h>

#include "image.h"

/**
 * @brief frees an IplImage
 *
 * @param image  the \c IplImage to free
 */
void koki_image_free(IplImage *image)
{

	assert(image != NULL);
	cvReleaseImage(&image);

}
