/*************************************************************************
 * test-transfo-spherical.c - 
 *
 * $Id: test-transfo-spherical.c,v 1.2 2000/10/05 16:01:43 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Mon Sep 11 18:52:32 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <transfo.h>

int main (int argc, char *argv[] )
{
  int i, n=4;
  double v[3];
  double u1[3], u2[3], u3[3];
  double w;
  double t, p;

  double max = 2147483647; /* (2^31)-1 */
  double r2d = 180.0 / 3.1415926536;

  (void)srandom(time(0));

  if ( argc >= 2 ) {
    if ( sscanf( argv[1], "%d", &n ) != 1 ) {
      n = 4;
    }
  }

  for ( i=0; i<n ;i++ ) {
    printf( "trial %3d/%d\n", i+1, n );
    v[0] = random() / max * 2.0 - 1.0;
    v[1] = random() / max * 2.0 - 1.0;
    v[2] = random() / max * 2.0 - 1.0;

    w = sqrt( v[0]*v[0] + v[1]*v[1] + v[2]*v[2] );
    if ( w < 0.00000001 ) {
      i --;
      continue;
    }
    
    v[0] /= w;
    v[1] /= w;
    v[2] /= w;
    
    printf( " ... (%f,%f,%f) -> ", v[0], v[1], v[2] );
    
    UnitVectorToSphericalAngles( v, &t, &p );
    
    printf( "(theta=%f, phi=%f) -> ", t*r2d, p*r2d );
    
    /*
    SphericalAnglesToUnitVector( t, p, u );
    printf( "(%g,%g,%g)\n", fabs(v[0]-u[0]), fabs(v[1]-u[1]), fabs(v[2]-u[2]) );
    */

    SphericalAnglesToUnitsVectors( t, p, u1, u2, u3 );
    printf( "(%g,%g,%g)\n", fabs(v[0]-u1[0]), fabs(v[1]-u1[1]), fabs(v[2]-u1[2]) );
    printf( " |u1| = %f, |u2| = %f, |u3| = %f\n",
	    sqrt( u1[0]*u1[0] + u1[1]*u1[1] + u1[2]*u1[2] ),
	    sqrt( u2[0]*u2[0] + u2[1]*u2[1] + u2[2]*u2[2] ),
	    sqrt( u3[0]*u3[0] + u3[1]*u3[1] + u3[2]*u3[2] ) );
    printf( " |u1.u2| = %f, |u1.u3| = %f, |u2.u3| = %f\n",
	    fabs( u1[0]*u2[0] + u1[1]*u2[1] + u1[2]*u2[2] ),
	    fabs( u1[0]*u3[0] + u1[1]*u3[1] + u1[2]*u3[2] ),
	    fabs( u2[0]*u3[0] + u2[1]*u3[1] + u2[2]*u3[2] ) );

  }

  return( 0 );
}
  
  
