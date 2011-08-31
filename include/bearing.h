#ifndef _KOKI_BEARING_H_
#define _KOKI_BEARING_H_

/**
 * @file  bearing.h
 * @brief Header file for estimating the relative bearing to the marker
 */

#include "points.h"
#include "marker.h"


koki_bearing_t koki_bearing_estimate_point(koki_point3Df_t point);

void koki_bearing_estimate(koki_marker_t *marker);

#endif /* _KOKI_BEARING_H_ */
