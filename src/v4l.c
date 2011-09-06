
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

#include "v4l.h"


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

