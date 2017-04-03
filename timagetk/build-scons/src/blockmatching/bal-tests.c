/*************************************************************************
 * bal-tests.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
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
#include <time.h>

#include <bal-transformation-tools.h>
#include <bal-pyramid.h>

/*--------------------------------------------------*
 *
 * TESTS
 *
 *--------------------------------------------------*/

void BAL_TestPyramidLevel( int n ) 
{
  int x, y, z, i;
  int m = 1000;
  bal_pyramid_level *pyramid_level = NULL;
  int l, nl, levels;
  bal_image theIm;
  bal_blockmatching_pyramidal_param p;

  srandom( time(0) );

  levels = (int)( log((double)(m)) / log(2.0) ) + 2;
  pyramid_level = (bal_pyramid_level *)malloc( (levels) * sizeof(bal_pyramid_level) ); 

  BAL_InitBlockMatchingPyramidalParameters( &p );
  p.pyramid_lowest_level = 0;;
  p.pyramid_highest_level = levels-1;
  p.blocks_fraction.lowest = 0.25;
  p.blocks_fraction.highest = 0.5;

  for (i=0; i<n; i++ ) {
    
    x = (int)( m * (double)random() / (double)RAND_MAX );
    y = (int)( m * (double)random() / (double)RAND_MAX );
    z = (int)( m * (double)random() / (double)RAND_MAX );
    (void)BAL_InitAllocImage( &theIm, "test.inr.gz", x, y, z, 1, UCHAR );
    nl = _ComputePyramidLevel( pyramid_level, levels-1, &theIm, &p );

    fprintf( stdout, "\n" );
    fprintf( stdout, "TEST #%d : x=%4d y=%4d z=%4d\n", i, x, y, z );
    if ( nl < 0 ) {
      fprintf( stdout, "- no pyramid level\n" );
    }
    else { 
      for ( l=0; l<=nl; l++ ) 
	_PrintPyramidLevel( stdout, &(pyramid_level[l]), 1 ); 
    }
    BAL_FreeImage( &theIm );
  }
  
}




/* 
   - pyramid_gaussian_filtering
     the pyramid is built with smooting
   - test_inverse_trsf
     test the consistance between direct and inverse transformation
 */
void BAL_TestPyramidImage( bal_image *theIm, 
			   int pyramid_gaussian_filtering,
			   int test_inverse_trsf ) 
{
  int m;
  bal_pyramid_level *pyramid_level = NULL;
  int l, nl, levels;
  bal_image resIm;
  bal_blockmatching_pyramidal_param p;
  char imagename[256];
  bal_transformation subsampling_trsf, inv_trsf, product;


  srandom( time(0) );

  if ( theIm->nplanes == 1 )
    m = max ( theIm->ncols, theIm->nrows );
  else 
    m = max ( theIm->ncols, max( theIm->nrows, theIm->nplanes ) );

  levels = (int)( log((double)(m)) / log(2.0) ) + 2;
  pyramid_level = (bal_pyramid_level *)malloc( (levels) * sizeof(bal_pyramid_level) ); 

  BAL_InitBlockMatchingPyramidalParameters( &p );
  p.pyramid_lowest_level = 0;
  p.pyramid_highest_level = levels-1;
  p.blocks_fraction.lowest = 0.25;
  p.blocks_fraction.highest = 0.5;

  if ( theIm->nplanes == 1 ) {
    p.block_dim.z = 1;
  }

  nl = _ComputePyramidLevel( pyramid_level, levels-1, theIm, &p );
  
  if ( nl < 0 ) {
    fprintf( stdout, "- no pyramid level\n" );
    }
  else { 
    (void)BAL_AllocTransformation( &subsampling_trsf, AFFINE_3D, (bal_image *)NULL );
    if ( test_inverse_trsf ) {
      (void)BAL_AllocTransformation( &inv_trsf, AFFINE_3D, (bal_image *)NULL );
      (void)BAL_AllocTransformation( &product, AFFINE_3D, (bal_image *)NULL );
    }
    for ( l=0; l<=nl; l++ ) {
      _PrintPyramidLevel( stdout, &(pyramid_level[l]), 1 ); 
      sprintf( imagename, "imageatlevel%02d.hdr", l );
      (void)BAL_InitAllocImage( &resIm, imagename, pyramid_level[l].ncols, 
				pyramid_level[l].nrows, pyramid_level[l].nplanes, 
				theIm->vdim, theIm->type );
      resIm.vx = (double)theIm->ncols * theIm->vx / (double)resIm.ncols;
      resIm.vy = (double)theIm->nrows * theIm->vy / (double)resIm.nrows;
      resIm.vz = (double)theIm->nplanes * theIm->vz / (double)resIm.nplanes;
      
      (void)BAL_ComputeImageToImageTransformation( &resIm, theIm, &subsampling_trsf );
      BAL_PrintTransformation( stderr, &subsampling_trsf, "resampling transformation" );

      if ( test_inverse_trsf ) {
	(void)BAL_ComputeImageToImageTransformation( theIm, &resIm, &inv_trsf );
	_mult_mat( &(subsampling_trsf.mat), &(inv_trsf.mat), &(product.mat) );
	BAL_PrintTransformation( stderr, &inv_trsf, "inverse transformation" );	
	BAL_PrintTransformation( stderr, &product, "product = resampling x inverse" );
      }

      /* subsampled image computation
       */
      if ( 1 ) {
	(void)_ComputeSubsampledImage( &resIm, theIm, &subsampling_trsf,
				       &(pyramid_level[l]), pyramid_gaussian_filtering );
      }
      else {
	(void)BAL_ResampleImage( theIm, &resIm, &subsampling_trsf, LINEAR );
      }

      (void)BAL_WriteImage( &resIm, imagename );
      BAL_FreeImage( &resIm );
    }
    BAL_FreeTransformation( &subsampling_trsf );
    if ( test_inverse_trsf ) {
      BAL_FreeTransformation( &inv_trsf );
      BAL_FreeTransformation( &product );
    }
  }
}
