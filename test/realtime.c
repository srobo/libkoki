#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

#include "koki.h"


int main(void)
{

	CvCapture *cap = cvCaptureFromCAM(0);
	cvNamedWindow("window", CV_WINDOW_AUTOSIZE);


	while (1){

		IplImage *frame = cvQueryFrame(cap);
		assert(frame != NULL);

		koki_labelled_image_t *l = koki_label_image(frame, 0.3);

		for (int i=0; i<l->clips->len; i++){

			if (!koki_label_useable(l, i))
				continue;

			GSList *contour = koki_contour_find(l, i);

			koki_quad_t *quad = koki_quad_find_vertices(contour);

			if (quad == NULL){
				koki_contour_free(contour);
				continue;
			}

			koki_quad_refine_vertices(quad);

			koki_contour_draw_on_frame(frame, contour);
			koki_quad_draw_on_frame(frame, quad);

			koki_contour_free(contour);
			koki_quad_free(quad);

		}//for

		cvShowImage("window", frame);
		cvWaitKey(1);

		koki_labelled_image_free(l);

	}

	return 0;

	cvReleaseCapture(&cap);
	cvDestroyWindow("window");

}
