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
	return koki;
}

/**
 * @brief destroy a libkoki context
 */
void koki_destroy( koki_t* koki )
{
	g_free( koki );
}
