
/**
 * @file  contour.c
 * @brief Implemetation for extracting contours from a labelled image
 */

#include <stdint.h>
#include <glib.h>

#include "labelling.h"
#include "points.h"

#include "contour.h"


/**
 * @brief identifies the first point on the top row of the clip region that is
 *        in the region
 *
 * The clip region is the smallest rectangle that completely encloses the
 * labelled region.  This function identifies the first point, scanning
 * left to right, which is part of the region in question.
 *
 * @param labelled_image  the labelled image containing the required clip
 *                        regions
 * @param region          the index of the \c clips \c GArray in \c
 *                        labelled_image
 * @return                a pointer to the first labelled point on the top row
 */
static koki_point2Di_t* first_labeled_on_top_row(koki_labelled_image_t *labelled_image,
						 uint16_t region)
{

	koki_clip_region_t clip;
	koki_point2Di_t *ret, *point;

	point = g_slice_new(koki_point2Di_t);
	assert(point != NULL);

	ret = NULL;

	assert(region < labelled_image->clips->len);
	clip = g_array_index(labelled_image->clips, koki_clip_region_t, region);

	uint16_t x;
	for (x=clip.min.x; x<=clip.max.x; x++){

		uint16_t label, alias;

		label = KOKI_LABELLED_IMAGE_LABEL(labelled_image,
						  x, clip.min.y);

		if (label == 0)
			continue;

		alias = g_array_index(labelled_image->aliases,
				      uint16_t, label-1);

		if (alias != region+1)
			continue;

		point->x = x;
		point->y = clip.min.y;
		ret = point;
		break;

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
			       uint16_t region)
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
	uint16_t label;

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
