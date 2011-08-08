
/**
 * @file  labeling.c
 * @brief Implemetation for thresholding and labeling images
 */

#include <stdint.h>
#include <glib.h>
#include <stdlib.h>
#include <cv.h>

#include "labeling.h"


/**
 * @brief produces a new labeled image and initialises its fields
 *
 * @param w  the width of the image to represent
 * @param h  the height of the image to represent
 * @return   a pointer to an initialised labeled image
 */
koki_labeled_image_t* koki_labeled_image_new(uint16_t w, uint16_t h)
{

	/* allocate space for a labeled image */
	koki_labeled_image_t *labeled_image;
	labeled_image = malloc(sizeof(koki_labeled_image_t));
	assert(labeled_image != NULL);

	labeled_image->w = 0;
	labeled_image->h = 0;

	/* alocate the label data array */
	uint16_t data_size = (w+2) * (h+2) * sizeof(uint16_t);
	labeled_image->data = malloc(data_size);
	assert(labeled_image->data != NULL);

	/* init a GArray for label aliases */
	labeled_image->aliases = g_array_new(FALSE,
					     TRUE,
					     sizeof(uint16_t));

	/* init a GArray for clip regions */
	labeled_image->clips = g_array_new(FALSE,
					   FALSE,
					   sizeof(koki_clip_region_t));

	/* zero the perimeter (makes life easier later
	   when looking for connected regions) */
	for (uint16_t i=0; i<w+2; i++){
		/* top row */
		labeled_image->data[i] = 0;
		/* bottom row */
		labeled_image->data[(h+1) * (w+2) + i] = 0;
	}
	for (uint16_t i=1; i<h+1; i++){
		/* left col */
		labeled_image->data[(w+2) * i] = 0;
		/* right col */
		labeled_image->data[(w+2) * i + w + 1] = 0;
	}

	return labeled_image;

}



/**
 * @brief frees a labeled image and its associated allocated memory
 *
 * @param labeled_image  the labeled image to free
 */
void koki_labeled_image_free(koki_labeled_image_t *labeled_image)
{

	free(labeled_image->data);
	g_array_free(labeled_image->aliases, TRUE);
	g_array_free(labeled_image->clips, TRUE);
	free(labeled_image);

}


