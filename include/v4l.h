#ifndef _KOKI_V4L_H_
#define _KOKI_V4L_H_

/**
 * @file  v4l.h
 * @brief Header file for interacting with cameras using V4L2
 */

#include <sys/time.h> /* needed by videodev2.h */
#include <linux/videodev2.h>
#include <stdint.h>

/**
 * @brief a structure for representing a memory-mapped buffer
 */
typedef struct {
	uint8_t *start; /**< the start of the data array */
	size_t length;  /**< the length of the array */
} koki_buffer_t;

int koki_v4l_open_cam(const char* filename);

void koki_v4l_close_cam(int fd);

struct v4l2_format koki_v4l_get_format(int fd);

void koki_v4l_print_format(struct v4l2_format fmt);

int koki_v4l_set_format(int fd, struct v4l2_format fmt);

struct v4l2_format koki_v4l_create_YUYV_format(unsigned int w, unsigned int h);

struct v4l2_capability koki_v4l_get_capability(int fd);

void koki_v4l_print_capability(struct v4l2_capability cap);

int koki_v4l_get_control(int fd, unsigned int id);

int koki_v4l_set_control(int fd, unsigned int id, unsigned int value);

koki_buffer_t* koki_v4l_prepare_buffers(int fd, int *count);

int koki_v4l_start_stream(int fd);

int koki_v4l_stop_stream(int fd);

uint8_t* koki_v4l_get_frame_array(int fd, koki_buffer_t *buffers);

IplImage *koki_v4l_YUYV_frame_to_RGB_image(uint8_t *frame,
					   uint16_t w, uint16_t h);

IplImage *koki_v4l_YUYV_frame_to_grayscale_image(uint8_t *frame,
						 uint16_t w, uint16_t h);

#endif /* _KOKI_V4L_H_ */
