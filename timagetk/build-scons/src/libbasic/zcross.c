/*************************************************************************
 * zcross.c - zero-crossings
 *
 * $Id: zcross.c,v 1.2 2003/06/25 07:48:54 greg Exp $
 *
 * Copyright (c) INRIA 2000-2013
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Tue Nov 28 10:00:36 MET 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 * - Wed Jan  2 10:50:44 CET 2013
 *   'contours' procedure are put into an other file
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include <zcross.h>


static int _verbose_ = 1;
/*
static int _debug_ = 0;
*/


void ZeroCrossings_verbose ( )
{
  _verbose_ = 1;
}

void ZeroCrossings_noverbose ( )
{
  _verbose_ = 0;
}





/*
 * `sign' of the zero-crossings
 * if > 0, zero-crossings are chosen in the `positive' region
 *         ie are points M such that I(M) > 0 et I(M+v) <= 0
 * if < 0 zero-crossings are chosen in the `negative' region
 *         ie are points M such that I(M) < 0 et I(M+v) >= 0
 */

static int signZeroCrossing = 1;

void ZeroCrossings_Are_Positive()
{
  signZeroCrossing = 1;
}

void ZeroCrossings_Are_Negative()
{
  signZeroCrossing = -1;
}





/************************************************************
 *
 * 
 *
 ************************************************************/



int positiveZeroCrossing2D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims )
{
  /* les passage par zeros sont definis par
     I(M) > 0 et I(M+v) <= 0
     on marque M

     Dans un contexte de detection de contours (ex laplacien)
     cela marque dans les zones sombres (par rapport a clair)
  */
     
  char *proc="positiveZeroCrossing2D";
  size_t x, y, z, iz, iy;
  size_t dx  = bufferDims[0];
  size_t dxy = dx * bufferDims[1];
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }

  if ( bufferIn == bufferOut ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: input buffer should not be equal to output.\n", proc );
    return( -1 );
  }



  /*
   * points to be marked
   * - (x,y) when (x,y) > 0 and (x+1,y) <= 0
   * - (x,y) when (x,y) > 0 and (x,y+1) <= 0
   * - (x+1,y) when (x+1,y) > 0 and (x,y) <= 0
   * - (x,y+1) when (x,y+1) > 0 and (x,y) <= 0
   *
   * // the (bufferDims[1]-1) first rows
   * for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *   // the (dx-1) first points of the row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y)   if (x,y) > 0  && ((x+1,y) <= 0 || (x,y+1) <= 0)
   *     // we mark (x+1,y) if (x,y) <= 0 &&  (x+1,y) > 0
   *     // we mark (x,y+1) if (x,y) <= 0 &&  (x,y+1) > 0
   *
   *   // the last point of the row
   *   // we mark (x,y)   if (x,y) > 0  && (x,y+1) <= 0
   *   // we mark (x,y+1) if (x,y) <= 0 && (x,y+1) > 0
   *
   * // the last row
   * for ( x=0; x<dx-1; x++ )
   *   // we mark (x,y)   if (x,y) > 0  && (x+1,y) <= 0
   *   // we mark (x+1,y) if (x,y) <= 0 && (x+1,y) > 0
   *
   */
  
#define _POSITIVE_ZERO_CROSSINGS_2D_( TYPE, VAL ) {             \
  TYPE *resBuf = (TYPE*)bufferOut;                              \
  iz = bufferDims[2]*dxy;                                       \
  for ( x=0; x<iz; x++ ) resBuf[x] = 0;                         \
  for ( z=0, iz=0; z<(size_t)bufferDims[2]; z++, iz+=dxy ) {    \
    for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) { \
      for ( x=0; x<dx-1; x++ ) {                                \
        if ( theBuf[iz+iy+x] > 0 ) {                            \
          if ( theBuf[iz+iy+x+1] <= 0 || theBuf[iz+iy+x+dx] <= 0 ) resBuf[iz+iy+x] = VAL; \
        } else {                                                \
          if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL; \
          if ( theBuf[iz+iy+x+dx] > 0 ) resBuf[iz+iy+x+dx] = VAL; \
        }                                                       \
      }                                                         \
      if ( theBuf[iz+iy+x] > 0 ) {                              \
        if ( theBuf[iz+iy+x+dx] <= 0 ) resBuf[iz+iy+x] = VAL;   \
      } else {                                                  \
        if ( theBuf[iz+iy+x+dx] > 0 )  resBuf[iz+iy+x+dx] = VAL; \
      }                                                         \
    }                                                           \
    for ( x=0; x<dx-1; x++ ) {                                  \
      if ( theBuf[iz+iy+x] > 0 ) {                              \
        if ( theBuf[iz+iy+x+1] <= 0 ) resBuf[iz+iy+x] = VAL;    \
      } else {                                                  \
        if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL;  \
      }                                                         \
    }                                                           \
  }                                                             \
}

  switch( typeIn ) {
  case FLOAT :
    {
      r32 *theBuf = (r32*)bufferIn;
    
      switch( typeOut ) {

      case UCHAR :
        _POSITIVE_ZERO_CROSSINGS_2D_( u8, 255 )
        break;

      case SCHAR :
        _POSITIVE_ZERO_CROSSINGS_2D_( s8, 127 )
        break;

      case USHORT :
        _POSITIVE_ZERO_CROSSINGS_2D_( u16, 65535 )
        break;

      case SSHORT :
        _POSITIVE_ZERO_CROSSINGS_2D_( s16, 32767 )
        break;

      case FLOAT :
        _POSITIVE_ZERO_CROSSINGS_2D_( r32, 1.0 )
        break;

      default :
        if ( _verbose_ > 0 )
          fprintf( stderr, "%s: such output type not handled yet\n", proc );
        return( -1 );
      }

    } /* typeIn == FLOAT */
    break;

  default :
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: such input type not handled yet\n", proc );
    return( -1 );
  }
  return( 1 );
}





int negativeZeroCrossing2D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims )
{
  /* les passage par zeros sont definis par
     I(M) < 0 et I(M+v) >= 0
     on marque M

     Dans un contexte de detection de contours (ex laplacien)
     cela marque dans les zones sombres (par rapport a clair)
  */
     
  char *proc="negativeZeroCrossing2D";
  size_t x, y, z, iz, iy;
  size_t dx  = bufferDims[0];
  size_t dxy = dx * bufferDims[1];

  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }

  if ( bufferIn == bufferOut ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: input buffer should not be equal to output.\n", proc );
    return( -1 );
  }


  
  /*
   * points to be marked
   * - (x,y) when (x,y) < 0 and (x+1,y) >= 0
   * - (x,y) when (x,y) < 0 and (x,y+1) >= 0
   * - (x+1,y) when (x+1,y) < 0 and (x,y) >= 0
   * - (x,y+1) when (x,y+1) < 0 and (x,y) >= 0
   *
   * // the (bufferDims[1]-1) first rows
   * for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *   // the (dx-1) first points of the row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y)   if (x,y) < 0  && ((x+1,y) >= 0 || (x,y+1) >= 0)
   *     // we mark (x+1,y) if (x,y) >= 0 &&  (x+1,y) < 0
   *     // we mark (x,y+1) if (x,y) >= 0 &&  (x,y+1) < 0
   *
   *   // the last point of the row
   *   // we mark (x,y)   if (x,y) < 0  && (x,y+1) >= 0
   *   // we mark (x,y+1) if (x,y) >= 0 && (x,y+1) < 0
   *
   * // the last row
   * for ( x=0; x<dx-1; x++ )
   *   // we mark (x,y)   if (x,y) < 0  && (x+1,y) >= 0
   *   // we mark (x+1,y) if (x,y) >= 0 && (x+1,y) < 0
   *
   */
  
#define _NEGATIVE_ZERO_CROSSINGS_2D_( TYPE, VAL ) {             \
  TYPE *resBuf = (TYPE*)bufferOut;                              \
  iz = bufferDims[2]*dxy;                                       \
  for ( x=0; x<iz; x++ ) resBuf[x] = 0;                         \
  for ( z=0, iz=0; z<(size_t)bufferDims[2]; z++, iz+=dxy ) {    \
    for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) { \
      for ( x=0; x<dx-1; x++ ) {                                \
        if ( theBuf[iz+iy+x] < 0 ) {                            \
          if ( theBuf[iz+iy+x+1] >= 0 || theBuf[iz+iy+x+dx] >= 0 ) resBuf[iz+iy+x] = VAL; \
        } else {                                                \
          if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL; \
          if ( theBuf[iz+iy+x+dx] < 0 ) resBuf[iz+iy+x+dx] = VAL; \
        }                                                       \
      }                                                         \
      if ( theBuf[iz+iy+x] < 0 ) {                              \
        if ( theBuf[iz+iy+x+dx] >= 0 ) resBuf[iz+iy+x] = VAL;   \
      } else {                                                  \
        if ( theBuf[iz+iy+x+dx] < 0 )  resBuf[iz+iy+x+dx] = VAL; \
      }                                                         \
    }                                                           \
    for ( x=0; x<dx-1; x++ ) {                                  \
      if ( theBuf[iz+iy+x] < 0 ) {                              \
        if ( theBuf[iz+iy+x+1] >= 0 ) resBuf[iz+iy+x] = VAL;    \
      } else {                                                  \
        if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL;  \
      }                                                         \
    }                                                           \
  }                                                             \
}


  switch( typeIn ) {
  case FLOAT :
    {
      r32 *theBuf = (r32*)bufferIn;
    
      switch( typeOut ) {

      case UCHAR :
        _NEGATIVE_ZERO_CROSSINGS_2D_( u8, 255 )
        break;

      case SCHAR :
        _NEGATIVE_ZERO_CROSSINGS_2D_( s8, 127 )
        break;

      case USHORT :
        _NEGATIVE_ZERO_CROSSINGS_2D_( u16, 65535 )
        break;

      case SSHORT :
        _NEGATIVE_ZERO_CROSSINGS_2D_( s16, 32767 )
        break;

      case FLOAT :
        _NEGATIVE_ZERO_CROSSINGS_2D_( r32, 1.0 )
        break;

      default :
        if ( _verbose_ > 0 )
          fprintf( stderr, " Error in %s: such output type not handled.\n", proc );
        return( -1 );
      }
      
    }
    break;
    
  default :
    if ( _verbose_ > 0 )
      fprintf( stderr, " Error in %s: such input type not handled.\n", proc );
    return( -1 );
  }
  return( 1 );
}
 




int positiveZeroCrossing3D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims )
{
  /* les passage par zeros sont definis par
     I(M) > 0 et I(M+v) <= 0
     on marque M

     Dans un contexte de detection de contours (ex laplacien)
     cela marque dans les zones sombres (par rapport a clair)
  */
     
  char *proc="positiveZeroCrossing3D";
  size_t x, y, z, iz, iy;
  size_t dx  = bufferDims[0];
  size_t dxy = dx * bufferDims[1];
  
  if ( bufferDims[2] == 1 ) 
    return( positiveZeroCrossing2D( bufferIn, typeIn, bufferOut,
                                    typeOut, bufferDims ) );
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }

  if ( bufferIn == bufferOut ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: input buffer should not be equal to output.\n", proc );
    return( -1 );
  }



  /*
   * points to be marked
   * - (x,y,z) when (x,y,z) > 0 and (x+1,y,z) <= 0
   * - (x,y,z) when (x,y,z) > 0 and (x,y+1,z) <= 0
   * - (x,y,z) when (x,y,z) > 0 and (x,y,z+1) <= 0
   * - (x+1,y,z) when (x+1,y,z) > 0 and (x,y,z) <= 0
   * - (x,y+1,z) when (x,y+1,z) > 0 and (x,y,z) <= 0
   * - (x,y,z+1) when (x,y,z+1) > 0 and (x,y,z) <= 0
   *
   * // the (bufferDims[2]-1) first planes
   * for ( z=0, iz=0; z<bufferDims[2]; z++, iz+=dxy ) {
   *
   *   // the (bufferDims[1]-1) first rows
   *   for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *     // the (dx-1) first points of the row
   *     for ( x=0; x<dx-1; x++ )
   *       // we mark (x,y,z)   if (x,y,z) > 0  && ((x+1,y,z) <= 0 
   *       //                                    || (x,y+1,z) <= 0 
   *       //                                    || (x,y,z+1) <= 0)
   *       // we mark (x+1,y,z) if (x,y,z) <= 0 && (x+1,y,z) > 0
   *       // we mark (x,y+1,z) if (x,y,z) <= 0 && (x,y+1,z) > 0
   *       // we mark (x,y,z+1) if (x,y,z) <= 0 && (x,y,z+1) > 0
   *
   *     // the last point of the row
   *     // we mark (x,y,z)   if (x,y,z) > 0  && ((x,y+1,z) <= 0
   *     //                                    || (x,y,z+1) <= 0)
   *     // we mark (x,y+1,z) if (x,y,z) <= 0 && (x,y+1,z) > 0
   *     // we mark (x,y,z+1) if (x,y,z) <= 0 && (x,y,z+1) > 0
   *
   *   // the last row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y,z)   if (x,y,z) > 0  && ((x+1,y,z) <= 0 
   *     //                                    || (x,y,z+1) <= 0)
   *     // we mark (x+1,y,z) if (x,y,z) <= 0 && (x+1,y,z) > 0
   *     // we mark (x,y,z+1) if (x,y,z) <= 0 && (x,y,z+1) > 0
   *
   * // the last plane
   *
   * // the (bufferDims[1]-1) first rows
   * for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *   // the (dx-1) first points of the row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y,z)   if (x,y,z) > 0  && ((x+1,y,z) <= 0 
   *     //                                    || (x,y+1,z) <= 0) 
   *     // we mark (x+1,y,z) if (x,y,z) <= 0 && (x+1,y,z) > 0
   *     // we mark (x,y+1,z) if (x,y,z) <= 0 && (x,y+1,z) > 0
   *
   *   // the last point of the row
   *   // we mark (x,y,z)   if (x,y,z) > 0  && ((x,y+1,z) <= 0)
   *   // we mark (x,y+1,z) if (x,y,z) <= 0 && (x,y+1,z) > 0
   *
   * // the last row
   * for ( x=0; x<dx-1; x++ )
   *   // we mark (x,y,z)   if (x,y,z) > 0  && ((x+1,y,z) <= 0)
   *   // we mark (x+1,y,z) if (x,y,z) <= 0 && (x+1,y,z) > 0
   *
   */
  
#define _POSITIVE_ZERO_CROSSINGS_3D_( TYPE, VAL ) {                 \
  TYPE *resBuf = (TYPE*)bufferOut;                                  \
  iz = bufferDims[2]*dxy;                                           \
  for ( x=0; x<iz; x++ ) resBuf[x] = 0;                             \
  for ( z=0, iz=0; z<(size_t)(bufferDims[2]-1); z++, iz+=dxy ) {    \
    for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) {   \
      for ( x=0; x<dx-1; x++ ) {                                    \
        if ( theBuf[iz+iy+x] > 0 ) {                                \
          if ( theBuf[iz+iy+x+1] <= 0 || theBuf[iz+iy+x+dx] <= 0    \
               || theBuf[iz+iy+x+dxy] <= 0 ) resBuf[iz+iy+x] = VAL; \
        }                                                           \
        else {                                                      \
          if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL;    \
          if ( theBuf[iz+iy+x+dx] > 0 ) resBuf[iz+iy+x+dx] = VAL;   \
          if ( theBuf[iz+iy+x+dxy] > 0 ) resBuf[iz+iy+x+dxy] = VAL; \
        }                                                           \
      }                                                             \
      if ( theBuf[iz+iy+x] > 0 ) {                                  \
        if ( theBuf[iz+iy+x+dx] <= 0 || theBuf[iz+iy+x+dxy] <= 0 )  \
          resBuf[iz+iy+x] = VAL;                                    \
      } else {                                                      \
        if ( theBuf[iz+iy+x+dx] > 0 )  resBuf[iz+iy+x+dx] = VAL;    \
        if ( theBuf[iz+iy+x+dxy] > 0 ) resBuf[iz+iy+x+dxy] = VAL;   \
      }                                                             \
    }                                                               \
    for ( x=0; x<dx-1; x++ ) {                                      \
      if ( theBuf[iz+iy+x] > 0 ) {                                  \
        if ( theBuf[iz+iy+x+1] <= 0 || theBuf[iz+iy+x+dxy] <= 0 )   \
          resBuf[iz+iy+x] = VAL;                                    \
      } else {                                                      \
        if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL;      \
        if ( theBuf[iz+iy+x+dxy] > 0 ) resBuf[iz+iy+x+dxy] = VAL;   \
      }                                                             \
    }                                                               \
  }                                                                 \
  for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) {     \
    for ( x=0; x<dx-1; x++ ) {                                      \
      if ( theBuf[iz+iy+x] > 0 ) {                                  \
        if ( theBuf[iz+iy+x+1] <= 0 || theBuf[iz+iy+x+dx] <= 0 )    \
          resBuf[iz+iy+x] = VAL;                                    \
      }                                                             \
      else {                                                        \
        if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL;      \
        if ( theBuf[iz+iy+x+dx] > 0 ) resBuf[iz+iy+x+dx] = VAL;     \
      }                                                             \
    }                                                               \
    if ( theBuf[iz+iy+x] > 0 ) {                                    \
      if ( theBuf[iz+iy+x+dx] <= 0 ) resBuf[iz+iy+x] = VAL;              \
    } else {                                                        \
      if ( theBuf[iz+iy+x+dx] > 0 )  resBuf[iz+iy+x+dx] = VAL;      \
    }                                                               \
  }                                                                 \
  for ( x=0; x<dx-1; x++ ) {                                        \
    if ( theBuf[iz+iy+x] > 0 ) {                                    \
      if ( theBuf[iz+iy+x+1] <= 0 ) resBuf[iz+iy+x] = VAL;          \
    } else {                                                        \
      if ( theBuf[iz+iy+x+1] > 0 )  resBuf[iz+iy+x+1] = VAL;        \
    }                                                               \
  }                                                                 \
}
  
  switch( typeIn ) {
  case FLOAT :
    {
      r32 *theBuf = (r32*)bufferIn;
    
      switch( typeOut ) {

      case UCHAR :
        _POSITIVE_ZERO_CROSSINGS_3D_( u8, 255 )
        break;

      case SCHAR :
        _POSITIVE_ZERO_CROSSINGS_3D_( s8, 127 )
        break;

      case USHORT :
        _POSITIVE_ZERO_CROSSINGS_3D_( u16, 65535 )
        break;

      case SSHORT :
        _POSITIVE_ZERO_CROSSINGS_3D_( s16, 32767 )
        break;

      case FLOAT :
        _POSITIVE_ZERO_CROSSINGS_3D_( r32, 1.0 )
        break;

      default :
        if ( _verbose_ > 0 )
          fprintf( stderr, "%s: such output type not handled yet\n", proc );
        return( -1 );
      }

    } /* typeIn == FLOAT */
    break;

  default :
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: such input type not handled yet\n", proc );
    return( -1 );
  }
  return( 1 );
}





int negativeZeroCrossing3D( void *bufferIn,
                            bufferType typeIn,
                            void *bufferOut,
                            bufferType typeOut,
                            int *bufferDims )
{
  /* les passage par zeros sont definis par
     I(M) > 0 et I(M+v) <= 0
     on marque M

     Dans un contexte de detection de contours (ex laplacien)
     cela marque dans les zones sombres (par rapport a clair)
  */
     
  char *proc="negativeZeroCrossing3D";
  size_t x, y, z, iz, iy;
  size_t dx  = bufferDims[0];
  size_t dxy = dx * bufferDims[1];
  
  if ( bufferDims[2] == 1 ) 
    return( negativeZeroCrossing2D( bufferIn, typeIn, bufferOut,
                                    typeOut, bufferDims ) );
  
  /* 
   * We check the buffers' dimensions.
   */
  if ( (bufferDims[0] <= 0) || (bufferDims[1] <= 0) || (bufferDims[2] <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: improper buffer's dimension.\n", proc );
    return( -1 );
  }

  if ( bufferIn == bufferOut ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: input buffer should not be equal to output.\n", proc );
    return( -1 );
  }



  /*
   * points to be marked
   * - (x,y,z) when (x,y,z) < 0 and (x+1,y,z) >= 0
   * - (x,y,z) when (x,y,z) < 0 and (x,y+1,z) >= 0
   * - (x,y,z) when (x,y,z) < 0 and (x,y,z+1) >= 0
   * - (x+1,y,z) when (x+1,y,z) < 0 and (x,y,z) >= 0
   * - (x,y+1,z) when (x,y+1,z) < 0 and (x,y,z) >= 0
   * - (x,y,z+1) when (x,y,z+1) < 0 and (x,y,z) >= 0
   *
   * // the (bufferDims[2]-1) first planes
   * for ( z=0, iz=0; z<bufferDims[2]; z++, iz+=dxy ) {
   *
   *   // the (bufferDims[1]-1) first rows
   *   for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *     // the (dx-1) first points of the row
   *     for ( x=0; x<dx-1; x++ )
   *       // we mark (x,y,z)   if (x,y,z) < 0  && ((x+1,y,z) >= 0 
   *       //                                    || (x,y+1,z) >= 0 
   *       //                                    || (x,y,z+1) >= 0)
   *       // we mark (x+1,y,z) if (x,y,z) >= 0 && (x+1,y,z) < 0
   *       // we mark (x,y+1,z) if (x,y,z) >= 0 && (x,y+1,z) < 0
   *       // we mark (x,y,z+1) if (x,y,z) >= 0 && (x,y,z+1) < 0
   *
   *     // the last point of the row
   *     // we mark (x,y,z)   if (x,y,z) < 0  && ((x,y+1,z) >= 0
   *     //                                    || (x,y,z+1) >= 0)
   *     // we mark (x,y+1,z) if (x,y,z) >= 0 && (x,y+1,z) < 0
   *     // we mark (x,y,z+1) if (x,y,z) >= 0 && (x,y,z+1) < 0
   *
   *   // the last row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y,z)   if (x,y,z) < 0  && ((x+1,y,z) >= 0 
   *     //                                    || (x,y,z+1) >= 0)
   *     // we mark (x+1,y,z) if (x,y,z) >= 0 && (x+1,y,z) < 0
   *     // we mark (x,y,z+1) if (x,y,z) >= 0 && (x,y,z+1) < 0
   *
   * // the last plane
   *
   * // the (bufferDims[1]-1) first rows
   * for ( y=0, iy=0; y<bufferDims[1]-1; y++, iy+=dx )
   *
   *   // the (dx-1) first points of the row
   *   for ( x=0; x<dx-1; x++ )
   *     // we mark (x,y,z)   if (x,y,z) < 0  && ((x+1,y,z) >= 0 
   *     //                                    || (x,y+1,z) >= 0) 
   *     // we mark (x+1,y,z) if (x,y,z) >= 0 && (x+1,y,z) < 0
   *     // we mark (x,y+1,z) if (x,y,z) >= 0 && (x,y+1,z) < 0
   *
   *   // the last point of the row
   *   // we mark (x,y,z)   if (x,y,z) < 0  && ((x,y+1,z) >= 0)
   *   // we mark (x,y+1,z) if (x,y,z) >= 0 && (x,y+1,z) < 0
   *
   * // the last row
   * for ( x=0; x<dx-1; x++ )
   *   // we mark (x,y,z)   if (x,y,z) < 0  && ((x+1,y,z) >= 0)
   *   // we mark (x+1,y,z) if (x,y,z) >= 0 && (x+1,y,z) < 0
   *
   */
  
#define _NEGATIVE_ZERO_CROSSINGS_3D_( TYPE, VAL ) {                 \
  TYPE *resBuf = (TYPE*)bufferOut;                                  \
  iz = bufferDims[2]*dxy;                                           \
  for ( x=0; x<iz; x++ ) resBuf[x] = 0;                             \
  for ( z=0, iz=0; z<(size_t)(bufferDims[2]-1); z++, iz+=dxy ) {    \
    for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) {   \
      for ( x=0; x<dx-1; x++ ) {                                    \
        if ( theBuf[iz+iy+x] < 0 ) {                                \
          if ( theBuf[iz+iy+x+1] >= 0 || theBuf[iz+iy+x+dx] >= 0    \
               || theBuf[iz+iy+x+dxy] >= 0 ) resBuf[iz+iy+x] = VAL; \
        }                                                           \
        else {                                                      \
          if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL;    \
          if ( theBuf[iz+iy+x+dx] < 0 ) resBuf[iz+iy+x+dx] = VAL;   \
          if ( theBuf[iz+iy+x+dxy] < 0 ) resBuf[iz+iy+x+dxy] = VAL; \
        }                                                           \
      }                                                             \
      if ( theBuf[iz+iy+x] < 0 ) {                                  \
        if ( theBuf[iz+iy+x+dx] >= 0 || theBuf[iz+iy+x+dxy] >= 0 )  \
          resBuf[iz+iy+x] = VAL;                                    \
      } else {                                                      \
        if ( theBuf[iz+iy+x+dx] < 0 )  resBuf[iz+iy+x+dx] = VAL;    \
        if ( theBuf[iz+iy+x+dxy] < 0 ) resBuf[iz+iy+x+dxy] = VAL;   \
      }                                                             \
    }                                                               \
    for ( x=0; x<dx-1; x++ ) {                                      \
      if ( theBuf[iz+iy+x] < 0 ) {                                  \
        if ( theBuf[iz+iy+x+1] >= 0 || theBuf[iz+iy+x+dxy] >= 0 )   \
          resBuf[iz+iy+x] = VAL;                                    \
      } else {                                                      \
        if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL;      \
        if ( theBuf[iz+iy+x+dxy] < 0 ) resBuf[iz+iy+x+dxy] = VAL;   \
      }                                                             \
    }                                                               \
  }                                                                 \
  for ( y=0, iy=0; y<(size_t)(bufferDims[1]-1); y++, iy+=dx ) {     \
    for ( x=0; x<dx-1; x++ ) {                                      \
      if ( theBuf[iz+iy+x] < 0 ) {                                  \
        if ( theBuf[iz+iy+x+1] >= 0 || theBuf[iz+iy+x+dx] >= 0 )    \
          resBuf[iz+iy+x] = VAL;                                    \
      }                                                             \
      else {                                                        \
        if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL;      \
        if ( theBuf[iz+iy+x+dx] < 0 ) resBuf[iz+iy+x+dx] = VAL;     \
      }                                                             \
    }                                                               \
    if ( theBuf[iz+iy+x] < 0 ) {                                    \
      if ( theBuf[iz+iy+x+dx] >= 0 ) resBuf[iz+iy+x] = VAL;         \
    } else {                                                        \
      if ( theBuf[iz+iy+x+dx] < 0 )  resBuf[iz+iy+x+dx] = VAL;      \
    }                                                               \
  }                                                                 \
  for ( x=0; x<dx-1; x++ ) {                                        \
    if ( theBuf[iz+iy+x] < 0 ) {                                    \
      if ( theBuf[iz+iy+x+1] >= 0 ) resBuf[iz+iy+x] = VAL;          \
    } else {                                                        \
      if ( theBuf[iz+iy+x+1] < 0 )  resBuf[iz+iy+x+1] = VAL;        \
    }                                                               \
  }                                                                 \
}
  
  switch( typeIn ) {
  case FLOAT :
    {
      r32 *theBuf = (r32*)bufferIn;
    
      switch( typeOut ) {

      case UCHAR :
        _NEGATIVE_ZERO_CROSSINGS_3D_( u8, 255 )
        break;

      case SCHAR :
        _NEGATIVE_ZERO_CROSSINGS_3D_( s8, 127 )
        break;

      case USHORT :
        _NEGATIVE_ZERO_CROSSINGS_3D_( u16, 65535 )
        break;

      case SSHORT :
        _NEGATIVE_ZERO_CROSSINGS_3D_( s16, 32767 )
        break;

      case FLOAT :
        _NEGATIVE_ZERO_CROSSINGS_3D_( r32, 1.0 )
        break;

      default :
        if ( _verbose_ > 0 )
          fprintf( stderr, "%s: such output type not handled yet\n", proc );
        return( -1 );
      }

    } /* typeIn == FLOAT */
    break;

  default :
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: such input type not handled yet\n", proc );
    return( -1 );
  }
  return( 1 );
}





/************************************************************
 *
 * main procedures
 *
 ************************************************************/



int zeroCrossing2D( void *bufferIn, bufferType typeIn,
                    void *bufferOut, bufferType typeOut, int *bufferDims )
{
  if ( signZeroCrossing > 0 ) 
    return( positiveZeroCrossing2D( bufferIn, typeIn, bufferOut,
                                    typeOut, bufferDims ) );
  return( negativeZeroCrossing2D( bufferIn, typeIn, bufferOut,
                                  typeOut, bufferDims ) );
}





int zeroCrossing3D( void *bufferIn, bufferType typeIn,
                    void *bufferOut, bufferType typeOut, int *bufferDims )
{
  if ( bufferDims[2] == 1 ) 
    return( zeroCrossing2D( bufferIn, typeIn, bufferOut,
                            typeOut, bufferDims ) );

  if ( signZeroCrossing > 0 ) 
    return( positiveZeroCrossing3D( bufferIn, typeIn, bufferOut,
                                    typeOut, bufferDims ) );
  return( negativeZeroCrossing3D( bufferIn, typeIn, bufferOut,
                                  typeOut, bufferDims ) );
}





int zeroCrossing( void *bufferIn, bufferType typeIn,
                  void *bufferOut, bufferType typeOut, int *bufferDims )
{
  if ( bufferDims[2] == 1 ) 
    return( zeroCrossing2D( bufferIn, typeIn, bufferOut,
                            typeOut, bufferDims ) );
  
  return( zeroCrossing3D( bufferIn, typeIn, bufferOut,
                          typeOut, bufferDims ) );
}





int positiveZeroCrossing( void *bufferIn, bufferType typeIn,
                          void *bufferOut, bufferType typeOut, int *bufferDims )
{
  if ( bufferDims[2] == 1 ) 
    return( positiveZeroCrossing2D( bufferIn, typeIn, bufferOut,
                            typeOut, bufferDims ) );
  
  return( positiveZeroCrossing3D( bufferIn, typeIn, bufferOut,
                          typeOut, bufferDims ) );
}





int negativeZeroCrossing( void *bufferIn, bufferType typeIn,
                          void *bufferOut, bufferType typeOut, int *bufferDims )
{
  if ( bufferDims[2] == 1 ) 
    return( negativeZeroCrossing2D( bufferIn, typeIn, bufferOut,
                            typeOut, bufferDims ) );
  
  return( negativeZeroCrossing3D( bufferIn, typeIn, bufferOut,
                          typeOut, bufferDims ) );
}



