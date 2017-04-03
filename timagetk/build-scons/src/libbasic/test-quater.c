/*************************************************************************
 * test-quater.c - Rigid transformation with least squares
 *
 * $Id: test-quater.c,v 1.1 2000/10/09 09:02:10 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Fri Oct  6 21:51:48 MEST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */
#include <stdio.h>
#include <transfo.h>
#include <time.h>

#include <stdlib.h>


int main (int argc, char *argv[] )
{
  int nb=100;
  int n=4;
  double *l1, *l2;
  int i;
  double e;



  (void)srandom(time(0));  

  if ( argc >= 2 ) {
    if ( sscanf( argv[1], "%d", &nb ) != 1 ) {
      nb = 100;
    }
  }
  if ( argc >= 3 ) {
    if ( sscanf( argv[2], "%d", &n ) != 1 ) {
      n = 4;
    }
  }

  l1 = (double*)malloc( 3*nb * sizeof(double) );
  l2 = (double*)malloc( 3*nb * sizeof(double) );

  for ( i=0; i<n; i++ ) {
    printf( "trial %3d/%d\n", i+1, n );
    e = _TestComputeRigidTransfoBetweenListsOfPts( l1, l2, nb );
    printf( "      sum squared errors = %f  -  mean squared errors = %f\n",
	    e, e/nb );
  }

  free( l1 );  
  free( l2 );

  return( 1 );
}
