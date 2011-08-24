
/**
 * @file  marker.c
 * @brief Implementation of marker related things
 */

#include <stdint.h>
#include <stdbool.h>
#include <cv.h>

#include "quad.h"
#include "code_grid.h"
#include "unwarp.h"
#include "threshold.h"

#include "marker.h"


/**
 * @brief creates a marker, copying data from a quad, and returns a pointer
 *        to said marker
 *
 * @param quad  the quad the transfer data from
 * @return      a pointer to the new marker
 */
koki_marker_t* koki_marker_new(koki_quad_t *quad)
{

	koki_marker_t *marker;
	float sum[2] = {0, 0};

	marker = malloc(sizeof(koki_marker_t));
	assert(marker != NULL);

	/* copy quad vertex values over */
	for (uint8_t i=0; i<4; i++){
		marker->vertices[i].image = quad->vertices[i];
		sum[0] += quad->vertices[i].x;
		sum[1] += quad->vertices[i].y;
	}

	/* calculate image centre */
	marker->centre.image.x = sum[0]/4;
	marker->centre.image.y = sum[1]/4;

	/* zero the rotations */
	marker->rotation.x = 0;
	marker->rotation.y = 0;
	marker->rotation.z = 0;

	return marker;

}



/**
 * @brief frees an allocated marker
 *
 * @param marker  the marker to free
 */
void koki_marker_free(koki_marker_t *marker)
{

	free(marker);

}



/**
 * @brief recovers the code from a marker, if possible
 *
 * @param marker  the marker to try and get the code for
 * @param frame   the original image, used to extract the marker's pixels from
 * @return        TRUE if a good code is found, indicating the marker structure
 *                has been changed to reflect this; FALSE if no success
 */
bool koki_marker_recover_code(koki_marker_t *marker, IplImage *frame)
{

	IplImage *unwarped, *sub;
	koki_grid_t grid;
	uint16_t grid_thresh;
	float rotation;
	int16_t code;

	assert(marker != NULL);
	assert(frame != NULL);

	/* unwarp */
	unwarped = koki_unwarp_marker(marker, frame, 50);
	assert(unwarped != NULL);

	/* get auto threshold for marker -- threshold just the pixels
	   in the code area/grid */
	sub = koki_code_sub_image(unwarped);
	assert(sub != NULL);

	grid_thresh = koki_threshold_auto(sub);
	koki_grid_from_IplImage(unwarped, grid_thresh/255.0, &grid);


	/* recover code */
	code = koki_code_recover_from_grid(&grid, &rotation);

	if (code < 0){ /* code not recovered */

		cvReleaseImage(&unwarped);
		cvReleaseImage(&sub);
		return FALSE;

	}

	/* add code to the marker */
	marker->code = code;

	/* add rotation info to the marker, with 0-360 degree angles */
	marker->rotation.z = (marker->rotation.z + rotation);
	marker->rotation.z -= marker->rotation.z >= 360 ? 360 : 0;

	return TRUE;

}
