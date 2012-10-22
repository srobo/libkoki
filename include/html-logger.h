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
#ifndef _HTML_LOGGER_H_
#define _HTML_LOGGER_H_

/**
 * @file html-logger.h
 * @brief Header file for the HTML logger
 *
 * The HTML logger can cope with both text and images.
 */

#include <stdio.h>
#include <stdint.h>

#include "logger.h"

typedef struct {
	char* dpath;	/**< the path of the directory we're logging to */

	FILE* html;		/**< the HTML file we're writing to */
	uint32_t img_index;	/**< the number of images that we've written out already  */
} koki_html_logger_t;

koki_html_logger_t* koki_html_logger_new( const char* dir_path );

void koki_html_logger_destroy( koki_html_logger_t* hlog );

extern const logger_callbacks_t koki_html_logger_callbacks;

#endif	/* _HTML_LOGGER_H_ */
