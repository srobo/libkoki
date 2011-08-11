#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <glib.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

#include "koki.h"


/* Example use of libkoki (used as basic testing) */

void display_frame(IplImage *frame)
{
	const char *title = "Frame Window";
	cvNamedWindow(title, CV_WINDOW_AUTOSIZE);
	cvShowImage(title, frame);
	cvWaitKey(0);
	cvDestroyWindow(title);

}


int main(void)
{

	IplImage *frame = cvLoadImage("test/test.jpg", CV_LOAD_IMAGE_COLOR);
	assert(frame != NULL);

	koki_labelled_image_t *l = koki_label_image(frame, 0.5);

	for(int i=0; i<l->clips->len; i++){

		if (!koki_label_useable(l, i))
			continue;

		GSList *contour = koki_contour_find(l, i);

		koki_point2Di_t *point;
		GSList *gslist;

		gslist = contour;
		while(gslist->next != NULL){
			point = gslist->data;
			printf("(%d, %d), ", point->x, point->y);
			gslist = gslist->next;
		}
		printf("\n");

		koki_contour_draw_on_frame(frame, contour);

		koki_contour_free(contour);

	}

	/*
	IplImage *limage = koki_labelled_image_to_IplImage(l);	
	display_frame(limage);
	cvReleaseImage(&limage);
	*/

	display_frame(frame);

	koki_labelled_image_free(l);
	cvReleaseImage(&frame);

	return 0;

}
