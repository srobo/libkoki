
/**
 * @file  marker.c
 * @brief Implementation of marker related things
 */

#include <stdint.h>

#include "quad.h"

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
