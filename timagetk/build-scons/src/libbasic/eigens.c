/*************************************************************************
 * eigens.c -
 *
 * $Id: eigens.c,v 1.5 2000/10/09 09:00:32 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Sep  7 11:04:44 MET DST 2000
 *
 *
 * DESCRIPTION
 *
 * Compute '_ComputeEigensOfSymetricSquareMatrix()'
 * and eventually sort '_ComputeAndSortEigensOfSymetricSquareMatrix()'
 * the eigen values and vectors of a symetric square matrix.
 * 
 * Sort is done in increasing order: the first value is the smallest one.
 * 
 *
 * The eigenvector associated with the eigenvalue #i is the ith column
 * of the matrix eigenvectors, ie eigenvectors[j*dim+i], j=0...dim-1
 *
 *
 *
 * ADDITIONS, CHANGES
 * - Sat Oct  7 00:01:48 MEST 2000, G. Malandain
 *   suppression de _MultiplyMatrixByVector() et de #include <systlin.h>
 *
 */


#include <eigens.h>





static void _SymetricSquareMatrixToTriDiagonal( double *tr,
						double *td,
						int dim )
{
  /*   tr : matrice originale symetrique                                  */
  /*        au retour, elle contiendra la matrice de passage              */
  /*   td : matrice tridiagonale : td[0-(dim-2)] elements non diagonaux   */
  /*                               td[n-(2dim-1)] elements diagonaux      */
  /*   dim : dimension de la matrice                                      */
  /************************************************************************/
	
  int i, j, k, l;
  double scale, hh, h, g, f;
  
  for (i=dim-1; i>=1 ; i--) {
    l = i-1;
    h = scale = 0.0;
    if (l > 0) {
      for (k=0; k<=l; k++)
	scale += fabs(tr[i*dim+k]);
      if (scale == 0.0)
	td[i] = tr[i*dim+l];
      else {
	for (k=0; k<=l; k++) {
	  tr[i*dim+k] /= scale;
	  h += tr[i*dim+k]*tr[i*dim+k];
	}
	f = tr[i*dim+l];
	g = f>0 ? -sqrt(h) : sqrt(h);
	td[i] = scale*g;
	h -= f*g;
	tr[i*dim+l] = f-g;
	f = 0.0;
	for (j=0; j<=l; j++) {
	  /* Next statement can be omitted if eigenvectors not wanted */
	  tr[j*dim+i] = tr[i*dim+j]/h;
	  g = 0.0;
	  for (k=0 ;k<=j; k++)
	    g += tr[j*dim+k]*tr[i*dim+k];
	  for (k=j+1; k<=l; k++)
	    g += tr[k*dim+j]*tr[i*dim+k];
	  td[j] = g/h;
	  f += td[j]*tr[i*dim+j];
	}
	hh = f/(h+h);
	for (j=0; j<=l; j++) {
	  f = tr[i*dim+j];
	  td[j] = g = td[j]-hh*f;
	  for (k=0; k<=j; k++)
	    tr[j*dim+k] -= (f*td[k]+g*tr[i*dim+k]);
	}
      }
    } else
      td[i] = tr[i*dim+l];
    td[dim+i] = h;
  }

  /* Next statement can be omitted if eigenvectors not wanted */
  td[dim] = 0.0;
  td[0] = 0.0;

  /* Contents of this loop can be omitted if eigenvectors not
     wanted except for statement d[i]=tr[i*dim+i]; */
  for (i=0;i<dim;i++) {
    l = i-1;
    if (td[dim+i]) {
      for (j=0; j<=l; j++) {
	g = 0.0;
	for (k=0; k<=l; k++)
	  g += tr[i*dim+k]*tr[k*dim+j];
	for (k=0; k<=l; k++)
	  tr[k*dim+j] -= g*tr[k*dim+i];
      }
    }
    td[dim+i] = tr[i*dim+i];
    tr[i*dim+i] = 1.0;
    for (j=0; j<=l; j++)
      tr[j*dim+i] = tr[i*dim+j]=0.0;
  }

  for (i=1;i<dim;i++)
    td[i-1] = td[i];
  td[dim-1] = 0.0;
  
}




















static int _ComputeEigensOfTriDiagonalMatrix( double *td, 
					      double *tr, 
					      int dim )
{
  int i, l, k, iter, m;
  double b, c, dd, f, g, p, r, s;
  
  for (l=0; l<dim; l++) {
    iter = 0;

    do {
      for (m=l; m<dim-1; m++) {
	dd = fabs( td[dim+m] ) + fabs( td[dim+m+1] );
	if ((double)(fabs(td[m]) + dd) == dd)
	  break;
      }
      if (m != l) {

	if (iter++ == 30) return( 0 );

	g = (td[dim+l+1] - td[dim+l]) / (2.0*td[l]);
	r = sqrt((g*g)+1.0);
	g = td[dim+m] - td[dim+l] + td[l]/( g + ((g<0) ? -fabs(r) : fabs(r)) );
	s = c =1.0;
	p = 0.0;
	for (i=m-1; i>=l; i--) {
	  f = s*td[i];
	  b = c*td[i];
	  if (fabs(f) >= fabs(g)) {
	    c = g/f;
	    r = sqrt((c*c)+1.0);
	    td[i+1] = f*r;
	    c *= (s=1.0/r);
	  }
	  else {
	    s = f/g;
	    r = sqrt((s*s)+1.0);
	    td[i+1] = g*r;
	    s *= (c=1.0/r);
	  }
	  g = td[dim+i+1] - p;
	  r = (td[dim+i]-g)*s + 2.0*c*b;
	  p = s*r;
	  td[dim+i+1] =g+p;
	  g = c*r-b;

	  /* Next loop can be omitted if eigenvectors not wanted */
	  for (k=0; k<dim; k++) {
	    f = tr[k*dim+i+1];
	    tr[k*dim+i+1] = s*tr[k*dim+i]+c*f;
	    tr[k*dim+i] = c*tr[k*dim+i]-s*f;
	  }

	}
	td[dim+l] = td[dim+l]-p;
	td[l] = g;
	td[m] = 0.0;
      }
    } while (m != l);

  }
  return( 1 );
}


















static void _SortEigenValuesInIncreasingOrder( double *val, int *prm, int left, int right )
{
  int i, midd, last;
  double dtmp;
  int    itmp;

  if (left>=right) return;

  /* swap: left <- -> (left+right)/2
   */
  midd = (left+right)/2;
  dtmp = val[left]; val[left] = val[midd]; val[midd] = dtmp;
  itmp = prm[left]; prm[left] = prm[midd]; prm[midd] = itmp;

  last = left;

  for ( i=left+1; i<=right; i++ )
    if ( val[i] < val[left] ) {
      /* last is either left (initialization) or the last
	 point with value < left
	 if we find such a point, we increase last
	 and swap #last with #i
	 so we get 
	 #left, #left+1, ..., #last, #last+1, ...
	 such that {#left+1, ..., #last} with have a value < #left
      */
      last++;
      dtmp = val[last]; val[last] = val[i]; val[i] = dtmp;
      itmp = prm[last]; prm[last] = prm[i]; prm[i] = itmp;
    }
  
  /* swap #left <- -> #last
     so we get 
     #left, #left+1, ..., #last, #last+1, ..., #right
     such that {#left, ... #last-1} < #last
     and {#last+1, ..., #right} >= #last
  */
  dtmp = val[left]; val[left] = val[last]; val[last] = dtmp;
  itmp = prm[left]; prm[left] = prm[last]; prm[last] = itmp;

  _SortEigenValuesInIncreasingOrder( val, prm, left, last-1 );
  _SortEigenValuesInIncreasingOrder( val, prm, last+1, right);
}










static void _SortEigenValuesInAbsIncreasingOrder( double *val, int *prm, int left, int right )
{
  int i, midd, last;
  double dtmp;
  int    itmp;

  if (left>=right) return;

  /* swap: left <- -> (left+right)/2
   */
  midd = (left+right)/2;
  dtmp = val[left]; val[left] = val[midd]; val[midd] = dtmp;
  itmp = prm[left]; prm[left] = prm[midd]; prm[midd] = itmp;

  last = left;

  for ( i=left+1; i<=right; i++ )
    if ( fabs( val[i] ) < fabs( val[left] ) ) {
      /* last is either left (initialization) or the last
	 point with value < left
	 if we find such a point, we increase last
	 and swap #last with #i
	 so we get 
	 #left, #left+1, ..., #last, #last+1, ...
	 such that {#left+1, ..., #last} with have a value < #left
      */
      last++;
      dtmp = val[last]; val[last] = val[i]; val[i] = dtmp;
      itmp = prm[last]; prm[last] = prm[i]; prm[i] = itmp;
    }
  
  /* swap #left <- -> #last
     so we get 
     #left, #left+1, ..., #last, #last+1, ..., #right
     such that {#left, ... #last-1} < #last
     and {#last+1, ..., #right} >= #last
  */
  dtmp = val[left]; val[left] = val[last]; val[last] = dtmp;
  itmp = prm[left]; prm[left] = prm[last]; prm[last] = itmp;

  _SortEigenValuesInAbsIncreasingOrder( val, prm, left, last-1 );
  _SortEigenValuesInAbsIncreasingOrder( val, prm, last+1, right);
}





















#define DBLZERO 1e-12


int _ComputeEigensOfSymetricSquareMatrix( const double *mat,
					  double *eigenvalues,
					  double *eigenvectors,
					  int dim )
{
  int i, j;
  double n;
  double *td = (double*)NULL;

  /* allocation pour une matrice tridiagonale
   */
  td = (double*)malloc( 2*dim*sizeof( double ) );
  if ( td == (double*)NULL ) {
    return( 0 );
  }
  
  /* calcul d'une matrice tri-diagonale
     1. on recopie 'mat' dans 'eigenvectors'
     2. on calcule
   */
  (void)memcpy( eigenvectors, mat, dim*dim*sizeof(double) );
  _SymetricSquareMatrixToTriDiagonal( eigenvectors, td, dim );
  
  if ( _ComputeEigensOfTriDiagonalMatrix( td, eigenvectors, dim ) != 1 ) {
    free( td );
    return( 0 );
  }

  for (i=0; i<dim; i++)
    eigenvalues[i] = td[dim+i];

  /* normalize the eigenvectors
   */
  for ( i=0; i<dim; i++ ) {
    n = 0.0;
    for (j=0; j<dim; j++) n += eigenvectors[j*dim+i]*eigenvectors[j*dim+i];
    if ( n < DBLZERO ) continue;
    n = sqrt( n );
    for (j=0; j<dim; j++) eigenvectors[j*dim+i] /= n;
  }


  free( td );
  return( 1 );
}


















typedef enum {
  _INCREASING_,
  _ABS_INCREASING_
} enumTypeSort;




static int _SortEigens( double *eigenvalues,
			double *eigenvectors,
			int dim,
			enumTypeSort typeSort )
{
  int i, j, k;
  int *perm1 = (int*)NULL;
  int *perm2 = (int*)NULL;
  double dtmp;
  int    itmp;



  /* sort the eigen values
     1. Allocation and initialisation
     2. sort
   */

  perm1 = (int*)malloc( 2*dim*sizeof(int) );
  if ( perm1 == (int*)NULL ) {
    return( 0 );
  }
  perm2 = perm1 + dim;
  for (i=0; i<dim; i++)
    perm1[i] = perm2[i] = i;
  
  
  /* sort the eigenvalues in increasing order
     (the first one is the smallest)
     'perm1' gives the permutation for the eigenvectors
  */
  switch( typeSort ) {
  default :
  case _ABS_INCREASING_ :
    _SortEigenValuesInAbsIncreasingOrder( eigenvalues, perm1, 0, dim-1 );
    break;
  case _INCREASING_ :
    _SortEigenValuesInIncreasingOrder( eigenvalues, perm1, 0, dim-1 );
    break;
  }
  
  /* Apply the permutation to the eigenvectors
   */
  for ( i=0; i<dim; i++ ) {
    for (j=i; ( (j<dim) && (perm2[j]!=perm1[i]) ); j++) ;
    if ( j != i ) {
      itmp = perm2[i]; perm2[i] = perm2[j]; perm2[j] = itmp;
      for ( k=0; k<dim; k++ ) {
	dtmp = eigenvectors[k*dim+i];
	eigenvectors[k*dim+i] = eigenvectors[k*dim+j];
	eigenvectors[k*dim+j] = dtmp;
      }
    }
  }

  free( perm1 );

  return( 1 );
  
}






int _SortEigensInIncreasingOrder( double *eigenvalues,
				  double *eigenvectors,
				  int dim )
{
  return( _SortEigens( eigenvalues, eigenvectors, dim, _INCREASING_ ) );
}


int _SortEigensInAbsIncreasingOrder( double *eigenvalues,
				  double *eigenvectors,
				  int dim )
{
  return( _SortEigens( eigenvalues, eigenvectors, dim, _ABS_INCREASING_ ) );
}











#include <stdio.h>
#include <time.h>
#include <math.h>

#include <stdlib.h>


void _TestEigens( int matrixDim )
{
  int dim = matrixDim;
  double *mat, *val, *vec, *v, *w;
  double nv, nw;
  
  double max = 2147483647; /* (2^31)-1 */
  int i, j, k;
  double p;
  double lmin = 0.0, lmax = 0.0, lsum = 0.0;
  double tmin = 0.0, tmax = 0.0, tsum = 0.0;
  double r2d = 180.0 / 3.1415926536;
  double angle;


  if ( dim < 2 ) dim = 4;

  mat = (double *)malloc( dim*dim*sizeof(double) );
  val = (double *)malloc( dim*sizeof(double) );  
  vec = (double *)malloc( dim*dim*sizeof(double) );
  v   = (double *)malloc( dim*sizeof(double) );
  w   = (double *)malloc( dim*sizeof(double) );

  
  for ( i=0; i<dim; i++ ) {
    mat[i*dim+i] = random() / max;
    for (j=i+1; j<dim; j++ )
      mat[j*dim+i] = mat[i*dim+j] = random() / max;
  }

  if ( _ComputeEigensOfSymetricSquareMatrix( mat, val, vec, dim ) == 1 ) {
    

    if ( 0 )
      (void)_SortEigensInIncreasingOrder( val, vec, dim );
    else
      (void)_SortEigensInAbsIncreasingOrder( val, vec, dim );
    

    for ( i=0; i<dim; i++ ) {
      for (j=0; j<dim; j++ ) v[j] = vec[j*dim+i];
      for (j=0; j<dim; j++ ) {
	w[j] = 0;
	for (k=0; k<dim; k++ )
	  w[j] += mat[j*dim+k] * v[k];
      }
      


      nv = nw = p = 0;
      for (j=0; j<dim; j++ ) {
	nv += v[j]*v[j];
	nw += w[j]*w[j];
	p  += v[j]*w[j];
      }

      if ( p / sqrt(nv * nw) < -1.0 + 1e-18 ) {
	angle = 180.0;
      } else if ( p / sqrt(nv * nw) > 1.0 - 1e-18 ) {
	angle = 0.0;
      } else {
	angle = r2d * acos( p / sqrt(nv * nw) );
      }
      if ( angle > 90.0 ) angle = 180.0 - angle;
      

      if ( i == 0 ) {
	lmin = lmax = lsum = fabs( fabs(val[i]) - sqrt( nw / nv ) );
	tmin = tmax = tsum = angle;
      } else {
	if ( lmin > fabs(fabs(val[i]) - sqrt(nw/nv)) ) lmin =  fabs(fabs(val[i]) - sqrt(nw/nv));
	if ( lmax < fabs(fabs(val[i]) - sqrt(nw/nv)) ) lmax =  fabs(fabs(val[i]) - sqrt(nw/nv));
	lsum +=  fabs(fabs(val[i]) - sqrt(nw/nv));
	if ( tmin > angle ) tmin = angle;
	if ( tmax < angle ) tmax = angle;
	tsum += angle;
      }
    }
    
    printf( " ... eigen values and vectors of M (dim=%d) computed\n", dim );

    if ( 1 ) {
      printf( " (%f", val[0] );
      for ( i=1; i<dim; i++ ) printf( ",%f", val[i] );
      printf( ")\n" );

      for ( p=0,j=0; j<dim; j++ ) p += vec[j*dim]*vec[j*dim];
      printf( " (|%f|", sqrt( p ) );
      for ( i=1; i<dim; i++ ) {
	for ( p=0,j=0; j<dim; j++ ) p += vec[j*dim+i]*vec[j*dim+i];
	printf( ",|%f|", sqrt( p ) );
      }
      printf( ")\n" );
    }


    printf( "     errors ||li| - |A*Vi|/|Vi|| are:" );
    printf( " min=%12g   average=%12g   max=%12g\n",
	    lmin, lsum/dim, lmax );
    printf( "     errors acos((A*Vi.Vi)/(|A*Vi||Vi|)) (degrees):" );
    printf( " min=%12g   average=%12g   max=%12g\n",
	    tmin, tsum/dim, tmax );
  } else {
    printf( " ... error when computing eigens\n" );
  }
  
  free( mat );
  free( val );
  free( vec );
  free( v );
  free( w );
}
