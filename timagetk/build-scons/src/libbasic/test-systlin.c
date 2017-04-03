/*************************************************************************
 * test-systlin.c - Resolution of linear system
 *
 * $Id: test-systlin.c,v 1.4 2000/09/08 07:21:41 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 22 11:08:39 MET 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <systlin.h>
#include <time.h>


int main (int argc, char *argv[] )
{
  int i, n=1;
  int dim=4;
  
  (void)srandom(time(0));

  if ( argc >= 2 ) {
    if ( sscanf( argv[1], "%d", &dim ) != 1 ) {
      dim = 4;
    }
  }
  if ( argc >= 3 ) {
    if ( sscanf( argv[2], "%d", &n ) != 1 ) {
      n = 4;
    }
  }

  for ( i=0; i<n ;i++ ) {
    printf( "trial %3d/%d\n", i+1, n );
    _TestSystLin( dim );
  }


  return( 0 );
}


