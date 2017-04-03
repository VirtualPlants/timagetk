/*************************************************************************
 * bal-pyramid.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Dec 25 20:10:58 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */





#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <bal-transformation-tools.h>
#include <bal-pyramid.h>

static int _verbose_ = 1;
static int _debug_ = 0;


static int MAXIMAL_DIMENSION = 32;





int BAL_GetVerboseInBalPyramid(  )
{
  return( _verbose_ );
}

void BAL_SetVerboseInBalPyramid( int v )
{
  _verbose_ = v;
}

void BAL_IncrementVerboseInBalPyramid(  )
{
  _verbose_ ++;
}

void BAL_DecrementVerboseInBalPyramid(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



int BAL_GetDebugInBalPyramid(  )
{
  return( _debug_ );
}

void BAL_SetDebugInBalPyramid( int d )
{
  _debug_ = d;
}

void BAL_IncrementDebugInBalPyramid(  )
{
  _debug_ ++;
}





/*--------------------------------------------------*
 *
 * Pyramid tools
 *
 *--------------------------------------------------*/

/* computes the series of decreasing dimensions
   and the associated sigma (for gaussian filtering)
   for one dimension
*/
static void _FillPyramidDimension( int *d, double *s, int dim, int levels )
{
  double power;
  int l;
  
  if ( levels <= 0 ) return;

  d[0] = dim;
  s[0] = 0.0;

  if ( levels <= 1 ) return;

  /* if dim is something like 2^n,
     then the dimension can be divided at each step
        d[0] = dim, d[1]= dim/2, etc ...
     else the first step is the closest (and inferior) 2^n number
        d[0] = dim, d[1]=2^n, d[2]=d[1]/2, etc
        for the sigma, we smooth the first level is 2^n is significantly
        different from dim
  */
     
  power = log((double)(dim)) / log(2.0);
  if ( power - (int)power < EPSILON ) {
    d[1] = d[0] / 2;
    s[1] = 2.0;
  }
  else {  
    d[1] = (int)pow(2.0, (int) (power) );
    if ( power - (int)power < 0.5 )
      s[1] = 0.0;
    else
      s[1] = 2.0;
  }
  
  /* from one level to the next
     - the dimension is divided by 2
     - the sigma is calculated with the formula
       found in baladin
     stop changing if the dimension is too small
  */
  for ( l=2; l<levels; l++ ) {
    d[l] = d[l-1] / 2;
    if ( power - (int)power < EPSILON || power - (int)power >= 0.5 )
      s[l] = 2.0 * sqrt ( (double)l  );
    else
      s[l] = 2.0 * sqrt ( (double)(l-1)  );
    
    if ( d[l] < 1 ) {
      d[l] = 1;
      s[l] = s[l-1];
    }
  }

}




/* pyramid level construction 
   from #0 to #levels
*/

int _ComputePyramidLevel( bal_pyramid_level *pyramid_level, 
			  int pyramid_levels,  
			  bal_image *theInrimage_ref, 
			  bal_blockmatching_pyramidal_param *p )
{
  char *proc = "_ComputePyramidLevel";

  int *d = NULL;
  int *dx, *dy, *dz;
  int *dmax;
  
  double *s = NULL;
  double *sx, *sy, *sz;

  int levels = pyramid_levels;
  int l, ix, iy, iz;

  /* build the series of decreasing dimension
     for each direction (X, Y, Z)
  */
  d = (int*)malloc( 3 * (levels+1) * sizeof( int ) );
  if ( d == (int*)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate dimension array\n", proc );
    return( -1 );
  }
  dx = d;
  dy = d + (levels+1);
  dz = d + 2*(levels+1);

  s = (double*)malloc( 3 * (levels+1) * sizeof( double ) );
  if ( s == (double*)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate sigma array\n", proc );
    free( d );
    return( -1 );
  }
  sx = s;
  sy = s + (levels+1);
  sz = s + 2*(levels+1);

  _FillPyramidDimension( dx, sx, (int)theInrimage_ref->ncols, (levels+1) );
  _FillPyramidDimension( dy, sy, (int)theInrimage_ref->nrows, (levels+1) );
  _FillPyramidDimension( dz, sz, (int)theInrimage_ref->nplanes, (levels+1) );



  /* find the largest dimension
   */
  if ( dx[0] >= dy[0] && dx[0] >= dz[0] ) {
    dmax = dx;
  }
  else {
    if ( dy[0] >= dx[0] && dy[0] >= dz[0] )
      dmax = dy;
    else 
      dmax = dz;
  }


  /* build the pyramid
   */
  for ( l=0, ix=0, iy=0, iz=0; l<=levels; l++ ) {

    /* compute the dimensions
     */
    while ( dx[ix] > dmax[l] ) ix++;
    while ( dy[iy] > dmax[l] ) iy++;
    while ( dz[iz] > dmax[l] ) iz++;
    pyramid_level[l].ncols = dx[ix];
    pyramid_level[l].nrows = dy[iy];
    pyramid_level[l].nplanes = dz[iz];

    pyramid_level[l].vx = theInrimage_ref->ncols * theInrimage_ref->vx / (double)pyramid_level[l].ncols;
    pyramid_level[l].vy = theInrimage_ref->nrows * theInrimage_ref->vy / (double)pyramid_level[l].nrows;
    pyramid_level[l].vz = theInrimage_ref->nplanes * theInrimage_ref->vz / (double)pyramid_level[l].nplanes;
    
    pyramid_level[l].sigma.x = sx[ix];
    pyramid_level[l].sigma.y = sy[iy];
    pyramid_level[l].sigma.z = sz[iz];

    pyramid_level[l].param.pyramid_level = l;

    BAL_InitBlockMatchingParametersFromPyramidalOnes( p, &(pyramid_level[l].param) );

    /* check 2D computation
     */
    if ( theInrimage_ref->nplanes == 1 ) {
      pyramid_level[l].param.block_dim.z = 1;
      pyramid_level[l].param.block_border.z = 0;
      pyramid_level[l].param.block_spacing.z = 0;
      pyramid_level[l].param.half_neighborhood_size.z = 0;
      pyramid_level[l].param.step_neighborhood_search.z = 0;
    }
    
  }

  

  /* verbose: built levels
   */
  if ( _verbose_ >= 2 ) {
    fprintf( stderr, "%s:\n", proc );
    for ( l=0; l<=levels; l++ ) {
      if ( _debug_ ) 
	_PrintPyramidLevel( stdout, &(pyramid_level[l]), 0 ); 
      else 
	fprintf( stderr, " - build pyramid level #%2d: %4d x %4d x %4d\n", l,
		 pyramid_level[l].ncols, pyramid_level[l].nrows, pyramid_level[l].nplanes );
    }
    fprintf( stderr, "\n" );
  }

  
  
  /* check the pyramid levels
     - maximal dimension should be larger than MAXIMAL_DIMENSION
     - blocks have to be included
   */
  for ( l=levels; l>=0; l-- ) {
    if ( max( pyramid_level[l].ncols, max( pyramid_level[l].nrows, pyramid_level[l].nplanes ) ) < MAXIMAL_DIMENSION ) {
      if ( _verbose_ >= 2 ) {
	fprintf( stderr, "level #%d/%d is discarded\n", l, levels );
	fprintf( stderr, "\t maximal dimension (=%d) < %d\n", 
		 max( pyramid_level[l].ncols, max( pyramid_level[l].nrows, pyramid_level[l].nplanes ) ), MAXIMAL_DIMENSION );
      }
      levels --;
      continue;
    }
    if ( pyramid_level[l].ncols < p->block_dim.x || 
	 pyramid_level[l].nrows < p->block_dim.y || 
	 pyramid_level[l].nplanes < p->block_dim.z ) {
      if ( _verbose_ ) {
	fprintf( stderr, "level #%d/%d is discarded\n", l, levels );
	fprintf( stderr, "\t blocks (%dx%dx%d) are not included in the image  (%dx%dx%d)\n", 
		 p->block_dim.x, p->block_dim.y, p->block_dim.z, 
		 pyramid_level[l].ncols, pyramid_level[l].nrows, pyramid_level[l].nplanes );
      }
      levels --;
      continue;
    }
  }


  
  /* parameters that evolved in the pyramid
   */

  /* blocks_fraction */
  for ( l=0; l<= p->pyramid_lowest_level; l++ )
    pyramid_level[l].param.blocks_fraction = p->blocks_fraction.lowest;
  for ( l=p->pyramid_lowest_level+1; l<=levels; l++ ) {
    pyramid_level[l].param.blocks_fraction = p->blocks_fraction.lowest 
      + (p->blocks_fraction.highest - p->blocks_fraction.lowest) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
  }
  for ( l=levels+1; l<=pyramid_levels; l++ ) 
    pyramid_level[l].param.blocks_fraction = p->blocks_fraction.highest;


  /* elastic_regularization_sigma  */
  for ( l=0; l<= p->pyramid_lowest_level; l++ ) {
    pyramid_level[l].param.elastic_regularization_sigma = p->elastic_regularization_sigma.lowest;
  }
  for ( l=p->pyramid_lowest_level+1; l<=levels; l++ ) {
    pyramid_level[l].param.elastic_regularization_sigma.x = p->elastic_regularization_sigma.lowest.x 
      + (p->elastic_regularization_sigma.highest.x - p->elastic_regularization_sigma.lowest.x) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
    pyramid_level[l].param.elastic_regularization_sigma.y = p->elastic_regularization_sigma.lowest.y 
      + (p->elastic_regularization_sigma.highest.y - p->elastic_regularization_sigma.lowest.y) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
    pyramid_level[l].param.elastic_regularization_sigma.z = p->elastic_regularization_sigma.lowest.z 
      + (p->elastic_regularization_sigma.highest.z - p->elastic_regularization_sigma.lowest.z) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
  }  
  for ( l=levels+1; l<=pyramid_levels; l++ ) {
    pyramid_level[l].param.elastic_regularization_sigma = p->elastic_regularization_sigma.highest;
  }

  /* estimator */
  for ( l=0; l<= p->pyramid_lowest_level; l++ ) {
    pyramid_level[l].param.estimator = p->estimator.lowest;
  }
  for ( l=p->pyramid_lowest_level+1; l<=levels; l++ ) {
     pyramid_level[l].param.estimator.max_iterations = (int)( p->estimator.lowest.max_iterations 
      + ( p->estimator.highest.max_iterations - p->estimator.lowest.max_iterations ) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level) + 0.5 );
     pyramid_level[l].param.estimator.standard_deviation_threshold = p->estimator.lowest.standard_deviation_threshold
      + ( p->estimator.highest.standard_deviation_threshold - p->estimator.lowest.standard_deviation_threshold) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
     pyramid_level[l].param.estimator.retained_fraction = p->estimator.lowest.retained_fraction
      + ( p->estimator.highest.retained_fraction - p->estimator.lowest.retained_fraction) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);

     pyramid_level[l].param.estimator.sigma.x = p->estimator.lowest.sigma.x
      + ( p->estimator.highest.sigma.x - p->estimator.lowest.sigma.x) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
     pyramid_level[l].param.estimator.sigma.y = p->estimator.lowest.sigma.y
      + ( p->estimator.highest.sigma.y - p->estimator.lowest.sigma.y) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
     pyramid_level[l].param.estimator.sigma.z = p->estimator.lowest.sigma.z
      + ( p->estimator.highest.sigma.z - p->estimator.lowest.sigma.z) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level);
  }
  
  for ( l=levels+1; l<=pyramid_levels; l++ ) {
    pyramid_level[l].param.estimator = p->estimator.highest;
  }

  /* max_iterations  */
  for ( l=0; l<= p->pyramid_lowest_level; l++ ) {
    pyramid_level[l].param.max_iterations = p->max_iterations.lowest;
  }
  for ( l=p->pyramid_lowest_level+1; l<=levels; l++ ) {
    pyramid_level[l].param.max_iterations = (int)( p->max_iterations.lowest 
      + ( p->max_iterations.highest - p->max_iterations.lowest ) 
      * (double)(l - p->pyramid_lowest_level) / (double)(levels - p->pyramid_lowest_level) + 0.5 );
  }
  
  for ( l=levels+1; l<=pyramid_levels; l++ ) {
    pyramid_level[l].param.max_iterations = p->max_iterations.highest;
  }




  free( s );
  free( d );
  return( levels );
}





void _PrintPyramidLevel( FILE *f, bal_pyramid_level *p, int print_params ) 
{
  fprintf( f, "\n" );
  fprintf( f, "=====================================================================\n" );
  fprintf( f, "\n" );
  fprintf( f, "At level %d\n", p->param.pyramid_level );
  fprintf( f, "- image dimensions = %3d x %3d x %3d\n",
	   p->ncols, p->nrows, p->nplanes );
  fprintf( f, "- voxel dimensions = %f x %f x %f\n", p->vx, p->vy, p->vz );
  fprintf( f, "- sigma            = %f x %f x %f\n",
	   p->sigma.x, p->sigma.y, p->sigma.z );
  fprintf( f, "- block fraction = %f\n", p->param.blocks_fraction );

  if ( print_params )
    BAL_PrintBlockMatchingParameters( f, &(p->param) );
}





int _ComputeSubsampledImage( bal_image *subsampled_image,
			     bal_image *image_to_be_subsampled,
			     bal_transformation *subsampling_trsf,
			     bal_pyramid_level *p, 
			     int pyramid_gaussian_filtering )
{
  char *proc = "_ComputeSubsampledImage";
  bal_image tmp_image;

  if ( pyramid_gaussian_filtering ) {
    if ( BAL_InitAllocImage( &tmp_image, "smoothed_reference_image.inr",
			     image_to_be_subsampled->ncols, image_to_be_subsampled->nrows, 
			     image_to_be_subsampled->nplanes, image_to_be_subsampled->vdim, 
			     image_to_be_subsampled->type ) != 1 ) {
      if ( _verbose_ ) 
	fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
    tmp_image.vx = image_to_be_subsampled->vx;
    tmp_image.vy = image_to_be_subsampled->vy;
    tmp_image.vz = image_to_be_subsampled->vz;
    if ( BAL_SmoothImageIntoImage( image_to_be_subsampled, &tmp_image,
				   &(p->sigma) ) != 1 ) {
      if ( _verbose_ ) 
	fprintf( stderr, "%s: unable to filter auxiliary image\n", proc );
      BAL_FreeImage( &tmp_image );
      return( -1 );
    }
    if ( BAL_ResampleImage( &tmp_image, subsampled_image, subsampling_trsf, LINEAR ) != 1 ) {
      if ( _verbose_ ) 
	fprintf( stderr, "%s: unable to resample auxiliary image\n", proc );
      BAL_FreeImage( &tmp_image );
      return( -1 );
    }
    BAL_FreeImage( &tmp_image );
  }
  else {
    if ( BAL_ResampleImage( image_to_be_subsampled, subsampled_image, subsampling_trsf, LINEAR ) != 1 ) {
      if ( _verbose_ ) 
	fprintf( stderr, "%s: unable to resample image\n", proc );
      return( -1 );
    }
  }
  return( 1 );
}





/*--------------------------------------------------*
 *
 * Pyramid construction
 *
 *--------------------------------------------------*/

#define STRLENGTH 1024 

int BAL_BuildPyramidImage( bal_image *theIm, 
                           stringList *image_names,
                           stringList *trsfs_names,
			   int pyramid_lowest_level,
			   int pyramid_highest_level,
			   int pyramid_gaussian_filtering )
{
  char * proc = "BAL_BuildPyramidImage";
  int m;
  bal_pyramid_level *pyramid_level = NULL;
  int l, nl, levels;
  int highest_level = pyramid_highest_level;
  int lowest_level = pyramid_lowest_level;
  bal_image resIm;
  bal_blockmatching_pyramidal_param p;
  char *name;
  bal_transformation subsampling_trsf;
  int maximal_dimension = MAXIMAL_DIMENSION;


  MAXIMAL_DIMENSION = 16;

  /* number of pyramid levels
   */

  if ( theIm->nplanes == 1 )
    m = max ( theIm->ncols, theIm->nrows );
  else 
    m = max ( theIm->ncols, max( theIm->nrows, theIm->nplanes ) );

  levels = (int)( log((double)(m)) / log(2.0) ) + 2;
  pyramid_level = (bal_pyramid_level *)malloc( (levels) * sizeof(bal_pyramid_level) ); 

  BAL_InitBlockMatchingPyramidalParameters( &p );
  p.pyramid_lowest_level = 0;
  p.pyramid_highest_level = levels-1;
  if ( theIm->nplanes == 1 ) {
    p.block_dim.z = 1;
  }

  nl = _ComputePyramidLevel( pyramid_level, levels-1, theIm, &p );




  
  /* levels to be written
   */

  if ( nl < 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no pyramid level\n", proc );
    MAXIMAL_DIMENSION = maximal_dimension;
    return( -1 );
  }

  highest_level =  ( pyramid_highest_level < 0 ) ? nl : pyramid_highest_level;
  lowest_level =  ( pyramid_lowest_level < 0 ) ? 0 : pyramid_lowest_level;

  if ( nl < highest_level ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: start at level %d instead of %d\n", proc, nl, highest_level );
    highest_level = nl;
  }
    
  /* verbose: built levels
   */
  if ( _verbose_ ) {
    fprintf( stderr, "%s:\n", proc );
    for ( l=lowest_level; l<=highest_level; l++ ) {
      if ( _debug_ )
        _PrintPyramidLevel( stdout, &(pyramid_level[l]), 0 );
      else
        fprintf( stderr, " - build pyramid level #%2d: %4d x %4d x %4d\n", l,
                 pyramid_level[l].ncols, pyramid_level[l].nrows, pyramid_level[l].nplanes );
    }
    fprintf( stderr, "\n" );
  }



  /* build transformation and images
   */

  if ( BAL_AllocTransformation( &subsampling_trsf, AFFINE_3D, (bal_image *)NULL ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate transformation\n", proc );
    MAXIMAL_DIMENSION = maximal_dimension;
    return( -1 );
  }



  for ( l=highest_level; l>=lowest_level; l-- ) {

    fprintf( stderr, "%s: processing level %d\n", proc, l );
    _PrintPyramidLevel( stderr, &(pyramid_level[l]), 0 ); 
    
    /* image name
     * we assume that image names has been built from
     * 'pyramid_lowest_level' to 'pyramid_highest_level'
     */
    name = (char*)NULL;
    if ( image_names != (stringList*)NULL
         && l-pyramid_lowest_level < image_names->n ) {
        name = image_names->data[l-pyramid_lowest_level];
    }
    else {
        if ( _debug_ || _verbose_ >= 2 ) {
            fprintf( stderr, "%s: no image name for level #%d\n", proc, l );
        }
    }

    /* image computation
     */
    if ( BAL_InitAllocImage( &resIm, name, pyramid_level[l].ncols, 
			     pyramid_level[l].nrows, pyramid_level[l].nplanes, 
			     theIm->vdim, theIm->type ) != 1 ) {
      BAL_FreeTransformation( &subsampling_trsf );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to allocate image at level %d\n", proc, l );
      MAXIMAL_DIMENSION = maximal_dimension;
      return( -1 );
    }

    resIm.vx = (double)theIm->ncols * theIm->vx / (double)resIm.ncols;
    resIm.vy = (double)theIm->nrows * theIm->vy / (double)resIm.nrows;
    resIm.vz = (double)theIm->nplanes * theIm->vz / (double)resIm.nplanes;
      
    if ( BAL_ComputeImageToImageTransformation( &resIm, theIm, &subsampling_trsf ) != 1 ) {
      BAL_FreeImage( &resIm );
      BAL_FreeTransformation( &subsampling_trsf );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to compute transformation at level %d\n", proc, l );
      MAXIMAL_DIMENSION = maximal_dimension;
      return( -1 );
    }

    if ( 0 )
      BAL_PrintTransformation( stderr, &subsampling_trsf, "resampling transformation" );

      
    /* subsampled image computation
     */
    if ( 1 ) {
      (void)_ComputeSubsampledImage( &resIm, theIm, &subsampling_trsf,
				     &(pyramid_level[l]), pyramid_gaussian_filtering );
    }
    else {
      (void)BAL_ResampleImage( theIm, &resIm, &subsampling_trsf, LINEAR );
    }
    

    /* writing results
     */
    if ( name != (char*)NULL ) {
        if ( BAL_WriteImage( &resIm, name ) != 1 ) {
            BAL_FreeImage( &resIm );
            BAL_FreeTransformation( &subsampling_trsf );
            if ( _verbose_ )
                fprintf( stderr, "%s: unable to write image '%s' at level %d\n", proc, name, l );
            MAXIMAL_DIMENSION = maximal_dimension;
            return( -1 );
        }
    }

    BAL_FreeImage( &resIm );

    /* transformation name
     */
    name = (char*)NULL;
    if ( trsfs_names != (stringList*)NULL
         && l-pyramid_lowest_level < trsfs_names->n ) {
        name = trsfs_names->data[l-pyramid_lowest_level];
    }
    else {
        if ( _debug_ || _verbose_ >= 2 ) {
            fprintf( stderr, "%s: no transformation name for level #%d\n", proc, l );
        }
    }

    if ( name != (char*)NULL ) {
        if ( BAL_WriteTransformation( &subsampling_trsf, name ) != 1 ) {
          BAL_FreeTransformation( &subsampling_trsf );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to write transformation at level %d\n", proc, l );
          MAXIMAL_DIMENSION = maximal_dimension;
          return( -1 );
        }
    }
    

  }

  BAL_FreeTransformation( &subsampling_trsf );
  
  MAXIMAL_DIMENSION = maximal_dimension;
  return( 1 );
}
