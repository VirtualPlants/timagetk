/*************************************************************************
 * bal-behavior-11000.h -
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



#ifndef BAL_BEHAVIOR_H
#define BAL_BEHAVIOR_H


#ifdef _ORIGINAL_BALADIN_

#define _ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ 
#define _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ 
#define _ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ 
#define _ORIGINAL_BALADIN_FLOAT_FIELD_ 
#define _ORIGINAL_BALADIN_PRINTS_ 
#define _ORIGINAL_BALADIN_QSORT_IN_LTS_ 
#define _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ 
#define _ORIGINAL_BALADIN_TWO_DIMENSIONS_
#define _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
#define _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_

#else

#undef _ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ 
#undef _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ 
#undef _ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ 
#undef _ORIGINAL_BALADIN_FLOAT_FIELD_ 
#undef _ORIGINAL_BALADIN_PRINTS_ 
#undef _ORIGINAL_BALADIN_QSORT_IN_LTS_ 
#undef _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ 
#undef _ORIGINAL_BALADIN_TWO_DIMENSIONS_
#undef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
#undef _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_

#endif

#define _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_

#define _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ 
#define _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ 

#define _ORIGINAL_BALADIN_PRINTS_ 
#define _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_

/* 
   #define _ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ 

   In the first implementation of baladin,
   in case of error when computing the incremental
   matrix at a pyramid level, the returned matrix was
   the identity.
   In the new implementation, it was the currently
   estimated matrix
*/



/* 
   #define _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ 
   
   In the first implementation of baladin,
   for the least trimmed square estimation of similitude and 
   rigid transformation, the 'tolerance' for rotation and
   translation were respectively to 1e-4 and 1e-2 for the
   non-weighted version, and 1e-1 and 1e-1 for the
   weighted version.
   In the new implementation, they were both set to 
   1e-4 for both version
*/



/* 
   #define _ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ 

   In the first implementation of baladin
   the voxel size was in float.
   In the new implementation, the voxel size
   is in double.
*/

#ifdef _ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_
  typedef float typeVoxelSize;
#else
  typedef double typeVoxelSize;
#endif



/* 
   #define _ORIGINAL_BALADIN_FLOAT_FIELD_ 
   
   In the first implementation of baladin
   the displacement field was encoded in float.
   In the new implementation, the voxel size
   is in double.
*/

#ifdef _ORIGINAL_BALADIN_FLOAT_FIELD_
  typedef float typeField;
#else
  typedef double typeField;
#endif



/* 
   #define _ORIGINAL_BALADIN_PRINTS_ 

   Try to do the same prints/trace than the first
   implementation of baladin
*/



/*    
   #define _ORIGINAL_BALADIN_QSORT_IN_LTS_ 

   In the first implementation of baladin,
   we use qsort to sort all the residuals. 
   In the new implementation, only a partial ordering 
   w.r.t. the hth value is done.
   This should not change the results, but setting this
   define allows to get the same displays.
*/


/*    
   #define _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ 

   In the first implementation of baladin,
   the identity matrix is returned in case of error in the
   LTS estimation of the transformation.
   In the new implementation, it was the current estimation of
   the matrix.
*/


/* 
   #define _ORIGINAL_BALADIN_TWO_DIMENSIONS_

   The first implementation of baladin does not have
   a dedicated behavior for 2D case in some places.
   This should not change the results in 3D.
*/



/* 
   #define _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_

   The first implementation built the block list
   with the three imbricated loops
   for ( x= ... ) for ( y= ... ) for ( z= ... )
   It may also have, depending on the parameters,
   some allocation problem, and some blocks are not
   considered, they have to be discarded.
   The new implementation uses the loops
   for ( z= ... ) for ( y= ... ) for ( x= ... ) 
*/

/*
  #define _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_

  Normalize and cast the images to be registered on unsigned 
  char
*/

#include <stdio.h>
#include <stdlib.h>

extern void BAL_PrintDefines( FILE *f );


#endif
