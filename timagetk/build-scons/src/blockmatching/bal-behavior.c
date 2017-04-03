/*************************************************************************
 * bal-behavior.c -
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




#include <bal-behavior.h>


void BAL_PrintDefines( FILE *f )
{
  char *proc="BAL_PrintDefines";

  if ( 0 ) fprintf( f, "%s:\n", proc );
#ifdef _ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ 
  fprintf( f, "_ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_ERROR_AT_PYRAMID_LEVEL_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ 
  fprintf( f, "_ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_TOLERANCE_IN_LTS_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ 
  fprintf( f, "_ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_FLOAT_VOXEL_SIZE_ is not set\n" );
#endif
  fprintf( f, "   size of typeVoxelSize is %d\n", (int)sizeof(typeVoxelSize) );

#ifdef _ORIGINAL_BALADIN_FLOAT_FIELD_ 
  fprintf( f, "_ORIGINAL_BALADIN_FLOAT_FIELD_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_FLOAT_FIELD_ is not set\n" );
#endif
  fprintf( f, "   size of typeField is %d\n", (int)sizeof(typeField) );

#ifdef _ORIGINAL_BALADIN_PRINTS_ 
  fprintf( f, "_ORIGINAL_BALADIN_PRINTS_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_PRINTS_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_QSORT_IN_LTS_ 
  fprintf( f, "_ORIGINAL_BALADIN_QSORT_IN_LTS_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_QSORT_IN_LTS_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ 
  fprintf( f, "_ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_ERROR_RETURN_IN_LTS_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_TWO_DIMENSIONS_
  fprintf( f, "_ORIGINAL_BALADIN_TWO_DIMENSIONS_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_TWO_DIMENSIONS_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_
  fprintf( f, "_ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_BLOCKS_MANAGEMENT_ is not set\n" );
#endif

#ifdef _ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_
  fprintf( f, "_ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_ is set\n" );
#else
  fprintf( f, "_ORIGINAL_BALADIN_UNSIGNED_CHAR_IMAGE_ is not set\n" );
#endif
  if ( 0 ) fprintf( f, "\n" );
}
