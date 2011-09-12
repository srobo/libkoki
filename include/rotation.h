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
