/*************************************************************************
 * basictransform.c - basic transformations
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



#include <basictransform.h>

static int _verbose_ = 1;



/*------------------------------------------------------------
 *
 * symmetries w.r.t. planes defined by a principal direction
 *
 ------------------------------------------------------------*/



/* change (x,y,z) into (dimx-1-x,y,z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define SYMMETRYWRTX( TYPE ) {                                                                \
  TYPE *theBuf = (TYPE *)inputBuf;                                                            \
  TYPE *resBuf = (TYPE *)outputBuf;                                                           \
  TYPE t;                                                                                     \
  if ( inputBuf == outputBuf ) {                                                              \
    for ( z=0; z<dimz; z++ )                                                                  \
    for ( y=0; y<dimy; y++ )                                                                  \
    for ( x=0, i=z*dimx*dimy+y*dimx, j=z*dimx*dimy+y*dimx+dimx-1; x<dimx/2; x++, i++, j-- ) { \
      t = resBuf[j];                                                                          \
      resBuf[j] = theBuf[i];                                                                  \
      theBuf[i] = t;                                                                          \
    }                                                                                         \
  }                                                                                           \
  else {                                                                                      \
    for ( z=0; z<dimz; z++ )                                                                  \
    for ( y=0; y<dimy; y++ )                                                                  \
    for ( x=0, i=z*dimx*dimy+y*dimx, j=z*dimx*dimy+y*dimx+dimx-1; x<dimx; x++, i++, j-- ) {   \
      resBuf[j] = theBuf[i];                                                                  \
    }                                                                                         \
  }                                                                                           \
}

int SymmetryWRT_X( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_X";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i, j;

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTX( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTX( s8 )
    break;

  case USHORT :
    SYMMETRYWRTX( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTX( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTX( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,dimy-1-y,z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define SYMMETRYWRTY( TYPE ) {                                                                     \
  TYPE *theBuf = (TYPE *)inputBuf;                                                                 \
  TYPE *resBuf = (TYPE *)outputBuf;                                                                \
  TYPE t;										           \
  if ( inputBuf == outputBuf ) {								   \
    for ( z=0; z<dimz; z++ )								           \
    for ( x=0; x<dimx; x++ )								           \
    for ( y=0, i=z*dimx*dimy+x, j=z*dimx*dimy+(dimy-1)*dimx+x; y<dimy/2; y++, i+=dimx, j-=dimx ) { \
      t = resBuf[j];										   \
      resBuf[j] = theBuf[i];									   \
      theBuf[i] = t;										   \
    }												   \
  }												   \
  else {										           \
    for ( z=0; z<dimz; z++ )									   \
    for ( x=0; x<dimx; x++ )								           \
    for ( y=0, i=z*dimx*dimy+x, j=z*dimx*dimy+(dimy-1)*dimx+x; y<dimy; y++, i+=dimx, j-=dimx ) {   \
      resBuf[j] = theBuf[i];								           \
    }												   \
  }												   \
}

int SymmetryWRT_Y( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_Y";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i, j;

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTY( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTY( s8 )
    break;

  case USHORT :
    SYMMETRYWRTY( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTY( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTY( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,y,dimz-1-z)
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define SYMMETRYWRTZ( TYPE ) {                                                                          \
  TYPE *theBuf = (TYPE *)inputBuf;                                                                      \
  TYPE *resBuf = (TYPE *)outputBuf;                                                                     \
  TYPE t;										                \
  if ( inputBuf == outputBuf ) {                                                                        \
    for ( y=0; y<dimy; y++ )                                                                            \
    for ( x=0; x<dimx; x++ )                                                                            \
    for ( z=0, i=y*dimx+x, j=(dimz-1)*dimx*dimy+y*dimx+x; z<dimz/2; z++, i+=dimx*dimy, j-=dimx*dimy ) { \
      t = resBuf[j];                                                                                    \
      resBuf[j] = theBuf[i];                                                                            \
      theBuf[i] = t;                                                                                    \
    }                                                                                                   \
  }                                                                                                     \
  else {                                                                                                \
    for ( y=0; y<dimy; y++ )                                                                            \
    for ( x=0; x<dimx; x++ )                                                                            \
    for ( z=0, i=y*dimx+x, j=(dimz-1)*dimx*dimy+y*dimx+x; z<dimz; z++, i+=dimx*dimy, j-=dimx*dimy ) {   \
      resBuf[j] = theBuf[i];                                                                            \
    }                                                                                                   \
  }                                                                                                     \
}

int SymmetryWRT_Z( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_Z";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i, j;

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTZ( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTZ( s8 )
    break;

  case USHORT :
    SYMMETRYWRTZ( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTZ( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTZ( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/*------------------------------------------------------------
 *
 * symmetries w.r.t. planes defined by a diagonal and a principal direction
 *
 ------------------------------------------------------------*/



/* change (x,y,z) into (y,x,z)
   result image dimensions are (dimy, dimx, dimz)
   is inverted by the same transformation
 */

#define SYMMETRYWRTXYA( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ z*dimy*dimx + x*dimy + y ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_XY1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_XY1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTXYA( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTXYA( s8 )
    break;

  case USHORT :
    SYMMETRYWRTXYA( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTXYA( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTXYA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (dimy-1-y,dimx-1-x,z)
   result image dimensions are (dimy, dimx, dimz)
   is inverted by the same transformation
 */

#define SYMMETRYWRTXYB( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ z*dimy*dimx + (dimx-1-x)*dimy + dimy-1-y ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_XY2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_XY2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTXYB( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTXYB( s8 )
    break;

  case USHORT :
    SYMMETRYWRTXYB( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTXYB( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTXYB( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (z,y,x)
   result image dimensions are (dimz, dimy, dimx)
   is inverted by the same transformation
 */

#define SYMMETRYWRTXZA( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ x*dimz*dimy + y*dimz + z ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_XZ1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_XZ1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTXZA( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTXZA( s8 )
    break;

  case USHORT :
    SYMMETRYWRTXZA( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTXZA( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTXZA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (dimz-1-z,y,dimx-1-x)
   result image dimensions are (dimz, dimy, dimx)
   is inverted by the same transformation
 */

#define SYMMETRYWRTXZB( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ (dimx-1-x)*dimz*dimy + y*dimz + (dimz-1-z) ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_XZ2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_XZ2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTXZB( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTXZB( s8 )
    break;

  case USHORT :
    SYMMETRYWRTXZB( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTXZB( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTXZB( r32 )
    break;


  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,z,y)
   result image dimensions are (dimx, dimz, dimy)
   is inverted by the same transformation
 */

#define SYMMETRYWRTYZA( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ y*dimx*dimz + z*dimx + x ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_YZ1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_YZ1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTYZA( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTYZA( s8 )
    break;

  case USHORT :
    SYMMETRYWRTYZA( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTYZA( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTYZA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,dimz-1-z,dimy-1-y)
   result image dimensions are (dimx, dimz, dimy)
   is inverted by the same transformation
 */

#define SYMMETRYWRTYZB( TYPE ) {                    \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ (dimy-1-y)*dimx*dimz + (dimz-1-z)*dimx + x ] = theBuf[i]; \
  }	                                            \
}

int SymmetryWRT_YZ2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "SymmetryWRT_YZ2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    SYMMETRYWRTYZB( u8 )
    break;

  case SCHAR :
    SYMMETRYWRTYZB( s8 )
    break;

  case USHORT :
    SYMMETRYWRTYZB( u16 )
    break;

  case SSHORT :
    SYMMETRYWRTYZB( s16 )
    break;

  case FLOAT :
    SYMMETRYWRTYZB( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/*------------------------------------------------------------
 *
 * rotations
 *
 ------------------------------------------------------------*/



/* change (x,y,z) into (dimy-1-y,x,z)
   rotation of 90 degrees
   result image dimensions are (dimy, dimx, dimz)
   is inverted by RotationWRT_RZ3()       
 */

#define ROTATIONWRTRZA( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ z*dimy*dimx + x*dimy + dimy-1-y ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RZ1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RZ1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRZA( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRZA( s8 )
    break;

  case USHORT :
    ROTATIONWRTRZA( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRZA( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRZA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (dimx-1-x,dimy-1-y,z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define ROTATIONWRTRZB( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ z*dimx*dimy + (dimy-1-y)*dimx + dimx-1-x ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RZ2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RZ2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRZB( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRZB( s8 )
    break;

  case USHORT :
    ROTATIONWRTRZB( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRZB( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRZB( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (y,dimx-1-x,z)
   rotation of 270 degrees
   result image dimensions are (dimy, dimx, dimz)
   is inverted by RotationWRT_RZ1()
 */

#define ROTATIONWRTRZC( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ z*dimy*dimx + (dimx-1-x)*dimy + y ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RZ3( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RZ3";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRZC( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRZC( s8 )
    break;

  case USHORT :
    ROTATIONWRTRZC( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRZC( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRZC( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (dimz-1-z,y,x)
   rotation of 90 degrees
   result image dimensions are (dimz, dimy, dimx)
   is inverted by RotationWRT_RY3()
 */

#define ROTATIONWRTRYA( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ x*dimz*dimy + y*dimz + dimz-1-z ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RY1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RY1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRYA( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRYA( s8 )
    break;

  case USHORT :
    ROTATIONWRTRYA( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRYA( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRYA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (dimx-1-x,y,dimz-1-z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define ROTATIONWRTRYB( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ (dimz-1-z)*dimx*dimy + y*dimx + dimx-1-x ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RY2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RY2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRYB( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRYB( s8 )
    break;

  case USHORT :
    ROTATIONWRTRYB( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRYB( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRYB( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (z,y,dimx-1-x)
   rotation of 270 degrees
   result image dimensions are (dimz, dimy, dimx)
   is inverted by RotationWRT_RY1()
 */

#define ROTATIONWRTRYC( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ (dimx-1-x)*dimz*dimy + y*dimz + z ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RY3( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RY3";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRYC( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRYC( s8 )
    break;

  case USHORT :
    ROTATIONWRTRYC( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRYC( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRYC( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,dimz-1-z,y)
   rotation of 90 degrees
   result image dimensions are (dimx, dimz, dimy)
   is inverted by RotationWRT_RX3()
 */

#define ROTATIONWRTRXA( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ y*dimx*dimz + (dimz-1-z)*dimx + x ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RX1( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RX1";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRXA( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRXA( s8 )
    break;

  case USHORT :
    ROTATIONWRTRXA( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRXA( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRXA( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,dimy-1-y,dimz-1-z)
   rotation of 180 degrees
   result image dimensions are (dimx, dimy, dimz)
   is inverted by the same transformation
 */

#define ROTATIONWRTRXB( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ (dimz-1-z)*dimx*dimy + (dimy-1-y)*dimx + x ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RX2( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RX2";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRXB( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRXB( s8 )
    break;

  case USHORT :
    ROTATIONWRTRXB( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRXB( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRXB( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (x,z,dimy-1-y)
   rotation of 270 degrees
   result image dimensions are (dimx, dimz, dimy)
   is inverted by RotationWRT_RX1()
 */

#define ROTATIONWRTRXC( TYPE ) {                           \
  TYPE *theBuf = (TYPE *)inputBuf;                         \
  TYPE *resBuf = (TYPE *)outputBuf;                        \
  for ( i=0, z=0; z<dimz; z++ )	                           \
  for ( y=0; y<dimy; y++ )	                           \
  for ( x=0; x<dimx; x++, i++ ) {	                   \
    resBuf[ (dimy-1-y)*dimx*dimz + z*dimx + x ] = theBuf[i]; \
  }	                                                   \
}

int RotationWRT_RX3( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "RotationWRT_RX3";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    ROTATIONWRTRXC( u8 )
    break;

  case SCHAR :
    ROTATIONWRTRXC( s8 )
    break;

  case USHORT :
    ROTATIONWRTRXC( u16 )
    break;

  case SSHORT :
    ROTATIONWRTRXC( s16 )
    break;

  case FLOAT :
    ROTATIONWRTRXC( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/*------------------------------------------------------------
 *
 * transpositions
 *
 ------------------------------------------------------------*/



/* change (x,y,z) into (z,x,y)
   result image dimensions are (dimz, dimx, dimy)
   is inverted by Transpose2()
 */

#define TRANSPOSEZXY( TYPE ) {                      \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ y*dimz*dimx + x*dimz + z ] = theBuf[i]; \
  }	                                            \
}

int Transpose_ZXY( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "Transpose_ZXY";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    TRANSPOSEZXY( u8 )
    break;

  case SCHAR :
    TRANSPOSEZXY( s8 )
    break;

  case USHORT :
    TRANSPOSEZXY( u16 )
    break;

  case SSHORT :
    TRANSPOSEZXY( s16 )
    break;

  case FLOAT :
    TRANSPOSEZXY( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}





/* change (x,y,z) into (y,z,x)
   result image dimensions are (dimy, dimz, dimx)
   is inverted by Transpose1()
 */

#define TRANSPOSEYZX( TYPE ) {                      \
  TYPE *theBuf = (TYPE *)inputBuf;                  \
  TYPE *resBuf = (TYPE *)outputBuf;                 \
  for ( i=0, z=0; z<dimz; z++ )	                    \
  for ( y=0; y<dimy; y++ )	                    \
  for ( x=0; x<dimx; x++, i++ ) {	            \
    resBuf[ x*dimy*dimz + z*dimy + y ] = theBuf[i]; \
  }	                                            \
}

int Transpose_YZX( void *inputBuf,
		    void *outputBuf,
		    bufferType type,
		    int *theDim )
{
  char *proc = "Transpose_YZX";
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int x, y, z, i;

  if ( inputBuf == outputBuf ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input and output buffers have to be different\n", proc );
    return( -1 );
  }

  switch ( type ) {

  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input type is not handled yet\n", proc );
    return( -1 );
    break;
    
  case UCHAR :
    TRANSPOSEYZX( u8 )
    break;

  case SCHAR :
    TRANSPOSEYZX( s8 )
    break;

  case USHORT :
    TRANSPOSEYZX( u16 )
    break;

  case SSHORT :
    TRANSPOSEYZX( s16 )
    break;

  case FLOAT :
    TRANSPOSEYZX( r32 )
    break;

  } /* end of switch ( type ) */

  return( 1 );
}
