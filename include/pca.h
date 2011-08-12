#ifndef _KOKI_PCA_H_
#define _KOKI_PCA_H_

/**
 * @file  pca.h
 * @brief Header file for Principal Component Analysis (PCA) functionality
 */

#include <glib.h>

#include "points.h"

void koki_perform_pca(GSList *start, GSList *end,
		      koki_point2Df_t eigen_vectors[2],
		      float eigen_values[2]);


#endif /* _KOKI_PCA_H_ */
