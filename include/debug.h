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
#define KOKI_DEBUG_LEVEL KOKI_LOG_NONE
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
