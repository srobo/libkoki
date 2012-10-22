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
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

/**
 * @file context.h
 * @brief Header file for libkoki context functions
 */

#include <glib.h>

#include "logger.h"

/**
 * @brief a libkoki context structure
 */
typedef struct {
	logger_callbacks_t logger; /**< the logger callbacks */
	void *logger_userdata;	   /**< the userdata to pass to the logger callbacks */
} koki_t;

koki_t* koki_new( void );

void koki_set_logger( koki_t* koki, const logger_callbacks_t *logger, void* userdata );

void koki_destroy( koki_t* koki );

void koki_log( koki_t* koki, const char* text, IplImage* img );

gboolean koki_is_logging( koki_t* koki );

#endif	/* _CONTEXT_H_ */
