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

#include <stdio.h>
#include <assert.h>
#include <cv.h>
#include <highgui.h>
#include <glib.h>

#include "koki.h"


int main(int argc, const char *argv[])
{
	const char *filename;
	IplImage *frame;
	GPtrArray *markers;
	koki_camera_params_t params;
	koki_t *koki = koki_new();

	if (argc != 2){
		fprintf(stderr, "You must pass a filename.\n");
		return 1;
	}

	filename = argv[1];

	frame = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
	assert(frame != NULL);

	params.size.x = frame->width;
	params.size.y = frame->height;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;

/*
	if (!koki_cam_read_params("cam_640x480.yaml", &params)){
		printf("Unable to read params\n");
	}
*/
	/* get markers */
	markers = koki_find_markers(koki, frame, 0.11, &params);

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
