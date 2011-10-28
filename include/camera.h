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
#ifndef _KOKI_CAMERA_H_
#define _KOKI_CAMERA_H_

/**
 * @file  camera.h
 * @brief Header file for camera related activities
 */

#include <stdint.h>

#include "points.h"


/**
 * @brief a structure for camera related parameters
 */
typedef struct {
	koki_point2Df_t principal_point; /**< the focal centre of the image */
	koki_point2Df_t focal_length;    /**< the focal length of the camera,
					      in pixel related units (as per
					      OpenCV) */
	koki_point2Di_t size;            /**< the dimensions of the image */
} koki_camera_params_t;


#endif /* _KOKI_CAMERA_H_ */
