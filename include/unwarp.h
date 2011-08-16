#ifndef _KOKI_UNWARP_H_
#define _KOKI_UNWARP_H_

/**
 * @file  unwarp.h
 * @brief Header file for unwarping a marker
 */

#include <stdint.h>
#include <cv.h>

IplImage* koki_unwarp_marker(koki_marker_t *marker, IplImage *frame,
			     uint16_t unwarped_width);


#endif /* _KOKI_UNWARP_H_ */
