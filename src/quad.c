
/**
 * @file  quad.c
 * @brief Implemenatation for discovering quadrilaterals in contour chains
 */

#include <stdint.h>
#include <glib.h>

#include "contour.h"
#include "points.h"

#include "quad.h"


/**
 * @brief finds the point in \c chain that is the furthest from \c start
 *
 * @param start  the \c GSList node (of type \c koki_point2Di_t) with the
 *               start point in it
 * @param chain  the chain of pixels (of type \c koki_point2Di_t) in which
 *               to find the most distant
 * @return       the \c GSList (of type \c koki_point2Di_t) containing the
 *               furthest point
 */
static GSList* furthest_point(GSList *start, GSList *chain)
{

	int32_t max_dist_squared, dist_squared, dx, dy;
	koki_point2Di_t *s, *p;
	GSList *furthest;

	if (start == NULL)
		return NULL;

	max_dist_squared = dist_squared = 0;
	furthest = start;
	s = start->data;

	while (chain != NULL){

		p = chain->data;

		dx = p->x - s->x;
		dy = p->y - s->y;

		dist_squared = dx*dx + dy*dy;

		if (dist_squared > max_dist_squared){

			max_dist_squared = dist_squared;
			furthest = chain;

		}

		chain = chain->next;

	}

	return furthest;

}



/**
 * @brief finds the point in a chain that is the furthest from the line between
 *        the start and the end of said chain
 *
 * In the code below, \c s means 'start', \c e means 'end' and \c t means 'test'
 * for the point we'd like to check. For example, the variable \c xe_minus_xs
 * is the X co-ordinate of the end point minus the X co-ordinate of the start
 * point.
 *
 * Imagine a point chain, starting at \c start and ending at \c end, which
 * contains a point we'd like to test, \c link (in the code below, using \c t
 * as above). Now imagine a line drawn from \c start to \c end. We construct
 * a line that is perpendicular to the line from \c start to \c end which
 * passes through point \c t (or \c link).  The aim of this function is to
 * find the point \c t in the chain where the distance from \c t to the
 * intersection of the perpendicular line and the line from \c start to \c end
 * is greatest.
 *
 * As this function is used to find vertices of shapes, a final check is
 * performed to determine whether the angle between \c start, \c t and \c end
 * is sufficiently small enough for it to be considered a vertex.

 *
 * @param start  a pointer to the first item in the chain to be considered
 * @param end    a pointer to the last item in the chain to be considered
 * @return       a pointer to the chain link that is the furthest from the line.
 *               If no such point exists or if there is a problem, NULL is
 *               returned.
 */
static GSList* furthest_point_perpendicular_to_line(GSList *start, GSList *end)
{

	int16_t ys_minus_ye, xe_minus_xs, xt_minus_xs, yt_minus_ys;
	int16_t x_dist, y_dist;
	int32_t dist_squared, max_dist_squared = 0;
	int32_t threshold;
	koki_point2Di_t *start_point, *end_point, *chain_point;
	float scale_fraction, scale_fraction_dividend, scale_fraction_divisor;
	GSList *link, *furthest = NULL;


	if (start == NULL || end == NULL)
		return NULL;

	/* get the points out */
	start_point = start->data;
	end_point = end->data;

	xe_minus_xs = end_point->x - start_point->x;
	ys_minus_ye = start_point->y - end_point->y;

	if (xe_minus_xs == 0 && ys_minus_ye == 0)
		return NULL;

	/* calculate a threshold based on the area involved. It will
	   be used to decide whether or not furthest point is likely
	   to be a vertex. If threshold is not exceeded, the the angle
	   between the vectors start->point and end->point is too close
	   to a straight line. */
	threshold = (xe_minus_xs * xe_minus_xs +
		     ys_minus_ye * ys_minus_ye) / 75;

	for (link = start->next; link != NULL && link != end; link = link->next){

		/* get the associated point */
		chain_point = link->data;

		xt_minus_xs = chain_point->x - start_point->x;
		yt_minus_ys = chain_point->y - start_point->y;

		scale_fraction_dividend = (float)(ys_minus_ye * xt_minus_xs +
						  xe_minus_xs * yt_minus_ys);

		scale_fraction_divisor = (float)(-(xe_minus_xs * xe_minus_xs)
						 -(ys_minus_ye * ys_minus_ye));

		scale_fraction =
			scale_fraction_dividend
			/ scale_fraction_divisor;

		/* now we can calculate the distance to the line */
		x_dist = ys_minus_ye * scale_fraction;
		y_dist = xe_minus_xs * scale_fraction;

		dist_squared = x_dist * x_dist + y_dist * y_dist;

		/* is it the furthest we've seen? */
		if (dist_squared > max_dist_squared){
			max_dist_squared = dist_squared;
			furthest = link;
		}

	}//for

	/* is the angle at the point pointy enough? */
	if (max_dist_squared < threshold)
		return NULL;

	return furthest;

}


