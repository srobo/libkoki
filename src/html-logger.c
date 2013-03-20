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
 * @file html-logger.c
 * @brief Implementation of the html logger
 */
#define _GNU_SOURCE

#include <errno.h>
#include <glib.h>
#include <highgui.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "html-logger.h"

koki_html_logger_t* koki_html_logger_new( const char* dir_path )
{
	koki_html_logger_t* hlog = g_malloc( sizeof( koki_html_logger_t ) );
	char *fname;

	hlog->dpath = g_strdup(dir_path);
	hlog->img_index = 0;

	if( mkdir( dir_path, 0770 ) != 0 ) {
		fprintf( stderr, "html_logger: Failed to create directory '%s': %m\n",
			 dir_path );
		exit(1);
	}

	g_assert( asprintf( &fname, "%s/log.html", dir_path ) != -1 );

	/* Open the HTML file */
	hlog->html = fopen( fname, "w" );
	if( hlog->html == NULL ) {
		fprintf( stderr, "html_logger: Failed to open '%s': %m\n",
			 fname );
		exit(1);
	}

	fprintf( hlog->html, "<html>\n<body>\n" );

	free( fname );

	return hlog;
}

void koki_html_logger_destroy( koki_html_logger_t* hlog )
{
	/* End the document */
	fprintf( hlog->html, "</body>\n</html>\n" );
	fclose( hlog->html );

	g_free( hlog->dpath );
	g_free( hlog );
}

static void html_log_init( void* _logger )
{
	koki_html_logger_t* hlog = _logger;

}

static void html_log_log( const char* text,
			  IplImage *img,
			  void* _logger )
{
	koki_html_logger_t* hlog = _logger;

	fprintf( hlog->html, "<div>\n" );

	if( img != NULL ) {
		char *fname;

		/* Generate a filename for the image */
		g_assert( asprintf( &fname, "%s/%6.6i.png",
				    hlog->dpath, hlog->img_index ) != -1 );

		/* Write the image out to that file */
		cvSaveImage( fname, img, 0 );

		/* Write an <img> tag into the HTML file */
		fprintf( hlog->html,
			 "<img src='%6.6i.png' /> ", hlog->img_index );

		/* Free the generated filename */
		free( fname );

		/* Next image needs to be a higher image */
		hlog->img_index += 1;
	}

	if( text != NULL ) {
		/* Write the text to the HTML file */
		/* TODO: turn newlines into <br/> tags */
		fputs( text, hlog->html );
	}

	fprintf( hlog->html, "</div>\n" );
}

const logger_callbacks_t koki_html_logger_callbacks = {
	.init = html_log_init,
	.log = html_log_log,
};
