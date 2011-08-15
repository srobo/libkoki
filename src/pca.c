
/**
 * @file  pca.c
 * @brief Implementation of Principal Component Analysis (PCA) functionality
 */

#include <glib.h>
#include <cv.h>
#include <stdint.h>

#include "points.h"

#include "pca.h"


/**
 * @brief performs Principal Component Analysis on a list of \c koki_point2Di_t
 *
 * @param start          the start of the contour chain to use
 * @param end            the end of the contour chain to use (NULL is
 *                       essentially the end)
 * @param eigen_vectors  the array that will have the eigen vectors written to
 * @param eigen values   the array that will have \c eigen_vector's corresponding
 *                       eigen values written to
 */
void koki_perform_pca(GSList *start, GSList *end,
		      koki_point2Df_t eigen_vectors[2],
		      float eigen_values[2], koki_point2Df_t *averages)
{

	CvMat **data       = malloc(sizeof(CvMat));
	CvMat *covar       = cvCreateMat(2, 2, CV_64FC1);
	CvMat *avg         = cvCreateMat(2, 1, CV_64FC1);
	CvMat *eigen_vals  = cvCreateMat(2, 1, CV_64FC1);
	CvMat *eigen_vects = cvCreateMat(2, 2, CV_64FC1);

	uint16_t len = 1;
	GSList *l;
	koki_point2Di_t *p;

	assert (start != NULL);

	/* zero matrices */
	cvSetZero(covar);
	cvSetZero(avg);
	cvSetZero(eigen_vals);
	cvSetZero(eigen_vects);

	/* work out how many elements we're dealing with */
	l = start;
	while (l != end && l != NULL && l->next != NULL){
		l = l->next;
		len++;
	}

	assert(len >= 2);

	/* create and fill the data matrix */
	data[0] = cvCreateMat(2, len, CV_64FC1);

	l = start;
	for (uint16_t i = 0; i < len; i++, l = l->next){
		p = l->data;
		cvmSet(data[0], 0, i, p->x);
		cvmSet(data[0], 1, i, p->y);
	}

	/* calculate the covariance matrix */
	cvCalcCovarMatrix((const CvArr **)data, 1, covar, avg,
			  CV_COVAR_NORMAL | CV_COVAR_SCALE | CV_COVAR_COLS);

	/* calculate eigen vectors and values */
	cvEigenVV(covar, eigen_vects, eigen_vals, 0, -1, -1);

	/* a row of eigen_vects is an eigen vector */
	eigen_vectors[0].x = cvmGet(eigen_vects, 0, 0);
	eigen_vectors[0].y = cvmGet(eigen_vects, 0, 1);
	eigen_vectors[1].x = cvmGet(eigen_vects, 1, 0);
	eigen_vectors[1].y = cvmGet(eigen_vects, 1, 1);

	/* get eigen vals */
	eigen_values[0] = cvmGet(eigen_vals, 0, 0);
	eigen_values[1] = cvmGet(eigen_vals, 1, 0);

	/* get averages */
	averages->x = cvmGet(avg, 0, 0);
	averages->y = cvmGet(avg, 1, 0);

	/* clean up */
	cvReleaseMat(&data[0]);
	free(data);
	cvReleaseMat(&covar);
	cvReleaseMat(&avg);
	cvReleaseMat(&eigen_vects);
	cvReleaseMat(&eigen_vals);

}
