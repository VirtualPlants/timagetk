/*************************************************************************
 * test-copy.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mar 24 sep 2013 02:09:49 CEST
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

#include <bal-point.h>



int main(int argc, char *argv[])
{
  bal_typeFieldPointList list;
  
  if ( argc <= 1 ) {
    fprintf( stderr, "must provide an image file name\n" );
    exit(-1);
  }

  BAL_InitTypeFieldPointList( &list );

  if ( BAL_ReadTypeFieldPointList( &list, argv[1] ) != 1 ) {
    fprintf( stderr, "%s: can not read '%s'\n", argv[0], argv[1] );
    exit( -1 );
  }

  fprintf( stderr, "%d points have been read in '%s'\n", list.n_data, argv[1] );

  BAL_PrintTypeFieldPointList( stdout, &list );

  BAL_FreeTypeFieldPointList( &list );
  return( 0 );
}
