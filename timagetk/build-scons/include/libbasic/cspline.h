/*************************************************************************
 * cspline.h - Cubic splines
 *
 * $Id: cspline.h,v 1.2 2000/10/23 14:32:34 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Alexis Roche
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Oct 12 12:08:12 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *	
 *	
 *	
 *
 */

#ifndef _cspline_h_
#define _cspline_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <typedefs.h>
#include <convert.h>

typedef struct {

  /* dimension du buffer
     theDim[0] -> dimension selon X
     theDim[1] -> dimension selon Y
     theDim[2] -> dimension selon Z
  */
  int theDim[3];
  /* buffer contenant les coefficients
   */
  float *theCoeff;

} typeCSplineCoefficients;



extern typeCSplineCoefficients *ComputeCSplineCoefficients( void *theBuf,
						     bufferType theType,
						     int *theDim );



extern int CSplineFilterOnBuffer( void *bufferIn,  
			   bufferType typeIn,
			   void *bufferOut,
			   bufferType typeOut,
			   int *bufferDims,
			   int *derivatives );
extern int CSplineFilterOnBufferWithCoefficients( typeCSplineCoefficients *theCoeff,
						  void *bufferOut,
						  bufferType typeOut,
						  int *derivatives );




/* met a zero les elements de la structure
   typeCSplineCoefficients
*/
extern void InitTypeCSplineCoefficients( typeCSplineCoefficients *t );
extern void FreeTypeCSplineCoefficients( typeCSplineCoefficients **t );





extern int Reech3DCSpline4x4WithCoefficients( typeCSplineCoefficients *theCoeff,
		       r32 *resBuf,  /* result buffer */
		       int *resDim,  /* dimensions of this buffer */
		       double *mat,
		       int slice, int *derivative );

extern int Reech2DCSpline4x4WithCoefficients( typeCSplineCoefficients *theCoeff,
		       r32* resBuf,  /* result buffer */
		       int *resDim,  /* dimensions of this buffer */
		       double *mat,
		       int *derivative );

extern int ReechCSpline4x4( void* theBuf, bufferType theType, int *theDim,
			      void* resBuf, bufferType resType, int *resDim,
			      double *mat,
			      int *derivative );




#ifdef __cplusplus
}
#endif

#endif
