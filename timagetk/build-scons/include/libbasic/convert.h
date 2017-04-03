/*************************************************************************
 * convert.h - conversion between types
 *
 * $Id: convert.h,v 1.3 1999/12/08 16:35:12 greg Exp $
 *
 * Copyright (c) INRIA 1998
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 * ADDITIONS, CHANGES
 *
 */

#ifndef _convert_h_
#define _convert_h_

#ifdef __cplusplus
extern "C" {
#endif






#include <stdio.h> /* memcpy */
#include <string.h> /* memcpy */
#include <typedefs.h>



extern void setVerboseInConvert( int v );
extern void incrementVerboseInConvert(  );
extern void decrementVerboseInConvert(  );



extern void printType( FILE *f, bufferType type );


/* Converts buffers from one type into another.
 *
 * DESCRIPTION:
 *
 * Conversions from types DOUBLE or FLOAT to 
 * others types are all implemented. I hope
 * I do not make mistakes.
 *
 * - When a real value is converted into an integer
 *   one, we take the nearest integer, i.e.
 *   (int)(real value - 0.5) for negative
 *   numbers, and (int)(real value + 0.5) for 
 *   positive ones.
 *
 * - Some conversions are not yet implemented.
 *
 * - When types and pointers are equal, nothing is
 *   done.
 */
extern int ConvertBuffer( void *bufferIn, /* buffer to be converted */
			   bufferType typeIn, /* type of this buffer */
			   void *bufferOut, /* result buffer */
			   bufferType typeOut, /* type of this buffer */
			   size_t bufferLength /* buffers' length */
			   );

extern void Convert_r32_to_s8( r32 *theBuf,
			       s8 *resBuf,
			       size_t size );

extern void Convert_r32_to_u8( r32 *theBuf,
			       u8 *resBuf,
			       size_t size );

extern void Convert_r32_to_s16( r32 *theBuf,
				s16 *resBuf,
				size_t size );

extern void Convert_r32_to_u16( r32 *theBuf,
				u16 *resBuf,
				size_t size );


extern int ConvertScaleBuffer( void *bufferIn,
				bufferType typeIn,
				void *bufferOut,
				bufferType typeOut,
				size_t bufferLength );



#ifdef __cplusplus
}
#endif

#endif
