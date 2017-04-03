/*************************************************************************
 * convolution1D.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 30 23:29:17 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _convolution_oned_h_
#define _convolution_oned_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>
#include <linearFiltering-common.h>

typedef struct {
  int length;
  int halflength; /* length = 2 * halflength + 1 */
  double *data;
} type1DConvolutionMask;


extern void init1DConvolutionMask( type1DConvolutionMask *m );

extern void free1DConvolutionMask( type1DConvolutionMask *m );

extern void print1DConvolutionMask( FILE *theFile, type1DConvolutionMask *m, char *d );

extern double * _build1DGaussianMask( double sigma, int length, derivativeOrder derivative );
extern double * _buildGaussianMask( double sigma, int length );



extern void _compute1DDoubleConvolution( double *theBuf,
					 double *resBuf,
					 int dim,
					 double *mask,
					 int halfMaskLength );
extern int compute1DConvolutionWithMask( void *theLine,
					 bufferType typeIn,
					 void *resLine,
					 bufferType typeOut,
					 int dim,
					 double *mask,
					 int halfMaskLength );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _convolution_oned_h_ */
