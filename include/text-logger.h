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
#ifndef _TEXT_LOGGER_H_
#define _TEXT_LOGGER_H_

/**
 * @file text-logger.h
 * @brief Header file for the text logger
 */

#include <glib.h>
#include <stdio.h>

#include "logger.h"

typedef struct {
	FILE* f;     /**< the file that we're logging to */
	gboolean close_on_destroy; /**< whether to close the file descriptor upon destruction  */
} koki_text_logger_t;

koki_text_logger_t* koki_text_logger_new( const char* fname );

koki_text_logger_t* koki_text_logger_new_from_stream( FILE* f );

void koki_text_logger_destroy( koki_text_logger_t* tlog );

extern const logger_callbacks_t koki_text_logger_callbacks;

#endif	/* _TEXT_LOGGER_H_ */
