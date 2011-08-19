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

	koki_camera_params_t params;


	CvCapture *cap = cvCaptureFromCAM(0);
	cvNamedWindow("window", CV_WINDOW_AUTOSIZE);

	IplImage *frame = cvQueryFrame(cap);
	assert(frame != NULL);

	params.size.x = frame->width;
	params.size.y = frame->height;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;


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

			koki_contour_draw_on_frame(frame, contour);
			//koki_quad_draw_on_frame(frame, quad);
			koki_quad_refine_vertices(quad);
			koki_quad_draw_on_frame(frame, quad);

			koki_marker_t *marker;
			marker = koki_marker_new(quad);

			koki_pose_estimate(marker, 0.11, &params);

			printf("pose:\n");
			for (int i=0; i<4; i++)
				printf("%d: (%f, %f, %f)\n", i,
				       marker->vertices[i].world.x,
				       marker->vertices[i].world.y,
				       marker->vertices[i].world.z);

			koki_contour_free(contour);
			koki_quad_free(quad);
			koki_marker_free(marker);

		}//for

		cvShowImage("window", frame);
		cvWaitKey(1);

		koki_labelled_image_free(l);

	}

	return 0;

	cvReleaseCapture(&cap);
	cvDestroyWindow("window");

}
