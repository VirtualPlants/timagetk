/*************************************************************************
 * convolution1D.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 30 23:32:10 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convolution1D.h>


static int _verbose_ = 1;





/************************************************************
 *
 * general functions
 *
 ************************************************************/

void init1DConvolutionMask( type1DConvolutionMask *m )
{
  m->length = 0;
  m->halflength = 0;
  m->data = NULL;
}

void free1DConvolutionMask( type1DConvolutionMask *m )
{
  if ( m->data != NULL ) free( m->data );
  init1DConvolutionMask( m );
}

void print1DConvolutionMask( FILE *theFile, type1DConvolutionMask *m, char *d )
{
  char *proc = "print1DConvolutionMask";
  FILE *f;
  int i;

  f =  ( theFile == (FILE*)NULL ) ? stderr : theFile;
  
  if ( d != (char*)NULL ) 
    fprintf( f, " - %s: %s\n", proc, d );
  
  if ( m == (type1DConvolutionMask*)NULL ) {
    fprintf( f, "   NULL mask\n" );
    return;
  }

  fprintf( f, " - mask of length %d, of half length %d\n", m->length, m->halflength);
  if ( m->data == NULL ) 
    fprintf( f, " - NOT ALLOCATED\n" );
  else {
    for ( i=0; i<m->length; i++ )
      fprintf( f,"  c[%2d] = %g\n", i, m->data[i] );
  }
  
}





/************************************************************
 *
 * building masks
 *
 ************************************************************/

double * _build1DGaussianMask( double sigma, int length, derivativeOrder derivative )
{
  char *proc = "_build1DGaussianMask";
  double *m = NULL;
  double sum = 0.0;
  double psum = 0.0;
  double nsum = 0.0;
  int i, half;


  
  switch( derivative ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such derivative order not handled yet\n", proc );
    return( (double*)NULL );
  case DERIVATIVE_0 :
  case DERIVATIVE_1 :
  case DERIVATIVE_1_CONTOURS :
  case DERIVATIVE_2 :
    break;
  }

  half = (int)(length/2);
  if ( length <= 0 || length != 2*half+1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: length should be positive and odd\n", proc );
    return( (double*)NULL );
  }

  if ( sigma <= 0 )  {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: sigma should be positive\n", proc );
    return( (double*)NULL );
  }

  m = (double*)malloc( length * sizeof(double) );
  if ( m == (double*)NULL ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to allocate mask\n", proc );
    return( (double*)NULL );
  }


  

  switch( derivative ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such derivative order not handled yet\n", proc );
    return( (double*)NULL );

  case DERIVATIVE_0 :
    for ( i = -half; i <= half; i++ ) {
      m[half+i] = exp( -0.5 * (i/sigma) * (i/sigma) );
      sum += m[half+i];
    }
    for ( i = 0; i<length; i++ ) m[i] /= sum;
    break;

  case DERIVATIVE_1 :
  case DERIVATIVE_1_CONTOURS :
    for ( i = -half; i <= half; i++ )
      m[half+i] = - i * exp( -0.5 * (i/sigma) * (i/sigma) );
    if ( derivative == DERIVATIVE_1 )  {
      for ( i = -half; i <= half; i++ )
	sum += i * m[half-i];
    }
    else if ( derivative == DERIVATIVE_1_CONTOURS ) {
      for ( i = 1; i <= half; i++ )
	sum += m[half-i];
    }
    for ( i = 0; i<length; i++ ) m[i] /= sum;
    break;

  case DERIVATIVE_2 :
    for ( i = -half; i <= half; i++ )
      m[half+i] = (i*i - sigma*sigma) * exp( -0.5 * (i/sigma) * (i/sigma) );

    for ( i = -half; i <= half; i++ ) {
      if ( m[half+i] > 0 ) psum += m[half+i];
      else nsum +=  m[half+i];
    }
    for ( i = -half; i <= half; i++ ) {
      if ( m[half+i] > 0 ) m[half+i] *= ((psum+nsum)/2.0)/psum;
      else m[half+i] *= - ((psum+nsum)/2.0)/nsum;
    }

    for ( i = -half; i <= half; i++ )
      sum += i * i / 2.0 * m[half-i];
    
    for ( i = 0; i<length; i++ ) m[i] /= sum;
    break;


  }
  
  return( m );
}


double * _buildGaussianMask( double sigma, int length )
{
  return( _build1DGaussianMask( sigma, length, DERIVATIVE_0 ) );
}





/************************************************************
 *
 * 1D convolution
 *
 ************************************************************/
void _compute1DDoubleConvolution( double *theBuf,
				  double *resBuf,
				  int dim,
				  double *mask,
				  int halfMaskLength /* mask is supposed 
							to be of length 2 * halfMaskLength + 1 
							and centered 
						     */
				  )
{
  int x, i, j, k;
  int imin, imax;
  int xmin, xmax;
  int c = halfMaskLength;
  double val;

  /* convolution formula
     res[x] = sum_{i=-half, i=+half} buf[x+i] * mask[half-i] 
  */
  
  /* first points (half-length of the mask)
     the mask is truncated at the beginning
     or at both extremities if the line length is smaller
     than the mask half-length
  */

  xmax = c; 
  if ( xmax >= dim ) xmax = dim-1;

  for ( x = 0; x <= xmax ; x ++ ) {
    val = 0.0;
    imin = -c; if ( imin+x < 0    ) imin = -x;
    imax =  c; if ( imax+x >= dim ) imax = dim-1-x;
    for ( i=imin, j=x+imin, k=c-imin; i<=imax; i++, j++, k-- )
      val += theBuf[j]*mask[k];
    resBuf[x] = val;
  }

  if ( c < dim ) {
    
    /* the line length is longer than the maks half-length
       process the point in the middle
       then the last points
    */
    
    xmin = c;
    xmax = dim-1-c;
    for ( x = xmin; x <= xmax; x ++ ) {
      val = 0.0;
      for ( i= -c, j=x-c, k=2*c; i<=c; i++, j++, k-- )
	val += theBuf[j]*mask[k];
      resBuf[x] = val;
    }
    for ( x = dim-c; x < dim; x ++ ) {
      val = 0.0;
      imax =  c; if ( imax+x >= dim ) imax = dim-1-x;
      for ( i=-c, j=x-c, k=2*c; i<=imax; i++, j++, k-- )
	val += theBuf[j]*mask[k];
      resBuf[x] = val;
    }
  }
}



int compute1DConvolutionWithMask( void *theLine,
				  bufferType typeIn,
				  void *resLine,
				  bufferType typeOut,
				  int dim,
				  double *mask,
				  int halfMaskLength /* mask is supposed 
							to be of length 2 * halfMaskLength + 1 
							and centered 
						     */
				  )
{
  char *proc = "compute1DConvolutionWithMask";

  if ( theLine == resLine ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: input and output have to be different\n", proc );
    return( 0 );
  }

  switch( typeIn ) {
  default :
    if ( _verbose_ ) 
      fprintf( stderr, "%s: such input type not handled in switch\n", proc );
    return( 0 );
  case DOUBLE :
    
    switch( typeOut ) {
    default :
      if ( _verbose_ ) 
	fprintf( stderr, "_convolution1D: such output type not handled in switch\n" );
      return( 0 );
    case DOUBLE :
      _compute1DDoubleConvolution( (double *)theLine, (double *)resLine, dim,
				   mask, halfMaskLength );
      break;
    }

    break;
  }

  return( 1 );
}
