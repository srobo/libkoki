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
 * @file  quad.c
 * @brief Implemenatation for discovering quadrilaterals in contour chains
 */

#include <stdint.h>
#include <glib.h>
#include <cv.h>
#include <math.h>

#include "contour.h"
#include "points.h"
#include "pca.h"
#include "debug.h"

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
 * @param start           a pointer to the first item in the chain to be
 *                        considered
 * @param end             a pointer to the last item in the chain to be
 *                        considered
 * @param furthest point  a pointer to a \c GSList* where the furthest point
 *                        will be stored
 * @return                the length of the line perpendicular to \c start -->
 *                        \c end
 */
static int32_t furthest_point_perpendicular_to_line(GSList *start, GSList *end,
						    GSList **furthest_point)
{

	int16_t ys_minus_ye, xe_minus_xs, xt_minus_xs, yt_minus_ys;
	int16_t x_dist, y_dist;
	int32_t dist_squared, max_dist_squared = -1;
	int32_t threshold;
	koki_point2Di_t *start_point, *end_point, *chain_point;
	float scale_fraction, scale_fraction_dividend, scale_fraction_divisor;
	GSList *link, *furthest = NULL;


	if (start == NULL || end == NULL){
		*furthest_point = NULL;
		return -1;
	}

	/* get the points out */
	start_point = start->data;
	end_point = end->data;

	xe_minus_xs = end_point->x - start_point->x;
	ys_minus_ye = start_point->y - end_point->y;

	if (xe_minus_xs == 0 && ys_minus_ye == 0){
		*furthest_point = NULL;
		return -1;
	}

	/* calculate a threshold based on the area involved. It will
	   be used to decide whether or not furthest point is likely
	   to be a vertex. If threshold is not exceeded, the the angle
	   between the vectors start->point and end->point is too close
	   to a straight line. */

	int32_t n = 300;
	threshold = (xe_minus_xs * xe_minus_xs +
		     ys_minus_ye * ys_minus_ye) / n + 1;

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


	/* is the angle at the point 'pointy' enough? */

	if (max_dist_squared < threshold){

		koki_debug(KOKI_DEBUG_INFO,
			   "(%d, %d)<-->(%d, %d) Not pointy enough, mds: %d, threshold:%d\n",
			   start_point->x, start_point->y, end_point->x, end_point->y,
			   max_dist_squared, threshold);

		*furthest_point = NULL;
		return -1;

	}

	*furthest_point = furthest;
	double dist = sqrt((double)max_dist_squared);

	return (int32_t)dist;

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
	int32_t dist;
	koki_point2Di_t *start_point, *end_point;

	start_point = start->data;
	end_point = end->data;

	koki_debug(KOKI_DEBUG_INFO, "f_i_v: start: (%d, %d), end: (%d, %d)\n",
		   start_point->x, start_point->y, end_point->x, end_point->y);

	dist = furthest_point_perpendicular_to_line(start, end, &furthest);

	if (dist < 0)
		return;

	/* add the point as a found vertex */
	(*vertices_found)++;
	points[*num_points] = furthest;
	(*num_points)++;

	koki_debug(KOKI_DEBUG_INFO, "added vertex (%d, %d)\n",
		   ((koki_point2Di_t*)furthest->data)->x,
		   ((koki_point2Di_t*)furthest->data)->y);

	/* now for the recursive bit */

	/* make sure we don't find too many for a quad */
	if (*vertices_found > 4)
		return;

	/* start --> furthest */
	koki_debug(KOKI_DEBUG_INFO, "First f_i_v recursive call\n");
	find_intermediate_vertices(start, furthest, points, num_points,
				   vertices_found);

	/* make sure we don't find too many for a quad */
	if (*vertices_found > 4)
		return;

	/* furthest --> end */
	koki_debug(KOKI_DEBUG_INFO, "Second f_i_v recursive call\n");
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

	while (l->next != end){
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
	koki_point2Df_t centre;

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


	/* check we don't ave a boomerang shape */
	centre.x = (float)(quad->vertices[0].x + quad->vertices[2].x) / 2;

	if ( (centre.x - quad->vertices[1].x) *
	     (centre.x - quad->vertices[3].x) > 0){

	  /* it's a boomerang shape */

	  free(quad);
	  quad = NULL;

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

	koki_debug(KOKI_DEBUG_INFO, "v1: (%d, %d), v2: (%d, %d)\n",
		   ((koki_point2Di_t*)v1->data)->x,
		   ((koki_point2Di_t*)v1->data)->y,
		   ((koki_point2Di_t*)v2->data)->x,
		   ((koki_point2Di_t*)v2->data)->y);

	/* find the last in the chain */
	end = last_in_slist(contour);

	/* make sure everything's in order */
	if (v1 == NULL || v2 == NULL || end == NULL)
		return NULL;

	/* now find vertices between v1 and v2, and v2 and the end */

	vertices_found = 2;
	num_points1 = num_points2 = 0;

	koki_debug(KOKI_DEBUG_INFO, "First *initial* f_i_v call\n");
	find_intermediate_vertices(v1, v2, points1, &num_points1,
				   &vertices_found);

	koki_debug(KOKI_DEBUG_INFO, "Second *initial* f_i_v call\n");
	find_intermediate_vertices(v2, end, points2, &num_points2,
				   &vertices_found);


	if (num_points1 == 1 && num_points2 == 1){

		/* v1 and v2 are opposite corners of a square,
		   add the found vertices as v3 and v4 */
		koki_debug(KOKI_DEBUG_INFO, "'perfect' square (v1 opposite v2)\n");
		v3 = points1[0];
		v4 = points2[0];

	} else {

		/* one of the chains (v1 -> v2, v2 -> end) */

		vertices_found = 2;

		if (num_points1 == 0 && num_points2 > 1){

			/* the second chain contains 2 vertices */
			koki_debug(KOKI_DEBUG_INFO,
				   "v1-->v2 contains 0 vertices, v2-->end has more than 1\n");

			tmp = slist_middle(v2, end);

			num_points1 = 0;
			find_intermediate_vertices(v2, tmp, points1,
						   &num_points1,
						   &vertices_found);

			num_points2 = 0;
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
			koki_debug(KOKI_DEBUG_INFO,
				   "v1-->v2 contains more than 1 vertices, v2-->end has 0\n");

			tmp = slist_middle(v1, v2);

			num_points1 = 0;
			find_intermediate_vertices(v1, tmp, points1,
						   &num_points1,
						   &vertices_found);

			num_points2 = 0;
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
 * @brief given 2 lines each defined by a point on the line (in this
 *        case, the dataset's mean) and a vector for its direction,
 *        the function find the point at which the 2 lines intersect
 *
 * A point of intersection can be calculated as follows:
 *
 *   \code point_of_intersection = mean + k * vect \endcode
 *
 * where \c k is a scale factor for the vector which, when positioned
 * at \c mean, ends it at the point of intersection.
 *
 * For 2 such lines that intersect, the following statement holds:
 *
 *  \code a_mean + a_k * a_vect = b_mean + b_k * b_vect \endcode
 *
 * Doing some simultaneous equation stuff, one can remove one of the
 * scale factors (\c a_k or \c b_k) from the equation, and solve.
 *
 * Below, we remove \c b_k and solve for \c a_k. The point of
 * intersection can then be calculated in terms of \c a_k.
 *
 * @param a_mean  the mean of line A
 * @param a_vect  the most significant eigen vector for line A
 * @param b_mean  the mean of line B
 * @param b_vect  the most significant eigen vector for line B
 * @return        the point of intersection of lines A and B
 */
static koki_point2Df_t point_of_intersection(koki_point2Df_t a_mean,
					     koki_point2Df_t a_vect,
					     koki_point2Df_t b_mean,
					     koki_point2Df_t b_vect)
{

	float a_k; /* vector scale factor */
	koki_point2Df_t p;

	a_k = b_vect.y * (b_mean.x - a_mean.x)
		- b_vect.x * (b_mean.y - a_mean.y);

	a_k = a_k / (-b_vect.x * a_vect.y + b_vect.y * a_vect.x);

	p.x = a_mean.x + a_vect.x * a_k;
	p.y = a_mean.y + a_vect.y * a_k;

	return p;

}



/**
 * @brief returns the most significant of the 2 eigen vectors
 *
 * The most significant eigen vector is the one who's corresponding eigen
 * value is the greatest.
 *
 * @param vects  the eigen vectors
 * @param vals   the corresponding eigen values
 * @return       the most significant eigen vector of the two
 */
static koki_point2Df_t significant_eigen_vector(koki_point2Df_t vects[2],
						     float vals[2])
{

	return vals[0] > vals[1] ? vects[0] : vects[1];

}


/**
 * @brief outputs some debug info for the PCA reults
 *
 * @param vects  the eigen vectors
 * @param vals   the eigen values
 * @param avgs   the segment averages
 * @param side   the side/edge of the quad (just for output purposes)
 */
static void pca_output_debug(koki_point2Df_t vects[2], float vals[2],
			     koki_point2Df_t avgs, uint8_t side)
{

	koki_debug(KOKI_DEBUG_INFO, "side %d\n", side);
	koki_debug(KOKI_DEBUG_INFO, "  vects: [%f, %f], [%f, %f]\n",
		   vects[0].x, vects[0].y, vects[1].x, vects[1].y);
	koki_debug(KOKI_DEBUG_INFO, "  vals : %f, %f\n", vals[0], vals[1]);
	koki_debug(KOKI_DEBUG_INFO, "  avgs : (%f, %f)\n", avgs.x, avgs.y);


}



/**
 * @brief finds the number of nodes in a list between two points
 *
 * @param start  the start node
 * @param end    the end node (can be \c NULL for the end of the list)
 * @return       the length of sublist \c start --> \c end
 */
static uint16_t length_between(GSList *start, GSList *end)
{

	uint16_t ret = 0;

	while (start != NULL && start != end){
		ret++;
		start = start->next;
	}

	return ret;

}



/**
 * @brief finds the middle of a chain (identified by a start and end node)
 *        that is the same as the original, but with 5% of each end removed
 *
 * @param src_start  the start node of the original list
 * @param src_end    the end node of the original list
 * @param dst_start  a pointer to a \c GSList* to store the start node of the
 *                   new list
 * @param dst_end    a pointer to a \c GSList* to store the end node of the
 *                   new list
 * @return           the length of the new list
 */
static uint16_t get_centre_section(GSList *src_start, GSList *src_end,
			GSList **dst_start, GSList **dst_end)
{

	uint16_t len, start_offset, new_len;
	GSList *l = NULL;

	assert(src_start != NULL);
	assert(dst_start != NULL && dst_end != NULL);

	len = length_between(src_start, src_end);
	if (len < 10){
		*dst_start = NULL;
		*dst_end = NULL;
	}

	new_len = (uint16_t)(len * 0.9);
	start_offset = (uint16_t)(len * 0.05);

	l = src_start;
	for (uint16_t i=0; i<start_offset; i++)
		l = l->next;

	*dst_start = l;

	for (uint16_t i=0; i<new_len; i++)
		l = l->next;

	*dst_end = l;

	return new_len;

}



/**
 * @brief given a quad, this function applies linear regression to the points
 *        between each of the vertices to improve the estimate of where the
 *        vertices actually are
 *
 * @param quad  the \c koki_quad_t* that should be refined
 */
void koki_quad_refine_vertices(koki_quad_t *quad)
{

	koki_point2Df_t vects[4][2];
	float vals[4][2];
	koki_point2Df_t avgs[4];
	GSList *start, *end;

	if (quad == NULL)
		return;


	/* perform PCA on edges between vertices */
	koki_debug(KOKI_DEBUG_INFO, "PCA on quad\n");
	koki_debug(KOKI_DEBUG_INFO, "-----------\n");

	/* side 0 (v0 --> v1) */
	get_centre_section(quad->links[0], quad->links[1], &start, &end);
	koki_pca(start, end, vects[0], vals[0], &avgs[0]);
	pca_output_debug(vects[0], vals[0], avgs[0], 0);

	/* side 1 (v1 --> v2) */
	get_centre_section(quad->links[1], quad->links[2], &start, &end);
	koki_pca(start, end, vects[1], vals[1], &avgs[1]);
	pca_output_debug(vects[1], vals[1], avgs[1], 1);

	/* side 2  (v2 --> v3) */
	get_centre_section(quad->links[2], quad->links[3], &start, &end);
	koki_pca(start, end, vects[2], vals[2], &avgs[2]);
	pca_output_debug(vects[2], vals[2], avgs[2], 2);

	/* side 3 (v3 --> [end]) */
	get_centre_section(quad->links[3], NULL, &start, &end);
	koki_pca(start, end, vects[3], vals[3], &avgs[3]);
	pca_output_debug(vects[3], vals[3], avgs[3], 3);


	/* set vertex positions based on the intersection of PCA's
	   significant eigen vectors */

	/* vertex 0 (intersection of e3 and e0) */
	quad->vertices[0]
		= point_of_intersection(avgs[3],
					significant_eigen_vector(vects[3], vals[3]),
					avgs[0],
					significant_eigen_vector(vects[0], vals[0]));

	/* vertex 1 (intersection of e0 and e1) */
	quad->vertices[1]
		= point_of_intersection(avgs[0],
					significant_eigen_vector(vects[0], vals[0]),
					avgs[1],
					significant_eigen_vector(vects[1], vals[1]));

	/* vertex 2 (intersection of e1 and e2) */
	quad->vertices[2]
		= point_of_intersection(avgs[1],
					significant_eigen_vector(vects[1], vals[1]),
					avgs[2],
					significant_eigen_vector(vects[2], vals[2]));

	/* vertex 3 (intersection of e2 and e3) */
	quad->vertices[3]
		= point_of_intersection(avgs[2],
					significant_eigen_vector(vects[2], vals[2]),
					avgs[3],
					significant_eigen_vector(vects[3], vals[3]));



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



static void draw_cross_pixel(IplImage *frame, uint16_t x, uint16_t y)
{
	if (frame->nChannels == 1){

		KOKI_IPLIMAGE_GS_ELEM(frame, x, y) = 127;

	} else if (frame->nChannels == 3){

		KOKI_IPLIMAGE_ELEM(frame, x, y, R) = KOKI_QUAD_RED;
		KOKI_IPLIMAGE_ELEM(frame, x, y, G) = KOKI_QUAD_GREEN;
		KOKI_IPLIMAGE_ELEM(frame, x, y, B) = KOKI_QUAD_BLUE;

	}
}



/**
 * @brief draws a cross (i.e. a pixel at (x, y), then N, E, S and W of it)
 *
 * @param frame  the \c IplImage to draw on to
 * @param p      the integer point to which represents the centre
 */
static void draw_cross(IplImage *frame, koki_point2Di_t p)
{
	if (p.x == 0 || p.y == 0 ||
	    p.x == frame->width-1 || p.y == frame->height-1)
		return;

	/* centre */
	draw_cross_pixel(frame, p.x, p.y);

	/* above */
	draw_cross_pixel(frame, p.x, p.y-1);

	/* below */
	draw_cross_pixel(frame, p.x, p.y+1);

	/* left */
	draw_cross_pixel(frame, p.x-1, p.y);

	/* right */
	draw_cross_pixel(frame, p.x+1, p.y);

}



/**
 * @brief draws a little cross on each corner of a quad
 *
 * @param frame  the \c IplImage to draw on to
 * @param quad   the quad that should be drawn
 */
void koki_quad_draw(IplImage *frame, koki_quad_t *quad)
{

	koki_point2Di_t p;

	assert(frame != NULL);
	assert(quad != NULL);

	for (uint8_t i=0; i<4; i++){

		p.x = quad->vertices[i].x;
		p.y = quad->vertices[i].y;

		if (p.x < 0 || p.x > frame->width ||
		    p.y < 0 || p.y > frame->height)
			continue;

		draw_cross(frame, p);

	}

}
