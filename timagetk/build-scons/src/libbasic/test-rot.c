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
#include <math.h>

#include <stdlib.h>


int main (int argc, char *argv[] )
{
    int i, imax=10;
    double vec[3], n;
    double t, theta;
    double mat[9];
    double rot[3];
    double max = 2147483647; /* (2^31)-1 */

    (void)srandom(time(0));

    t = ( random() / max ) * 3.1415927;

    for ( i=0; i<imax; i++, t /= 10 ) {
        vec[0] = 2.0*random() / max - 1.0;
        vec[1] = 2.0*random() / max - 1.0;
        vec[2] = 2.0*random() / max - 1.0;

        n = sqrt( vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2] );
        vec[0] /= n;
        vec[1] /= n;
        vec[2] /= n;

        /* t = ( random() / max ) * 3.1415927; */

        fprintf( stderr, "trial #%2d\n", i );
        fprintf( stderr, "   random vector = [%f %f %f], angle = %g degrees\n", vec[0], vec[1], vec[2], t * 180.0/3.1415927 );

        vec[0] *= t;
        vec[1] *= t;
        vec[2] *= t;

        RotationMatrixFromRotationVector( mat, vec );
        fprintf( stderr, "   matrix = [ %f %f %f ]\n", mat[0], mat[1], mat[2] );
        fprintf( stderr, "            [ %f %f %f ]\n", mat[3], mat[4], mat[5] );
        fprintf( stderr, "            [ %f %f %f ]\n", mat[6], mat[7], mat[8] );

        RotationVectorFromRotationMatrix( rot, mat );

        theta = sqrt( rot[0]*rot[0] + rot[1]*rot[1] + rot[2]*rot[2] );
        fprintf( stderr, "   vector = [%f %f %f], angle = %f degrees\n", rot[0]/theta, rot[1]/theta, vec[2]/theta, theta * 180.0/3.1415927 );
        fprintf( stderr, "   error on angle = %f (%f %%)\n", theta - t, (theta-t)/theta * 100.0 );
    }

  return( 1 );
}
