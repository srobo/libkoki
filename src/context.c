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

/**
 * @file context.c
 * @brief Implementation of libkoki context functions
 */

#include <glib.h>

#include "context.h"

/**
 * @brief create a libkoki context
 *
 * @return a freshly allocated libkoki context
 */
koki_t* koki_new( void )
{
	koki_t *koki = g_malloc( sizeof(koki_t) );

	/* By default, use the null logger (i.e. throw everything away) */
	koki->logger = koki_null_logger;

	return koki;
}

/**
 * @brief set the logger callbacks to use
 *
 * @param koki      the libkoki context
 * @param logger    the logger callbacks
 * @param userdata  the userdata to pass to the logger callbacks
  */
void koki_set_logger( koki_t* koki,
		      const logger_callbacks_t *logger,
		      void *userdata )
{
	g_assert( koki != NULL );
	g_assert( logger != NULL );

	koki->logger = *logger;
	koki->logger_userdata = userdata;
}

/**
 * @brief destroy a libkoki context
 */
void koki_destroy( koki_t* koki )
{
	g_free( koki );
}

/**
 * @brief send a log message out to the logger
 *
 * @param koki   the libkoki context
 * @param text   the text of the log message -- can be NULL
 * @param img    the image of the log message -- can be NULL
 */
void koki_log( koki_t* koki, const char* text, IplImage* img )
{
	koki->logger.log( text, img, koki->logger_userdata );
}

/**
 * @brief report whether the context is logging
 *
 * @return TRUE if the context is logging
 */
gboolean koki_is_logging( koki_t* koki )
{
	if( koki->logger.init == koki_null_logger.init
	    && koki->logger.log == koki_null_logger.log )
		return FALSE;

	return TRUE;
}
