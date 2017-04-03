/*************************************************************************
 * extract.c - extract sub-buffer
 *
 * $Id: extract.c,v 1.1 2000/03/27 17:44:27 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Mon Mar 27 18:12:37 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <extract.h>

static int _verbose_ = 1;
static int _debug_ = 0;

void ExtractFromBuffer( void *theInputBuf,
			int *theBufDims,
			void *resOutputBuf,
			int *resBufDims,
			int *theOrigin,
			bufferType typeBuffer )
{
  char *proc = "ExtractFromBuffer";
  int size = 0;

  char *theBuf = (char*)theInputBuf;
  char *resBuf = (char*)resOutputBuf;

  size_t theBufSlice = theBufDims[0] * theBufDims[1];
  int theBufLine  = theBufDims[0];
  int theBufBegLine = theOrigin[0];
  int windowLine = resBufDims[0];
  int theBufEndLine = theBufLine - theBufBegLine - windowLine;
    
  int z, y;
  
   if ( resBufDims[0] < 1 || resBufDims[1] < 1 || resBufDims[2] < 1 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: bad dimensions for output image\n", proc );	       
    }
    return;
  }

  if ( theOrigin[0] < 0 || theOrigin[0] >= theBufDims[0] ||
       theOrigin[1] < 0 || theOrigin[1] >= theBufDims[1] ||
       theOrigin[2] < 0 || theOrigin[2] >= theBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: window origin incompatible with input image dimensions\n", proc );	       
    }
    return;
  }
  
  if ( theOrigin[0]+resBufDims[0] > theBufDims[0] ||
       theOrigin[1]+resBufDims[1] > theBufDims[1] ||
       theOrigin[2]+resBufDims[2] > theBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: window dimensions incompatible with input image dimensions\n", proc );
    }
    return;
  }

  switch ( typeBuffer ) {
  case UCHAR  : size = sizeof( u8 );  break;
  case SCHAR  : size = sizeof( s8 );  break;
  case USHORT : size = sizeof( u16 ); break;
  case SSHORT : size = sizeof( s16 ); break;
  case UINT   : size = sizeof( u32 ); break;
  case SINT    : size = sizeof( i32 ); break;
  case ULINT  : size = sizeof( u64 ); break;
  case FLOAT  : size = sizeof( r32 ); break;
  case DOUBLE : size = sizeof( r64 ); break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );	       
    }
    return;
  }

  theBufSlice *= size;
  theBufLine *= size;
  theBufBegLine *= size;
  theBufEndLine *= size;

  windowLine *= size;

  /* we do not need to copy any part of these slices
   */
  for ( z = 0; z < theOrigin[2]; z++ ) theBuf += theBufSlice;

  for ( z = 0; z < resBufDims[2]; z++ ) {
    /* we do not need to copy any part of these lines
     */
    theBuf += theOrigin[1]*theBufLine;

    for ( y = 0; y < resBufDims[1]; y++ ) {
      theBuf += theBufBegLine;
      (void)memcpy( resBuf, theBuf, windowLine );
      theBuf += windowLine+theBufEndLine;
      resBuf += windowLine;
    }
    
    theBuf += (theBufDims[1] - resBufDims[1] - theOrigin[1])*theBufLine;
  }
  
}





void ExtractFromBufferAndMelt( void *theInputBuf,
			       int *theBufDims,
			       void *resOuputBuf,
			       int *resBufDims,
			       int *theOrigin,
			       int *resOrigin,
			       int *theDims,
			       bufferType typeBuffer )
{
  char *proc = "ExtractFromBufferAndMelt";
  int size = 0;

  char *theBuf = (char*)theInputBuf;
  char *resBuf = (char*)resOuputBuf;

  size_t theBufSlice = theBufDims[3] * theBufDims[0] * theBufDims[1];
  int theBufLine  = theBufDims[3] * theBufDims[0];
  int theBufBegLine = theBufDims[3] * theOrigin[0];
  size_t resBufSlice = resBufDims[3] * resBufDims[0] * resBufDims[1];
  int resBufLine  = resBufDims[3] * resBufDims[0];
  int resBufBegLine = resBufDims[3] * resOrigin[0];
  int windowLine = theBufDims[3] * theDims[0];
  int theBufEndLine = theBufLine - theBufBegLine - windowLine;
  int resBufEndLine = resBufLine - resBufBegLine - windowLine;
    
  int z, y;

  if ( _debug_ ) {
    fprintf( stderr, "\t source window origin is (%d %d %d)\n", theOrigin[0], theOrigin[1], theOrigin[2] );
    fprintf( stderr, "\t window dimensions are [%d %d %d]\n", theDims[0], theDims[1], theDims[2] );
    fprintf( stderr, "\t source image dimensions are [[%d] %d %d %d]\n", theBufDims[3], theBufDims[0], theBufDims[1], theBufDims[2] );
    fprintf( stderr, "\t target window origin is (%d %d %d)\n", resOrigin[0], resOrigin[1], resOrigin[2] );
    fprintf( stderr, "\t target image dimensions are [[%d] %d %d %d]\n", resBufDims[3], resBufDims[0], resBufDims[1], resBufDims[2] );
  }

  if ( resBufDims[3] != theBufDims[3] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: different vectorial dimensions for input and output images\n", proc );	       
    }
    return;
  }

  if ( resBufDims[0] < 1 || resBufDims[1] < 1 || resBufDims[2] < 1 || resBufDims[3] < 1 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: bad dimensions for output image\n", proc );	       
    }
    return;
  }

  if ( theDims[0] < 1 || theDims[1] < 1 || theDims[2] < 1 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: bad dimensions for window\n", proc );	       
    }
    return;
  }

  if ( theOrigin[0] < 0 || theOrigin[0] >= theBufDims[0] ||
       theOrigin[1] < 0 || theOrigin[1] >= theBufDims[1] ||
       theOrigin[2] < 0 || theOrigin[2] >= theBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: window origin incompatible with input image dimensions\n", proc );	       
    }
    return;
  }

  if ( theOrigin[0]+theDims[0] > theBufDims[0] ||
       theOrigin[1]+theDims[1] > theBufDims[1] ||
       theOrigin[2]+theDims[2] > theBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "\t source window origin is (%d %d %d)\n", theOrigin[0], theOrigin[1], theOrigin[2] );
      fprintf( stderr, "\t window dimensions are [%d %d %d]\n", theDims[0], theDims[1], theDims[2] );
      fprintf( stderr, "\t source image dimensions are [[%d] %d %d %d]\n", theBufDims[3], theBufDims[0], theBufDims[1], theBufDims[2] );
      fprintf( stderr, "%s: window dimensions incompatible with input image dimensions\n", proc );
    }
    return;
  }

  if ( resOrigin[0] < 0 || resOrigin[0] >= resBufDims[0] ||
       resOrigin[1] < 0 || resOrigin[1] >= resBufDims[1] ||
       resOrigin[2] < 0 || resOrigin[2] >= resBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "\t target window origin is (%d %d %d)\n", resOrigin[0], resOrigin[1], resOrigin[2] );
      fprintf( stderr, "\t target image dimensions are [[%d] %d %d %d]\n", resBufDims[3], resBufDims[0], resBufDims[1], resBufDims[2] );
      fprintf( stderr, "%s: window origin incompatible with output image dimensions\n", proc );	       
    }
    return;
  }
  
  if ( resOrigin[0]+theDims[0] > resBufDims[0] ||
       resOrigin[1]+theDims[1] > resBufDims[1] ||
       resOrigin[2]+theDims[2] > resBufDims[2] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: window dimensions incompatible with output image dimensions\n", proc );
    }
    return;
  }

  switch ( typeBuffer ) {
  case UCHAR  : size = sizeof( u8 );  break;
  case SCHAR  : size = sizeof( s8 );  break;
  case USHORT : size = sizeof( u16 ); break;
  case SSHORT : size = sizeof( s16 ); break;
  case UINT   : size = sizeof( u32 ); break;
  case SINT    : size = sizeof( i32 ); break;
  case ULINT  : size = sizeof( u64 ); break;
  case FLOAT  : size = sizeof( r32 ); break;
  case DOUBLE : size = sizeof( r64 ); break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );	       
    }
    return;
  }

  theBufSlice *= size;
  theBufLine *= size;
  theBufBegLine *= size;
  theBufEndLine *= size;

  resBufSlice *= size;
  resBufLine *= size;
  resBufBegLine *= size;
  resBufEndLine *= size;
  
  windowLine *= size;

  if ( _debug_ ) {
    fprintf( stderr, "\t input buffer slice size is %lu\n", theBufSlice );
    fprintf( stderr, "\t input buffer line size is %d = %d + %d + %d (%d)\n",
	     theBufLine, theBufBegLine, windowLine, theBufEndLine, theBufBegLine+windowLine+theBufEndLine );
    fprintf( stderr, "\t output buffer slice size is %lu\n", resBufSlice );
    fprintf( stderr, "\t output buffer line size is %d = %d + %d + %d (%d)\n",
	     resBufLine, resBufBegLine, windowLine, resBufEndLine, resBufBegLine+windowLine+resBufEndLine );
  }

  /* we do not need to copy any part of these slices
   */
  for ( z = 0; z < theOrigin[2]; z++ ) theBuf += theBufSlice;
  for ( z = 0; z < resOrigin[2]; z++ ) resBuf += resBufSlice;

  for ( z = 0; z < theDims[2]; z++ ) {
    /* we do not need to copy any part of these lines
     */
    theBuf += theOrigin[1]*theBufLine;
    resBuf += resOrigin[1]*resBufLine;

    for ( y = 0; y < theDims[1]; y++ ) {
      theBuf += theBufBegLine;
      resBuf += resBufBegLine;
      (void)memcpy( resBuf, theBuf, windowLine );
      theBuf += windowLine+theBufEndLine;
      resBuf += windowLine+resBufEndLine;
    }

    theBuf += (theBufDims[1] - theDims[1] - theOrigin[1])*theBufLine;
    resBuf += (resBufDims[1] - theDims[1] - resOrigin[1])*resBufLine;
  }
  
}
			




/* copy nBlocks of nSlices from input to output
   block origins are [the|res]Z + i *  [the|res]Skip
   i=0...nBlocks-1

   thus it is equivalent to specify 1 block of n slices
   or blocks of 1 slice (with a skip value of 1)
*/
void ExtractSlicesAndMelt( void *theInputBuf,
			   int *theBufDims,
			   void *resOuputBuf,
			   int *resBufDims,
			   int theZ, int theSkip,
			   int resZ, int resSkip,
			   int nSlices, int nBlocks,
			   bufferType typeBuffer )
{
  char *proc = "ExtractSlicesAndMelt";

  char *theBuf = (char*)theInputBuf;
  char *resBuf = (char*)resOuputBuf;

  int theSliceSize = 0;
  int size = 0;
  int i, tz, rz;



  if ( _debug_ ) {
    fprintf( stderr, "%s: input dimensions = %d x %d x %d\n", proc,
	     theBufDims[0], theBufDims[1], theBufDims[2] );
    fprintf( stderr, "\t output dimensions = %d x %d x %d\n",
	     resBufDims[0], resBufDims[1], resBufDims[2] );
    fprintf( stderr, "\t copy %d blocks of %d slices\n", nBlocks, nSlices );
    fprintf( stderr, "\t from z=%d +=%d to z=%d +=%d\n", theZ, theSkip, resZ, resSkip );
  }



  if ( theBufDims[0] != resBufDims[0] || theBufDims[1] != resBufDims[1] ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: slice dimensions of the two images are different\n", proc );	       
    }
    return;
  }

  theSliceSize =  theBufDims[0] * theBufDims[1];

  switch ( typeBuffer ) {
  case UCHAR  : size = sizeof( u8 );  break;
  case SCHAR  : size = sizeof( s8 );  break;
  case USHORT : size = sizeof( u16 ); break;
  case SSHORT : size = sizeof( s16 ); break;
  case UINT   : size = sizeof( u32 ); break;
  case SINT    : size = sizeof( i32 ); break;
  case ULINT  : size = sizeof( u64 ); break;
  case FLOAT  : size = sizeof( r32 ); break;
  case DOUBLE : size = sizeof( r64 ); break;
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );	       
    }
    return;
  }

  theSliceSize *= size;

  for ( tz=0; tz<theZ; tz++ ) theBuf += theSliceSize;
  for ( rz=0; rz<resZ; rz++ ) resBuf += theSliceSize;
  
  for ( tz = theZ, rz = resZ, i = 0;
	tz+nSlices <= theBufDims[2] && rz+nSlices <= resBufDims[2] && i < nBlocks;
	theBuf += theSliceSize * theSkip, resBuf += theSliceSize * resSkip, 
	  tz+=theSkip, rz+=resSkip, i++ ) {
    (void)memcpy( resBuf, theBuf, theSliceSize*nSlices );
  }
}
