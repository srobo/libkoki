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
#include <stdint.h>
#include <assert.h>
#include <glib.h>
#include <cv.h>
#include <highgui.h>
#include <stdlib.h>

#include <linux/videodev2.h>

#include "koki.h"

#define WIDTH  640
#define HEIGHT 480


int main(void)
{
	koki_t* koki = koki_new();
	koki_camera_params_t params;

	params.size.x = WIDTH;
	params.size.y = HEIGHT;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;

	int fd = koki_v4l_open_cam("/dev/video0");
	struct v4l2_format fmt = koki_v4l_create_YUYV_format(WIDTH, HEIGHT);
	koki_v4l_set_format(fd, fmt);

	int num_buffers = 1;
	koki_buffer_t *buffers;
	buffers = koki_v4l_prepare_buffers(fd, &num_buffers);

	koki_v4l_start_stream(fd);

	while (1){

		uint8_t *yuyv = koki_v4l_get_frame_array(fd, buffers);
		IplImage *frame = koki_v4l_YUYV_frame_to_grayscale_image(yuyv, WIDTH, HEIGHT);

		IplImage *thresholded;
		thresholded = koki_threshold_adaptive(frame, 5, 3,
						      KOKI_ADAPTIVE_MEAN);
		cvShowImage("thresh", thresholded);

		koki_labelled_image_t *l = koki_label_image(thresholded, 128);

		for (int i=0; i<l->clips->len; i++){

			if (!koki_label_useable(l, i))
				continue;

			GSList *contour = koki_contour_find(l, i);

			koki_quad_t *quad = koki_quad_find_vertices(contour);

			if (quad == NULL){
				koki_contour_free(contour);
				continue;
			}

			koki_contour_draw(frame, contour);
			koki_quad_refine_vertices(quad);
			koki_quad_draw(frame, quad);

			koki_marker_t *marker;
			marker = koki_marker_new(quad);

			if (koki_marker_recover_code(koki, marker, frame)){

				koki_pose_estimate(marker, 0.11, &params);
				koki_bearing_estimate(marker);

				printf("marker code: %d\n", marker->code);

			}

			koki_contour_free(contour);
			koki_quad_free(quad);
			koki_marker_free(marker);

		}//for

		cvShowImage("frame", frame);
		cvWaitKey(1);

		koki_labelled_image_free(l);
		cvReleaseImage(&thresholded);
		cvReleaseImage(&frame);

	}

	return 0;

	cvDestroyWindow("frame");
	cvDestroyWindow("thresh");

}
