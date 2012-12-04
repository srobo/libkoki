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
#ifndef _KOKI_UNWARP_H_
#define _KOKI_UNWARP_H_

/**
 * @file  unwarp.h
 * @brief Header file for unwarping a marker
 */

#include <stdint.h>
#include <cv.h>

#include "koki.h"
#include "marker.h"

IplImage* koki_unwarp_marker( koki_t* koki, koki_marker_t *marker, IplImage *frame,
			      uint16_t unwarped_width );


#endif /* _KOKI_UNWARP_H_ */
