/*************************************************************************
 * zcross.h - zero-crossings
 *
 * $Id: zcross.h,v 1.2 2003/06/25 07:48:54 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Tue Nov 28 10:06:22 MET 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _zcross_h_
#define _zcross_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <typedefs.h>



extern void ZeroCrossings_verbose ( );
extern void ZeroCrossings_noverbose ( );

extern void ZeroCrossings_Are_Positive( );
extern void ZeroCrossings_Are_Negative( );




extern int Extract_ZeroCrossing_2D ( void *bufferIn, bufferType typeIn,
				     void *bufferOut, bufferType typeOut, 
				     int *bufferDims );




extern int Extract_PositiveZeroCrossing_2D ( void *bufferIn,
					     bufferType typeIn,
					     void *bufferOut,
					     bufferType typeOut,
					     int *bufferDims );
extern int Extract_NegativeZeroCrossing_2D ( void *bufferIn,
					     bufferType typeIn,
					     void *bufferOut,
					     bufferType typeOut,
					     int *bufferDims );







extern int zeroCrossing2D( void *bufferIn, bufferType typeIn,
			 void *bufferOut, bufferType typeOut, int *bufferDims );
extern int zeroCrossing3D( void *bufferIn, bufferType typeIn,
			 void *bufferOut, bufferType typeOut, int *bufferDims );
extern int zeroCrossing( void *bufferIn, bufferType typeIn,
			 void *bufferOut, bufferType typeOut, int *bufferDims );
extern int positiveZeroCrossing( void *bufferIn, bufferType typeIn,
				 void *bufferOut, bufferType typeOut, int *bufferDims );
extern int negativeZeroCrossing( void *bufferIn, bufferType typeIn,
				 void *bufferOut, bufferType typeOut, int *bufferDims );


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _zcross_h_ */
