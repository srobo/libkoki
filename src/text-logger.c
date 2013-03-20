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
 * @file text-logger.c
 * @brief Implementation of a text-based logger
 */

#include "text-logger.h"

/**
 * @brief create a text logger from a given stream
 *
 * @param f    the stream
 * @return     a newly allocated text logger
 */
koki_text_logger_t* koki_text_logger_new_from_stream( FILE* f )
{
	koki_text_logger_t* tlog = g_malloc( sizeof(koki_text_logger_t) );

	tlog->f = f;
	tlog->close_on_destroy = FALSE;

	return tlog;
}

/**
 * @brief create a text logger to log to a given file
 *
 * @param fname   the path of the file to log to
 * @return a newly allocated text logger
 */
koki_text_logger_t* koki_text_logger_new( const char* fname )
{
	FILE* f = fopen( fname, "w" );
	koki_text_logger_t *tlog = koki_text_logger_new_from_stream( f );

	/* Close the file upon destruction, since we opened it */
	tlog->close_on_destroy = TRUE;

	return tlog;
}

/**
 * @brief free a text logger
 *
 * @param tlog  the text logger to destroy
 */
void koki_text_logger_destroy( koki_text_logger_t* tlog )
{
	if( tlog->close_on_destroy ) {
		fclose( tlog->f );
	}

	g_free( tlog );
}

/**
 * @brief the init function for the text logger
 */
static void text_log_init( void* _logger )
{
	koki_text_logger_t* tlog = _logger;

}

/**
 * @brief the log message function for the text logger
 */
static void text_log_log( const char* text,
			  IplImage *img,
			  void* _logger )
{
	koki_text_logger_t* tlog = _logger;

	if( text != NULL ) {
		fputs( text, tlog->f );
	}

	if( img != NULL ) {
		fprintf( tlog->f, "%ix%i image (text logger cannot output images...)\n",
			 img->width, img->height );
	}
}

const logger_callbacks_t koki_text_logger_callbacks = {
	.init = text_log_init,
	.log = text_log_log,
};
