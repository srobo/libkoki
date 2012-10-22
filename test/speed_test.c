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
	koki_t* koki = koki_new();

	if (argc != 3){
		printf("Usage: ./speed_test <iterations> <filename>\n");
		return 1;
	}

	const char *iters_str = argv[1];
	int iters = atoi(iters_str);
	const char *filename = argv[2];

	IplImage *frame = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
	assert(frame != NULL);

	koki_camera_params_t params;
	params.size.x = frame->width;
	params.size.y = frame->height;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;


	for (int iteration=0; iteration<iters; iteration++){

		/* get markers */
		GPtrArray *markers = koki_find_markers(koki, frame, 0.11, &params);

		koki_markers_free(markers);

	}


	cvReleaseImage(&frame);

	return 0;


}
