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

#endif /* _KOKI_V4L_H_ */
