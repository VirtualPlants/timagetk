/*************************************************************************
 * pixel-operation.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Tue Jan  1 21:45:19 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _pixel_operation_h_
#define _pixel_operation_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>



extern void setVerboseInPixelOperation( int v );
extern void incrementVerboseInPixelOperation(  );
extern void decrementVerboseInPixelOperation(  );



extern int maskImage( void *bufferIn,   bufferType typeIn,
		      void *bufferMask, bufferType typeMask,
		      void *bufferOut,  bufferType typeOut,
		      int *bufferDims );



extern int minImages( void *bufferIn1, bufferType typeIn1,
		     void *bufferIn2, bufferType typeIn2,
		     void *bufferOut, bufferType typeOut,
		     int *bufferDims );

extern int maxImages( void *bufferIn1, bufferType typeIn1,
		     void *bufferIn2, bufferType typeIn2,
		     void *bufferOut, bufferType typeOut,
		     int *bufferDims );

extern int addImages( void *bufferIn1, bufferType typeIn1,
		      void *bufferIn2, bufferType typeIn2,
		      void *bufferOut, bufferType typeOut,
		      int *bufferDims );

extern int subtractImages( void *bufferIn1, bufferType typeIn1,
			    void *bufferIn2, bufferType typeIn2,
			    void *bufferOut, bufferType typeOut,
			    int *bufferDims );

extern int multiplyImages( void *bufferIn1, bufferType typeIn1,
			   void *bufferIn2, bufferType typeIn2,
			   void *bufferOut, bufferType typeOut,
			   int *bufferDims );

extern int divideImages( void *bufferIn1, bufferType typeIn1,
			 void *bufferIn2, bufferType typeIn2,
			 void *bufferOut, bufferType typeOut,
			 int *bufferDims );




extern int minImageDouble( void *bufferIn1, bufferType typeIn1,
			   double scalar,
			   void *bufferOut, bufferType typeOut,
			   int *bufferDims );

extern int maxImageDouble( void *bufferIn1, bufferType typeIn1,
			   double scalar,
			   void *bufferOut, bufferType typeOut,
			   int *bufferDims );

extern int addImageDouble( void *bufferIn1, bufferType typeIn1,
			   double scalar,
			   void *bufferOut, bufferType typeOut,
			   int *bufferDims );

extern int subtractImageDouble( void *bufferIn1, bufferType typeIn1,
				 double scalar,
				 void *bufferOut, bufferType typeOut,
				 int *bufferDims );

extern int multiplyImageDouble( void *bufferIn1, bufferType typeIn1,
				double scalar,
				void *bufferOut, bufferType typeOut,
				int *bufferDims );

extern int divideImageDouble( void *bufferIn1, bufferType typeIn1,
			      double scalar,
			      void *bufferOut, bufferType typeOut,
			      int *bufferDims );



extern int sqrImage( void *bufferIn, bufferType typeIn,
		     void *bufferOut, bufferType typeOut,
		     int *bufferDims ) ;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _pixel_operation_h_ */
