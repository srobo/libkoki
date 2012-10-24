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
#include <GL/glut.h>
#include <cv.h>
#include <highgui.h>
#include <assert.h>
#include <glib.h>
#include <stdint.h>

#include "koki.h"

#include <linux/videodev2.h>

static int fd;
static koki_buffer_t *buffers;
static int num_buffers = 1;
static koki_t* koki = NULL;

//static CvCapture *cap;
static koki_camera_params_t params;

#define WIDTH  1280
#define HEIGHT 1024

void reshape(int w, int h)
{

	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	glScalef(1, -1, 1);
	glTranslatef(0, -h, 0);

}


IplImage* grab_frame()
{

/*
  IplImage *f, *ret;

  f = cvQueryFrame(cap);
  assert(f != NULL);

  ret = cvCreateImage(cvGetSize(f), f->depth, f->nChannels);
  cvCopy(f, ret, NULL);

  glutPostRedisplay();

  return ret;
*/

	uint8_t *frame;
	frame = koki_v4l_get_frame_array(fd, buffers);

	glutPostRedisplay();

	return koki_v4l_YUYV_frame_to_RGB_image(frame, WIDTH, HEIGHT);


}



void draw_marker(koki_marker_t *marker)
{

	for (int i=0; i < (int)marker->rotation_offset / 90; i++){
		koki_point2Df_t tmp;
		tmp = marker->vertices[0].image;
		marker->vertices[0].image = marker->vertices[1].image;
		marker->vertices[1].image = marker->vertices[2].image;
		marker->vertices[2].image = marker->vertices[3].image;
		marker->vertices[3].image = tmp;
	}

	glColor3f(1, 1, 1);

	glBindTexture(GL_TEXTURE_2D, 2);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2i(marker->vertices[0].image.x, marker->vertices[0].image.y);

	glTexCoord2f(1, 0);
	glVertex2i(marker->vertices[1].image.x, marker->vertices[1].image.y);

	glTexCoord2f(1, 1);
	glVertex2i(marker->vertices[2].image.x, marker->vertices[2].image.y);

	glTexCoord2f(0, 1);
	glVertex2i(marker->vertices[3].image.x, marker->vertices[3].image.y);

	glEnd();

}



void display()
{

	int bg = 1;

	//glClear(GL_COLOR_BUFFER_BIT);
	//glColor3f(0, 0, 0);

	IplImage *frame = grab_frame();
	assert(frame != NULL);
	IplImage *gs = cvCreateImage(cvSize(frame->width, frame->height),
				     IPL_DEPTH_8U, 1);
	assert(gs != NULL);
	cvCvtColor(frame, gs, CV_RGB2GRAY);

	glutReshapeWindow(frame->width, frame->height);

	/* prepare frame as texture */

	glBindTexture(GL_TEXTURE_2D, bg);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame->width, frame->height,
		     0, GL_BGR, GL_UNSIGNED_BYTE, frame->imageData);

	glFlush();


	glColor3f(1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, bg);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2i(0, 0);

	glTexCoord2f(1, 0);
	glVertex2i(frame->width, 0);

	glTexCoord2f(1, 1);
	glVertex2i(frame->width, frame->height);

	glTexCoord2f(0, 1);
	glVertex2i(0, frame->height);

	glEnd();


	/* get markers */
	GPtrArray *markers = koki_find_markers(koki, gs, 0.11, &params);

	//printf("num markers: %d\n", markers->len);

	/* draw markers */
	for (int i=0; i<markers->len; i++){
		koki_marker_t *marker;
		marker = g_ptr_array_index(markers, i);
		draw_marker(marker);

		printf("Code: %d\n", marker->code);

		printf("Rotation: (%f\t%f\t%f)\n",
		       marker->rotation.x,
		       marker->rotation.y,
		       marker->rotation.z);

		printf("Centre  : (%5.2f\t%5.2f\t%5.2f)\n",
		       marker->centre.world.x,
		       marker->centre.world.y,
		       marker->centre.world.z);

		printf("Bearing: (%5.2f\t%5.2f)\n",
		       marker->bearing.x, marker->bearing.y);

		printf("Distance: %fm\n\n", marker->distance);

	}

	 koki_markers_free(markers);

	glFlush();

	cvReleaseImage(&frame);
	cvReleaseImage(&gs);
	glutSwapBuffers();

}


void create_sr_texture()
{

	IplImage *img = cvLoadImage("sr_round_flat.png", CV_LOAD_IMAGE_COLOR);
	assert(img != NULL);

	glBindTexture(GL_TEXTURE_2D, 2);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height,
		     0, GL_BGR, GL_UNSIGNED_BYTE, img->imageData);


}



int main(int argc, char **argv)
{

	/* GLUT setup */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE);
	glutCreateWindow("display");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);

	//cap = cvCaptureFromCAM(0);

	//cvSetCaptureProperty( cap, CV_CAP_PROP_FRAME_WIDTH, 640 );
	//cvSetCaptureProperty( cap, CV_CAP_PROP_FRAME_HEIGHT, 480 );

	//IplImage *frame = cvQueryFrame(cap);
	//assert(frame != NULL);

	//glutInitWindowSize(frame->width, frame->height);

	/* The libkoki context to use */
	koki = koki_new();

	fd = koki_v4l_open_cam("/dev/video0");

	struct v4l2_format fmt;

	fmt = koki_v4l_create_YUYV_format(WIDTH, HEIGHT);
	koki_v4l_set_format(fd, fmt);
	fmt = koki_v4l_get_format(fd);
	koki_v4l_print_format(fmt);

	buffers = koki_v4l_prepare_buffers(fd, &num_buffers);
	koki_v4l_free_buffers(buffers, num_buffers);
	buffers = koki_v4l_prepare_buffers(fd, &num_buffers);

	koki_v4l_start_stream(fd);

	params.size.x = WIDTH;
	params.size.y = HEIGHT;
	params.principal_point.x = params.size.x / 2;
	params.principal_point.y = params.size.y / 2;
	params.focal_length.x = 571.0;
	params.focal_length.y = 571.0;

	/* gl config */
	glEnable(GL_TEXTURE_2D);

	create_sr_texture();

	/* enter main loop */
	glutMainLoop();

	//cvReleaseCapture(&cap);

	koki_v4l_stop_stream(fd);
	koki_v4l_close_cam(fd);

	return 0;

}
