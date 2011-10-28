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
#ifndef _KOKI_POSE_H_
#define _KOKI_POSE_H_

/**
 * @file  pose.h
 * @brief Header file for estimating the position of markerin 3D space
 */


#include "points.h"
#include "camera.h"
#include "marker.h"

void koki_pose_estimate_arrays(koki_point2Df_t img[4],
			       koki_point3Df_t world[4],
			       float marker_width,
			       koki_camera_params_t *params);

void koki_pose_estimate(koki_marker_t *marker,
			float marker_width,
			koki_camera_params_t *params);


#endif /* _KOKI_POSE_H_ */
