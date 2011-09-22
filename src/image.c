
/**
 * @file  image.c
 * @brief Implementation for helpful image functions
 */

#include <cv.h>

#include "image.h"

/**
 * @brief frees an IplImage
 *
 * @param image  the \c IplImage to free
 */
void koki_image_free(IplImage *image)
{

	assert(image != NULL);
	cvReleaseImage(&image);

}
