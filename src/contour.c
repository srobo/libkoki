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

/**
 * @file  contour.c
 * @brief Implemetation for extracting contours from a labelled image
 */

#include <stdint.h>
#include <glib.h>

#include "labelling.h"
#include "points.h"

#include "contour.h"


#define KOKI_CONTOUR_RED   255
#define KOKI_CONTOUR_GREEN 0
#define KOKI_CONTOUR_BLUE  255


/**
 * @brief identifies the most extreme point on the top row of the clip region
 *
 * The clip region is the smallest rectangle that completely encloses the
 * labelled region.  This function identifies the first point, scanning
 * left, right, left, right, etc...  working towards the centre of the top
 * row of the region.
 *
 * @param labelled_image  the labelled image containing the required clip
 *                        regions
 * @param region          the index of the \c clips \c GArray in \c
 *                        labelled_image
 * @return                a pointer to the first labelled point on the top row
 */
static koki_point2Di_t* first_labeled_on_top_row(koki_labelled_image_t *labelled_image,
						 label_t region)
{

	koki_clip_region_t clip;
	koki_point2Di_t *ret = NULL, *point;
	uint16_t width;

	point = g_slice_new(koki_point2Di_t);
	assert(point != NULL);

	assert(region < labelled_image->clips->len);
	clip = g_array_index(labelled_image->clips, koki_clip_region_t, region);

	width = clip.max.x - clip.min.x + 1;

	if (width % 2 == 1) /* make it even */
		width++;


	for (uint16_t i = 0; i < width/2; i++){

		label_t label, alias;

		/* check left side */
		label = KOKI_LABELLED_IMAGE_LABEL(labelled_image,
						  clip.min.x + i,
						  clip.min.y);

		if (label != 0){

			alias = g_array_index(labelled_image->aliases,
					      label_t, label-1);

			if (alias == region+1){

				point->x = clip.min.x + i;
				point->y = clip.min.y;
				ret = point;
				break;

			}

		}// if label != 0


		/* check right side */
		label = KOKI_LABELLED_IMAGE_LABEL(labelled_image,
						  clip.max.x - i,
						  clip.min.y);

		if (label != 0){

			alias = g_array_index(labelled_image->aliases,
					      label_t, label-1);

			if (alias == region+1){

				point->x = clip.max.x - i;
				point->y = clip.min.y;
				ret = point;
				break;

			}

		}// if label != 0

	}//for

	if (ret == NULL)
		g_slice_free(koki_point2Di_t, point);

	return ret;

}



/**
 * @brief returns a 2D integer point in the direction specified
 *
 * @param current  the point to move from
 * @param dir      the direction to move in
 * @return         the point in direction \c dir from \ current
 */
static koki_point2Di_t get_point_in_direction(koki_point2Di_t current,
					      enum DIRECTION dir)
{

	koki_point2Di_t p;

	switch (dir){
	case N:  p.x = current.x;   p.y = current.y-1; break;
	case NE: p.x = current.x+1; p.y = current.y-1; break;
	case E:  p.x = current.x+1; p.y = current.y;   break;
	case SE: p.x = current.x+1; p.y = current.y+1; break;
	case S:  p.x = current.x;   p.y = current.y+1; break;
	case SW: p.x = current.x-1; p.y = current.y+1; break;
	case W:  p.x = current.x-1; p.y = current.y;   break;
	case NW: p.x = current.x-1; p.y = current.y-1; break;
	}

	return p;

}



/**
 * @brief returns the next clockwise direction from the given \c dir
 *
 * For example, @code get_next_clockwise_direction(N) == NE @endcode
 *
 * @param dir  the direction to move from
 * @return     one compass direction clockwise from \c dir (e.g. N-->NE)
 */
static enum DIRECTION get_next_clockwise_direction(enum DIRECTION dir)
{

	return (dir + 1) % 8;

}



/**
 * @brief returns the compass direction opposite \c dir
 *
 * For example, @code get_opposite_direction(N) == S @endcode
 *
 * @param dir  the direction to move from
 * @return     the direction opposite \c dir (e.g. N-->S)
 */
static enum DIRECTION get_opposite_direction(enum DIRECTION dir)
{

	return (dir + 4) % 8;

}



/**
 * @brief allocates and initialises a new \c koki_point2Di_t in memory
 *
 * @param x  the X co-ordinate of the point
 * @param y  the Y co-ordinate of the point
 * @return   a pointer to the newly allocated \c koki_point2Di_t
 */
static koki_point2Di_t* point2Di_new(uint16_t x, uint16_t y)
{

	koki_point2Di_t *p;
	p = g_slice_new(koki_point2Di_t);
	assert(p != NULL);
	p->x = x;
	p->y = y;
	return p;

}



/**
 * @brief finds the contour for a given region and returns a list
 *        of points in said contour as a \c GSList*
 *
 * @param labelled_image  the labelled image that has been labelled
 * @param region          the index to the labelled image's clip
 *                        \c GArray
 * @return                a pointer to the first GSList node
 */
GSList* koki_contour_find(koki_labelled_image_t *labelled_image,
			  label_t region)
{

	GSList *contour = NULL;
	koki_point2Di_t *first_point, current, check;

	/* get the first point in the chain */
	first_point = first_labeled_on_top_row(labelled_image, region);
	assert(first_point != NULL);

	/* prepend it (will be reversed later) */
	contour = g_slist_prepend(contour, first_point);

	enum DIRECTION dir = N;
	bool first_run = TRUE;
	label_t label;

	current = *first_point;

	while (TRUE){

		/* there are 8 possible directions */
		for (int i=0; i<8; i++){

			label = get_connected_label(labelled_image,
						    current.x,
						    current.y,
						    dir);

			check = get_point_in_direction(current, dir);

			if (label != 0){

				/* prepend it to the list (prepend
				   for speed -- no walking to end
				   necessary) */
				koki_point2Di_t *p;
				p = point2Di_new(check.x, check.y);
				contour = g_slist_prepend(contour, p);

				break;

			}

			dir = get_next_clockwise_direction(dir);

		}//for

		/* check to see if we've done a full circle */
		if (!first_run
		    && current.x == first_point->x
		    && current.y == first_point->y)
			break;

		current = check;

		/* get the direction to the previous point, then
		   advance once */
		dir = get_opposite_direction(dir);
		dir = get_next_clockwise_direction(dir);

		first_run = FALSE;

	}//while

	/* reverse the slist so it can be walked in a
	   clockwise manner */
	return g_slist_reverse(contour);

}



/**
 * @brief frees a contour \c GSList and it's associated allocated gslices
 *
 * @param contour  the GSList of a contour to free
 */
void koki_contour_free(GSList *contour)
{

	if (contour == NULL)
		return;

	GSList *l = contour;

	do {
		g_slice_free(koki_point2Di_t, l->data);
		l = l->next;

	} while (l != NULL);

	g_slist_free(contour);

}



/**
 * @brief draws a contour on to an \c IplImage
 *
 * Ideally, the \c IplImage would be the original or a copy of the frame the
 * contour was created for. Doing so will outline the regions the contour
 * detection has found.
 *
 * @param frame    a pointer the the \c IplImage to draw on to
 * @param contour  the contour to draw
 */
void koki_contour_draw(IplImage *frame, GSList *contour)
{

	while (contour != NULL){

		koki_point2Di_t *p;
		p = contour->data;

		if (frame->nChannels == 3){

			KOKI_IPLIMAGE_ELEM(frame, p->x, p->y, R) = KOKI_CONTOUR_RED;
			KOKI_IPLIMAGE_ELEM(frame, p->x, p->y, G) = KOKI_CONTOUR_GREEN;
			KOKI_IPLIMAGE_ELEM(frame, p->x, p->y, B) = KOKI_CONTOUR_BLUE;

		} else if (frame->nChannels == 1){

			KOKI_IPLIMAGE_GS_ELEM(frame, p->x, p->y) = 127;

		}

		contour = contour->next;

	}

}
