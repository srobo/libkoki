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
