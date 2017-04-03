/*************************************************************************
 * extract.h - extract sub-buffer
 *
 * $Id: extract.h,v 1.1 2000/03/27 17:44:27 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * http://www.inria.fr/epidaure/personnel/malandain/
 * 
 * CREATION DATE: 
 * Mon Mar 27 18:54:09 MET DST 2000
 *
 *
 *
 */

#ifndef _extract_h_
#define _extract_h_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <string.h>

#include <typedefs.h>

extern void ExtractFromBuffer( void *bufferIn,
			       int *bufferInDims,
			       void *bufferOut,
			       int *bufferOutDims,
			       int *firstPoint,
			       bufferType typeBuffer );

extern void ExtractFromBufferAndMelt( void *theBuf,
				      int *theBufDims,
				      void *resBuf,
				      int *resBufDims,
				      int *theOrigin,
				      int *resOrigin,
				      int *theDims,
				      bufferType typeBuffer );

extern void ExtractSlicesAndMelt( void *theInputBuf,
			   int *theBufDims,
			   void *resOuputBuf,
			   int *resBufDims,
			   int theZ, int theSkip,
			   int resZ, int resSkip,
			   int nSlices, int nBlocks,
			   bufferType typeBuffer );

#ifdef __cplusplus
}
#endif

#endif
