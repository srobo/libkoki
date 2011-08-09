
/**
 * @file  labelling.c
 * @brief Implemetation for thresholding and labelling images
 */

#include <stdint.h>
#include <glib.h>
#include <stdlib.h>
#include <cv.h>

#include "labelling.h"


/**
 * @brief produces a new labelled image and initialises its fields
 *
 * @param w  the width of the image to represent
 * @param h  the height of the image to represent
 * @return   a pointer to an initialised labelled image
 */
koki_labelled_image_t* koki_labelled_image_new(uint16_t w, uint16_t h)
{

	/* allocate space for a labelled image */
	koki_labelled_image_t *labelled_image;
	labelled_image = malloc(sizeof(koki_labelled_image_t));
	assert(labelled_image != NULL);

	labelled_image->w = w;
	labelled_image->h = h;

	/* alocate the label data array */
	uint16_t data_size = (w+2) * (h+2) * sizeof(uint16_t);
	labelled_image->data = malloc(data_size);
	assert(labelled_image->data != NULL);

	/* init a GArray for label aliases */
	labelled_image->aliases = g_array_new(FALSE,
					     TRUE,
					     sizeof(uint16_t));

	/* init a GArray for clip regions */
	labelled_image->clips = g_array_new(FALSE,
					   FALSE,
					   sizeof(koki_clip_region_t));

	/* zero the perimeter (makes life easier later
	   when looking for connected regions) */
	for (uint16_t i=0; i<w+2; i++){
		/* top row */
		labelled_image->data[i] = 0;
		/* bottom row */
		labelled_image->data[(h+1) * (w+2) + i] = 0;
	}
	for (uint16_t i=1; i<h+1; i++){
		/* left col */
		labelled_image->data[(w+2) * i] = 0;
		/* right col */
		labelled_image->data[(w+2) * i + w + 1] = 0;
	}

	return labelled_image;

}



/**
 * @brief frees a labelled image and its associated allocated memory
 *
 * @param labelled_image  the labelled image to free
 */
void koki_labelled_image_free(koki_labelled_image_t *labelled_image)
{

	free(labelled_image->data);
	g_array_free(labelled_image->aliases, TRUE);
	g_array_free(labelled_image->clips, TRUE);
	free(labelled_image);

}


