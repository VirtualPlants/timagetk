/*************************************************************************
 * basictransform.h - basic transformations
 *
 * $Id$
 *
 * Copyright (c) INRIA 2010
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Tue Sep 14 18:25:49 CEST 2010
 *
 * ADDITIONS, CHANGES
 *
 * - 
 *
 *
 *
 *
 */



#ifndef _basictransform_h_
#define _basictransform_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <typedefs.h>





/* change (x,y,z) into (dimx-1-x,y,z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int SymmetryWRT_X( void *inputBuf,
			   void *outputBuf,
			   bufferType type,
			   int *theDim );



/* change (x,y,z) into (x,dimy-1-y,z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int SymmetryWRT_Y( void *inputBuf,
			  void *outputBuf,
			  bufferType type,
			   int *theDim );



/* change (x,y,z) into (x,y,dimz-1-z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int SymmetryWRT_Z( void *inputBuf,
			  void *outputBuf,
			  bufferType type,
			   int *theDim );



/* change (x,y,z) into (y,x,z)
   result image dimensions are (dimy, dimx, dimz)
   is inverted by the same transformation
 */
extern int SymmetryWRT_XY1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimy-1-y,dimx-1-x,z)
   result image dimensions are (dimy, dimx, dimz)
   is inverted by the same transformation
 */
extern int SymmetryWRT_XY2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (z,y,x)
   result image dimensions are (dimz, dimy, dimx)
   is inverted by the same transformation
 */
extern int SymmetryWRT_XZ1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimz-1-z,y,dimx-1-x)
   result image dimensions are (dimz, dimy, dimx)
   is inverted by the same transformation
 */
extern int SymmetryWRT_XZ2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (x,z,y)
   result image dimensions are (dimx, dimz, dimy)
   is inverted by the same transformation
 */
extern int SymmetryWRT_YZ1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (x,dimz-1-z,dimy-1-y)
   result image dimensions are (dimx, dimz, dimy)
   is inverted by the same transformation
 */
extern int SymmetryWRT_YZ2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimy-1-y,x,z)
   rotation of 90 degrees
   result image dimensions are (dimy, dimx, dimz)
   is inverted by RotationWRT_RZ3()
 */
extern int RotationWRT_RZ1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimx-1-x,dimy-1-y,z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int RotationWRT_RZ2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (y,dimx-1-x,z)
   rotation of 270 degrees
   result image dimensions are (dimy, dimx, dimz)
   is inverted by RotationWRT_RZ1()
 */
extern int RotationWRT_RZ3( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimz-1-z,y,x)
   rotation of 90 degrees
   result image dimensions are (dimz, dimy, dimx)
   is inverted by RotationWRT_RY3()
 */
extern int RotationWRT_RY1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (dimx-1-x,y,dimz-1-z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int RotationWRT_RY2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (z,y,dimx-1-x)
   rotation of 270 degrees
   result image dimensions are (dimz, dimy, dimx)
   is inverted by RotationWRT_RY1()
 */
extern int RotationWRT_RY3( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (x,dimz-1-z,y)
   rotation of 90 degrees
   result image dimensions are (dimx, dimz, dimy)
   is inverted by RotationWRT_RX3()
 */
extern int RotationWRT_RX1( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (x,dimy-1-y,dimz-1-z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */
extern int RotationWRT_RX2( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );


/* change (x,y,z) into (x,z,dimy-1-y)
   rotation of 270 degrees
   result image dimensions are (dimx, dimz, dimy)
   is inverted by RotationWRT_RX1()
 */
extern int RotationWRT_RX3( void *inputBuf,
			    void *outputBuf,
			    bufferType type,
			    int *theDim );



/* change (x,y,z) into (z,x,y)
   result image dimensions are (dimz, dimx, dimy)
   is inverted by Transpose_YZX()
 */
extern int Transpose_ZXY( void *inputBuf,
		       void *outputBuf,
		       bufferType type,
		       int *theDim );



/* change (x,y,z) into (y,z,x)
   result image dimensions are (dimy, dimz, dimx)
   is inverted by Transpose_ZXY()
 */
extern int Transpose_YZX( void *inputBuf,
		       void *outputBuf,
		       bufferType type,
		       int *theDim );



#ifdef __cplusplus
}
#endif

#endif

