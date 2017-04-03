/*************************************************************************
 * sour-filter.c
 *
 * $Id: sour-filter.c,v 1.2 2000/08/16 08:44:02 greg Exp $
 *
 * Copyright (c) INRIA 1998
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Thu Dec 16 17:07:10 MET 1999
 *
 * ADDITIONS, CHANGES
 *
 */


#include <sour-filter.h>

static int _verbose_ = 1;




void _InitMask( typeMask *theMask )
{
  theMask->nb = 0;
  theMask->theCoefs = (typeCoefficient *)NULL;
  theMask->maxdim = 0;
}

void _FreeMask( typeMask *theMask )
{
  if ( theMask->theCoefs != (typeCoefficient *)NULL )
    free( theMask->theCoefs );
  _InitMask( theMask );
}









int _CreateGaussianMask( typeMask *theMask,
			 double radiusInVoxels,
			 double sigma )
{
  int x, y, z, d, n;
  double squareR = radiusInVoxels*radiusInVoxels;
  double squareS = sigma*sigma;

  if ( radiusInVoxels <= 0 ) return( 0 );
  if ( sigma <= 0.01 ) return( 0 );

  if ( theMask->nb > 0 && theMask->theCoefs != (typeCoefficient *)NULL )
    _FreeMask( theMask );

  d = (int)(radiusInVoxels+1);
  n = 0;
  for ( z = -d; z <= d ; z ++ )
  for ( y = -d; y <= d ; y ++ )
  for ( x = -d; x <= d ; x ++ ) {
    if ( x*x + y*y + z*z <= squareR ) n++;
  }
  
  if ( n <= 0 ) return( 0 );

  theMask->theCoefs = (typeCoefficient *)NULL;
  theMask->theCoefs = (typeCoefficient *)malloc( n * sizeof(typeCoefficient) );
  if ( theMask->theCoefs == (typeCoefficient *)NULL ) {
    return( 0 );
  }
  n=0;
  for ( z = -d; z <= d ; z ++ )
  for ( y = -d; y <= d ; y ++ )
  for ( x = -d; x <= d ; x ++ ) {
    if ( x*x + y*y + z*z <= squareR ) {
      theMask->theCoefs[n].xoffset = x;
      theMask->theCoefs[n].yoffset = y;
      theMask->theCoefs[n].zoffset = z;
      theMask->theCoefs[n].c = exp( - (x*x + y*y + z*z)/squareS );
      n++;
    }
  }
  theMask->nb = n;
  theMask->maxdim = d;

  return( 1 );
}






void _ComputeTotalOffsetOfAMask( typeMask *theMask,
				 int *theDim )
{
  int i;

  for (i=0; i<theMask->nb; i++) {
    theMask->theCoefs[i].totaloffset = 
      theMask->theCoefs[i].zoffset * theDim[0] * theDim[1] +
      theMask->theCoefs[i].yoffset * theDim[0] + theMask->theCoefs[i].xoffset;
  }
}







int _FilterWithMaskAndWeights( float *theIm,
			       float *resIm,
			       float *theWeights,
			       int *theDim,
			       typeMask *theMask )
{
  int u, x, y, z;
  int dx = theDim[0];
  int dy = theDim[1];
  int dz = theDim[2];
  double sum, c;
  int i;

  _ComputeTotalOffsetOfAMask( theMask, theDim );

  u=0;
  for (z=0; z<dz; z++) {

    if ( _verbose_ ) {
      fprintf( stderr, " ... processing slice #%3d\r", z );
    }

    for (y=0; y<dy; y++)
    for (x=0; x<dx; x++,u++) {

      resIm[u] = 0.0;
      sum = 0.0;

      if ( x >= theMask->maxdim && x < dx-theMask->maxdim &&
	   y >= theMask->maxdim && y < dy-theMask->maxdim &&
	   z >= theMask->maxdim && z < dz-theMask->maxdim ) {
	for (i=0; i<theMask->nb; i++) {
	  c = (1.0 - theWeights[ u+theMask->theCoefs[i].totaloffset ])
	    * theMask->theCoefs[i].c;
	  resIm[u] += c * theIm[ u+theMask->theCoefs[i].totaloffset ];
	  sum += c;
	}

      } else {

	for (i=0; i<theMask->nb; i++) {
	  if ( z+theMask->theCoefs[i].zoffset < 0 || 
	       z+theMask->theCoefs[i].zoffset >= dz ) continue;
	  if ( y+theMask->theCoefs[i].yoffset < 0 || 
	       y+theMask->theCoefs[i].yoffset >= dy ) continue;
	  if ( x+theMask->theCoefs[i].xoffset < 0 || 
	       x+theMask->theCoefs[i].xoffset >= dx ) continue;
	  c = (1.0 - theWeights[ u+theMask->theCoefs[i].totaloffset ])
	    * theMask->theCoefs[i].c;
	  resIm[u] += c * theIm[ u+theMask->theCoefs[i].totaloffset ];
	  sum += c;
	}
      }

      if ( sum > 0.0 ) resIm[u] /= sum;
    }
    
  }
  return( 1 );
}
