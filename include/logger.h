/* Copyright 2012 Rob Spanton

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
#ifndef _LOGGER_H_
#define _LOGGER_H_

/**
 * @file logger.h
 * @brief Header file for libkoki logger types
 */

#include <cv.h>

/**
 * @brief a structure to contain function pointers for a logger
  */
typedef struct {

	void (*init) ( void* userdata ); /**< initialisation function */

	void (*log) ( const char* text,
		      IplImage *img,
		      void* userdata ); /**< log event function: text or img can be NULL */
} logger_callbacks_t;

extern const logger_callbacks_t koki_null_logger;

#endif	/* _LOGGER_H_ */
