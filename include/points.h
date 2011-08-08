#ifndef _KOKI_POINTS_H_
#define _KOKI_POINTS_H_

/**
 * @file  points.h
 * @brief Header file for point/co-ordinate structures
 */

#include <stdint.h>

/**
 * @brief A structure for an integer-valued point in 2D space.
 */
typedef struct {
	uint16_t x;  /**< the X co-ordinate */
	uint16_t y;  /**< the Y co-ordinate */
} koki_point2Di_t;



/**
 * @brief A structure for a floating-point valued point in 2D space.
 */
typedef struct {
	float x;  /**< the X co-ordinate */
	float y;  /**< the Y co-ordinate */
} koki_point2Df_t;



/**
 * @brief A structure for a floating-point valued point in 3D space.
 */
typedef struct {
	float x;  /**< the X co-ordinate */
	float y;  /**< the Y co-ordinate */
	float z;  /**< the Z co-ordinate */
} koki_point3Df_t;

#endif /* _KOKI_POINTS_H_ */
