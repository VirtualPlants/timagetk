/*************************************************************************
 * linearFiltering-contours.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Jan  2 10:30:58 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <convert.h>
#include <linearFiltering.h>
#include <pixel-operation.h>
#include <zcross.h>


static int _verbose_ = 1;
static int _debug_ = 0;



void setVerboseInLinearFilteringContours( int v )
{
  _verbose_ = v;
}

void incrementVerboseInLinearFilteringContours(  )
{
  _verbose_ ++;
}

void decrementVerboseInLinearFilteringContours(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}


/************************************************************
 *
 * Gradient extrema based contours
 *
 ************************************************************/


/* 
 * epsilon value to select gradient extrema candidates
 * points with gradient modulus below this threshold are not
 * considered as potential extrema
 */
static double _EPSILON_NORM_ = 0.5;

void setGradientModulusThresholdInLinearFilteringContours( double epsilon )
{
  if ( epsilon > 0.0 ) {
    _EPSILON_NORM_ = epsilon;
  }
}





/*
 * epsilon value to decide of the interpolation type.
 * If one derivative's absolute value is larger than this
 * epsilon (close to one), then we use the nearest value
 * else we perform a [bi,tri]linear interpolation.
 */
static double _EPSILON_DERIVATIVE_ = 0.95;

void setGradientDerivativeThresholdInLinearFilteringContours( double epsilon )
{
  if ( (epsilon > 0.0) && (epsilon < 1.0) ) {
    _EPSILON_DERIVATIVE_ = 1.0 - epsilon;
  }
}
 




static void localGradientModulus2D( float *gradient_modulus,
                               float *derivative_along_X,
                               float *derivative_along_Y,
                               int length )
{
  register int i;
  register float *norme = gradient_modulus;
  register float *gx = derivative_along_X;
  register float *gy = derivative_along_Y;
  
  for ( i=0; i<length; i++, norme++, gx++, gy++ )
    *norme = sqrt( (*gx)*(*gx) + (*gy)*(*gy) );
}





static void localGradientModulus3D( float *gradient_modulus,
                        float *derivative_along_X,
                        float *derivative_along_Y,
                        float *derivative_along_Z,
                        int length )
{
  register int i;
  register float *norme = gradient_modulus;
  register float *gx = derivative_along_X;
  register float *gy = derivative_along_Y;
  register float *gz = derivative_along_Z;
  
  for ( i=0; i<length; i++, norme++, gx++, gy++, gz++ )
    *norme = sqrt( (*gx)*(*gx) + (*gy)*(*gy) + (*gz)*(*gz) );
}





static void removeGradientNonMaxima2D( float *maxima,
                                       float *gx,
                                       float *gy,
                                       float *norme,
                                       int *bufferDims )
{
  /* 
   * the buffer norme[0] contains the gradient modulus of the 
   * previous slice, the buffer norme[1] the ones of the
   * slice under study, while norme[2] containes the ones
   * of the next slice.
   */
  /*
   * dimensions
   */
  register int dimx = bufferDims[0];
  int dimy = bufferDims[1];
  int dimxMinusOne = dimx - 1;
  int dimxPlusOne = dimx + 1;
  int dimyMinusOne = dimy - 1;
  /* 
   * pointers
   */
  register float *fl_pt1 = (float*)NULL;
  register float *fl_pt2 = (float*)NULL;
  register float *fl_max = (float*)NULL;
  register float *fl_nor = (float*)NULL;
  register float *fl_upper_left = (float*)NULL;
  /*
   * coordinates and vector's components
   */
  register int x, y;
  register double normalized_gx;
  register double normalized_gy;
  register double x_point_to_be_interpolated;
  register double y_point_to_be_interpolated;
  int x_upper_left_corner;
  int y_upper_left_corner;
  /*
   * coefficients
   */ 
  register double dx, dy, dxdy;
  double c00, c01, c10, c11;
  /*
   * modulus
   */
  double interpolated_norme;

  /*
   * we set the image border to zero.
   * First the borders along X direction,
   * second, the borders along the Y direction.
   */
  fl_pt1 = maxima;
  fl_pt2 = maxima + (dimy-1)*dimx;
  for (x=0; x<dimx; x++, fl_pt1++, fl_pt2++ )
    *fl_pt1 = *fl_pt2 = 0.0;
  fl_pt1 = maxima + dimx;
  fl_pt2 = maxima + dimx + dimx - 1;
  for (y=1; y<dimy-1; y++, fl_pt1+=dimx, fl_pt2+=dimx )
    *fl_pt1 = *fl_pt2 = 0.0;
  
  /*
   * We investigate the middle of the image.
   */
  /* 
   * Pointers are set to the first point
   * to be processed.
   */
  fl_max = maxima + dimx + 1;
  fl_pt1 = gx + dimx + 1;
  fl_pt2 = gy + dimx + 1;
  fl_nor = norme + dimx + 1;
  for ( y=1; y<dimyMinusOne; y++, fl_max+=2, fl_pt1+=2, fl_pt2+=2, fl_nor+=2 )
  for ( x=1; x<dimxMinusOne; x++, fl_max++,  fl_pt1++,  fl_pt2++,  fl_nor++ ) {
    /*
     * If the modulus is too small, go to the next point.
     */
    if ( *fl_nor < _EPSILON_NORM_ ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * We normalize the vector gradient.
     */
    normalized_gx = *fl_pt1 / *fl_nor;
    normalized_gy = *fl_pt2 / *fl_nor;

    /*
     * May we use the nearest value?
     */
    if ( (-normalized_gx > _EPSILON_DERIVATIVE_) ||
         (normalized_gx > _EPSILON_DERIVATIVE_) ||
         (-normalized_gy > _EPSILON_DERIVATIVE_) ||
         (normalized_gy > _EPSILON_DERIVATIVE_) ) {
      /*
       * First point to be interpolated.
       */
      x_upper_left_corner = (int)( (double)x + normalized_gx + 0.5 );
      y_upper_left_corner = (int)( (double)y + normalized_gy + 0.5 );
      interpolated_norme = *(norme + (x_upper_left_corner + y_upper_left_corner * dimx));
      if ( *fl_nor <= interpolated_norme ) {
        *fl_max = 0.0;
        continue;
      }
      /*
       * Second point to be interpolated.
       */
      x_upper_left_corner = (int)( (double)x - normalized_gx + 0.5 );
      y_upper_left_corner = (int)( (double)y - normalized_gy + 0.5 );
      interpolated_norme = *(norme + (x_upper_left_corner + y_upper_left_corner * dimx));
      if ( *fl_nor < interpolated_norme ) {
        *fl_max = 0.0;
        continue;
      }
      /*
       * We found a gradient extrema.
       */
      *fl_max = *fl_nor;
      continue;
    }
    

    /*
     * From here we perform a bilinear interpolation
     */

    /*
     * First point to be interpolated.
     * It is the current point + an unitary vector
     * in the direction of the gradient.
     * It must be inside the image.
     */
    x_point_to_be_interpolated = (double)x + normalized_gx;
    y_point_to_be_interpolated = (double)y + normalized_gy;
    if ( (x_point_to_be_interpolated < 0.0) ||
         (x_point_to_be_interpolated >= dimxMinusOne) ||
         (y_point_to_be_interpolated < 0.0) ||
         (y_point_to_be_interpolated >= dimyMinusOne) ) {
      *fl_max = 0.0;
      continue;
    }
    /* 
     * Upper left corner,
     * coordinates of the point to be interpolated
     * with respect to this corner.
     */
    x_upper_left_corner = (int)x_point_to_be_interpolated;
    y_upper_left_corner = (int)y_point_to_be_interpolated;
    dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
    dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
    dxdy = dx * dy;
    /* 
     * bilinear interpolation of the gradient modulus 
     * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
     *   norme[0,0] * ( 1 - dx) * ( 1 - dy ) +
     *   norme[1,0] * ( dx ) * ( 1 - dy ) +
     *   norme[0,1] * ( 1 - dx ) * ( dy ) +
     *   norme[1,1] * ( dx ) * ( dy )
     */
    c00 = 1.0 - dx - dy + dxdy;
    c10 = dx - dxdy;
    c01 = dy - dxdy;
    c11 = dxdy;
    fl_upper_left = norme + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c00 +
      *(fl_upper_left + 1) * c10 +
      *(fl_upper_left + dimx) * c01 +
      *(fl_upper_left + dimxPlusOne) * c11;
    /*
     * We compare the modulus of the point with the
     * interpolated modulus. It must be larger to be
     * still considered as a potential gradient extrema.
     *
     * Here, we consider that it is strictly superior.
     * The next comparison will be superior or equal.
     * This way, the extrema is in the light part of the
     * image. 
     * By inverting both tests, we can put it in the
     * dark side of the image.
     */
    if ( *fl_nor <= interpolated_norme ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * Second point to be interpolated.
     * It is the current point - an unitary vector
     * in the direction of the gradient.
     * It must be inside the image.
     */
    x_point_to_be_interpolated = (double)x - normalized_gx;
    y_point_to_be_interpolated = (double)y - normalized_gy;
    if ( (x_point_to_be_interpolated < 0.0) ||
         (x_point_to_be_interpolated >= dimxMinusOne) ||
         (y_point_to_be_interpolated < 0.0) ||
         (y_point_to_be_interpolated >= dimyMinusOne) ) {
      *fl_max = 0.0;
      continue;
    }
    /* 
     * Upper left corner.
     */
    x_upper_left_corner = (int)x_point_to_be_interpolated;
    y_upper_left_corner = (int)y_point_to_be_interpolated;
    /* we do not recompute the coefficients
    dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
    dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
    dxdy = dx * dy;
    */
    /*
     * We may use the previous coefficients.
     * norme[x_point_to_be_interpolated, y_point_to_be_interpolated] =
     *   norme[0,0] * c11 +
     *   norme[1,0] * c01 +
     *   norme[0,1] * c10 +
     *   norme[1,1] * c00
     *
     * WARNING: it works only if the cases where one derivative is close
     *          to -/+ 1 are already be independently processed, else
     *          it may lead to errors.
     */
    /* we do not recompute the coefficients
    c00 = 1.0 - dx - dy + dxdy;
    c10 = dx - dxdy;
    c01 = dy - dxdy;
    c11 = dxdy;
    fl_upper_left = norme + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c00 +
      *(fl_upper_left + 1) * c10 +
      *(fl_upper_left + dimx) * c01 +
      *(fl_upper_left + dimxPlusOne) * c11;
    */
    fl_upper_left = norme + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c11 +
      *(fl_upper_left + 1) * c01 +
      *(fl_upper_left + dimx) * c10 +
      *(fl_upper_left + dimxPlusOne) * c00;
    /*
     * Last test to decide whether or not we 
     * have an extrema
     */
    if ( *fl_nor < interpolated_norme ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * We found a gradient extrema.
     */
    *fl_max = *fl_nor;
  }
}


static void removeGradientNonMaxima3D( float *maxima,
                                       float *gx,
                                       float *gy,
                                       float *gz,
                                       float **norme,
                                       int *bufferDims )
{
  /* 
   * the buffer norme[0] contains the gradient modulus of the 
   * previous slice, the buffer norme[1] the ones of the
   * slice under study, while norme[2] containes the ones
   * of the next slice.
   */
  /*
   * dimensions
   */
  register int dimx = bufferDims[0];
  int dimy = bufferDims[1];
  int dimxMinusOne = dimx - 1;
  int dimxPlusOne = dimx + 1;
  int dimyMinusOne = dimy - 1;
  /* 
   * pointers
   */
  register float *fl_pt1 = (float*)NULL;
  register float *fl_pt2 = (float*)NULL;
  register float *fl_pt3 = (float*)NULL;
  register float *fl_max = (float*)NULL;
  register float *fl_nor = (float*)NULL;
  register float *fl_upper_left = (float*)NULL;
  /*
   * coordinates and vector's components
   */
  register int x, y;
  int z;
  register double normalized_gx;
  register double normalized_gy;
  register double normalized_gz;
  register double x_point_to_be_interpolated;
  register double y_point_to_be_interpolated;
  register double z_point_to_be_interpolated;
  int x_upper_left_corner;
  int y_upper_left_corner;
  int z_upper_left_corner;
  /*
   * coefficients
   */ 
  register double dx, dy, dz;
  register double dxdy, dxdz, dydz;
  double c000, c010, c100, c110;
  double c001, c011, c101, c111;
  /*
   * modulus
   */
  double interpolated_norme;

  /*
   * we set the image border to zero.
   * First the borders along X direction,
   * second, the borders along the Y direction.
   */
  fl_pt1 = maxima;
  fl_pt2 = maxima + (dimy-1)*dimx;
  for (x=0; x<dimx; x++, fl_pt1++, fl_pt2++ )
    *fl_pt1 = *fl_pt2 = 0.0;
  fl_pt1 = maxima + dimx;
  fl_pt2 = maxima + dimx + dimx - 1;
  for (y=1; y<dimy-1; y++, fl_pt1+=dimx, fl_pt2+=dimx )
    *fl_pt1 = *fl_pt2 = 0.0;
  
  /*
   * We investigate the middle of the image.
   */
  /* 
   * Pointers are set to the first point
   * to be processed.
   */
  fl_max = maxima + dimx + 1;
  fl_pt1 = gx + dimx + 1;
  fl_pt2 = gy + dimx + 1;
  fl_pt3 = gz + dimx + 1;
  fl_nor = norme[1] + dimx + 1;
  z = 1;
  for ( y=1; y<dimyMinusOne; y++, fl_max+=2, fl_pt1+=2, fl_pt2+=2, fl_pt3+=2, fl_nor+=2 )
  for ( x=1; x<dimxMinusOne; x++, fl_max++,  fl_pt1++,  fl_pt2++,  fl_pt3++,  fl_nor++ ) {

    /*
     * If the modulus is too small, go to the next point.
     */
    if ( *fl_nor < _EPSILON_NORM_ ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * We normalize the vector gradient.
     */
    normalized_gx = *fl_pt1 / *fl_nor;
    normalized_gy = *fl_pt2 / *fl_nor;
    normalized_gz = *fl_pt3 / *fl_nor;

    /*
     * May we use the nearest value?
     */
    if ( (-normalized_gx > _EPSILON_DERIVATIVE_) ||
         (normalized_gx > _EPSILON_DERIVATIVE_) ||
         (-normalized_gy > _EPSILON_DERIVATIVE_) ||
         (normalized_gy > _EPSILON_DERIVATIVE_) ||
         (-normalized_gz > _EPSILON_DERIVATIVE_) ||
         (normalized_gz > _EPSILON_DERIVATIVE_) ) {
      /*
       * First point to be interpolated.
       */
      x_upper_left_corner = (int)( (double)x + normalized_gx + 0.5 );
      y_upper_left_corner = (int)( (double)y + normalized_gy + 0.5 );
      z_upper_left_corner = (int)( (double)z + normalized_gz + 0.5 );
      interpolated_norme = *(norme[z_upper_left_corner] 
                             + (x_upper_left_corner + y_upper_left_corner * dimx));
      if ( *fl_nor <= interpolated_norme ) {
        *fl_max = 0.0;
        continue;
      }
      /*
       * Second point to be interpolated.
       */
      x_upper_left_corner = (int)( (double)x - normalized_gx + 0.5 );
      y_upper_left_corner = (int)( (double)y - normalized_gy + 0.5 );
      z_upper_left_corner = (int)( (double)z - normalized_gz + 0.5 );
      interpolated_norme = *(norme[z_upper_left_corner] 
                             + (x_upper_left_corner + y_upper_left_corner * dimx));
      if ( *fl_nor < interpolated_norme ) {
        *fl_max = 0.0;
        continue;
      }
      /*
       * We found a gradient extrema.
       */
      *fl_max = *fl_nor;
      continue;
    }
    

    /*
     * From here we perform a trilinear interpolation
     */

    /*
     * First point to be interpolated.
     * It is the current point + an unitary vector
     * in the direction of the gradient.
     * It must be inside the image.
     */
    x_point_to_be_interpolated = (double)x + normalized_gx;
    y_point_to_be_interpolated = (double)y + normalized_gy;
    z_point_to_be_interpolated = (double)z + normalized_gz;
    if ( (x_point_to_be_interpolated < 0.0) ||
         (x_point_to_be_interpolated >= dimxMinusOne) ||
         (y_point_to_be_interpolated < 0.0) ||
         (y_point_to_be_interpolated >= dimyMinusOne) ) {
      *fl_max = 0.0;
      continue;
    }

    /* 
     * Upper left corner,
     * coordinates of the point to be interpolated
     * with respect to this corner.
     */
    x_upper_left_corner = (int)x_point_to_be_interpolated;
    y_upper_left_corner = (int)y_point_to_be_interpolated;
    z_upper_left_corner = (int)z_point_to_be_interpolated;
    dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
    dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
    dz = z_point_to_be_interpolated - (double)z_upper_left_corner;
    /* 
     * trilinear interpolation of the gradient modulus 
     * norme[x_point_to_be_interpolated, 
     *       y_point_to_be_interpolated,
     *       z_point_to_be_interpolated] =
     *   norme[0,0,0] * ( 1 - dx) * ( 1 - dy ) * ( 1 - dz ) +
     *   norme[1,0,0] * ( dx ) * ( 1 - dy ) * ( 1 - dz ) +
     *   norme[0,1,0] * ( 1 - dx ) * ( dy ) * ( 1 - dz ) +
     *   norme[1,1,0] * ( dx ) * ( dy ) * ( 1 - dz ) +
     *   norme[0,0,1] * ( 1 - dx) * ( 1 - dy ) * ( dz ) +
     *   norme[1,0,1] * ( dx ) * ( 1 - dy ) * ( dz ) +
     *   norme[0,1,1] * ( 1 - dx ) * ( dy ) * ( dz ) +
     *   norme[1,1,1] * ( dx ) * ( dy ) * ( dz )
     */
    dxdy = dx * dy;
    dydz = dy * dz;
    dxdz = dx * dz;
    c111 = dxdy * dz;
    c011 = dydz - c111;
    c101 = dxdz - c111;
    c001 = dz - dxdz - c011;
    c110 = dxdy - c111;
    c010 = dy - dxdy - c011;
    c100 = dx - dxdy - c101;
    c000 = 1.0 - dx - dy + dxdy - c001;
    fl_upper_left = norme[z_upper_left_corner]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c000 +
      *(fl_upper_left + 1) * c100 +
      *(fl_upper_left + dimx) * c010 +
      *(fl_upper_left + dimxPlusOne) * c110;
    fl_upper_left = norme[z_upper_left_corner+1]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme += *(fl_upper_left) * c001 +
      *(fl_upper_left + 1) * c101 +
      *(fl_upper_left + dimx) * c011 +
      *(fl_upper_left + dimxPlusOne) * c111;
    /*
     * We compare the modulus of the point with the
     * interpolated modulus. It must be larger to be
     * still considered as a potential gradient extrema.
     *
     * Here, we consider that it is strictly superior.
     * The next comparison will be superior or equal.
     * This way, the extrema is in the light part of the
     * image. 
     * By inverting both tests, we can put it in the
     * dark side of the image.
     */
    if ( *fl_nor <= interpolated_norme ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * Second point to be interpolated.
     * It is the current point - an unitary vector
     * in the direction of the gradient.
     * It must be inside the image.
     */
    x_point_to_be_interpolated = (double)x - normalized_gx;
    y_point_to_be_interpolated = (double)y - normalized_gy;
    z_point_to_be_interpolated = (double)z - normalized_gz;
    if ( (x_point_to_be_interpolated < 0.0) ||
         (x_point_to_be_interpolated >= dimxMinusOne) ||
         (y_point_to_be_interpolated < 0.0) ||
         (y_point_to_be_interpolated >= dimyMinusOne) ) {
      *fl_max = 0.0;
      continue;
    }
    /* 
     * Upper left corner.
     */
    x_upper_left_corner = (int)x_point_to_be_interpolated;
    y_upper_left_corner = (int)y_point_to_be_interpolated;
    z_upper_left_corner = (int)z_point_to_be_interpolated;
    /* we do not recompute the coefficients
    dx = x_point_to_be_interpolated - (double)x_upper_left_corner;
    dy = y_point_to_be_interpolated - (double)y_upper_left_corner;
    dz = z_point_to_be_interpolated - (double)z_upper_left_corner;
    */
    /*
     * We use the previous coefficients. 
     * norme[x_point_to_be_interpolated, 
     *       y_point_to_be_interpolated,
     *       z_point_to_be_interpolated] =
     *   norme[0,0,0] * c111 +
     *   norme[1,0,0] * c011 +
     *   norme[0,1,0] * c101 +
     *   norme[1,1,0] * c001 +
     *   norme[0,0,1] * c110 +
     *   norme[1,0,1] * c010 +
     *   norme[0,1,1] * c100 +
     *   norme[1,1,1] * c000
     *
     
    fl_upper_left = norme[z_upper_left_corner]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c111 +
      *(fl_upper_left + 1) * c011 +
      *(fl_upper_left + dimx) * c101 +
      *(fl_upper_left + dimxPlusOne) * c001;
    fl_upper_left = norme[z_upper_left_corner+1]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme += *(fl_upper_left) * c110 +
      *(fl_upper_left + 1) * c010 +
      *(fl_upper_left + dimx) * c100 +
      *(fl_upper_left + dimxPlusOne) * c000;

     *
     * WARNING: as in the 2D case it works only if the cases where one
     *          derivative is close to -/+ 1 are already be independently
     *          processed, else it may lead to errors.
     */
    /* we do not recompute the coefficients
    dxdy = dx * dy;
    dydz = dy * dz;
    dxdz = dx * dz;
    c111 = dxdy * dz;
    c011 = dydz - c111;
    c101 = dxdz - c111;
    c001 = dz - dxdz - c011;
    c110 = dxdy - c111;
    c010 = dy - dxdy - c011;
    c100 = dx - dxdy - c101;
    c000 = 1.0 - dx - dy + dxdy - c001;
    fl_upper_left = norme[z_upper_left_corner]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c000 +
      *(fl_upper_left + 1) * c100 +
      *(fl_upper_left + dimx) * c010 +
      *(fl_upper_left + dimxPlusOne) * c110;
    fl_upper_left = norme[z_upper_left_corner+1]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme += *(fl_upper_left) * c001 +
      *(fl_upper_left + 1) * c101 +
      *(fl_upper_left + dimx) * c011 +
      *(fl_upper_left + dimxPlusOne) * c111;
    */
    
    fl_upper_left = norme[z_upper_left_corner]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme = *(fl_upper_left) * c111 +
      *(fl_upper_left + 1) * c011 +
      *(fl_upper_left + dimx) * c101 +
      *(fl_upper_left + dimxPlusOne) * c001;
    fl_upper_left = norme[z_upper_left_corner+1]
      + (x_upper_left_corner + y_upper_left_corner * dimx);
    interpolated_norme += *(fl_upper_left) * c110 +
      *(fl_upper_left + 1) * c010 +
      *(fl_upper_left + dimx) * c100 +
      *(fl_upper_left + dimxPlusOne) * c000;

    /*
     * Last test to decide whether or not we 
     * have an extrema
     */
    if ( *fl_nor < interpolated_norme ) {
      *fl_max = 0.0;
      continue;
    }
    /*
     * We found a gradient extrema.
     */
    *fl_max = *fl_nor;
  }
}





int gradientMaxima2D( void *bufferIn,
                      bufferType typeIn,
                      void *bufferOut,
                      bufferType typeOut,
                      int *bufferDims,
                      int *borderLengths,
                      typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientMaxima2D";
  /*
   * auxiliary buffer
   */ 
  float *tmpBuffer = (float*)NULL;
  /*
   * Pointers
   */
  float *gx = (float*)NULL;
  float *gy = (float*)NULL;
  float *norme = (float*)NULL;
  void *sliceIn = (void*)NULL;
  void *sliceOut = (void*)NULL;
  /*
   * additional parameters forfiltering
   */
  typeFilteringCoefficients filter[3];
  int sliceDims[3];
  /*
   *
   */
  int z;
  size_t dimxXdimy;



  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  dimxXdimy = bufferDims[0] * bufferDims[1];
  sliceDims[0] = bufferDims[0];
  sliceDims[1] = bufferDims[1];
  sliceDims[2] = 1;

  
  
  /* 
   * Allocation of auxiliary buffer.
   * We need a slice buffer for each gradients' component
   * plus one for the modulus.
   */
  tmpBuffer = (float*)malloc( 3 * dimxXdimy * sizeof( float ) );
  if ( tmpBuffer == (float*)NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer.\n", proc );
    return( -1 );
  }
  norme = tmpBuffer;
  gy = norme + dimxXdimy;
  gx = gy + dimxXdimy;



  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];



  /* 
   * slice by slice processing.
   *
   * For each slice, we compute both the X and Y
   * components of the gradient, its modulus,
   * and we suppress the non-maxima of the
   * gradient. Finally, we put the result
   * in the buffer bufferOut.
   *
   * An other solution may consist in computing
   * the  X and Y components of the gradient
   * for the whole 3D buffer, and performing 
   * the non-maxima suppression slice 
   * by slice.
   */
  for ( z=0; z<bufferDims[2]; z++ ) {
    if ( (_verbose_ > 1 || _debug_ ) && (bufferDims[2] > 1) ) {
      fprintf( stderr, " %s: Processing slice #%d.\n", proc, z );
    }
    sliceIn = (void*)NULL;
    /*
     * sliceIn points towards the slice #z of
     * the buffer bufferIn.
     */
    switch( typeIn ) {
    case UCHAR :
      sliceIn = (((u8*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case SCHAR :
      sliceIn = (((s8*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case USHORT :
      sliceIn = (((u16*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case SSHORT :
      sliceIn = (((s16*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case SINT :
      sliceIn = (((i32*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case FLOAT :
      sliceIn = (((r32*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    case DOUBLE :
      sliceIn = (((r64*)bufferIn) + (size_t)z * dimxXdimy);
      break;
    default :
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: such input type not handled.\n", proc );
      free( tmpBuffer );
      return( -1 );
    }
    /*
     * computing the X and Y component
     * of the gradient.
     */
    filter[0].derivative = DERIVATIVE_1_EDGES;
    filter[1].derivative = DERIVATIVE_0;
    filter[2].derivative = NODERIVATIVE;
    if ( separableLinearFiltering( sliceIn, typeIn, gx, FLOAT,
                                   sliceDims, borderLengths, filter ) != 1 ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to compute X gradient for slice #%d.\n", proc, z );
      free( tmpBuffer );
      return( -1 );
    }
    
    filter[0].derivative = DERIVATIVE_0;
    filter[1].derivative = DERIVATIVE_1_EDGES;
    filter[2].derivative = NODERIVATIVE;
    if ( separableLinearFiltering( sliceIn, typeIn, gy, FLOAT,
                                   sliceDims, borderLengths, filter ) != 1 ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to compute Y gradient for slice #%d.\n", proc, z );
      free( tmpBuffer );
      return( -1 );
    }
    
    /*
     * Modulus of the gradient
     */ 
    localGradientModulus2D( norme, gx, gy, dimxXdimy );

    /*
     * Suppression of the non maxima of the gradient
     * in the direction of the gradient.
     *
     * If the type of the result buffer bufferOut is
     * FLOAT, then we compute directly the result
     * into the slice #z of the result buffer.
     * Else, we compute the suppression of the
     * non maxima into the gx buffer, and we 
     * convert it into the result buffer type.
     */
    if (typeOut == FLOAT ) {
      sliceOut = (((float*)bufferOut) + (size_t)z * dimxXdimy);
      removeGradientNonMaxima2D( sliceOut, gx ,gy,
                                 norme, sliceDims );
    } else {
      removeGradientNonMaxima2D( gx, gx ,gy,
                                 norme, sliceDims );
      switch( typeOut ) {
      case UCHAR :
        sliceOut = (((u8*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      case SCHAR :
        sliceOut = (((s8*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      case USHORT :
        sliceOut = (((u16*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      case SSHORT :
        sliceOut = (((s16*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      case SINT :
        sliceOut = (((i32*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      case DOUBLE :
        sliceOut = (((r64*)bufferOut) + (size_t)z * dimxXdimy);
        break;
      default :
        if ( _verbose_ > 0 )
          fprintf( stderr, " Error in %s: such output type not handled.\n", proc );
        free( tmpBuffer );
        return( -1 );
      }
      if ( ConvertBuffer( gx, FLOAT, sliceOut, typeOut, dimxXdimy) != 1 ) {
        if ( _verbose_ > 0 )
          fprintf( stderr, " Error in %s: unable to convert such image type\n", proc );
        free( tmpBuffer );
        return( -1 );
      }
    }
  }


  free( tmpBuffer );
  return( 1 );
}





int gradientMaxima3D( void *bufferIn,
                      bufferType typeIn,
                      void *bufferOut,
                      bufferType typeOut,
                      int *bufferDims,
                      int *borderLengths,
                      typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientMaxima3D";
  /*
   * auxiliary buffer
   */ 
  float *tmpBuffer = (float*)NULL;
  float *bufferZsmoothed = (float*)NULL;
  float *bufferZderivated = (float*)NULL;
  /*
   * Pointers
   */
  /* 
   * gx[0] points toward the X gradient of the current slice
   * gx[0] points toward the X gradient of the next slice
   */
  float *gx[2] = { (float*)NULL, (float*)NULL };
  /*
   * gy: idem gx but for the Y gradient
   */
  float *gy[2] = { (float*)NULL, (float*)NULL };
  float *gz = (float*)NULL;
  /*
   * norme[0] points toward the gradient modulus of the previous slice
   * norme[1] points toward the gradient modulus of the current slice
   * norme[2] points toward the gradient modulus of the next slice
   */
  float *norme[3] = { (float*)NULL, (float*)NULL, (float*)NULL }; 
  float *sliceZsmoothed = (float*)NULL;
  float *pt = (float*)NULL;
  /*
   * additional parameters for recursive filtering
   */
  typeFilteringCoefficients filter[3];
  /*
  derivativeOrder Xgradient[3] = { DERIVATIVE_1_EDGES, SMOOTHING, NODERIVATIVE };
  derivativeOrder Ygradient[3] = { SMOOTHING, DERIVATIVE_1_EDGES, NODERIVATIVE };
  derivativeOrder Zgradient[3] = { SMOOTHING, SMOOTHING, DERIVATIVE_1_EDGES };
  derivativeOrder Zsmoothing[3] = { NODERIVATIVE, NODERIVATIVE, SMOOTHING };
  */
  int sliceDims[3];
  /*
   *
   */
  int z;
  size_t dimxXdimy;

  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }

  /*
   * May we perform a 3D edge detection?
   */
  if ( bufferDims[2] == 1 ) {
    return( gradientMaxima2D( bufferIn, typeIn, bufferOut, typeOut,
                              bufferDims, borderLengths, theFilter ) );
  }

  /*
   *
   */
  dimxXdimy = bufferDims[0] * bufferDims[1];
  sliceDims[0] = bufferDims[0];
  sliceDims[1] = bufferDims[1];
  sliceDims[2] = 1;
  


  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];
  


  /* 
   * Allocation of auxiliary buffers.
   *
   * We need a 3D buffer for the Z component of the
   * gradient, plus a 3D buffer for the 3D buffer 
   * smoothed along Z, plus 7 2D buffers for the
   * X component of the gradient in both the current
   * and the next slices, idem for the Y component,
   * idem for the modulus plus one 2D buffer for
   * the modulus in the previous slice.
   *
   * If the buffer bufferOut is of type FLOAT,
   * we use it as the Z component of the gradient.
   *
   * This Z component will be used to stored the
   * extrema of the gradient.
   */
  tmpBuffer = (float*)malloc( 7 * dimxXdimy * sizeof( float ) );
  if ( tmpBuffer == (float*)NULL ) {
    if ( _verbose_ > 0 ) {
      fprintf( stderr, " Fatal error in %s:", proc );
      fprintf( stderr, " unable to allocate auxiliary buffer.\n" );      
    }
    return( -1 );
  }
  gx[0] = tmpBuffer;
  gx[1] = gx[0] + dimxXdimy;
  gy[0] = gx[1] + dimxXdimy;
  gy[1] = gy[0] + dimxXdimy;
  norme[0] = gy[1] + dimxXdimy;
  norme[1] = norme[0] + dimxXdimy;
  norme[2] = norme[1] + dimxXdimy;
  
  bufferZsmoothed = (float*)malloc( (size_t)bufferDims[2] * dimxXdimy * sizeof( float ) );
  if ( bufferZsmoothed == (float*)NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary first 3D buffer.\n", proc );
    free( tmpBuffer );
    return( -1 );
  }
  
  if ( typeOut == FLOAT ) {
    bufferZderivated = bufferOut;
  } else {
    bufferZderivated = (float*)malloc( (size_t)bufferDims[2] * dimxXdimy * sizeof( float ) );
    if ( bufferZderivated == (float*)NULL ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to allocate auxiliary second 3D buffer.\n", proc );
      free( tmpBuffer );
      free( bufferZsmoothed );
      return( -1 );
    }
  }

  
  
  /* 
   * Computation of the Z component of the gradient.
   * Computation of the input buffer smoothed along Z.
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = DERIVATIVE_1_EDGES;
  if ( separableLinearFiltering( bufferIn, typeIn,
                                 bufferZderivated, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to compute Z derivative.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }

  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = DERIVATIVE_0;
  if ( separableLinearFiltering( bufferIn, typeIn,
                                bufferZsmoothed, FLOAT,
                                bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to compute Z smoothed image.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }


  
  /*
   * First slice: extraction of 2D edges.
   *
   * - computation of the X component of the gradient
   *   for that slice
   * - idem for the Y component of the gradient
   * - computation of the modulus
   * - suppression of the 2D non maxima of the gradient
   */
  sliceZsmoothed = bufferZsmoothed;
  gz = bufferZderivated;
  
  filter[0].derivative = DERIVATIVE_1_EDGES;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                gx[0], FLOAT, sliceDims,
                                borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 ) 
      fprintf( stderr, "%s: unable to compute X gradient of the first slice.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }
  
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_1_EDGES;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                 gy[0], FLOAT, sliceDims,
                                 borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 ) 
      fprintf( stderr, "%s: unable to compute Y gradient of the first slice.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }

  localGradientModulus3D( norme[1], gx[0], gy[0], gz, dimxXdimy );
  removeGradientNonMaxima2D( gz, gx[0], gy[0], norme[1], sliceDims );
   
  

  /*
   * The first slice is already processed.
   * 
   * We prepare the processing of the next slice.
   * - computation of the X component of the gradient
   *   for that slice
   * - idem for the Y component of the gradient
   * - computation of the modulus
   */

  sliceZsmoothed += dimxXdimy;
  gz += dimxXdimy;

  filter[0].derivative = DERIVATIVE_1_EDGES;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                 gx[1], FLOAT, sliceDims,
                                 borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to compute X gradient of the second slice.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }
  
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_1_EDGES;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                 gy[1], FLOAT, sliceDims,
                                 borderLengths, filter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to compute Y gradient of the second slice.\n", proc );
    free( tmpBuffer );
    free( bufferZsmoothed );
    if ( typeOut != FLOAT ) free( bufferZderivated );
    return( -1 );
  }
  
  localGradientModulus3D( norme[2], gx[1], gy[1], gz, dimxXdimy );
 


  /*
   * slice by slice processing 
   */
  for ( z=1; z<bufferDims[2]-1; z++ ) {

    /*
     * slices permutations
     */
    pt = gx[0]; gx[0] = gx[1]; gx[1] = pt;
    pt = gy[0]; gy[0] = gy[1]; gy[1] = pt;
    pt = norme[0]; norme[0] = norme[1]; 
    norme[1] = norme[2]; norme[2] = pt;

    /*
     * gx[0] and gy[0] are the X and Y components
     * of the gradient of the current slice.
     * gx[1] and gy[1] are the X and Y components
     * of the gradient of the next slice.
     * norme[0] is the gradient modulus of the previous slice,
     * norme[1] is the gradient modulus of the current slice,
     * norme[2] is the gradient modulus of the next slice.
     */
    /*
     * Processing of the next slice.
     * - computation of the X component of the gradient
     *   for that slice
     * - idem for the Y component of the gradient
     * - computation of the modulus
     */

    sliceZsmoothed += dimxXdimy;
    gz += dimxXdimy;

    filter[0].derivative = DERIVATIVE_1_EDGES;
    filter[1].derivative = DERIVATIVE_0;
    filter[2].derivative = NODERIVATIVE;
    if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                   gx[1], FLOAT, sliceDims,
                                   borderLengths, filter ) != 1 ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to compute X gradient of slice #%d.\n", proc, z+1 );
      free( tmpBuffer );
      free( bufferZsmoothed );
      if ( typeOut != FLOAT ) free( bufferZderivated );
      return( -1 );
    }

    filter[0].derivative = DERIVATIVE_0;
    filter[1].derivative = DERIVATIVE_1_EDGES;
    filter[2].derivative = NODERIVATIVE;
    if ( separableLinearFiltering( sliceZsmoothed, FLOAT,
                                   gy[1], FLOAT, sliceDims,
                                   borderLengths, filter ) != 1 ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to compute Y gradient of slice #%d.\n", proc, z+1 );
      free( tmpBuffer );
      free( bufferZsmoothed );
      if ( typeOut != FLOAT ) free( bufferZderivated );
      return( -1 );
    }
    
    localGradientModulus3D( norme[2], gx[1], gy[1], gz, dimxXdimy );

    /*
     * suppression of the 3D non maxima of the gradient.
     */
    gz -= dimxXdimy;
    removeGradientNonMaxima3D( gz, gx[0], gy[0], gz, norme, sliceDims );
    gz += dimxXdimy;
  }



  /*
   * last slice 
   * 
   * Components and moduls of the gradient are 
   * already computed.
   *
   * - 2D suppression of the non maxima
   */
  removeGradientNonMaxima2D( gz, gx[1], gy[1], norme[2], sliceDims );

  

  /*
   * conversion of the buffer bufferZderivated of type FLOAT
   * into the buffer bufferOut.
   */
  
  if (typeOut != FLOAT ) {
    if ( ConvertBuffer( bufferZderivated, FLOAT, 
                        bufferOut, typeOut, (size_t)bufferDims[2]*dimxXdimy ) != 1 ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, "%s: unable to convert such image type\n", proc );
      free( tmpBuffer );
      free( bufferZsmoothed );
      if ( typeOut != FLOAT ) free( bufferZderivated );
      return( -1 );
    }
  }

  free( tmpBuffer );
  free( bufferZsmoothed );
  if ( typeOut != FLOAT ) free( bufferZderivated );
  return( 1 );
}





int gradientMaxima( void *bufferIn,
                      bufferType typeIn,
                      void *bufferOut,
                      bufferType typeOut,
                      int *bufferDims,
                      int *borderLengths,
                      typeFilteringCoefficients *theFilter )

{
  if ( bufferDims[2] == 1 ) 
    return( gradientMaxima2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
  else
    return( gradientMaxima3D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
}





/************************************************************
 *
 * Zero-crossing based contours
 *
 ************************************************************/



int gradientHessianGradientZeroCrossings2D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientHessianGradientZeroCrossings2D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( gradientHessianGradient2D( bufferIn, typeIn, auxBuf, FLOAT,
                    bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient.Hessian.gradient\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( zeroCrossing2D( auxBuf, FLOAT, bufferOut, typeOut,
                    bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when extracting zero-crossings\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientHessianGradientZeroCrossings3D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientHessianGradientZeroCrossings3D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  if ( bufferDims[2] == 1 ) 
    return( gradientHessianGradientZeroCrossings2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( gradientHessianGradient3D( bufferIn, typeIn, auxBuf, FLOAT,
                    bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient.Hessian.gradient\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( zeroCrossing3D( auxBuf, FLOAT, bufferOut, typeOut,
                    bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when extracting zero-crossings\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientHessianGradientZeroCrossings( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( gradientHessianGradientZeroCrossings2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
  else
    return( gradientHessianGradientZeroCrossings3D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
}





int laplacianZeroCrossings2D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  char *proc = "laplacianZeroCrossings2D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( laplacian2D( bufferIn, typeIn, auxBuf, FLOAT,
                    bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing laplacian\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( zeroCrossing2D( auxBuf, FLOAT, bufferOut, typeOut,
                    bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when extracting zero-crossings\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int laplacianZeroCrossings3D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  char *proc = "laplacianZeroCrossings3D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  if ( bufferDims[2] == 1 ) 
    return( laplacianZeroCrossings2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( laplacian3D( bufferIn, typeIn, auxBuf, FLOAT,
                    bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing laplacian\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( zeroCrossing3D( auxBuf, FLOAT, bufferOut, typeOut,
                    bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when extracting zero-crossings\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int laplacianZeroCrossings( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( laplacianZeroCrossings2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
  else
    return( laplacianZeroCrossings3D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
}





int gradientOnGradientHessianGradientZC2D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientOnGradientHessianGradientZC2D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(unsigned char) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( gradientHessianGradientZeroCrossings2D( bufferIn, typeIn, auxBuf, UCHAR,
                                 bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing zero-crossings of gradient.Hessian.gradient\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( gradientModulus2D( bufferIn, typeIn, bufferOut, typeOut, bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( maskImage( bufferOut, typeOut, auxBuf, UCHAR, bufferOut, typeOut, 
                      bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when masking gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientOnGradientHessianGradientZC3D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientOnGradientHessianGradientZC3D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  if ( bufferDims[2] == 1 ) 
    return( gradientOnGradientHessianGradientZC2D( bufferIn, typeIn, bufferOut, typeOut,
                                     bufferDims, borderLengths, theFilter ) );

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(unsigned char) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( gradientHessianGradientZeroCrossings3D( bufferIn, typeIn, auxBuf, UCHAR,
                                 bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing zero-crossings of gradient.Hessian.gradient\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( gradientModulus3D( bufferIn, typeIn, bufferOut, typeOut, bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( maskImage( bufferOut, typeOut, auxBuf, UCHAR, bufferOut, typeOut, 
                      bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when masking gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientOnGradientHessianGradientZC( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( gradientOnGradientHessianGradientZC2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
  else
    return( gradientOnGradientHessianGradientZC3D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
}





int gradientOnLaplacianZC2D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientOnLaplacianZC2D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(unsigned char) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( laplacianZeroCrossings2D( bufferIn, typeIn, auxBuf, UCHAR,
                                 bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing zero-crossings of laplacian\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( gradientModulus2D( bufferIn, typeIn, bufferOut, typeOut, bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( maskImage( bufferOut, typeOut, auxBuf, UCHAR, bufferOut, typeOut, 
                      bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when masking gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientOnLaplacianZC3D( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientOnLaplacianZC3D";
  float *auxBuf = NULL;
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  
  if ( bufferDims[2] == 1 ) 
    return( gradientOnLaplacianZC2D( bufferIn, typeIn, bufferOut, typeOut,
                                     bufferDims, borderLengths, theFilter ) );

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];
  
  sizeAuxBuf = dimx*dimy*dimz;

  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(unsigned char) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  if ( laplacianZeroCrossings3D( bufferIn, typeIn, auxBuf, UCHAR,
                                 bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing zero-crossings of laplacian\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( gradientModulus3D( bufferIn, typeIn, bufferOut, typeOut, bufferDims, borderLengths, theFilter ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when computing gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  if ( maskImage( bufferOut, typeOut, auxBuf, UCHAR, bufferOut, typeOut, 
                      bufferDims ) != 1 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: error when masking gradient modulus\n", proc );
    free( auxBuf );
    return( -1 );
  }

  free( auxBuf );
  return( 1 );
}





int gradientOnLaplacianZC( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims,
                            int *borderLengths,
                            typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( gradientOnLaplacianZC2D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
  else
    return( gradientOnLaplacianZC3D( bufferIn, typeIn, bufferOut, typeOut,
                         bufferDims, borderLengths, theFilter ) );
}
