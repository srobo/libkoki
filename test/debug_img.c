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
#include <cv.h>
#include <highgui.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "koki.h"

#define WAIT 0

void debug(IplImage *frame);

int main(int argc, const char *argv[])
{

	if (argc != 2){
		koki_debug(KOKI_DEBUG_SEVERE, "You must pass a filename.\n");
		return 1;
	}

	const char *filename = argv[1];

	IplImage *frame = cvLoadImage(filename, CV_LOAD_IMAGE_GRAYSCALE);
	assert(frame != NULL);

	debug(frame);

	cvReleaseImage(&frame);

	return 0;

}


void debug(IplImage *frame)
{
	koki_t* koki = koki_new();

	cvNamedWindow("thresholded", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("output", CV_WINDOW_AUTOSIZE);

	IplImage *thresholded = koki_threshold_adaptive(frame, 5, 5, KOKI_ADAPTIVE_MEAN);

	assert(thresholded != NULL);
	cvShowImage("thresholded", thresholded);
	//cvWaitKey(0);

	koki_labelled_image_t *l = koki_label_image(frame, 128);

	int waited = 0;

	for (int i=0; i<l->clips->len; i++){

		if (!koki_label_useable(l, i))
			continue;

		printf("=====================================\nlabel %d\n", i);

		GSList *contour = koki_contour_find(l, i);

		koki_contour_draw(frame, contour);

		cvShowImage("output", frame);
		cvWaitKey(WAIT);
		waited = 1;

		koki_quad_t *quad = koki_quad_find_vertices(contour);

		if (quad == NULL){
			printf("Quad not found\n");
			koki_contour_free(contour);
			continue;
		}

		koki_quad_refine_vertices(quad);
		koki_quad_draw(frame, quad);

		koki_marker_t *marker;
		marker = koki_marker_new(quad);
		assert(marker != NULL);

		if (koki_marker_recover_code(koki, marker, frame)){
			printf("Marker found. Code: %d, Z rotation: %f\n",
			       marker->code, marker->rotation.z);
		} else {

			printf("Either not a marker, or failed to recover code\n");

		}

		koki_marker_free(marker);
		koki_quad_free(quad);
		koki_contour_free(contour);

		cvShowImage("output", frame);
		cvWaitKey(WAIT);
		waited = 1;

	}//for

	if (!waited)
		cvWaitKey(WAIT);

	koki_labelled_image_free(l);

	cvDestroyWindow("thresholded");
	cvDestroyWindow("output");

}
