/*************************************************************************
 * local-operation.h - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sun Feb 17 20:27:49 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _local_operation_h_
#define _local_operation_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <typedefs.h>



extern void setVerboseInLocalOperation( int v );
extern void incrementVerboseInLocalOperation(  );
extern void decrementVerboseInLocalOperation(  );



/**************************************************
 *
 * operation on one buffer
 *
 **************************************************/

extern int medianFiltering( void *bufferIn,
			    bufferType typeIn,
			    void *bufferOut,
			    bufferType typeOut,
			    int *bufferDims,
			    int *windowDims );

extern int quantileFiltering( void *bufferIn,
			    bufferType typeIn,
			    void *bufferOut,
			    bufferType typeOut,
			    int *bufferDims,
			      int *windowDims,
			      double quantile );

extern int minFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims );

extern int maxFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims );

extern int meanFiltering( void *bufferIn,
			  bufferType typeIn,
			  void *bufferOut,
			  bufferType typeOut,
			  int *bufferDims,
			  int *windowDims );

extern int sumFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims );

extern int sumSquaresFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims );

extern int stddevFiltering( void *bufferIn,
			    bufferType typeIn,
			    void *bufferOut,
			    bufferType typeOut,
			    int *bufferDims,
			    int *windowDims );

extern int varFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims );

extern int robustMeanFiltering( void *bufferIn,
				bufferType typeIn,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims,
				double lts_fraction );



/**************************************************
 *
 * operation on an array of buffer
 *
 **************************************************/

extern int medianFilteringBuffers( void **bufferIn,
				   bufferType typeIn,
				   int nBuffers,
				   void *bufferOut,
				   bufferType typeOut,
				   int *bufferDims,
				   int *windowDims );

extern int quantileFilteringBuffers( void **bufferIn,
				     bufferType typeIn,
				     int nBuffers,
				     void *bufferOut,
				     bufferType typeOut,
				     int *bufferDims,
				     int *windowDims,
				     double quantile );

extern int minFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims );

extern int maxFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims );

extern int meanFilteringBuffers( void **bufferIn,
				 bufferType typeIn,
				 int nBuffers,
				 void *bufferOut,
				 bufferType typeOut,
				 int *bufferDims,
				 int *windowDims );

extern int sumFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims );

extern int sumSquaresFilteringBuffers( void **bufferIn,
				       bufferType typeIn,
				       int nBuffers,
				       void *bufferOut,
				       bufferType typeOut,
				       int *bufferDims,
				       int *windowDims );

extern int stddevFilteringBuffers( void **bufferIn,
				   bufferType typeIn,
				   int nBuffers,
				   void *bufferOut,
				   bufferType typeOut,
				   int *bufferDims,
				   int *windowDims );

extern int varFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims );

extern int robustMeanFilteringBuffers( void **bufferIn,
				       bufferType typeIn,
				       int nBuffers,
				       void *bufferOut,
				       bufferType typeOut,
				       int *bufferDims,
				       int *windowDims,
				       double lts_fraction );



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _local_operation_h_ */
