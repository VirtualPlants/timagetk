/*************************************************************************
 * test-pyramid-image.c -
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



#include <stdio.h>
#include <stdlib.h>

#include <bal-tests.h>



int main(int argc, char *argv[])
{
  bal_image theIm;
  int pyramid_gaussian_filtering = 1;
  int test_inverse_trsf = 1;
  
  if ( argc <= 1 ) {
    fprintf( stderr, "must provide an image file name\n" );
    exit(-1);
  }

  (void)BAL_ReadImage( &theIm, argv[1], 0 );
  BAL_TestPyramidImage( &theIm, pyramid_gaussian_filtering, test_inverse_trsf );
  BAL_FreeImage( &theIm );
  return( 0 );
}
