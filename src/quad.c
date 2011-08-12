
/**
 * @file  quad.c
 * @brief Implemenatation for discovering quadrilaterals in contour chains
 */

#include <stdint.h>
#include <glib.h>
#include <cv.h>

#include "contour.h"
#include "points.h"

#include "quad.h"


#define KOKI_QUAD_RED   0
#define KOKI_QUAD_GREEN 255
#define KOKI_QUAD_BLUE  0


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



/**
 * @brief given two points of a contour, \c start and \c end, this function
 *        finds vertices between them, if any.
 *
 * @param start           the \c GSList* of the start node
 * @param end             the \c GSList* of the end node
 * @param points          an array that may contain points on return
 * @param num_points      a pointer to an count for the number of points in
 *                        \c points
 * @param vertices_found  a pointer the the number of vertices found so far,
 *                        used to limit the recursion
 */
static void find_intermediate_vertices(GSList *start, GSList *end,
				       GSList *points[10], uint8_t *num_points,
				       uint8_t *vertices_found)
{

	GSList *furthest;

	furthest = furthest_point_perpendicular_to_line(start, end);

	if (furthest == NULL)
		return;

	/* add the point as a found vertex */
	(*vertices_found)++;
	points[*num_points] = furthest;
	(*num_points)++;


	/* now for the recursive bit */

	/* start --> furthest */
	find_intermediate_vertices(start, furthest, points, num_points,
				   vertices_found);

	/* make sure we don't find too many for a quad */
	if (*vertices_found > 4)
		return;

	/* furthest --> end */
	find_intermediate_vertices(furthest, end, points, num_points,
				   vertices_found);


}



/**
 * @brief a quick test to see if a \c GSList is of sufficient length for use
 *
 * @param l  the \c GSList to test the length of
 * @param n  the requred minimum length
 * @return   TRUE if it's length is greater than \c n, FALSE otherwise
 */
static bool slist_longer_than(GSList *l, uint16_t n)
{

	for (uint8_t i=0; i<n+1; i++){
		if (l == NULL)
			return FALSE;
		l = l->next;
	}

	return TRUE;

}



/**
 * @brief returns a pointer to the last element in the \c GSList
 *
 * @param l  a pointer to the \c GSList in question
 * @return   the last element
 */
static GSList* last_in_slist(GSList *l)
{

	if (l == NULL)
		return NULL;

	while (l->next != NULL)
		l = l->next;

	return l;

}



/**
 * @brief returns an element that is approximately in the middle of the
 *        \c GSList between two elements
 *
 * @param start  the element to be considered at the begining of the list
 * @param end    the element to be considered at the end of the list
 * @return       the \c GSList which is approximately in the centre of
 *               \c start and \c end
 */
static GSList* slist_middle(GSList *start, GSList *end)
{

	uint16_t i, len = 1;
	GSList *l;

	if (start == NULL || end == NULL)
		return NULL;

	l = start;

	while (l->next != NULL){
		len++;
		l = l->next;
	}

	for (l = start, i=0;
	     i < len/2;
	     i++, l=l->next);

	return l;

}



/**
 * @brief given 4 vertices and the contour they are from, this function
 *        returns a pointer to a \c koki_quad_t with the vertices ordered
 *        in a clockwise manner, starting at \c v1.
 *
 * @param v1       the first vertex
 * @param v2       the second vertex
 * @param v3       the third vertex
 * @param v4       the fourth vertex
 * @param contour  the contour the vertices are from
 */
static koki_quad_t* quad_from_vertices(GSList *v1, GSList *v2, GSList *v3,
				       GSList *v4, GSList *contour)
{

	koki_quad_t *quad;
	uint8_t n = 1;

	quad = malloc(sizeof(koki_quad_t));
	assert(quad != NULL);

	assert(v1 == contour);

	quad->links[0] = v1;

	while (contour != NULL){

		if (contour == v2){
			quad->links[n] = v2;
			n++;
		} else if (contour == v3){
			quad->links[n] = v3;
			n++;
		} else if (contour == v4){
			quad->links[n] = v4;
			n++;
		}

		contour = contour->next;

	}

	assert(n == 4);

	for (uint8_t i=0; i<4; i++){
		koki_point2Di_t *p;
		p = quad->links[i]->data;
		quad->vertices[i].x = p->x;
		quad->vertices[i].y = p->y;
	}

	return quad;

}



/**
 * @brief given a contour chain, the function works out if the chain
 *        could represent a quadrilateral
 *
 * @param contour  the chain to check
 * @return         a populated \c koki_quad_t if a quad has been found,
 *                 NULL otherwise
 */
koki_quad_t* koki_quad_find_vertices(GSList *contour)
{

	GSList *v1, *v2, *v3, *v4; /* number don't mean anything here */
	GSList *end, *tmp;
	uint8_t vertices_found, num_points1, num_points2;
	GSList *points1[10], *points2[10];

	/* make sure there are enough points to make a quad */
	if (!slist_longer_than(contour, 4))
		return NULL;

	/* get first 2 vertices (our starting point, and the point
	   furthest from it) */
	v1 = contour;
	v2 = furthest_point(v1, contour->next);

	/* find the last in the chain */
	end = last_in_slist(contour);

	/* make sure everything's in order */
	if (v1 == NULL || v2 == NULL || end == NULL)
		return NULL;

	/* now find vertices between v1 and v2, and v2 and the end */

	vertices_found = 2;
	num_points1 = num_points2 = 0;

	find_intermediate_vertices(v1, v2, points1, &num_points1,
				   &vertices_found);

	find_intermediate_vertices(v2, end, points2, &num_points2,
				   &vertices_found);


	if (num_points1 == 1 && num_points2 == 1){

		/* v1 and v2 are opposite corners of a square,
		   add the found vertices as v3 and v4 */
		v3 = points1[0];
		v4 = points2[0];

	} else {

		/* one of the chains (v1 -> v2, v2 -> end) */

		vertices_found = 2;
		num_points1 = num_points2 = 0;


		if (num_points1 == 0 && num_points2 > 1){

			/* the second chain contains 2 vertices */

			tmp = slist_middle(v2, end);

			find_intermediate_vertices(v2, tmp, points1,
						   &num_points1,
						   &vertices_found);

			find_intermediate_vertices(tmp, end, points2,
						   &num_points2,
						   &vertices_found);

			if (num_points1 == 1 && num_points2 == 1){

				v3 = points1[0];
				v4 = points2[0];

			} else {

				return NULL;

			}// if 1 and 1


		} else if (num_points1 > 1 && num_points2 == 0){

			/* the first chain contains 2 vertices */

			tmp = slist_middle(v1, v2);

			find_intermediate_vertices(v1, tmp, points1,
						   &num_points1,
						   &vertices_found);

			find_intermediate_vertices(tmp, v2, points2,
						   &num_points2,
						   &vertices_found);

			if (num_points1 == 1 && num_points2 == 1){

				v3 = points1[0];
				v4 = points2[0];

			} else {

				return NULL;

			}// if 1 and 1

		} else {

			return NULL;

		}//if else-if else

	}//if

	return quad_from_vertices(v1, v2, v3, v4, contour);

}



/**
 * @brief frees an allocated \c koki_quad_t*
 *
 * @param quad  a pointer to the allocated quad to free
 */
void koki_quad_free(koki_quad_t *quad)
{

	free(quad);

}



/**
 * @brief draws a cross (i.e. a pixel at (x, y), then N, E, S and W of it)
 *
 * @param frame  the \c IplImage to draw on to
 * @param p      the integer point to which represents the centre
 */
static void draw_cross(IplImage *frame, koki_point2Di_t p)
{

	/* centre */
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y, R) = KOKI_QUAD_RED;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y, G) = KOKI_QUAD_GREEN;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y, B) = KOKI_QUAD_BLUE;

	/* above */
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y-1, R) = KOKI_QUAD_RED;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y-1, G) = KOKI_QUAD_GREEN;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y-1, B) = KOKI_QUAD_BLUE;

	/* below */
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y+1, R) = KOKI_QUAD_RED;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y+1, G) = KOKI_QUAD_GREEN;
	KOKI_IPLIMAGE_ELEM(frame, p.x, p.y+1, B) = KOKI_QUAD_BLUE;

	/* left */
	KOKI_IPLIMAGE_ELEM(frame, p.x-1, p.y, R) = KOKI_QUAD_RED;
	KOKI_IPLIMAGE_ELEM(frame, p.x-1, p.y, G) = KOKI_QUAD_GREEN;
	KOKI_IPLIMAGE_ELEM(frame, p.x-1, p.y, B) = KOKI_QUAD_BLUE;

	/* right */
	KOKI_IPLIMAGE_ELEM(frame, p.x+1, p.y, R) = KOKI_QUAD_RED;
	KOKI_IPLIMAGE_ELEM(frame, p.x+1, p.y, G) = KOKI_QUAD_GREEN;
	KOKI_IPLIMAGE_ELEM(frame, p.x+1, p.y, B) = KOKI_QUAD_BLUE;


}



/**
 * @brief draws a little cross on each corner of a quad
 *
 * @param frame  the \c IplImage to draw on to
 * @param quad   the quad that should be drawn
 */
void koki_quad_draw_on_frame(IplImage *frame, koki_quad_t *quad)
{

	koki_point2Di_t p;

	assert(frame != NULL);
	assert(quad != NULL);

	for (uint8_t i=0; i<4; i++){
		p.x = quad->vertices[i].x;
		p.y = quad->vertices[i].y;
		draw_cross(frame, p);
	}

}
