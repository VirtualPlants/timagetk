/*************************************************************************
 * linederiv.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 1999
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Mar  9 16:52:10 MET 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */


#include <linederiv.h>


int absoluteDerivative1D( double *input, double *output, int n, double sigma )
{
  char *proc="absoluteDerivative1D";
  double *tmp = (double*)NULL;
  double *theIn, *theOut, *theTmp;
  int bord=20;
  int i;

  if ( input == (double*)NULL ) {
    fprintf( stderr, "%s: null input buffer\n", proc );
    return( 0 );
  }
  if ( output == (double*)NULL ) {
    fprintf( stderr, "%s: null input buffer\n", proc );
    return( 0 );
  }
  if ( n <= 0 ) {
    fprintf( stderr, "%s: null or negative buffer length\n", proc );
    return( 0 );
  }

  tmp = (double*)malloc( 3*(n+2*bord)*sizeof(double) );
  if ( tmp == (double*)NULL ) {
    fprintf( stderr, "%s: can not allocate auxiliary buffer\n", proc );
    return( 0 );
  }
  theIn = tmp;
  theOut = &(tmp[n+2*bord]);
  theTmp = &(tmp[2*(n+2*bord)]);
  
  

  for ( i=0; i<bord; i++ )
    theIn[i] = input[0];
  for ( i=bord; i<bord+n; i++ )
    theIn[i] = input[i-bord];
  for ( i=bord+n; i<bord+2*n; i++ )
    theIn[i] = input[n-1];
  
  InitRecursiveCoefficients( sigma, GAUSSIAN_DERICHE, DERIVATIVE_1 );
  if ( RecursiveFilter1D( theIn, theOut, theTmp, theOut, bord+2*n ) == 0 ) {
    fprintf( stderr, "%s: error in computation\n", proc );
    free( tmp );
    return( 0 );
  }


  for ( i=0; i<n; i++ )
    output[i] = ( theOut[bord+i] < 0.0  ) ? (-theOut[bord+i]) : theOut[bord+i];
  
  free(tmp);

  return( 1 );
}
