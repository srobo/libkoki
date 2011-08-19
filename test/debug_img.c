

#include <stdio.h>
#include <cv.h>
#include <highgui.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "koki.h"

#define WAIT 1

void debug(IplImage *frame, uint16_t thresh);

int main(int argc, const char *argv[])
{

	if (argc != 2){
		koki_debug(KOKI_DEBUG_SEVERE, "You must pass a filename.\n");
		return 1;
	}

	const char *filename = argv[1];

	IplImage *frame = cvLoadImage(filename, CV_LOAD_IMAGE_COLOR);
	assert(frame != NULL);

	debug(frame, (uint16_t)(0.3 * 255));

	cvReleaseImage(&frame);

	return 0;

}


void debug(IplImage *frame, uint16_t thresh)
{

	cvNamedWindow("thresholded", CV_WINDOW_AUTOSIZE);
	cvNamedWindow("output", CV_WINDOW_AUTOSIZE);

	IplImage *thresholded = koki_threshold_frame(frame, thresh);
	assert(thresholded != NULL);
	cvShowImage("thresholded", thresholded);
	cvReleaseImage(&thresholded);

	koki_labelled_image_t *l = koki_label_image(frame, thresh / 255.0);

	int waited = 0;

	for (int i=0; i<l->clips->len; i++){

		if (!koki_label_useable(l, i)){
			//printf("=====================================\nlabel %d\n", i);
			//printf("Label considered unuseable\n");
			continue;
		}

		printf("=====================================\nlabel %d\n", i);

		GSList *contour = koki_contour_find(l, i);

		koki_contour_draw_on_frame(frame, contour);

		cvShowImage("output", frame);
		cvWaitKey(WAIT);
		waited = 1;

		koki_quad_t *quad = koki_quad_find_vertices(contour);

		if (quad == NULL){
			printf("Quad not found\n");
			koki_contour_free(contour);
			continue;
		}

		//koki_quad_draw_on_frame(frame, quad);
		koki_quad_refine_vertices(quad);
		koki_quad_draw_on_frame(frame, quad);

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
