
/**
 * @file  bearing.c
 * @brief Implementation for estimating relative bearing to a marker
 */

#include <math.h>

#include "points.h"
#include "marker.h"

#include "bearing.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * @brief calculates the relative bearing (from the z-axis, i.e. the direction
 *        the camera is pointing) to the point specified
 *
 * Rotations are anti-clockwise about the given axis:
 *
 *    positive x => marker centre is above the principle point of the camera
 *    negative x => marker centre is below the principle point of the camer
 *    positive y => marker centre is left of the principle point of the camera
 *    negative y => marker centre is right of the principle point of the camera
 *             z => (currently unused)
 *
 * @param point  the 3D point to get the relative bearing to
 * @return       the bearing to the point, relative to the camera's direction
 */
koki_bearing_t koki_bearing_estimate_point(koki_point3Df_t point)
{

	koki_bearing_t bearing;
	float r;

	bearing.y = atan2(point.x, point.z);

	r = sqrt(pow(point.x, 2) + pow(point.y, 2) + pow(point.z, 2));
	bearing.x = asin(point.y / r);

	bearing.z = 0; /* not used (yet) */

	return bearing;

}



/**
 * @brief calculates the relative bearing to the centre of the given marker
 *
 * koki_bearing_estimate_point() has more details as to what the results mean.
 *
 * @param marker  the marker to calculate the relative bearing to and th store
 *                the result in
 */
void koki_bearing_estimate(koki_marker_t *marker)
{

	koki_bearing_t bearing;

	assert(marker != NULL);

	bearing = koki_bearing_estimate_point(marker->centre.world);

	bearing.x *= 180 / M_PI;
	bearing.y *= 180 / M_PI;
	bearing.z *= 180 / M_PI;

	marker->bearing = bearing;

}
