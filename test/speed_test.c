
#include <stdio.h>
#include <assert.h>
#include <cv.h>
#include <highgui.h>
#include <glib.h>

#include "koki.h"


int main(int argc, const char *argv[])
{

	if (argc != 3){
		printf("Usage: ./speed_test <iterations> <filename>\n");
		return 1;
	}

	const char *iters_str = argv[1];
	int iters = atoi(iters_str);
	const char *filename = argv[2];

	IplImage *frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
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
		GPtrArray *markers = koki_find_markers(frame, 0.11, &params);

		koki_markers_free(markers);

	}


	cvReleaseImage(&frame);

	return 0;


}
