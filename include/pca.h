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
#ifndef _KOKI_PCA_H_
#define _KOKI_PCA_H_

/**
 * @file  pca.h
 * @brief Header file for Principal Component Analysis (PCA) functionality
 */

#include <glib.h>
#include <stdint.h>

#include "points.h"

int8_t koki_pca(GSList *start, GSList *end,
		koki_point2Df_t eigen_vectors[2],
		float eigen_values[2], koki_point2Df_t *averages);


#endif /* _KOKI_PCA_H_ */
