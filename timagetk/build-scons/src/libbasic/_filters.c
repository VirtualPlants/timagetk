

#include <_filters.h>

#include <stdlib.h>
#include <recbuffer.h>

int _gradient( const monimage *image, 
	       monimage *gradient,
	       float sigma )
{
  int dimx, dimy;
  int i, x, y;
  float *input = NULL, *output;
  
  int dim[3];
  int border[3];
  float coeff[3];
  derivativeOrder derivative[3];

  /* les dimensions de l'image
     l'allocation
   */
  // dimx = image->dim.x;
  // dimy = image->dim.y;
  input = (float*)malloc( 2*dimx*dimy*sizeof( float ) );
  if ( input == NULL ) return( 0 );
  output = input;
  output += dimx*dimy;

  /* on copie l'image 
   */
  for ( i=0, y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {
    // input[i] = image->buf[y][x];
  }

  /* les parametres
     conversion `magique' de sigma en alpha
   */
  coeff[0] = 8.0 / ( 2 * sigma + 1 );
  coeff[1] = coeff[0];
  coeff[2] = coeff[0];

  dim[0] = dimx;
  dim[1] = dimy;
  dim[2] = 1;

  border[0] = ( 2*sigma > 10 ) ? 2 *sigma : 10;
  border[1] = border[0];
  border[2] = border[0];
  
  derivative[2] = NODERIVATIVE;

  
  /* la composante X du gradient
   */
  derivative[0] = DERIVATIVE_1_CONTOURS;
  derivative[1] = SMOOTHING;
  if ( RecursiveFilterOnBuffer( input, FLOAT, output, FLOAT, dim, border,
				derivative, coeff, ALPHA_DERICHE ) != 1 ) {
    free( input );
    return( 0 );
  }

  /* on copie l'image 
   */
  for ( i=0, y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {
    // gradient->x.buf[y][x] = output[i];
  }

  /* la composante Y du gradient
   */
  derivative[0] = SMOOTHING;
  derivative[1] = DERIVATIVE_1_CONTOURS;
  if ( RecursiveFilterOnBuffer( input, FLOAT, output, FLOAT, dim, border,
				derivative, coeff, ALPHA_DERICHE ) != 1 ) {
    free( input );
    return( 0 );
  }

  /* on copie l'image 
   */
  for ( i=0, y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {
    // gradient->y.buf[y][x] = output[i];
  }


  free( input );
  return( 1 );
}




int _gaussian( const monimage *image, 
	       monimage *smooth,
	       float sigma )
{
  int dimx, dimy;
  int i, x, y;
  float *input = NULL;
  
  int dim[3];
  int border[3];
  float coeff[3];
  derivativeOrder derivative[3];

  /* les dimensions de l'image
     l'allocation
   */
  // dimx = image->dim.x;
  // dimy = image->dim.y;
  input = (float*)malloc( dimx*dimy*sizeof( float ) );
  if ( input == NULL ) return( 0 );

  /* on copie l'image 
   */
  for ( i=0, y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {
    // input[i] = image->buf[y][x];
  }


  /* les parametres
   */
  coeff[0] = sigma;
  coeff[1] = coeff[0];
  coeff[2] = coeff[0];

  dim[0] = dimx;
  dim[1] = dimy;
  dim[2] = 1;

  border[0] = ( 2*sigma > 10 ) ? 2 *sigma : 10;
  border[1] = border[0];
  border[2] = border[0];
  
  derivative[0] = SMOOTHING;
  derivative[1] = SMOOTHING;
  derivative[2] = NODERIVATIVE;

  
  /* lissage
   */
  if ( RecursiveFilterOnBuffer( input, FLOAT, input, FLOAT, dim, border,
				derivative, coeff, GAUSSIAN_DERICHE ) != 1 ) {
    free( input );
    return( 0 );
  }

  /* on copie l'image 
   */
  for ( i=0, y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++, i++ ) {
    // smooth->buf[y][x] = input[i];
  }

  free( input );
  return( 1 );
}
