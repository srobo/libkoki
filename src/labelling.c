
/**
 * @file  labelling.c
 * @brief Implemetation for thresholding and labelling images
 */

#include <stdint.h>
#include <glib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cv.h>

#include "labelling.h"
#include "integral-image.h"
#include "threshold.h"

#define KOKI_MIN_REGION_MASS 64
#define KOKI_MIN_DISTANCE_FROM_BORDER 3


/**
 * @brief produces a new labelled image and initialises its fields
 *
 * @param w  the width of the image to represent
 * @param h  the height of the image to represent
 * @return   a pointer to an initialised labelled image
 */
koki_labelled_image_t* koki_labelled_image_new(uint16_t w, uint16_t h)
{

	/* allocate space for a labelled image */
	koki_labelled_image_t *labelled_image;
	labelled_image = malloc(sizeof(koki_labelled_image_t));
	assert(labelled_image != NULL);

	labelled_image->w = w;
	labelled_image->h = h;

	/* alocate the label data array */
	uint32_t data_size = (w+2) * (h+2) * sizeof(uint16_t);
	labelled_image->data = malloc(data_size);
	assert(labelled_image->data != NULL);

	/* init a GArray for label aliases */
	labelled_image->aliases = g_array_new(FALSE,
					     TRUE,
					     sizeof(uint16_t));

	/* init a GArray for clip regions */
	labelled_image->clips = g_array_new(FALSE,
					   FALSE,
					   sizeof(koki_clip_region_t));

	/* zero the perimeter (makes life easier later
	   when looking for connected regions) */
	for (uint16_t i=0; i<w+2; i++){
		/* top row */
		labelled_image->data[i] = 0;
		/* bottom row */
		labelled_image->data[(h+1) * (w+2) + i] = 0;
	}
	for (uint16_t i=1; i<h+1; i++){
		/* left col */
		labelled_image->data[(w+2) * i] = 0;
		/* right col */
		labelled_image->data[(w+2) * i + w + 1] = 0;
	}

	return labelled_image;

}



/**
 * @brief frees a labelled image and its associated allocated memory
 *
 * @param labelled_image  the labelled image to free
 */
void koki_labelled_image_free(koki_labelled_image_t *labelled_image)
{

	free(labelled_image->data);
	g_array_free(labelled_image->aliases, TRUE);
	g_array_free(labelled_image->clips, TRUE);
	free(labelled_image);

}



/**
 * @brief states whether or not the pixel at (x,y) in \c image is above or
 *        below the provided \c threshold.
 *
 * @param image          the image in question
 * @param x              the X co-ordinate of the pixel
 * @param y              the Y co-ordinate of the pixel
 * @param threshold_x_3  the threshold to apply, in the range \c 0-(255*3).
 *                       this param should be 3 times the threshold in the
 *                       range 0-255. (This is an optimization.)
 * @return               TRUE if the pixel is above the threshold, FALSE
 *                       otherwise.
 */
static bool above_threshold(IplImage *image, uint16_t x, uint16_t y,
			    uint16_t threshold_x_3)
{

	return (  KOKI_IPLIMAGE_ELEM(image, x, y, R)
		+ KOKI_IPLIMAGE_ELEM(image, x, y, G)
		+ KOKI_IPLIMAGE_ELEM(image, x, y, B))
		> threshold_x_3;

}



/**
 * @brief sets the label for a given pixel co-ordinate (x, y)
 *
 * @param labelled_image  the labelled image to write the label to
 * @param x               the X co-ordinate of the image
 * @param y               the Y co-ordinate of the image
 * @param label           the label that pixel (x, y) should have
 */
static void set_label(koki_labelled_image_t *labelled_image,
		      uint16_t x, uint16_t y, uint16_t label)
{

	if (label == 0){

		KOKI_LABELLED_IMAGE_LABEL(labelled_image, x, y) = 0;

	} else {

		assert(labelled_image->aliases != NULL
		       && labelled_image->aliases->len > label-1);

		KOKI_LABELLED_IMAGE_LABEL(labelled_image, x, y)
			= g_array_index(labelled_image->aliases,
					uint16_t, label-1);

	}

}



/**
 * @brief returns the label for the pixel in the given direction from (x, y)
 *
 * @param labelled_image  the labelled image in question
 * @param x               the X co-ordinate
 * @param y               the Y co-ordinate
 * @param direction       the direction to move in (from the \c DIRECTION enum)
 * @return                the label \c direction of (x, y)
 */
uint16_t get_connected_label(koki_labelled_image_t *labelled_image,
				    uint16_t x, uint16_t y,
				    enum DIRECTION direction)
{

	switch (direction){

	case N:  return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x,   y-1);
	case NE: return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x+1, y-1);
	case E:  return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x+1, y);
	case SE: return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x+1, y+1);
	case S:  return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x,   y+1);
	case SW: return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x-1, y+1);
	case W:  return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x-1, y);
	case NW: return KOKI_LABELLED_IMAGE_LABEL(labelled_image, x-1, y-1);
	default: return -1;

	}

}

/**
 * @brief find the canonical number for the given label
 *
 * @param l	The label number to find the canonical number for.
 *
 * @return The canonical number of the given label
  */
static uint16_t label_find_canonical( koki_labelled_image_t *lmg,
			       uint16_t l )
{
	while(1) {
		uint16_t a = g_array_index( lmg->aliases, uint16_t, l-1 );

		if( a == l )
			/* Found the lowest alias */
			return a;

		l = a;
	}
}

/**
 * @brief marks a label number as an alias of another.
 *
 * @param lmg		The labelled image
 * @param l_canon	The canonical label number to alias to
 * @param l_alias	The label number to mark as an alias
 */
static void label_alias( koki_labelled_image_t *lmg, uint16_t l_canon, uint16_t l_alias )
{
	uint16_t *l;

	/* Resolve to the minimum alias of l_alias */
	l_alias = label_find_canonical( lmg, l_alias );
	l_canon = label_find_canonical( lmg, l_canon );

	/* Alias l_alias to l_canon */
	l = &g_array_index( lmg->aliases, uint16_t, l_alias-1 );
	*l = l_canon;
}

static void label_dark_pixel( koki_labelled_image_t *lmg,
			      uint16_t x, uint16_t y )
{
	uint16_t label_tmp;

	/* if pixel above is labelled, join that label */
	label_tmp = get_connected_label(lmg, x, y, N);
	if (label_tmp > 0){
		set_label(lmg, x, y, label_tmp);
		return;
	}

	/* if NE pixel is labelled, some merging may need to occur */
	label_tmp = get_connected_label(lmg, x, y, NE);
	if (label_tmp > 0){

		int16_t label_w, label_nw, l1, l2, label_min, label_max;
		label_w  = get_connected_label(lmg, x, y, W);
		label_nw = get_connected_label(lmg, x, y, NW);

		/* if one of the pixels W or NW are labelled, they should
		   be merged together */
		if (label_w > 0 || label_nw > 0){

			l1 = g_array_index(lmg->aliases,
					   uint16_t, label_tmp-1);

			l2 = label_nw > 0
				? g_array_index(lmg->aliases,
						uint16_t, label_nw-1)
				: g_array_index(lmg->aliases,
						uint16_t, label_w-1);

			/* identify lowest label */
			label_max = l2;
			label_min = l1;
			if (label_max < label_min){
				label_min = l2;
				label_max = l1;
			}

			set_label(lmg, x, y, label_min);
			label_alias( lmg, label_min, label_max );
		} else {

			set_label(lmg, x, y, label_tmp);

		}

		return;
	}

	/* Otherwise, take the NW label, if present */
	label_tmp = get_connected_label(lmg, x, y, NW);
	if (label_tmp > 0){
		set_label(lmg, x, y, label_tmp);
		return;
	}

	/* Otherwise, take the W label, if present */
	label_tmp = get_connected_label(lmg, x, y, W);
	if (label_tmp > 0){
		set_label(lmg, x, y, label_tmp);
		return;
	}

	/* If we get this far, a new region has been found */
	label_tmp = lmg->aliases->len + 1;
	g_array_append_val(lmg->aliases, label_tmp);
	set_label(lmg, x, y, label_tmp);
}

/**
 * @brief performs the labelling algortihm on a particular pixel, setting its
 *        value in the labelled image.
 *
 * @param image           the IplImage that is being labeled
 * @param labelled_image  the labelled image that is being created
 * @param x               the X co-ordinate of the image in question
 * @param y               the Y co-ordinate of the image in question
 * @param threshold_x_3   the threshold to apply, in the range \c 0-(255*3).
 *                        this param should be 3 times the threshold in the
 *                        range 0-255. (This is an optimization.)
 */
static void label_pixel(IplImage *image, koki_labelled_image_t *labelled_image,
			uint16_t x, uint16_t y, uint16_t threshold_x_3)
{
	/* white thresholded pixel, not important */
	if (above_threshold(image, x, y, threshold_x_3)){
		set_label(labelled_image, x, y, 0);
		return;
	}

	/* must be a black pixel then... */
	label_dark_pixel( labelled_image, x, y );
}

static void label_image_calc_stats( koki_labelled_image_t *labelled_image )
{
	/* Now renumber all labels to ensure they're all canonical */
	for( uint16_t i=1; i<labelled_image->aliases->len; i++ ) {
		uint16_t *a = &g_array_index( labelled_image->aliases, uint16_t, i-1 );

		*a = label_find_canonical( labelled_image, i );
	}

	/* collect label statistics (mass, bounding box) */

	uint16_t max_alias = 0;
	GArray *aliases, *clips;

	aliases = labelled_image->aliases;
	clips = labelled_image->clips;

	/* find largest alias */
	for (int i=0; i<aliases->len; i++){
		uint16_t alias = g_array_index(aliases, uint16_t, i);
		if (alias > max_alias)
			max_alias = alias;
	}

	/* init clips */
	for (int i=0; i<max_alias; i++){
		koki_clip_region_t clip;
		clip.mass = 0;
		clip.max.x = 0;
		clip.max.y = 0;
		clip.min.x = 0xFFFF; /* max out so below works */
		clip.min.y = 0xFFFF;
		g_array_append_val(clips, clip);
	}

	/* gather stats */
	for (uint16_t y=0; y<labelled_image->h; y++){
		for (uint16_t x=0; x<labelled_image->w; x++){

			uint16_t label, alias;
			koki_clip_region_t *clip;

			label = KOKI_LABELLED_IMAGE_LABEL(labelled_image, x, y);

			/* a threshold white pixel, ignore */
			if (label == 0)
				continue;

			alias = g_array_index(aliases, uint16_t, label-1);

			clip = &g_array_index(clips,
					     koki_clip_region_t,
					     alias-1);

			clip->mass++;
			if (x > clip->max.x)
				clip->max.x = x;
			if (y > clip->max.y)
				clip->max.y = y;
			if (x < clip->min.x)
				clip->min.x = x;
			if (y < clip->min.y)
				clip->min.y = y;

		}//for col
	}//for row
}

/**
 * @brief produces a new labelled image from the given \c IplImage
 *
 * @param image      the \c IplImage to threshold and label
 * @param threshold  the threshold to apply (in the range \c 0-255)
 * @return           a pointer to a new labelled image
 */
koki_labelled_image_t* koki_label_image(IplImage *image, uint16_t threshold)
{
	/* threshold for (R+G+B) with R, G and B being in the range 0-255 */
	uint16_t threshold_x_3 = threshold * 3;

	/* create and initialise a labelled image */
	koki_labelled_image_t *labelled_image;
	labelled_image = koki_labelled_image_new(image->width, image->height);
	assert(labelled_image != NULL);

	/* label all the pixels */
	for (uint16_t row=0; row<image->height; row++){
		for (uint16_t col=0; col<image->width; col++){

			label_pixel(image, labelled_image, col, row,
				    threshold_x_3);

		}//for col
	}//for row

	label_image_calc_stats( labelled_image );

	return labelled_image;
}



/**
 * @brief Creates an \c IplImage from the provided labelled image for debugging
 *        purposes.
 *
 * Colours in the output image don't particularly mean anything, they are just
 * coloured differently.
 *
 * @param labelled_image  the labeled image to represent
 * @return                an \c IplImage representation of the labelled image
 */
IplImage* koki_labelled_image_to_image(koki_labelled_image_t *labelled_image)
{

	IplImage *image;
	uint16_t label;
	uint8_t r, g, b;

	image = cvCreateImage(cvSize(labelled_image->w, labelled_image->h),
			      IPL_DEPTH_8U, 3);

	for (int y=0; y<image->height; y++){
		for (int x=0; x<image->width; x++){

			label = KOKI_LABELLED_IMAGE_LABEL(labelled_image, x, y);

			/* some random numbers to make close regions
			   different colours */
			r = ((label + 37) * 791) % 256;
			g = ((label + 19) * 567) % 256;
			b = ((label + 51) * 354) % 256;

			KOKI_IPLIMAGE_ELEM(image, x, y, R) = r;
			KOKI_IPLIMAGE_ELEM(image, x, y, G) = g;
			KOKI_IPLIMAGE_ELEM(image, x, y, B) = b;

		}//for x
	}//for y

	return image;

}



/**
 * @brief determines whether or not a label is going to be useful
 *
 * It will return FALSE for any regions that are too small, or too near the
 * edge of the input image.
 *
 * @param labelled_image  the labelled image to use
 * @param region          the clip region number (i.e. an index for
 *                        \c labelled_image.clips)
 * @return                FALSE if the region is considered unusable, TRUE
 *                        otherwise
 */
bool koki_label_useable(koki_labelled_image_t *labelled_image, uint16_t region)
{

	koki_clip_region_t *clip;

	/* ensure the region number isn't too high */
	assert(labelled_image->clips->len > region);

	clip = &g_array_index(labelled_image->clips,
			      koki_clip_region_t,
			      region);

	/* are there enough pixels */
	if (clip->mass < KOKI_MIN_REGION_MASS)
		return FALSE;

	/* make sure we're not interacting with the edge of the image */
	if (   clip->min.x < KOKI_MIN_DISTANCE_FROM_BORDER
	    || clip->min.y < KOKI_MIN_DISTANCE_FROM_BORDER
	    || clip->max.x > labelled_image->w - KOKI_MIN_DISTANCE_FROM_BORDER
	    || clip->max.y > labelled_image->h - KOKI_MIN_DISTANCE_FROM_BORDER)
		return FALSE;

	return TRUE;

}

/**
 * @brief threshold and label the provided image
 *
 * This function wraps two stages of work together: it adaptively
 * thresholds the provided image, and labels it.  This function
 * performs a similar task to calling \c koki_threshold_frame and then 
 * \c koki_label_image, but it does it in a considerably more cache
 * friendly way.  (Furthermore, it internally progressively generates
 * and uses an integral image to speed up the adaptive thresholding.)
 *
 * @return the labelled image
 */
koki_labelled_image_t* koki_label_adaptive( const IplImage *frame,
					    uint16_t window_size,
					    int16_t thresh_margin )
{
	uint16_t x, y;
	koki_integral_image_t *iimg;
	koki_labelled_image_t *lmg;

	iimg = koki_integral_image_new( frame, false );
	lmg = koki_labelled_image_new( frame->width, frame->height );

	for( y=0; y<frame->height; y++ )
		for( x=0; x<frame->width; x++ ) {
			CvRect win;

			/* Get the ROI from the thresholder */
			koki_threshold_adaptive_calc_window( frame, &win,
							     window_size, x, y );

			/* Advance the integral image */
			if( x == 0 )
				koki_integral_image_advance( iimg,
							     frame->width - 1,
							     win.y + win.height - 1 );

			if( koki_threshold_adaptive_pixel( frame,
							   iimg,
							   &win, x, y, thresh_margin ) )
				/* Nothing exciting */
				set_label( lmg, x, y, 0);
			else
				/* Label the thing */
				label_dark_pixel( lmg, x, y );
		}

	/* Sort out all the remaining labelling related stuff */
	label_image_calc_stats( lmg );

	koki_integral_image_free( iimg );

	return lmg;
}
