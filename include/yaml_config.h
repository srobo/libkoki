#ifndef _KOKI_YAML_CONFIG_H_
#define _KOKI_YAML_CONFIG_H_

/**
 * @file  yaml_config.h
 * @brief Header file for reading camera config from a YAML file
 */

#include <stdbool.h>

#include "camera.h"


bool koki_cam_read_params(const char *filename, koki_camera_params_t *params);


#endif /* _KOKI_YAML_CONFIG_H_ */
