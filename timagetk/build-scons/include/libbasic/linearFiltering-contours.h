/*************************************************************************
 * linearFiltering-contours.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Dec 26 22:55:49 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _linearfilteringcontours_h_
#define _linearfilteringcontours_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>
#include <linearFiltering-common.h>



extern void setVerboseInLinearFilteringContours( int v );
extern void incrementVerboseInLinearFilteringContours(  );
extern void decrementVerboseInLinearFilteringContours(  );



/************************************************************
 *
 * Gradient extrema based contours
 *
 ************************************************************/

/* epsilon value to select gradient extrema candidates
   points with gradient modulus below this threshold are not
   considered as potential extrema
 */
extern void setGradientModulusThresholdInLinearFilteringContours( double epsilon );

/* epsilon value to decide of the interpolation type.
 * If one derivative's absolute value is larger than 
 * (1-epsilon), then we use the nearest value
 * else we perform a [bi,tri]linear interpolation.
 */
extern void setGradientDerivativeThresholdInLinearFilteringContours( double epsilon );



extern int gradientMaxima2D( void *bufferIn,
                             bufferType typeIn,
                             void *bufferOut,
                             bufferType typeOut,
                             int *bufferDims,
                             int *borderLengths,
                             typeFilteringCoefficients *theFilter );

extern int gradientMaxima3D( void *bufferIn,
                             bufferType typeIn,
                             void *bufferOut,
                             bufferType typeOut,
                             int *bufferDims,
                             int *borderLengths,
                             typeFilteringCoefficients *theFilter );

extern int gradientMaxima( void *bufferIn,
                             bufferType typeIn,
                             void *bufferOut,
                             bufferType typeOut,
                             int *bufferDims,
                             int *borderLengths,
                             typeFilteringCoefficients *theFilter );

/************************************************************
 *
 * Zero-crossing based contours
 *
 ************************************************************/

extern int gradientHessianGradientZeroCrossings2D( void *bufferIn,
                                                   bufferType typeIn,
                                                   void *bufferOut,
                                                   bufferType typeOut,
                                                   int *bufferDims,
                                                   int *borderLengths,
                                                   typeFilteringCoefficients *theFilter );
  
extern int gradientHessianGradientZeroCrossings3D( void *bufferIn,
                                                   bufferType typeIn,
                                                   void *bufferOut,
                                                   bufferType typeOut,
                                                   int *bufferDims,
                                                   int *borderLengths,
                                                   typeFilteringCoefficients *theFilter );

extern int gradientHessianGradientZeroCrossings( void *bufferIn,
                                                 bufferType typeIn,
                                                 void *bufferOut,
                                                 bufferType typeOut,
                                                 int *bufferDims,
                                                 int *borderLengths,
                                                 typeFilteringCoefficients *theFilter );

extern int laplacianZeroCrossings2D( void *bufferIn,
                                     bufferType typeIn,
                                     void *bufferOut,
                                     bufferType typeOut,
                                     int *bufferDims,
                                     int *borderLengths,
                                     typeFilteringCoefficients *theFilter );

extern int laplacianZeroCrossings3D( void *bufferIn,
                                     bufferType typeIn,
                                     void *bufferOut,
                                     bufferType typeOut,
                                     int *bufferDims,
                                     int *borderLengths,
                                     typeFilteringCoefficients *theFilter );

extern int laplacianZeroCrossings( void *bufferIn,
                                   bufferType typeIn,
                                   void *bufferOut,
                                   bufferType typeOut,
                                   int *bufferDims,
                                   int *borderLengths,
                                   typeFilteringCoefficients *theFilter );

extern int gradientOnGradientHessianGradientZC2D( void *bufferIn,
                                                bufferType typeIn,
                                                void *bufferOut,
                                                bufferType typeOut,
                                                int *bufferDims,
                                                int *borderLengths,
                                                typeFilteringCoefficients *theFilter );

extern int gradientOnGradientHessianGradientZC3D( void *bufferIn,
                                                bufferType typeIn,
                                                void *bufferOut,
                                                bufferType typeOut,
                                                int *bufferDims,
                                                int *borderLengths,
                                                typeFilteringCoefficients *theFilter );

extern int gradientOnGradientHessianGradientZC( void *bufferIn,
                                                bufferType typeIn,
                                                void *bufferOut,
                                                bufferType typeOut,
                                                int *bufferDims,
                                                int *borderLengths,
                                                typeFilteringCoefficients *theFilter );

extern int gradientOnLaplacianZC2D( void *bufferIn,
                                    bufferType typeIn,
                                    void *bufferOut,
                                    bufferType typeOut,
                                    int *bufferDims,
                                    int *borderLengths,
                                    typeFilteringCoefficients *theFilter );

extern int gradientOnLaplacianZC3D( void *bufferIn,
                                    bufferType typeIn,
                                    void *bufferOut,
                                    bufferType typeOut,
                                    int *bufferDims,
                                    int *borderLengths,
                                    typeFilteringCoefficients *theFilter );

extern int gradientOnLaplacianZC( void *bufferIn,
                                  bufferType typeIn,
                                  void *bufferOut,
                                  bufferType typeOut,
                                  int *bufferDims,
                                  int *borderLengths,
                                  typeFilteringCoefficients *theFilter );



/************************************************************
 *
 * These are obsolete
 *
 ************************************************************/
/*
extern int Gradient_On_Laplacian_ZeroCrossings_2D ( void *bufferIn,  bufferType typeIn,
                                                    void *bufferOut, bufferType typeOut,
                                                    int *bufferDims, int *borderLengths,
                                                    float *filterCoefs,
                                                    filterType filterType );


extern int Gradient_On_GradientHessianGradient_ZeroCrossings_2D( void *bufferIn,  
                                                                 bufferType typeIn,
                                                    void *bufferOut, bufferType typeOut,
                                                    int *bufferDims, int *borderLengths,
                                                    float *filterCoefs,
                                                    filterType filterType );
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _linearfilteringcontours_h_ */
