/*************************************************************************
 * convolution.h - 
 *
 * $Id: convolution.h,v 1.2 2003/07/04 08:16:47 greg Exp $
 *
 * Copyright (c) INRIA 2002
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Thu Sep 26 22:14:33 MEST 2002
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _convolution_h_
#define _convolution_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>


double * _buildGaussianMask( double sigma, int length );



extern int ConvolutionWithFwhm( void *bufferIn,  bufferType typeIn,
				void *bufferOut, bufferType typeOut,
				int *bufferDims,
				int *fwhm );

extern int Convolution( void *bufferIn,  bufferType typeIn,
			void *bufferOut, bufferType typeOut,
			int *bufferDims,
			void *mask,      bufferType typeMask,
			int *maskDims,   int *maskCenter );

int SeparableConvolution( void *bufferIn,
			  bufferType typeIn,
			  void *bufferOut,
			  bufferType typeOut,
			  int *bufferDims,
			  int *borderLengths,
			  double **convolutionMasks,
			  int *convolutionMaskLength );




/* Turn on verbose mode.
 *
 * DESCRIPTION:
 * Some information will be written on stderr when processing.
 */
extern void Convolution_verbose ( );

/* Turn off verbose mode.
 *
 * DESCRIPTION:
 * Nothing will be written on stderr when processing.
 */
extern void Convolution_noverbose ( );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _convolution_h_ */
