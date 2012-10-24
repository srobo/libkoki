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

#include "logger.h"

/**
 * @brief initialisation function for the null logger
 *
 * The null logger just throws away log messages.
 */
static void null_init( void* userdata )
{
	/* null logger does nothing */
}

/**
 * @brief log message function for the null logger
 */
static void null_log( const char* text, IplImage *img, void* userdata )
{
	/* null logger does nothing */
}

/**
 * @brief the logger callbacks for the null logger
 */
const logger_callbacks_t koki_null_logger = {
	.init = null_init,
	.log = null_log,
};
