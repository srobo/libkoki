
#include <stdio.h>
#include <assert.h>
#include <cv.h>
#include <highgui.h>
#include <glib.h>

#include "koki.h"


int main(int argc, const char *argv[])
{

	if (argc != 2){
		printf("You must pass a filename.\n");
		return 1;
	}

	const char *filename = argv[1];

	IplImage *frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	assert(frame != NULL);

	koki_camera_params_t params;
	params.size.x = frame->width;
	params.size.y = frame->height;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;


	/* get markers */
	GPtrArray *markers = koki_find_markers(frame, 0.11, &params);

	/* display info*/
	assert(markers != NULL);
	for (int i=0; i<markers->len; i++){

		koki_marker_t *marker;
		marker = g_ptr_array_index(markers, i);

		printf("\n(%d) Marker #%d:\n", i, marker->code);

		printf("\n\tCentre position (image, in pixels):\n\t\t(%f,\t%f)\n",
		       marker->centre.image.x,
		       marker->centre.image.y);


		printf("\n\tCentre position (world, in metres):\n\t\t(%f,\t%f,\t%f)\n",
		       marker->centre.world.x,
		       marker->centre.world.y,
		       marker->centre.world.z);

		printf("\n\tRotation about axes (world, in degrees):\n\t\t(%f,\t%f,\t%f)\n",
		       marker->rotation.x,
		       marker->rotation.y,
		       marker->rotation.z);

		printf("\n\tRelative bearing (world, in degrees):\n\t\t(%f,\t%f\t%f)\n",
		       marker->bearing.x,
		       marker->bearing.y,
		       marker->bearing.z);

		printf("\n\tVertex location (image, in pixels):\n");

		for (int i=0; i<4; i++){
			printf("\t\t(%f,\t%f)\n",
			       marker->vertices[i].image.x,
			       marker->vertices[i].image.y);
		}

		printf("\n\tVertex location (world, in metres):\n");

		for (int i=0; i<4; i++){
			printf("\t\t(%f,\t%f,\t%f)\n",
			       marker->vertices[i].world.x,
			       marker->vertices[i].world.y,
			       marker->vertices[i].world.z);
		}


	}//for

	koki_markers_free(markers);
	cvReleaseImage(&frame);

	return 0;


}
