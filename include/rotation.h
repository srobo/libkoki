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
#ifndef _KOKI_ROTATION_H_
#define _KOKI_ROTATION_H_

/**
 * @file  rotation.h
 * @brief Header file for rotation estimation
 */

#include "points.h"
#include "marker.h"


koki_marker_rotation_t koki_rotation_estimate_array(koki_point3Df_t points[4]);

void koki_rotation_estimate(koki_marker_t *marker);

#endif /* _KOKI_ROTATION_H_ */
