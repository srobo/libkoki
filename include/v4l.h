#ifndef _KOKI_V4L_H_
#define _KOKI_V4L_H_

/**
 * @file  v4l.h
 * @brief Header file for interacting with cameras using V4L2
 */

#include <sys/time.h> /* needed by videodev2.h */
#include <linux/videodev2.h>
#include <stdint.h>


int koki_v4l_open_cam(const char* filename);

void koki_v4l_close_cam(int fd);

struct v4l2_format koki_v4l_get_format(int fd);

void koki_v4l_print_format(struct v4l2_format fmt);

int koki_v4l_set_format(int fd, struct v4l2_format fmt);

struct v4l2_format koki_v4l_create_YUYV_format(unsigned int w, unsigned int h);

struct v4l2_capability koki_v4l_get_capability(int fd);

void koki_v4l_print_capability(struct v4l2_capability cap);

#endif /* _KOKI_V4L_H_ */
