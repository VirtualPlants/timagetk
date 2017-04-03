/*************************************************************************
 * systlin.h - Resolution of linear system
 *
 * $Id: systlin.h,v 1.6 2002/04/19 15:54:08 greg Exp $
 *
 * Copyright INRIA
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 22 11:08:39 MET 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#ifndef _systlin_h_
#define _systlin_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>

#ifdef _LINUX_
  /* long int random(void); */
#endif


extern int _SolveLinearSystem( double *mat, double *vec, 
			       double *x, int dim );
extern int _InverseSquareMatrix( double *mat, double *inv, int dim );

extern void _MultiplyTwoSquareMatrices( double *a, double *b, 
					double *res, int dim );

extern void _MultiplyMatrixByVector( double *mat, double *vec, 
				     double *res, int dim );

extern void _TestSystLin( int matrixDim );

#ifdef __cplusplus
}
#endif

#endif
