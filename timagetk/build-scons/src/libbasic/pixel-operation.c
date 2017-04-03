/*************************************************************************
 * pixel-operation.c - 
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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convert.h>
#include <pixel-operation.h>

static int _verbose_ = 1;



void setVerboseInPixelOperation( int v )
{
  _verbose_ = v;
}

void incrementVerboseInPixelOperation(  )
{
  _verbose_ ++;
}

void decrementVerboseInPixelOperation(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



typedef enum enumTwoImagesOperation {
  _MINIMUM_,
  _MAXIMUM_,
  _ADD_IMAGE_,
  _SUB_IMAGE_,
  _MUL_IMAGE_,
  _DIV_IMAGE_
} enumTwoImagesOperation;

typedef enum enumOneImageOperation {
  _SQR_
} enumOneImageOperation;

typedef enum enumImageScalarOperation {
  _MIN_SCALAR_,
  _MAX_SCALAR_,
  _ADD_SCALAR_,
  _SUB_SCALAR_,
  _MUL_SCALAR_,
  _DIV_SCALAR_
} enumImageScalarOperation;



int maskImage( void *bufferIn,   bufferType typeIn,
               void *bufferMask, bufferType typeMask,
               void *bufferOut,  bufferType typeOut,
               int *bufferDims )
{
  char *proc = "maskImage";
  size_t i, v;
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];
  
  if ( typeIn != typeOut ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: buffers in and out should have the same type.\n", proc );
    return( -1 );
  }

  
#define _MASK_( type ) { \
  type *theBuf = (type*)bufferIn;  \
  type *resBuf = (type*)bufferOut; \
  for (i=0; i<v; i++ ) {           \
    resBuf[i] = ( theMask[i] > 0 ) ? theBuf[i] : 0; \
  } \
}

  switch( typeMask ) {

  case UCHAR :
    {
      u8 *theMask = (u8*)bufferMask;
      switch( typeIn ) {
      case UCHAR :
        _MASK_( u8 )
        break;
      case SCHAR :
        _MASK_( s8 )
        break;
      case USHORT :
        _MASK_( u16 )
        break;
      case SSHORT :
        _MASK_( s16 )
        break;
      case SINT :
        _MASK_( i32 )
        break;
      case UINT :
        _MASK_( u32 )
        break;
      case FLOAT :
        _MASK_( r32 )
        break;
      case DOUBLE :
        _MASK_( r64 )
        break;
      default :
        if (_verbose_ > 0 )
          fprintf( stderr, "%s: such output type not handled.\n", proc );
        return( -1 );
      }
    } /* mask: end of UCHAR */
    break;

  case FLOAT :
    {
      r32 *theMask = (r32*)bufferMask;
      switch( typeIn ) {
      case UCHAR :
        _MASK_( u8 )
        break;
      case SCHAR :
        _MASK_( s8 )
        break;
      case USHORT :
        _MASK_( u16 )
        break;
      case SSHORT :
        _MASK_( s16 )
        break;
      case SINT :
        _MASK_( i32 )
        break;
      case UINT :
        _MASK_( u32 )
        break;
      case FLOAT :
        _MASK_( r32 )
        break;
      case DOUBLE :
        _MASK_( r64 )
        break;
      default :
        if (_verbose_ > 0 )
          fprintf( stderr, "%s: such output type not handled.\n", proc );
        return( -1 );
      }
    } /* mask: end of FLOAT */
    break;

  default :
    if (_verbose_ > 0 )
      fprintf( stderr, "%s: such mask type not handled.\n", proc );
    return( -1 );
  }

  return( 1 );
}





/**************************************************
 *
 * static procedures: two images
 *
 **************************************************/



static int _minTwoImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  char *proc = "_minTwoImages";
  size_t i, v;
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];
  
  if ( typeIn1 != typeIn2 || typeIn1 != typeOut ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: buffers should have the same type.\n", proc );
    return( -1 );
  }

  
#define _TWO_IMAGES_DEDICATED_MIN_( type ) { \
  type *theBuf1 = (type*)bufferIn1; \
  type *theBuf2 = (type*)bufferIn2; \
  type *resBuf = (type*)bufferOut;  \
  for (i=0; i<v; i++ ) {            \
    resBuf[i] = ( theBuf1[i] < theBuf2[i] ) ? theBuf1[i] : theBuf2[i]; \
  }                                 \
}

  switch( typeIn1 ) {

  case UCHAR :
    _TWO_IMAGES_DEDICATED_MIN_( u8 );
    break;
  case SCHAR :
    _TWO_IMAGES_DEDICATED_MIN_( s8 );
    break;
  case USHORT :
    _TWO_IMAGES_DEDICATED_MIN_( u16 );
    break;
  case SSHORT :
    _TWO_IMAGES_DEDICATED_MIN_( s16 );
    break;
  case SINT :
    _TWO_IMAGES_DEDICATED_MIN_( i32 );
    break;
  case UINT :
    _TWO_IMAGES_DEDICATED_MIN_( u32 );
    break;
  case FLOAT :
    _TWO_IMAGES_DEDICATED_MIN_( r32 );
    break;
  case DOUBLE :
    _TWO_IMAGES_DEDICATED_MIN_( r64 );
    break;
  default :
    if (_verbose_ > 0 )
      fprintf( stderr, "%s: such  type not handled.\n", proc );
    return( -1 );
  }

  return( 1 );
}




   
static int _maxTwoImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  char *proc = "_maxTwoImages";
  size_t i, v;
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];
  
  if ( typeIn1 != typeIn2 || typeIn1 != typeOut ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: buffers should have the same type.\n", proc );
    return( -1 );
  }

  
#define _TWO_IMAGES_DEDICATED_MAX_( type ) { \
  type *theBuf1 = (type*)bufferIn1; \
  type *theBuf2 = (type*)bufferIn2; \
  type *resBuf = (type*)bufferOut;  \
  for (i=0; i<v; i++ ) {            \
    resBuf[i] = ( theBuf1[i] > theBuf2[i] ) ? theBuf1[i] : theBuf2[i]; \
  }                                 \
}

  switch( typeIn1 ) {

  case UCHAR :
    _TWO_IMAGES_DEDICATED_MAX_( u8 );
    break;
  case SCHAR :
    _TWO_IMAGES_DEDICATED_MAX_( s8 );
    break;
  case USHORT :
    _TWO_IMAGES_DEDICATED_MAX_( u16 );
    break;
  case SSHORT :
    _TWO_IMAGES_DEDICATED_MAX_( s16 );
    break;
  case SINT :
    _TWO_IMAGES_DEDICATED_MAX_( i32 );
    break;
  case UINT :
    _TWO_IMAGES_DEDICATED_MAX_( u32 );
    break;
  case FLOAT :
    _TWO_IMAGES_DEDICATED_MAX_( r32 );
    break;
  case DOUBLE :
    _TWO_IMAGES_DEDICATED_MAX_( r64 );
    break;
  default :
    if (_verbose_ > 0 )
      fprintf( stderr, "%s: such  type not handled.\n", proc );
    return( -1 );
  }

  return( 1 );
}





static int _ArithmeticTwoImages( void *bufferIn1, bufferType typeIn1,
                              void *bufferIn2, bufferType typeIn2,
                              void *bufferOut, bufferType typeOut,
                              int *bufferDims,
                              enumTwoImagesOperation operation )
{
  char *proc = "_ArithmeticTwoImages";
  bufferType typeAux = TYPE_UNKNOWN;
  void *auxBuf = (void*)NULL;
  size_t i, v;

  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];


  /* auxiliary buffer for computation
   */
  switch ( typeIn1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
  case UINT :
  case SINT :
  case FLOAT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
    case SCHAR :
    case USHORT :
    case SSHORT :
    case UINT :
    case SINT :
    case FLOAT :
      typeAux = (typeOut == DOUBLE) ? DOUBLE : FLOAT;
      break;
    case ULINT :
    case DOUBLE :
      typeAux = (typeOut == FLOAT) ? FLOAT : DOUBLE;
      break;
    }
    break;
  case ULINT :
  case DOUBLE :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
    case SCHAR :
    case USHORT :
    case SSHORT :
    case UINT :
    case SINT :
    case FLOAT :
    case ULINT :
    case DOUBLE :
      typeAux = (typeOut == FLOAT) ? FLOAT : DOUBLE;
      break;
    }
    break;
  }

  if ( typeAux != typeOut ) {
    switch ( typeAux ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      auxBuf = (void*)malloc( v*sizeof( r32 ) );
      break;
    case DOUBLE :
      auxBuf = (void*)malloc( v*sizeof( r64 ) );
      break;
      break;
    }
    if ( auxBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
  }
  else {
    auxBuf = (void*)bufferOut;
  }


#define _TWO_IMAGES_AUXILIARY( TYPEAUX ) {        \
      TYPEAUX *resBuf = (TYPEAUX*)auxBuf;         \
      switch ( operation ) {                      \
      default :                                   \
        if ( typeAux != typeOut ) free( auxBuf ); \
        if ( _verbose_ )                          \
          fprintf( stderr, "%s: such operation not handled yet\n", proc ); \
        return( -1 );                             \
      case _MINIMUM_ :                            \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = ((TYPEAUX)theBuf1[i] < (TYPEAUX)theBuf2[i]) ? (TYPEAUX)theBuf1[i] : (TYPEAUX)theBuf2[i]; \
        break;                                    \
      case _MAXIMUM_ :                            \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = ((TYPEAUX)theBuf1[i] > (TYPEAUX)theBuf2[i]) ? (TYPEAUX)theBuf1[i] : (TYPEAUX)theBuf2[i]; \
        break;                                    \
      case _ADD_IMAGE_ :                          \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = (TYPEAUX)theBuf1[i] + (TYPEAUX)theBuf2[i]; \
        break;                                    \
      case _SUB_IMAGE_ :                                 \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = (TYPEAUX)theBuf1[i] - (TYPEAUX)theBuf2[i]; \
        break;                                    \
      case _MUL_IMAGE_ :                          \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = (TYPEAUX)theBuf1[i] * (TYPEAUX)theBuf2[i]; \
        break;                                    \
      case _DIV_IMAGE_ :                          \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = (TYPEAUX)theBuf1[i] / (TYPEAUX)theBuf2[i]; \
        break;                                    \
      }                                           \
}

#define _TWO_IMAGES_OPERATION( TYPE1, TYPE2 ) { \
  TYPE1 *theBuf1 = (TYPE1*)bufferIn1;           \
  TYPE2 *theBuf2 = (TYPE2*)bufferIn2;           \
  switch ( typeAux ) {                          \
  default :                                     \
    if ( typeAux != typeOut ) free( auxBuf );   \
    if ( _verbose_ )                            \
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc ); \
    return( -1 );                               \
  case FLOAT :                                  \
    _TWO_IMAGES_AUXILIARY( r32 );               \
    break;                                      \
  case DOUBLE :                                 \
    _TWO_IMAGES_AUXILIARY( r64 );               \
    break;                                      \
  }                                             \
}


  switch ( typeIn1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _TWO_IMAGES_OPERATION( u8, u8 );
      break;
    case SCHAR :
      _TWO_IMAGES_OPERATION( u8, s8 );
      break;
    case USHORT :
      _TWO_IMAGES_OPERATION( u8, u16 );
      break;
    case SSHORT :
      _TWO_IMAGES_OPERATION( u8, s16 );
      break;
    case FLOAT :
      _TWO_IMAGES_OPERATION( u8, r32 );
      break;
    }
    break;
  case SCHAR :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _TWO_IMAGES_OPERATION( s8, u8 );
      break;
    case SCHAR :
      _TWO_IMAGES_OPERATION( s8, s8 );
      break;
    case USHORT :
      _TWO_IMAGES_OPERATION( s8, u16 );
      break;
    case SSHORT :
      _TWO_IMAGES_OPERATION( s8, s16 );
      break;
    case FLOAT :
      _TWO_IMAGES_OPERATION( s8, r32 );
      break;
    }
    break;
  case USHORT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _TWO_IMAGES_OPERATION( u16, u8 );
      break;
    case SCHAR :
      _TWO_IMAGES_OPERATION( u16, s8 );
      break;
    case USHORT :
      _TWO_IMAGES_OPERATION( u16, u16 );
      break;
    case SSHORT :
      _TWO_IMAGES_OPERATION( u16, s16 );
      break;
    case FLOAT :
      _TWO_IMAGES_OPERATION( u16, r32 );
      break;
    }
    break;
  case SSHORT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _TWO_IMAGES_OPERATION( s16, u8 );
      break;
    case SCHAR :
      _TWO_IMAGES_OPERATION( s16, s8 );
      break;
    case USHORT :
      _TWO_IMAGES_OPERATION( s16, u16 );
      break;
    case SSHORT :
      _TWO_IMAGES_OPERATION( s16, s16 );
      break;
    case FLOAT :
      _TWO_IMAGES_OPERATION( s16, r32 );
      break;
    }
    break;
  case FLOAT :
    switch ( typeIn2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      _TWO_IMAGES_OPERATION( r32, u8 );
      break;
    case SCHAR :
      _TWO_IMAGES_OPERATION( r32, s8 );
      break;
    case USHORT :
      _TWO_IMAGES_OPERATION( r32, u16 );
      break;
    case SSHORT :
      _TWO_IMAGES_OPERATION( r32, s16 );
      break;
    case FLOAT :
      _TWO_IMAGES_OPERATION( r32, r32 );
      break;
    }
    break;
  }

  if ( typeAux != typeOut ) {
    if ( ConvertBuffer( auxBuf, typeAux, bufferOut, typeOut, v ) != 1 ) {
      free( auxBuf );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      return( -1 );
    }
    free( auxBuf );
  }
  
  return( 1 );
}






/**************************************************
 *
 * static procedures: image and double
 *
 **************************************************/









static int _ArithmeticImageDouble( void *bufferIn, bufferType typeIn,
                                   double scalar,
                                   void *bufferOut, bufferType typeOut,
                                   int *bufferDims,
                                   enumImageScalarOperation operation )
{
  char *proc = "_ArithmeticImageDouble";
  bufferType typeAux = TYPE_UNKNOWN;
  void *auxBuf = (void*)NULL;
  size_t i, v;

  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];


  /* auxiliary buffer for computation
   */
  switch ( typeIn ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
  case UINT :
  case SINT :
  case FLOAT :
    typeAux = (typeOut == DOUBLE) ? DOUBLE : FLOAT;
    break;
  case ULINT :
  case DOUBLE :
    typeAux = (typeOut == FLOAT) ? FLOAT : DOUBLE;
  }

  if ( typeAux != typeOut ) {
    switch ( typeAux ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      auxBuf = (void*)malloc( v*sizeof( r32 ) );
      break;
    case DOUBLE :
      auxBuf = (void*)malloc( v*sizeof( r64 ) );
      break;
      break;
    }
    if ( auxBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
  }
  else {
    auxBuf = (void*)bufferOut;
  }


#define _IMAGE_DOUBLE_AUXILIARY( TYPEAUX ) {       \
      TYPEAUX *resBuf = (TYPEAUX*)auxBuf;          \
      switch ( operation ) {                       \
      default :                                    \
        if ( typeAux != typeOut ) free( auxBuf );  \
        if ( _verbose_ )                           \
          fprintf( stderr, "%s: such operation not handled yet\n", proc ); \
        return( -1 );                              \
      case _MIN_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = ((TYPEAUX)theBuf[i] < scalar) ? (TYPEAUX)theBuf[i] : scalar; \
        break;                                     \
      case _MAX_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = ((TYPEAUX)theBuf[i] > scalar) ? (TYPEAUX)theBuf[i] : scalar; \
        break;                                     \
      case _ADD_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = (TYPEAUX)theBuf[i] + scalar; \
        break;                                     \
      case _SUB_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = (TYPEAUX)theBuf[i] - scalar; \
        break;                                     \
      case _MUL_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = (TYPEAUX)theBuf[i] * scalar; \
        break;                                     \
      case _DIV_SCALAR_ :                          \
        for ( i=0; i<v; i++ )                      \
          resBuf[i] = (TYPEAUX)theBuf[i] / scalar; \
        break;                                     \
      }                                            \
}

#define _IMAGE_DOUBLE_OPERATION( TYPE ) {        \
  TYPE *theBuf = (TYPE*)bufferIn;             \
  switch ( typeAux ) {                        \
  default :                                   \
    if ( typeAux != typeOut ) free( auxBuf ); \
    if ( _verbose_ )                          \
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc ); \
    return( -1 );                             \
  case FLOAT :                                \
    _IMAGE_DOUBLE_AUXILIARY( r32 );              \
    break;                                    \
  case DOUBLE :                               \
    _IMAGE_DOUBLE_AUXILIARY( r64 );              \
    break;                                    \
  }                                           \
}


  switch ( typeIn ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _IMAGE_DOUBLE_OPERATION( u8 );
    break;
  case SCHAR :
    _IMAGE_DOUBLE_OPERATION( s8 );
    break;
  case USHORT :
    _IMAGE_DOUBLE_OPERATION( u16 );
    break;
  case SSHORT :
    _IMAGE_DOUBLE_OPERATION( s16 );
    break;
  case FLOAT :
    _IMAGE_DOUBLE_OPERATION( r32 );
    break;
  }
  
  if ( typeAux != typeOut ) {
    if ( ConvertBuffer( auxBuf, typeAux, bufferOut, typeOut, v ) != 1 ) {
      free( auxBuf );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      return( -1 );
    }
    free( auxBuf );
  }

  return( 1 );
}





/**************************************************
 *
 * static procedures: one image
 *
 **************************************************/



static int _OperationImage( void *bufferIn, bufferType typeIn,
                            void *bufferOut, bufferType typeOut,
                            int *bufferDims,
                            enumOneImageOperation operation )
{
  char *proc = "_OperationImage";
  bufferType typeAux = TYPE_UNKNOWN;
  void *auxBuf = (void*)NULL;
  size_t i, v;

  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if (_verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
  v = (size_t)bufferDims[0] * (size_t)bufferDims[1] * (size_t)bufferDims[2];


  /* auxiliary buffer for computation
   */
  switch ( typeIn ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
  case UINT :
  case SINT :
  case FLOAT :
    typeAux = (typeOut == DOUBLE) ? DOUBLE : FLOAT;
    break;
  case ULINT :
  case DOUBLE :
    typeAux = (typeOut == FLOAT) ? FLOAT : DOUBLE;
  }

  if ( typeAux != typeOut ) {
    switch ( typeAux ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc );
      return( -1 );
    case FLOAT :
      auxBuf = (void*)malloc( v*sizeof( r32 ) );
      break;
    case DOUBLE :
      auxBuf = (void*)malloc( v*sizeof( r64 ) );
      break;
      break;
    }
    if ( auxBuf == (void*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
  }
  else {
    auxBuf = (void*)bufferOut;
  }


#define _ONE_IMAGE_AUXILIARY( TYPEAUX ) {         \
      TYPEAUX *resBuf = (TYPEAUX*)auxBuf;         \
      switch ( operation ) {                      \
      default :                                   \
        if ( typeAux != typeOut ) free( auxBuf ); \
        if ( _verbose_ )                          \
          fprintf( stderr, "%s: such operation not handled yet\n", proc ); \
        return( -1 );                             \
      case _SQR_ :                                \
        for ( i=0; i<v; i++ )                     \
          resBuf[i] = (TYPEAUX)theBuf[i] * (TYPEAUX)theBuf[i]; \
        break;                                    \
      }                                           \
}

#define _ONE_IMAGE_OPERATION( TYPE ) {        \
  TYPE *theBuf = (TYPE*)bufferIn;             \
  switch ( typeAux ) {                        \
  default :                                   \
    if ( typeAux != typeOut ) free( auxBuf ); \
    if ( _verbose_ )                          \
        fprintf( stderr, "%s: such auxiliary image type not handled yet\n", proc ); \
    return( -1 );                             \
  case FLOAT :                                \
    _ONE_IMAGE_AUXILIARY( r32 );              \
    break;                                    \
  case DOUBLE :                               \
    _ONE_IMAGE_AUXILIARY( r64 );              \
    break;                                    \
  }                                           \
}


  switch ( typeIn ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _ONE_IMAGE_OPERATION( u8 );
    break;
  case SCHAR :
    _ONE_IMAGE_OPERATION( s8 );
    break;
  case USHORT :
    _ONE_IMAGE_OPERATION( u16 );
    break;
  case SSHORT :
    _ONE_IMAGE_OPERATION( s16 );
    break;
  case FLOAT :
    _ONE_IMAGE_OPERATION( r32 );
    break;
  }
  
  if ( typeAux != typeOut ) {
    if ( ConvertBuffer( auxBuf, typeAux, bufferOut, typeOut, v ) != 1 ) {
      free( auxBuf );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      return( -1 );
    }
    free( auxBuf );
  }

  return( 1 );
}





/**************************************************
 *
 * operation on two images
 *
 **************************************************/


int minImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  if ( typeIn1 == typeIn2 && typeIn1 == typeOut ) 
    return( _minTwoImages( bufferIn1, typeIn1,
                        bufferIn2, typeIn2,
                        bufferOut, typeOut,
                        bufferDims ) );
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _MINIMUM_ ) );
}



int maxImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  if ( typeIn1 == typeIn2 && typeIn1 == typeOut ) 
    return( _maxTwoImages( bufferIn1, typeIn1,
                        bufferIn2, typeIn2,
                        bufferOut, typeOut,
                        bufferDims ) );
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _MAXIMUM_ ) );
}



int addImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _ADD_IMAGE_ ) );
}



int subtractImages( void *bufferIn1, bufferType typeIn1,
               void *bufferIn2, bufferType typeIn2,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _SUB_IMAGE_ ) );
}



int multiplyImages( void *bufferIn1, bufferType typeIn1,
                    void *bufferIn2, bufferType typeIn2,
                    void *bufferOut, bufferType typeOut,
                    int *bufferDims )
{
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _MUL_IMAGE_ ) );
}



int divideImages( void *bufferIn1, bufferType typeIn1,
                  void *bufferIn2, bufferType typeIn2,
                  void *bufferOut, bufferType typeOut,
                  int *bufferDims )
{
  return( _ArithmeticTwoImages( bufferIn1, typeIn1,
                             bufferIn2, typeIn2,
                             bufferOut, typeOut,
                             bufferDims, _DIV_IMAGE_ ) );
}





/**************************************************
 *
 * operation on one image and one scalar
 *
 **************************************************/



int minImageDouble( void *bufferIn1, bufferType typeIn1,
               double scalar,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _MIN_SCALAR_ ) );
}



int maxImageDouble( void *bufferIn1, bufferType typeIn1,
               double scalar,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _MAX_SCALAR_ ) );
}



int addImageDouble( void *bufferIn1, bufferType typeIn1,
               double scalar,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _ADD_SCALAR_ ) );
}



int subtractImageDouble( void *bufferIn1, bufferType typeIn1,
               double scalar,
               void *bufferOut, bufferType typeOut,
               int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _SUB_SCALAR_ ) );
}



int multiplyImageDouble( void *bufferIn1, bufferType typeIn1,
                    double scalar,
                    void *bufferOut, bufferType typeOut,
                    int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _MUL_SCALAR_ ) );
}



int divideImageDouble( void *bufferIn1, bufferType typeIn1,
                  double scalar,
                  void *bufferOut, bufferType typeOut,
                  int *bufferDims )
{
  return( _ArithmeticImageDouble( bufferIn1, typeIn1,
                             scalar,
                             bufferOut, typeOut,
                             bufferDims, _DIV_SCALAR_ ) );
}





/**************************************************
 *
 * operation on one image
 *
 **************************************************/

int sqrImage( void *bufferIn, bufferType typeIn,
              void *bufferOut, bufferType typeOut,
              int *bufferDims )
{
  return( _OperationImage( bufferIn, typeIn,
                            bufferOut, typeOut,
                            bufferDims, _SQR_ ) );
}
