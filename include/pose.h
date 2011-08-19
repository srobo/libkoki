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
