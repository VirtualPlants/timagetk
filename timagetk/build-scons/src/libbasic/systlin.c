/*************************************************************************
 * systlin.c - Resolution of linear system
 *
 * $Id: systlin.c,v 1.5 2001/07/12 13:29:40 greg Exp $
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

#include <systlin.h>











int _SolveLinearSystem( double *mat, double *vec, double *x, int dim )
{
  double *theAllocatedBuffer = (double*)NULL;
  double *m, *v;
  int i, j, k;
  int rank=dim;

  int jmax;
  double p, max;
    

  if ( dim <= 0 ) return( 0 );

  if ( dim == 1 ) {
    x[0] = 0.0;
    if ( mat[0] == 0.0 ) return( 0 );
    x[0] = vec[0] / mat[0];
    return( 1 );
  }

  if ( dim == 2 ) {
    p = mat[0] * mat[3] - mat[1] * mat[2];
    x[0] = x[1] = 0.0;
    if ( p == 0.0 ) return( 0 );
    max  = (mat[3] * vec[0] - mat[1] * vec[1]) / p;
    x[1] = (mat[0] * vec[1] - mat[2] * vec[0]) / p;
    x[0] = max;
    return( 1 );
  }

  theAllocatedBuffer = (double*)malloc( (dim*dim + dim)*sizeof(double) );
  if ( theAllocatedBuffer == (double*)NULL ) return( 0 );
  
  m = v = theAllocatedBuffer;
  m += dim;

  (void)memcpy( m, mat, dim*dim*sizeof(double) );
  (void)memcpy( v, vec, dim*sizeof(double) );

  for ( i=0; i<dim; i++ ) x[i] = 0.0;
  
  
  for ( i=0; i<dim && rank==dim; i++ ) {
    /* recherche du plus grand element non nul sur la ligne i 
     */
    for ( max = fabs( m[i*dim] ), jmax = 0, j = 1; j < dim; j++ ) {
      if ( max < fabs( m[i*dim+j] ) ) {
	max = fabs( m[i*dim+j] );
	jmax = j;
      }
    }

    /* ligne nulle
     */
    if ( max == 0.0 ) {
      rank --;
      continue;
    }

    /* on annule la colonne du plus grand element
     */
    for ( k=0; k<dim; k++ ) {
      if ( k == i ) continue;
      p = m[k*dim+jmax] / m[i*dim+jmax];
      for ( j = 0; j < dim; j++ ) m[k*dim+j] -= p * m[i*dim+j];
      v[k] -= p * v[i];
    }
  }

  if ( rank < dim ) {
    free( theAllocatedBuffer );
    return( 0 );
  }

  /* on resoud 
   */
  for (i=0; i<dim; i++) {
    /* recherche de l'element non nul sur la ligne i 
     */
    for ( max = m[i*dim], jmax = 0, j = 1; j < dim; j++ ) {
      if ( fabs( max ) < fabs( m[i*dim+j] ) ) {
	max = m[i*dim+j];
	jmax = j;
      }
    }
    x[jmax] = v[i] / max;
  }

  free( theAllocatedBuffer );
  return( 1 );
}











#define TINY 1e-12

int _InverseSquareMatrix( double *mat, double *inv, int dim )
{
  double *m = (double*)NULL;
  int i, j, k;
  int kmax;
  int rank=dim;
  register double c, max;

  if ( dim <= 0 ) return( 0 );

  if ( dim == 1 ) {
    if ( mat[0] == 0.0 ) return( 0 );
    inv[0] = 1.0 / mat[0];
    return( 1 );
  }

  
  
  m = (double*)malloc( (dim*dim)*sizeof(double) );
  if ( m  == (double*)NULL ) return( 0 );

  (void)memcpy( m, mat, dim*dim*sizeof(double) );


  /* unit matrix
   */
  for ( i=0; i<dim*dim; i++ ) inv[i] = 0.0;
  for ( i=0; i<dim; i++ )     inv[i*dim+i] = 1.0;
  

  
  for ( j=0; j<dim; j++ ) {

    /* on se sert de l'element diagonal 
       comme pivot
       S'il est trop petit, on additionne une ligne d'en dessous
    */
    
    if ( (m[j*dim+j] > (-TINY)) && (m[j*dim+j] < TINY) ) {
      
      for ( kmax = j, max = 0.0, k = j+1; k < dim; k++ ) {
	c = ( m[k*dim+j] > 0.0 ) ? m[k*dim+j] : (-m[k*dim+j]) ;
	if ( (c > TINY) && (c > max) ) { max = c; kmax = k; }
      }
      if ( kmax == j ) {
	/* la ligne est nulle */
	rank --;
      } else {
	/* sinon, on additionne */
	for ( i=0; i<dim; i++ ) {
	  m[j*dim+i] += m[kmax*dim+i];
	  inv[j*dim+i] += inv[kmax*dim+i];
	}
      }
    }
    
    /* le pivot est l'element diagonal
     */

    if ( (m[j*dim+j] < (-TINY)) || (m[j*dim+j] > TINY) ) {

      /* on annule la colonne j sur les autres lignes
       */
      for (k=0; k<dim; k++) {
	if ( k == j ) continue;
	c = m[k*dim + j] / m[j*dim + j];
	for ( i=0; i<dim; i++ ) {
	  m[k*dim + i] -= c * m[j*dim + i];
	  inv[k*dim + i] -= c * inv[j*dim + i];
	}
      }
      /* on met a 1 l'element diagonal
       */
      c = m[j*dim + j];
      for ( i=0; i<dim; i++ ) {
	m[j*dim + i] /= c;
	inv[j*dim + i] /= c;
      }
    }
  }

  free( m );
  return( rank );
}




void _MultiplyTwoSquareMatrices( double *a, double *b, double *res, int dim )
{
  int i, j, k;
  if ( a == res || b == res ) return;
  
  for ( i=0; i<dim; i++ )
  for ( j=0; j<dim; j++ )
  for ( res[ i*dim+j] = 0.0, k = 0; k < dim; k++ )
    res[ i*dim+j ] += a[ i*dim+k ]*b[ k*dim+j ];
  
  return;
}

void _MultiplyMatrixByVector( double *a, double *b, double *res, int dim )
{
  int i, j;
  if ( a == res || b == res ) return;
  
  for ( i=0; i<dim; i++ )
  for ( res[i] = 0.0, j = 0; j < dim; j++ )
    res[ i ] += a[ i*dim+j ]*b[ j ];
  
  return;
}












#include <stdio.h>
#include <time.h>
#include <math.h>

#include <stdlib.h>


void _TestSystLin( int matrixDim )
{
  int dim = matrixDim;
  double *m, *inv, *res, *v, *a, *x;
  int i, j;
  double max = 2147483647; /* (2^31)-1 */
  double errmax, errmin, errsum;
  double errdmax, errdmin, errdsum;

  if ( dim < 2 ) dim = 4;
  
  m = (double *)malloc( dim*dim*sizeof(double) );
  inv = (double *)malloc( dim*dim*sizeof(double) );
  res = (double *)malloc( dim*dim*sizeof(double) );
  a = (double *)malloc( dim*sizeof(double) );
  v = (double *)malloc( dim*sizeof(double) );
  x = (double *)malloc( dim*sizeof(double) );


  for ( i=0; i<dim*dim; i++ ) m[i] = random() / max;
  for ( i=0; i<dim; i++ ) a[i] = random() / max;


  
  _MultiplyMatrixByVector( m, a, v, dim );

  if ( _SolveLinearSystem( m,v,x,dim ) == 1 ) {
    errmax = errmin = errsum = fabs( x[0]-a[0] );
    for ( i=1; i<dim; i++ ) {
      if ( errmax < fabs( x[i]-a[i] ) ) errmax = fabs( x[i]-a[i] );
      if ( errmin > fabs( x[i]-a[i] ) ) errmin = fabs( x[i]-a[i] );
      errsum += fabs( x[i]-a[i] );
    }
    printf( " ... linear system AX=(AY) (dim=%d) solved;\n", dim );
    printf( "        errors |Yi-Xi| are min=%12g   average=%12g   max=%12g\n",
	    errmin, errsum/dim, errmax );
  } else {
    printf( " ... error when solving linear system\n" );
  } 
  

  
  if ( _InverseSquareMatrix( m, inv, dim ) == dim ) {
    
    _MultiplyTwoSquareMatrices( m, inv, res, dim );
    errdmin = errdmax = errdsum = res[0];
    errdsum = 0.0;
    errmin = errmax = errsum = res[1];
    errsum = 0.0;

    for ( i=0; i<dim; i++ )
    for ( j=0; j<dim; j++ ) {
      if ( i == j ) {
	if ( errdmax < res[i*dim+j] ) errdmax = res[i*dim+j];
	if ( errdmin > res[i*dim+j] ) errdmin = res[i*dim+j];
	errdsum += res[i*dim+j];
      } else {
	if ( errmax < res[i*dim+j] ) errmax = res[i*dim+j];
	if ( errmin > res[i*dim+j] ) errmin = res[i*dim+j];
	errsum += res[i*dim+j];
      }
    }

    printf( " ... matrix M (dim=%d) inverted;", dim );
    printf( " errors |(MM^(-1))i| are:\n" );
    printf( "        on the diagonal: min=%12g   average=%12g   max=%12g\n",
	    errdmin, errdsum/dim, errdmax );
    printf( "        elsewhere      : min=%12g   average=%12g   max=%12g\n",
	    errmin, errsum/((dim-1)*dim), errmax );
  } else {
    printf( " ... error when inverting matrix\n" );
  }
  
  free( m );
  free( inv );
  free( res );
  free( a );
  free( v );
  free( x );
}
