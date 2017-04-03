/*************************************************************************
 * test-copy.c -
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

#include <bal-image.h>



int main(int argc, char *argv[])
{
  bal_image theIm;
  
  if ( argc <= 2 ) {
    fprintf( stderr, "must provide an image file name and a destination\n" );
    exit(-1);
  }

  if ( BAL_ReadImage( &theIm, argv[1], 0 ) != 1 ) {
    fprintf( stderr, "%s: can not read '%s'\n", argv[0], argv[1] );
    exit( -1 );
  }

  fprintf( stderr, "'%s' has been read\n", argv[1] );

 if ( BAL_WriteImage( &theIm, argv[2] ) != 1 ) {
    BAL_FreeImage( &theIm );
    fprintf( stderr, "%s: can not write '%s'\n", argv[0], argv[2] );
    exit( -1 );
  }

  BAL_FreeImage( &theIm );
  return( 0 );
}
