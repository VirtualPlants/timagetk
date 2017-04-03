/****************************************************
 * topological-single-axon.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2015, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 *
 * CREATION DATE:
 * Lun  7 mar 2016 22:09:09 CET
 *
 * ADDITIONS, CHANGES
 *
 *
 * METHOD
 * - from an input binary image
 * - remove simple points below the low threshold
 *   until no changes
 * - add simple points above the low threshold
 *   until no points can be added or
 *   until all points above the high threshold have been
 *   reached
 * - remove points between the low and the high threshold
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <typedefs.h>
#include <t04t08.h>
#include <t06t26.h>
#include <connexe.h>

#include <topological-operations-common.h>
#include <topological-thinning.h>
#include <topological-single-axon.h>








/**************************************************
 *
 *
 *
 **************************************************/


static int _verbose_ = 1;
static int _debug_ = 0;
static int _trace_ = 0;



void setVerboseInTopologicalSingleAxon( int v )
{
  _verbose_ = v;
}

void incrementVerboseInTopologicalSingleAxon(  )
{
  _verbose_ ++;
}

void decrementVerboseInTopologicalSingleAxon(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void setDebugInTopologicalSingleAxon( int v )
{
  _debug_ = v;
}

void incrementDebugInTopologicalSingleAxon(  )
{
  _debug_ ++;
}

void decrementDebugInTopologicalSingleAxon(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}








/**************************************************
 *
 * parameter management
 *
 **************************************************/



void initTypeSingleAxonParameters( typeSingleAxonParameters *p )
{
  p->low_threshold = -1;
  p->high_threshold = -1;
  p->stepToStop = -1;
}











/**************************************************
 *
 *
 *
 **************************************************/



static int _checkInitImage( unsigned char *theBuf,
                            void *theValues, bufferType theType, int *theDim )
{
  char *proc = "_checkInitImage";
  int i;
  int v = theDim[0] * theDim[1] * theDim[2];
  int imax, max;

  for ( i=0; i<v; i++ ) {
    if ( theBuf[i] > 0 ) return( 1 );
  }

#define _INITSEEDS( TYPE ) {          \
  TYPE *bufValues = (TYPE*)theValues; \
  imax = 0;                           \
  max = bufValues[0];                 \
  for ( i=1; i<v; i++ ) {             \
    if ( max < bufValues[i] ) {       \
      max = bufValues[i];             \
      imax = i;                       \
    }                                 \
  }                                   \
}

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _INITSEEDS( u8 );
    break;
  case SSHORT :
    _INITSEEDS( s16 );
    break;
  case USHORT :
    _INITSEEDS( u16 );
    break;
  }

  if ( _debug_ )
    fprintf( stderr, "%s: maximum is at index #%d\n", proc, imax );

  theBuf[imax] = 255;

  return( 1 );
}



static int _maskValuesImage( unsigned char *theBuf,
                             void *theValues, bufferType theType, int *theDim )
{
  char *proc = "_maskValuesImage";
  int i;
  int v = theDim[0] * theDim[1] * theDim[2];


#define _MASKVALUESIMAGES( TYPE ) {   \
  TYPE *bufValues = (TYPE*)theValues; \
  for ( i=0; i<v; i++ ) {             \
    if ( theBuf[i] == 0 ) {           \
      bufValues[i] = 0;               \
    }                                 \
    else if ( bufValues[i] == 0 ) {   \
      bufValues[i] = 1;               \
    }                                 \
  }                                   \
}

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _MASKVALUESIMAGES( u8 );
    break;
  case SSHORT :
    _MASKVALUESIMAGES( s16 );
    break;
  case USHORT :
    _MASKVALUESIMAGES( u16 );
    break;
  }

  return( 1 );
}



static int _thinningPreparation( unsigned char *theBuf,
                                 void *theValues, bufferType theType, int *theDim,
                                 int ht )
{
  char *proc = "_thinningPreparation";
  int i;
  int v = theDim[0] * theDim[1] * theDim[2];

#define _THINNINGPREPARATION( TYPE ) { \
  TYPE *bufValues = (TYPE*)theValues;  \
  for ( i=0; i<v; i++ ) {              \
    if ( bufValues[i] >= ht ) {        \
      theBuf[i] = _ANCHOR_VALUE_;      \
    }                                  \
    else if ( bufValues[i] > 0 ) {     \
      theBuf[i] = _CANBECHANGED_VALUE_; \
    }                                  \
    else {                             \
      theBuf[i] = _BACKGROUND_VALUE_;  \
    }                                  \
  }                                    \
}

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _THINNINGPREPARATION( u8 );
    break;
  case SSHORT :
    _THINNINGPREPARATION( s16 );
    break;
  case USHORT :
    _THINNINGPREPARATION( u16 );
    break;
  }

  return( 1 );
}



static int _hysteresisPreparation( unsigned char *theBuf,
                                 void *theValues, bufferType theType, int *theDim,
                                 int lt )
{
  char *proc = "_hysteresisPreparation";
  int i;
  int v = theDim[0] * theDim[1] * theDim[2];

#define _HYSTERESISPREPARATION( TYPE ) { \
  TYPE *bufValues = (TYPE*)theValues;  \
  for ( i=0; i<v; i++ ) {              \
    if ( theBuf[i] > 0 )               \
      continue;                        \
    if ( bufValues[i] >= lt ) {        \
      theBuf[i] = 100;                 \
    }                                  \
  }                                    \
}

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    _HYSTERESISPREPARATION( u8 );
    break;
  case SSHORT :
    _HYSTERESISPREPARATION( s16 );
    break;
  case USHORT :
    _HYSTERESISPREPARATION( u16 );
    break;
  }

  return( 1 );
}














/**************************************************
 *
 *
 *
 **************************************************/



int treePropagation( unsigned char *theBuf,
                     void *theValues, bufferType theType, int *theDim,
                     typeSingleAxonParameters *p )
{
  char *proc = "treePropagation";
  unsigned char *tmpBuf;
  unsigned char *ccBuf = (unsigned char*)NULL;
  void *tmpValues;
  int i, v, size, r;
  int step = 0;
  typeThinningParameters thinPar;

  v = theDim[0]*theDim[1]*theDim[2];

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    size = v *sizeof( u8 );
    break;
  case SSHORT :
    size = v *sizeof( s16 );
    break;
  case USHORT :
    size = v *sizeof( u16 );
    break;
  }
  tmpValues = (void*)malloc( size );
  if ( tmpValues == (void*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation of auxiliary values buffer failed\n", proc );
    return( -1 );
  }
  if ( _trace_ ) {
    ccBuf = (unsigned char*)malloc( v *sizeof( u8 ) );
    if ( ccBuf == (unsigned char*)NULL ) {
      free( tmpValues );
      if ( _verbose_ )
        fprintf( stderr, "%s: allocation of trace buffer failed\n", proc );
      return( -1 );
    }
  }

  /* pick the point of maximum value
   * if theBuf is empty
   * then
   * 1. mask the values image by the binary image
   * 2. points >= high threshold are anchor points
   *    points > 0 can be deleted
   */
  if ( _checkInitImage( theBuf, theValues, theType, theDim ) != 1 ) {
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpValues );
    if ( _verbose_ )
      fprintf( stderr, "%s: initialisation failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( theBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components in input buffer (step #0: %d)\n",
             r, step );
  }

  (void)memcpy( tmpValues, theValues, size );

  if ( _maskValuesImage( theBuf, tmpValues, theType, theDim ) != 1 ) {
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpValues );
    if ( _verbose_ )
      fprintf( stderr, "%s: initial masking failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpValues, theType, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components in masked value input buffer (step #0: %d)\n",
             r, step );
  }

  tmpBuf = (unsigned char*)malloc( v * sizeof(unsigned char) );
  if ( tmpBuf == (unsigned char*)NULL ) {
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpValues );
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation of auxiliary buffer failed\n", proc );
    return( -1 );
  }

  if ( _verbose_ ) {
     fprintf( stderr, "   ... thinning #1\n" );
  }

  step ++;
  if ( _thinningPreparation( tmpBuf, tmpValues, theType, theDim,
                             p->high_threshold ) != 1 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: thinning preparation failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thinning preparation (step #1: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thinning preparation (step #1: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  /* step #1: thinning
   * remove from input buffer the points of value < high threshold
   * that are topologically simple
   */
  initTypeThinningParameters( &thinPar );

  thinPar.typeOrdering = _INCREASING_ORDER_;
  thinPar.typeChanging = _FOREGROUND_TO_BACKGROUND_;
  thinPar.typeThickness = _26THICKNESS_;
  thinPar.typeEndPoint = _NO_END_POINT_;

  step ++;
  if ( valueBasedThinning( tmpBuf, theValues, theType, theDim, &thinPar ) < 0 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: thinning failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thinning (step #1: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thinning (step #1: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );

  /* thickening
   * add to the input buffer the points of value >= low threshold
   * that are  topologically simple
   * (until all points of value >= high threshold are reached)
   * Note: this last condition (not implemented yet) is perhaps
   *   not adequate, since it will perhaps keep path of small values
   *   connecting islands of high values
   * 1. select points to be added
   *    hysteresis threshold with
   *    - low = points from theValues >= low threshold
   *    - high = point from tmpBuf > 0
   * 2. thickening
   *
   */

  if ( _verbose_ ) {
     fprintf( stderr, "   ... hysteresis thresholding\n" );
  }

  step ++;
  if ( _hysteresisPreparation( tmpBuf, theValues, theType, theDim,
                             p->low_threshold ) != 1 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: hysteresis preparation failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after hysteresis preparation (step #2: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after hysteresis preparation (step #2)\n", proc );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  step ++;
  if ( HysteresisThresholding( tmpBuf, UCHAR, tmpBuf, UCHAR, theDim,
                               (double)100, (double)255 ) <= 0 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: hysteresis thresholding failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after hysteresis (step #2: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after hysteresis (step #2)\n", proc );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }


  if ( _verbose_ ) {
     fprintf( stderr, "   ... thickening\n" );
  }

  step ++;
  for ( i=0; i<v; i++ ) {
    if ( tmpBuf[i] > 0 ) {
      tmpBuf[i] = ( theBuf[i] > 0 ) ? _ANCHOR_VALUE_ : _CANBECHANGED_VALUE_;
    }
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thickening preparation (step #2: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thickening preparation (step #2: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  initTypeThinningParameters( &thinPar );

  thinPar.typeOrdering = _DECREASING_ORDER_;
  thinPar.typeChanging = _BACKGROUND_TO_FOREGROUND_;
  thinPar.typeThickness = _26THICKNESS_;
  thinPar.typeEndPoint = _NO_END_POINT_;

  step ++;
  if ( valueBasedThinning( tmpBuf, theValues, theType, theDim, &thinPar ) < 0 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: thinning failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thickening (step #2: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thickening (step #2: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  /* last thinning
   */
  (void)memcpy( tmpValues, theValues, size );

  if ( _verbose_ ) {
     fprintf( stderr, "   ... thinning #2\n" );
  }

  if ( _maskValuesImage( tmpBuf, tmpValues, theType, theDim ) != 1 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: initial masking failed\n", proc );
    return( -1 );
  }

  step ++;
  if ( _thinningPreparation( tmpBuf, tmpValues, theType, theDim,
                          (float)p->high_threshold ) != 1 ) {
      free( tmpValues );
      if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
      free( tmpBuf );
      if ( _verbose_ )
        fprintf( stderr, "%s: thinning preparation failed (step #3)\n", proc );
      return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thinning preparation (step #3: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thinning preparation (step #3: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  initTypeThinningParameters( &thinPar );

  thinPar.typeOrdering = _INCREASING_ORDER_;
  thinPar.typeChanging = _FOREGROUND_TO_BACKGROUND_;
  thinPar.typeThickness = _26THICKNESS_;
  thinPar.typeEndPoint = _NO_END_POINT_;

  step ++;
  if ( valueBasedThinning( tmpBuf, theValues, theType, theDim, &thinPar ) < 0 ) {
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    if ( _verbose_ )
      fprintf( stderr, "%s: thinning failed\n", proc );
    return( -1 );
  }

  if ( _trace_ ) {
    r = CountConnectedComponents( tmpBuf, UCHAR, ccBuf, UCHAR, theDim );
    fprintf( stderr, "       %d connected components after thinning (step #3: %d)\n",
             r, step );
  }

  if ( p->stepToStop >= 0 && step >= p->stepToStop ) {
    fprintf( stderr, "%s: stop after thinning (step #3: %d)\n", proc, p->stepToStop );
    (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );
    free( tmpValues );
    if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
    free( tmpBuf );
    return( 1 );
  }

  (void)memcpy( theBuf, tmpBuf, v*sizeof(u8) );


  /* valueBasedThinning
   * ajouter une condition par rapport a une valeur seuil
   * pour ne pas etendre trop ou eroder trop
   */
  free( tmpValues );
  if ( ccBuf != (unsigned char*)NULL ) free( ccBuf );
  free( tmpBuf );
  return( 1 );
}
















