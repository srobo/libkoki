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

/**
 * @file  v4l.c
 * @brief Implemenatation for interacting with cameras using V4L2
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h> /* for videodev2.h */
#include <linux/videodev2.h>
#include <cv.h>

#include "labelling.h" /* for KOKI_IPLIMAGE_ELEM */

#include "v4l.h"

#define CLEAR(x) memset(&(x), 0, sizeof((x)))



/**
 * @brief opens a camera device and returns its file descriptor
 *
 * @param filename  the file name (e.g. '/dev/video0') of the camera
 * @return          the file dscriptor on successful opening, -1 otherwise
 */
int koki_v4l_open_cam(const char* filename)
{

	int fd;

	assert(filename != NULL);

	fd = open(filename, O_RDWR, 0);
	if (fd == -1){
		fprintf(stderr, "couldn't open cam\n");
	}

	return fd;

}



/**
 * @brief closes the camera
 *
 * @param fd  the camera's open file descriptor
 */
void koki_v4l_close_cam(int fd)
{

	close(fd);

}



/**
 * @brief gets the camera's current format (size, colour format, etc...)
 *
 * @param fd  the camera's file descriptor
 * @return    a filled V4L2's format structure
 */
struct v4l2_format koki_v4l_get_format(int fd)
{

	struct v4l2_format fmt;
	int ret;

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ret = ioctl(fd, VIDIOC_G_FMT, &fmt);

	assert(ret >= 0);

	return fmt;

}



/**
 * @brief outputs to \c stdout a format structure
 *
 * @param fmt  the V4L2 format structure
 */
void koki_v4l_print_format(struct v4l2_format fmt)
{
	printf("Camera Format:\n");

	printf("  bufer type: %d\n", fmt.type);

	printf("  dimensions: %d x %d\n",
	       fmt.fmt.pix.width, fmt.fmt.pix.height);

#define pix_fmt fmt.fmt.pix.pixelformat
	printf("  colour format: %c%c%c%c\n",
	        pix_fmt        & 0xFF,
	       (pix_fmt >> 8)  & 0xFF,
	       (pix_fmt >> 16) & 0xFF,
	       (pix_fmt >> 24) & 0xFF);
#undef pix_fmt

	printf("  field: %d\n", fmt.fmt.pix.field);

	printf("  bytes per line: %d\n", fmt.fmt.pix.bytesperline);

}



/**
 * @brief returns a YUYV format structure ready to send to the camera
 *
 * @param w  the desired width of the image
 * @param h  the desired height of the image
 * @return   a YUYV V4L2 format structure
 */
struct v4l2_format koki_v4l_create_YUYV_format(unsigned int w, unsigned int h)
{

	struct v4l2_format fmt;

	CLEAR(fmt);

	fmt.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width        = w;
	fmt.fmt.pix.height       = h;
	fmt.fmt.pix.pixelformat  = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.bytesperline = 2 * w;
	fmt.fmt.pix.sizeimage    = 2 * w * h;

	return fmt;

}



/**
 * @brief tries to set the format of the camera to the format specified
 *
 * @param fd  the camera's file descriptor
 * @param fmt the format to set
 * @return    a negative value on failure
 */
int koki_v4l_set_format(int fd, struct v4l2_format fmt)
{

	int ret;

	ret = ioctl(fd, VIDIOC_S_FMT, &fmt);
	if (ret < 0){
		fprintf(stderr, "unable to set format\n");
	}

	return ret;

}



/**
 * @brief returns the camera's capabilities (e.g. driver, streaming, etc...)
 *
 * @param fd  the camera's file descriptor
 * @return    a filled V4L2 capabilities structure for the camera
 */
struct v4l2_capability koki_v4l_get_capability(int fd)
{

	struct v4l2_capability cap;
	int ret;

	CLEAR(cap);

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0){
		fprintf(stderr, "couldn't get capabilities\n");
	}

	return cap;

}



/**
 * @brief prints to \c stdout a V4L2 capabilities structure
 *
 * @param cap  a V4L2 capability structure to output
 */
void koki_v4l_print_capability(struct v4l2_capability cap)
{

	printf("Capability:\n");
	printf("  driver: %s\n", cap.driver);
	printf("  card: %s\n", cap.card);
	printf("  version: %X\n", cap.version);
	printf("  capabilities: %X\n", cap.capabilities);
	printf("   - streaming?  %s\n",
	       cap.capabilities & V4L2_CAP_STREAMING ? "YES" : "NO");

}



/**
 * @brief returns the control value for the specified V4L2 control ID
 *
 * @param fd  the camera's file descriptor
 * @param id  the V4L2 control ID
 * @return    the value associated with the given control value
 */
int koki_v4l_get_control(int fd, unsigned int id)
{

	struct v4l2_control ctrl;
	int ret;

	CLEAR(ctrl);

	ctrl.id = id;

	ret = ioctl(fd, VIDIOC_G_CTRL, &ctrl);
	if (ret < 0){
		fprintf(stderr, "unable to get control\n");
	}

	return ctrl.value;

}


/**
 * @brief sets the V4L2 control ID as the given value
 *
 * @param fd     the camera's file descriptor
 * @param id     the V4L2 control ID to modify
 * @param value  the new value to set the control to
 */
int koki_v4l_set_control(int fd, unsigned int id, unsigned int value)
{

	struct v4l2_control ctrl;
	int ret;

	CLEAR(ctrl);

	ctrl.id = id;
	ctrl.value = value;

	ret = ioctl(fd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0){
		fprintf(stderr, "unable to set control\n");
	}

	return ret;

}



/**
 * @brief allocates all of the buffers required for memory mapped IO with
 *        the camera
 *
 * @param fd     the camera's file descriptor
 * @param count  the number of buffers to allocate (and where to store the
 *               number of actually allocated buffers)
 * @return       an array of \c koki_buffer_ts of length \c count (note that
 *               \c count may have changed after this function returns)
 */
koki_buffer_t* koki_v4l_prepare_buffers(int fd, int *count)
{

	struct v4l2_requestbuffers reqbuf;
	struct v4l2_buffer buffer;
	int ret;
	koki_buffer_t *buffers;

	CLEAR(reqbuf);

	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = *count;

	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);

	if (ret < 0){
		fprintf(stderr, "couldn't request buffers\n");
		*count = 0;
		return NULL;
	}

	*count = reqbuf.count;

	buffers = calloc(*count, sizeof(koki_buffer_t));
	if (buffers == NULL){
		fprintf(stderr, "couldn't allocate buffer pointers\n");
		return NULL;
	}

	for (int i=0; i < *count; i++){

		CLEAR(buffer);

		buffer.type = reqbuf.type;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;

		ret = ioctl(fd, VIDIOC_QUERYBUF, &buffer);
		if (ret < 0){
			fprintf(stderr, "querybuf request failure\n");
			free(buffers);
			return NULL;
		}

		buffers[i].length = buffer.length;
		buffers[i].start = mmap(NULL, buffer.length,
					PROT_READ | PROT_WRITE,
					MAP_SHARED, fd,
					buffer.m.offset);

		if (buffers[i].start == MAP_FAILED){
			fprintf(stderr, "map failed\n");
			free(buffers);
			return NULL;
		}

		memset(buffers[i].start, 0x41, buffers[i].length);

	}

	return buffers;

}



/**
 * @brief releases the mapped memory
 *
 * @oaram buffers  the array of \c koki_buffer_t that has been allocated
 * @param count    the length of the buffers array, i.e. the number of
 *                 actually allocated buffers (may not be the same as what
 *                 was requested)
 */
void koki_v4l_free_buffers(koki_buffer_t *buffers, int count)
{

	for (int i=0; i<count; i++)
		munmap(buffers[i].start, buffers[i].length);

	free(buffers);

}



/**
 * @brief tells the camera to start streaming so that frames can be grabbed
 *
 * @param fd  the camera's file descriptor
 * @return    a negative value on failure
 */
int koki_v4l_start_stream(int fd)
{

	int type, ret;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fd, VIDIOC_STREAMON, &type);
	if (ret < 0){
		fprintf(stderr, "failed to start stream\n");
	}

	return ret;

}



/**
 * @brief tells the camera to stop stremaing
 *
 * This shoud be done before the camera is closed.
 *
 * @param fd  the camera's file descriptor
 * @return    a negative number on failure
 */
int koki_v4l_stop_stream(int fd)
{

	int type, ret;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	ret = ioctl(fd, VIDIOC_STREAMOFF, &type);
	if (ret < 0){
		fprintf(stderr, "failed to stop stream\n");
	}

	return ret;

}



/**
 * @brief grabs a frame from the camera
 *
 * @param fd       the camera's file descriptor
 * @param buffers  the already allocated buffers structure
 * @return         a pointer to the image data array
 */
uint8_t* koki_v4l_get_frame_array(int fd, koki_buffer_t *buffers)
{

	struct v4l2_buffer buffer;
	int ret;

	assert(buffers != NULL);

	CLEAR(buffer);

	buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buffer.memory = V4L2_MEMORY_MMAP;
	buffer.index = 0;

	/* queue */
	ret = ioctl(fd, VIDIOC_QBUF, &buffer);
	if (ret < 0){
		fprintf(stderr, "failed to queue buffer\n");
		return NULL;
	}

	/* dequeue */
	ret = ioctl(fd, VIDIOC_DQBUF, &buffer);
	if (ret < 0){
		fprintf(stderr, "failed to dequeue buffer\n");
		return NULL;
	}

	return buffers[0].start;

}


/**
 * @brief recovers the Y, U and V values from a YUYV data array
 */
#define GET_YUV(yuyv_frame, x, y, w, h, Y, U, V) do {	\
		uint8_t *tmp;				\
		tmp = &yuyv_frame[((w) * 2 * (y)) +	\
				 (((x) & ~1) * 2)];	\
		U = tmp[1];				\
		V = tmp[3];				\
		Y = ((x) & 1) ? tmp[2] : tmp[0];	\
	}  while (0);					\

/**
 * @brief recovers just the Y value from a YUYV data array
 */
#define GET_Y(yuyv_frame, x, y, w, h, Y) do {		\
		uint8_t *tmp;				\
		tmp = &yuyv_frame[((w) * 2 * (y)) +	\
				  (((x) & ~1) * 2)];	\
		Y = CLIP(((x) & 1) ? tmp[2] : tmp[0]);	\
	}  while (0);					\


#ifndef MIN
#define MIN(a, b) a < b ? a : b;
#endif

#ifndef MAX
#define MAX(a, b) a > b ? a : b;
#endif

#define CLIP(x) MIN(255, MAX(0, (x)))


/**
 * @brief performs the conversion from Y, U and V to R, G and B
 */
#define YUV_TO_RGB(Y, U, V, R, G, B) do {			\
		int32_t c, d, e;				\
		c = Y - 16;					\
		d = U - 128;					\
		e = V - 128;					\
		R = CLIP((298*c + 409*e + 128) >> 8);		\
		G = CLIP((298*c - 100*d - 208*e + 128) >> 8);	\
		B = CLIP((298*c + 516*d + 128) >> 8);		\
	} while (0);						\



/**
 * @brief creates an RGB \c IplImage from a YUYV image data array
 *
 * @param frame  the YUYV image data, as recovered by
 *               \c koki_v4l_get_frame_array()
 * @param w      the image width
 * @param h      the image hieght
 * @return       an RGB \c IplImage of the data array
 */
IplImage *koki_v4l_YUYV_frame_to_RGB_image(uint8_t *frame,
					   uint16_t w, uint16_t h)
{

	IplImage *output;

	assert(frame != NULL);

	output = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 3);

	assert(output != NULL);

	for (uint16_t y=0; y<h; y++){
		for (uint16_t x=0; x<w; x++){

			uint8_t Y, U, V, r, g, b;
			GET_YUV(frame, x, y, w, h, Y, U, V);
			YUV_TO_RGB(Y, U, V, r, g, b);

			KOKI_IPLIMAGE_ELEM(output, x, y, 2) = r;
			KOKI_IPLIMAGE_ELEM(output, x, y, 1) = g;
			KOKI_IPLIMAGE_ELEM(output, x, y, 0) = b;

		}//for
	}//for

	return output;

}



/**
 * @brief creates a grayscale \c IplImage from a YUYV image data array
 *
 * The Y value is effectively the grayscale value, so that's used.
 *
 * @param frame  the YUYV image data, as recovered by
 *               \c koki_v4l_get_frame_array()
 * @param w      the image width
 * @param h      the image hieght
 * @return       a grayscale \c IplImage of the data array
 */
IplImage *koki_v4l_YUYV_frame_to_grayscale_image(uint8_t *frame,
						 uint16_t w, uint16_t h)
{

	IplImage *output;

	assert(frame != NULL);

	output = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);

	assert(output != NULL);

	for (uint16_t y=0; y<h; y++){
		for (uint16_t x=0; x<w; x++){

			uint8_t Y;
			GET_Y(frame, x, y, w, h, Y);
			((uint8_t*)(output->imageData + output->widthStep*y))[x] = Y;

		}//for
	}//for

	return output;

}
