/*************************************************************************
 * test-trsf-3x3.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 17:45:00 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <stdio.h>
#include <string.h>
#include <time.h>

#include <bal-matrix.h>
#include <bal-lineartrsf-tools.h>

int main(int argc, char *argv[])
{
  int k;
  
  double m[9], t[9], i[9], p[9];
  double angle_interval[2] = {0.0, 1.57079}; /* 0, pi/2 */
  double scale_interval[2] = {0.7, 1.4};
  double shear_interval[2] = {0.0, 0.3};

  long int seedRandom = time(0);
  
  _SetRandomSeed( seedRandom );



  for ( k=1; k<=6; k++ ) {
    fprintf( stdout, "\n" );

    switch( k ) {
    case 1 :
      fprintf( stdout, "--- 2D rotation\n" );
      _Random2DRotationMatrix( m, angle_interval );
      break;
    case 2 :
      fprintf( stdout, "--- 2D similitude\n" );
      _Random2DSimilitudeMatrix( m, angle_interval, scale_interval );
      break;
    case 3 :
      fprintf( stdout, "--- 2D affine\n" );
      _Random2DAffineMatrix( m, angle_interval, scale_interval, shear_interval );
      break;
    case 4 :
      fprintf( stdout, "--- 3D rotation\n" );
      _Random3DRotationMatrix( m, angle_interval );
      break;
    case 5 :
      fprintf( stdout, "--- 3D similitude\n" );
      _Random3DSimilitudeMatrix( m, angle_interval, scale_interval );
      break;
    case 6 :
      fprintf( stdout, "--- 3D affine\n" );
      _Random3DAffineMatrix( m, angle_interval, scale_interval, shear_interval );
      break;
    }
    fprintf( stdout, "\n" );

    E_DMMatPrint( stdout, m, 3, "- random matrix" );
    fprintf( stdout, "\n" );

    E_DMMatTrans( m, t, 3 );
    E_DMMatMul( m, t, p, 3 );
    E_DMMatPrint( stdout, p, 3, "- M * M^t" );
    fprintf( stdout, "\n" );

    (void)InverseMat3x3( m, i );
    E_DMMatMul( m, i, p, 3 );
    E_DMMatPrint( stdout, p, 3, "- M * M^(-1)" );
    fprintf( stdout, "\n" );
  }

  return( 0 );
}
