
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


