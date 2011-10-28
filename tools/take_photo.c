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
#include <cv.h>
#include <highgui.h>
#include <linux/videodev2.h>

#include "koki.h"

#define WIDTH  640
#define HEIGHT 480


int main(int argc, const char **argv)
{

	int fd;
	int num_buffers = 1;
	struct v4l2_format fmt;
	const char *dev;
	const char *filename;
	koki_buffer_t *buffers;
	uint8_t *frame;
	IplImage *output;

	if (argc != 3){
		printf("Usage: %s VIDEO_DEVICE FILENAME\n", argv[0]);
		return 1;
	}

	dev = argv[1];
	filename = argv[2];

	fd = koki_v4l_open_cam(dev);

	if (fd == -1){
		printf("Couldn't open camera '%s'\n", dev);
		return 1;
	}

	fmt = koki_v4l_create_YUYV_format(WIDTH, HEIGHT);
	if (koki_v4l_set_format(fd, fmt) < 0){
		printf("Unable to set format:\n");
		koki_v4l_print_format(fmt);
		return 1;
	}

	buffers = koki_v4l_prepare_buffers(fd, &num_buffers);

	if (num_buffers < 1){
		printf("Not enough buffer(s)\n");
		return 1;
	}

	if (koki_v4l_start_stream(fd) < 0){
		printf("Unable to start stream\n");
		return 1;
	}

	frame = koki_v4l_get_frame_array(fd, buffers);
	assert(frame != NULL);

	output = koki_v4l_YUYV_frame_to_RGB_image(frame, WIDTH, HEIGHT);

	cvSaveImage(filename, output, 0);

	cvReleaseImage(&output);

	koki_v4l_stop_stream(fd);
	koki_v4l_close_cam(fd);

	return 0;

}
