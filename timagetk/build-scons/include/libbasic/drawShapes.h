/*************************************************************************
 * drawShapes.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer 11 sep 2013 15:34:22 CEST
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _drawshapes_h_
#define _drawshapes_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <typedefs.h>

extern int drawCylinder( void *inputBuf,
			 void *resultBuf,
			 int *theDim,
			 bufferType type,
			 double *center,
			 double *vector,
			 double radius,
			 double value );

extern int drawLine( void *inputBuf,
		     void *resultBuf,
		     int *theDim,
		     bufferType type,
		     int *pt1,
		     int *pt2,
		     double value );

extern int drawRectangle( void *inputBuf,
			  void *resultBuf,
			  int *theDim,
			  bufferType type,
			  int *pt1,
			  int *pt2,
			  double value );


extern int drawSphere( void *inputBuf,
		       void *resultBuf,
		       int *theDim,
		       bufferType type,
		       double *center,
		       double radius,
		       double value );

#ifdef __cplusplus
}
#endif

#endif
