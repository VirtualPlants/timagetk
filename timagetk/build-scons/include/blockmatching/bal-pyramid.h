/*************************************************************************
 * bal-pyramid.h -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Dec 25 20:06:05 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */




#ifndef BAL_PYRAMID_H
#define BAL_PYRAMID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string-tools.h>

#include <bal-blockmatching-param.h> 
#include <bal-image.h>
#include <bal-transformation.h>



extern int BAL_GetVerboseInBalPyramid(  );
extern void BAL_SetVerboseInBalPyramid( int v );
extern void BAL_IncrementVerboseInBalPyramid(  );
extern void BAL_DecrementVerboseInBalPyramid(  );

extern int BAL_GetDebugInBalPyramid(  );
extern void BAL_SetDebugInBalPyramid( int d );
extern void BAL_IncrementDebugInBalPyramid(  );


typedef struct {
  /* image geometry
   */
  int ncols;
  int nrows;
  int nplanes;

  float vx;
  float vy;
  float vz;

  /* sigma for filtering
     [0] : x
     [1] : y
     [2] : z
   */
  bal_doublePoint sigma;

  /* matching parameters
   */
  bal_blockmatching_param param;
} bal_pyramid_level;



/* pyramid level computation
 */
extern int _ComputePyramidLevel( bal_pyramid_level *pyramid_level, 
				 int levels,  
				 bal_image *theInrimage_ref, 
				 bal_blockmatching_pyramidal_param *p );
extern void _PrintPyramidLevel( FILE *f, bal_pyramid_level *p, int print_params );
extern int _ComputeSubsampledImage( bal_image *subsampled_image,
				    bal_image *image_to_be_subsampled,
				    bal_transformation *subsampling_trsf,
				    bal_pyramid_level *p, 
				    int pyramid_gaussian_filtering );

extern int BAL_BuildPyramidImage( bal_image *theIm, 
                                  stringList *image_names,
                                  stringList *trsfs_names,
			   int pyramid_lowest_level,
			   int pyramid_highest_level,
			   int pyramid_gaussian_filtering );

#ifdef __cplusplus
}
#endif

#endif
