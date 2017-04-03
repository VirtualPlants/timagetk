/*************************************************************************
 * applyTrsf.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Etienne Delclaux (etienne.delclaux@inria.fr) 
 * From Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Sep 17 12:39:00 CET 2013
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */
#include <bal-applyTrsfToPoint.h>

static char* program = "applyTrsfToPoint";

int applyTrsfToPoint(
       bal_doublePoint thePt,
       bal_doublePoint* resPt,
       char *real_transformation_name,
       int verbose,
       int debug)
{
  bal_transformation theTrsf; 

  BAL_InitTransformation( &theTrsf );

  if ( real_transformation_name != NULL ) {
    if ( BAL_ReadTransformation( &theTrsf, real_transformation_name ) != 1 ) {
      if ( verbose )
	fprintf( stderr, "%s: unable to read 'real' transformation '%s'\n", program, real_transformation_name );
      return -1;
    }
  }
  else {
    if ( verbose )
      fprintf( stderr, "%s: no transformation \n", program );
    return -1;
  }

  if ( BAL_TransformDoublePoint( &thePt, resPt, &theTrsf ) != 1 ) {
    BAL_FreeTransformation( &theTrsf );
    return -1;
  }

  if ( debug ) 
    fprintf( stderr, "\t write %lf %lf %lf\n", resPt->x, resPt->y, resPt->z );

  BAL_FreeTransformation( &theTrsf );

  return 0;
}
