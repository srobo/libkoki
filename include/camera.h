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
