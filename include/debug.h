/* Copyright 2011 Chris Kirkham

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
#ifndef _KOKI_DEBUG_H_
#define _KOKI_DEBUG_H_

/**
 * @file  debug.h
 * @brief Header file for debugging
 */

#include <stdio.h>

#define KOKI_DEBUG_NONE    0
#define KOKI_DEBUG_SEVERE  1
#define KOKI_DEBUG_ERROR   2
#define KOKI_DEBUG_WARNING 3
#define KOKI_DEBUG_INFO    4

#ifndef KOKI_DEBUG_LEVEL
#define KOKI_DEBUG_LEVEL KOKI_DEBUG_NONE
#endif

#ifndef KOKI_DEBUG_OUTPUT
#define KOKI_DEBUG_OUTPUT stderr
#endif

/**
 * @brief prints out to \c KOKI_DEBUG_OUTPUT (\c stderr by default) the
 *        string passed, including any parameters -- it uses \c fprintf
 *        style formatted output
 *
 * e.g. \code koki_debug(KOKI_DEBUG_ERROR, "error message (%d)\n", 123); \endcode
 *
 * @param level  the debug level this message should be visible at
 * @param ...    a formatted string, followed by any parameters the required
 *               by the string
 */
#define koki_debug(level, ...)						\
	if(level != KOKI_DEBUG_NONE && level <= KOKI_DEBUG_LEVEL){	\
		fprintf(KOKI_DEBUG_OUTPUT, "[%d]  ", level);		\
		fprintf(KOKI_DEBUG_OUTPUT, __VA_ARGS__);		\
	}

#endif /* _KOKI_DEBUG_H_ */
