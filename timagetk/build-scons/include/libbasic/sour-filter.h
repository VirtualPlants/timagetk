/*************************************************************************
 * sour-filter.h
 *
 * $Id: sour-filter.h,v 1.1 1999/12/17 12:33:53 greg Exp $
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

#ifndef _sour_filter_h_
#define _sour_filter_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include <math.h>


typedef struct {
  int xoffset;
  int yoffset;
  int zoffset;
  int totaloffset;
  double c;
} typeCoefficient;

typedef struct {
  int nb;
  int maxdim;
  typeCoefficient *theCoefs;
} typeMask;


extern void _InitMask( typeMask *theMask );
extern void _FreeMask( typeMask *theMask );
extern int _CreateGaussianMask( typeMask *theMask,
			 double radiusInVoxels,
			 double sigma );
extern void _ComputeTotalOffsetOfAMask( typeMask *theMask,
				 int *theDim );

extern int _FilterWithMaskAndWeights( float *theIm,
			       float *resIm,
			       float *theWeights,
			       int *theDim,
			       typeMask *theMask );



#ifdef __cplusplus
}
#endif

#endif




