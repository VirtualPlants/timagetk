/****************************************************
 * regionalext.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Feb  6 08:29:34 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <typedefs.h>
#include <chunks.h>
#include <convert.h>
#include <regionalext.h>








/*
 * static global variables
 * verbose,
 * management of ambiguous cases
 * memory management
 * 
 */





static int _verbose_ = 1;


void setVerboseInRegionalExt( int v )
{
  _verbose_ = v;
}

void incrementVerboseInRegionalExt(  )
{
  _verbose_ ++;
}

void decrementVerboseInRegionalExt(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



static int _n_points_to_be_allocated_ = 1000;

void regionalext_setNumberOfPointsForAllocation( int n )
{
   if ( n > 0 )  _n_points_to_be_allocated_ = n;
}





/*************************************************************
 *
 * static structures and functions: list management
 *
 ************************************************************/



typedef struct typePoint {
  unsigned short int x;
  unsigned short int y;
  unsigned short int z;
  int index; /* x+y*dimx+z*dimx*dimy */
  char interior; /* if 1, all neighbors are in the image (allows to skip tests) */
} typePoint;

typedef struct typePointList {
  size_t n_points;  
  size_t n_allocated_points;
  typePoint *data;
  int intensityValue;
} typePointList;


#ifdef _UNUSED_
static void printPoint( FILE *f, typePoint *p )
{
  fprintf( f, "[%5d %5d %5d] = %8d, interior=%1d\n",
           p->x, p->y, p->z, p->index, p->interior );
}
#endif


static void initPointList( typePointList *l )
{
  l->n_points = 0;
  l->n_allocated_points = 0;
  l->data = (typePoint*)NULL;
  l->intensityValue = 0;
}



static void freePointList( typePointList *l )
{
  if ( l->data != NULL ) free ( l->data );
  initPointList( l );
}



static int allocPointList( typePointList **l, int theMin, int theMax )
{
  char *proc = "allocPointList";
  int i;

  if ( theMin > theMax  ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: minmum value larger than maximal one\n", proc );
    return( -1 );
  }

  *l = (typePointList*)malloc( (theMax-theMin+1) * sizeof( typePointList ) );

  if ( *l == (typePointList*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation failed\n", proc );
    return( -1 );
  }
  
  for ( i=theMin; i<=theMax; i++ ) {
    initPointList( &((*l)[i-theMin]) );
    (*l)[i-theMin].intensityValue = i;
  }
  return( 1 );
}



static void statsPointList( FILE *f, typePointList *l, int theMin, int theMax )
{
  int i, s=0, a=0;
  
  for ( i=theMax; i>=theMin; i-- ) {
    if ( l[i-theMin].n_points > 0 || _verbose_ >= 4 ) {
        fprintf( f, " list[%3d]", i-theMin );
        fprintf( f, " for value %3d", i );
        fprintf( f, ": %8lu points / %8lu allocated\n", l[i-theMin].n_points, l[i-theMin].n_allocated_points );
    }
    s += l[i-theMin].n_points;
    a += l[i-theMin].n_allocated_points;
  }
  fprintf( f, " total:     %8d points / %8d allocated\n", s, a );
}



static int addPointToList( typePointList *l, 
                           int x, int y, int z, int index, int interior )
{
  char *proc = "addPointToList";
  int n = l->n_points;
  int newn;
  typePoint *data = NULL;


  
  if ( (size_t)n == l->n_allocated_points ) {

    newn = l->n_allocated_points +  _n_points_to_be_allocated_;

    data = (typePoint *)malloc( newn * sizeof( typePoint ) );
    if ( data == NULL ) {
      if ( _verbose_ ) {
        fprintf( stderr, "%s: can not reallocate point list\n", proc );
        fprintf( stderr, "\t failed to add (%d,%d,%d) in list", x, y, z );
      }
      return( -1 );
    }
    
    if ( l->data != NULL ) {
      (void)memcpy( (void*)data, (void*)l->data,  l->n_allocated_points * sizeof( typePoint ) );
      free( l->data );
    }

    l->data = data;
    l->n_allocated_points = newn;
    
  }

  l->data[ n ].x = x;
  l->data[ n ].y = y;
  l->data[ n ].z = z;
  l->data[ n ].index = index;
  l->data[ n ].interior = interior;

  l->n_points ++;

  return( l->n_points );
}










/*************************************************************
 *
 * 
 *
 ************************************************************/


static int minMaxValues( void *theIm, bufferType theType, 
                         void *theMask, bufferType theMaskType,
                         int *theDim, int *theMin, int *theMax )

{
  char *proc = "minMaxValues";
  size_t i, v;
  int min, max;

  v = theDim[2] * theDim[1] * theDim[0];

  if ( theMask == (void*)NULL || theMaskType == TYPE_UNKNOWN ) {

#define _MINMAX_WITHOUTMASK( TYPE ) { \
    TYPE *b = (TYPE*)theIm;           \
    min = max = *b;                   \
    for ( i=0; i<v; i++, b++ ) {      \
      if ( min > *b ) min = *b;       \
      else if ( max < *b ) max = *b;  \
    }                                 \
}

    switch ( theType ) {
    default : 
      if ( _verbose_ )
        fprintf( stderr, "%s: such image type not handled yet", proc );
      return( -1 );
    case SCHAR :
      _MINMAX_WITHOUTMASK( s8 )
      break;
    case UCHAR :
      _MINMAX_WITHOUTMASK( u8 )
       break;
    case SSHORT :
       _MINMAX_WITHOUTMASK( s16 )
      break;
    case USHORT :
      _MINMAX_WITHOUTMASK( u16 )
      break;
    }

  }
  else {
    
#define _MINMAX_WITHMASK( TYPEMASK, TYPE, VALMIN, VALMAX ) { \
    TYPEMASK *m = (TYPEMASK*)theMask;                        \
    TYPE *b = (TYPE*)theIm;                                  \
    min = VALMAX;                                            \
    max = VALMIN;                                            \
    for ( i=0; i<v; i++, b++, m++ ) {                        \
      if ( *m == 0 ) continue;                               \
      if ( min > *b ) min = *b;                              \
      else if ( max < *b ) max = *b;                         \
    }                                                        \
}


    switch ( theMaskType ) {
    default : 
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such mask type not handled yet\n", proc );
      }
      return( -1 );
    case UCHAR :
      switch ( theType ) {
      default : 
        if ( _verbose_ )
          fprintf( stderr, "%s: such image type not handled yet", proc );
        return( -1 );
      case SCHAR :
        _MINMAX_WITHMASK( u8, s8, -128, 127 )
        break;
      case UCHAR :
        _MINMAX_WITHMASK( u8, u8, 0, 255 )
        break;
      case SSHORT :
        _MINMAX_WITHMASK( u8, s16, -32768, 32767 )
        break;
      case USHORT :
        _MINMAX_WITHMASK( u8, u16, 0, 65535 )
        break;
      }
      break;
    }
  }

  *theMin = min;
  *theMax = max;

  return( 1 );
}










/*************************************************************
 *
 * procedures d'initialisation
 *
 ************************************************************/






typedef struct _initAuxiliaryImageParam {
  void *theInput;
  void *theOutput;
  bufferType theType;
  void *theMask;
  bufferType theMaskType;
  int maskValue;
  int height;
  double multiplier;
  int theMin;
  int theMax;
} _initAuxiliaryImageParam;



static void _initInitAuxiliaryImageParam( _initAuxiliaryImageParam *p ) {
  p->theInput = (void*)NULL;
  p->theOutput = (void*)NULL;
  p->theType = TYPE_UNKNOWN;
  p->theMask = (void*)NULL;
  p->theMaskType = TYPE_UNKNOWN;
  p->maskValue = -1;
  p->height = 1;
  p->multiplier = 1.0;
  p->theMin = 0;
  p->theMax = 255;
}




static void *_initRegionalMaxAuxiliaryImageSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  char *proc = "_initRegionalMaxAuxiliaryImageSubroutine";

  bufferType theType = ((_initAuxiliaryImageParam*)parameter)->theType;
  void *bufMask = ((_initAuxiliaryImageParam*)parameter)->theMask;
  bufferType theMaskType = ((_initAuxiliaryImageParam*)parameter)->theMaskType;
  int maskValue =  ((_initAuxiliaryImageParam*)parameter)->maskValue;
  int height = ((_initAuxiliaryImageParam*)parameter)->height;
  double multiplier = ((_initAuxiliaryImageParam*)parameter)->multiplier;
  int theMin = ((_initAuxiliaryImageParam*)parameter)->theMin;

  size_t i;
  int min;

  if ( bufMask == (void*)NULL || theMaskType == TYPE_UNKNOWN ) {
    /* no mask
     */
    if ( 0.99 < multiplier && multiplier < 1.01 ) {
      
      /* no mask && no multiplicative coefficient
       */

#define _INITMAX_WITHOUTMASK_WITHOUTMULTIPLICATIVE( TYPE ) {                      \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        min =  theInput[i] - height;                                              \
        theOutput[i] = ( min < theMin ) ? theMin : min;                           \
      }                                                                           \
}

      switch( theType ) {
      default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case SCHAR :
        _INITMAX_WITHOUTMASK_WITHOUTMULTIPLICATIVE( s8 )
        break;
      case UCHAR :
        _INITMAX_WITHOUTMASK_WITHOUTMULTIPLICATIVE( u8 )
        break;
      case SSHORT :
        _INITMAX_WITHOUTMASK_WITHOUTMULTIPLICATIVE( s16 )
        break;
      case USHORT :
        _INITMAX_WITHOUTMASK_WITHOUTMULTIPLICATIVE( u16 )
        break;
      }
      
    }
    else {
      /* no mask && multiplicative coefficient
       */

#define _INITMAX_WITHOUTMASK_WITHMULTIPLICATIVE( TYPE ) {                          \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;    \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput;  \
      for ( i=first; i<=last; i++ ) {                                              \
        min =  theInput[i] - height;                                               \
        if ( min < theMin ) min = theMin;                                          \
        else {                                                                     \
          if ( min > theMin + (int)( (theInput[i] - theMin) * multiplier + 0.5 ) ) \
            min = theMin + (int)( (theInput[i] - theMin) * multiplier + 0.5 );     \
        }                                                                          \
        theOutput[i] = min;                                                        \
      }                                                                            \
}

      switch( theType ) {
      default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case SCHAR :
        _INITMAX_WITHOUTMASK_WITHMULTIPLICATIVE( s8 )
        break;
      case UCHAR :
        _INITMAX_WITHOUTMASK_WITHMULTIPLICATIVE( u8 )
        break;
      case SSHORT :
        _INITMAX_WITHOUTMASK_WITHMULTIPLICATIVE( s16 )
        break;
      case USHORT :
        _INITMAX_WITHOUTMASK_WITHMULTIPLICATIVE( u16 )
        break;
      }
      
    }
  }

  else {
    /* mask
     */
    if ( 0.99 < multiplier && multiplier < 1.01 ) {
      /* mask && no multiplicative coefficient
       */

#define _INITMAX_WITHMASK_WITHOUTMULTIPLICATIVE( TYPEMASK, TYPE ) {               \
      TYPEMASK *theMask = (TYPEMASK*)bufMask;                                     \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        if ( theMask[i] != maskValue ) {                                          \
          theOutput[i] = theInput[i];                                             \
        }                                                                         \
        else {                                                                    \
          min =  theInput[i] - height;                                            \
          theOutput[i] = ( min < theMin ) ? theMin : min;                         \
        }                                                                         \
      }                                                                           \
}

      switch ( theMaskType ) {
      default : 
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such mask type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case UCHAR :
        switch( theType ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          }
          chunk->ret = -1;
          return( (void*)NULL );
        case SCHAR :
          _INITMAX_WITHMASK_WITHOUTMULTIPLICATIVE( u8, s8 )
          break;
        case UCHAR :
          _INITMAX_WITHMASK_WITHOUTMULTIPLICATIVE( u8, u8 )
            break;
        case SSHORT :
          _INITMAX_WITHMASK_WITHOUTMULTIPLICATIVE( u8, s16 )
            break;
        case USHORT :
          _INITMAX_WITHMASK_WITHOUTMULTIPLICATIVE( u8, u16 )
            break;
        }
        break;
      }

    }
    else {
      /* mask && multiplicative coefficient
       */

#define _INITMAX_WITHMASK_WITHMULTIPLICATIVE( TYPEMASK, TYPE ) {                  \
      TYPEMASK *theMask = (TYPEMASK*)bufMask;                                     \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        if ( theMask[i] != maskValue ) {                                          \
          theOutput[i] = theInput[i];                                             \
        }                                                                         \
        else {                                                                    \
            min =  theInput[i] - height;                                               \
          if ( min < theMin ) min = theMin;                                          \
          else {                                                                     \
            if ( min > theMin + (int)( (theInput[i] - theMin) * multiplier + 0.5 ) ) \
              min = theMin + (int)( (theInput[i] - theMin) * multiplier + 0.5 );     \
          }                                                                          \
          theOutput[i] = min;                                                        \
        }                                                                         \
      }                                                                           \
}

      switch ( theMaskType ) {
      default : 
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such mask type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case UCHAR :
        switch( theType ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          }
          chunk->ret = -1;
          return( (void*)NULL );
        case SCHAR :
          _INITMAX_WITHMASK_WITHMULTIPLICATIVE( u8, s8 )
          break;
        case UCHAR :
          _INITMAX_WITHMASK_WITHMULTIPLICATIVE( u8, u8 )
            break;
        case SSHORT :
          _INITMAX_WITHMASK_WITHMULTIPLICATIVE( u8, s16 )
            break;
        case USHORT :
          _INITMAX_WITHMASK_WITHMULTIPLICATIVE( u8, u16 )
            break;
        }
        break;
      }

    }
  }

  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_initRegionalMinAuxiliaryImageSubroutine( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  char *proc = "_initRegionalMinAuxiliaryImageSubroutine";

  bufferType theType = ((_initAuxiliaryImageParam*)parameter)->theType;
  void *bufMask = ((_initAuxiliaryImageParam*)parameter)->theMask;
  bufferType theMaskType = ((_initAuxiliaryImageParam*)parameter)->theMaskType;
  int maskValue =  ((_initAuxiliaryImageParam*)parameter)->maskValue;
  int height = ((_initAuxiliaryImageParam*)parameter)->height;
  double multiplier = ((_initAuxiliaryImageParam*)parameter)->multiplier;
  int theMax = ((_initAuxiliaryImageParam*)parameter)->theMax;

  size_t i;
  int max;

  if ( bufMask == (void*)NULL || theMaskType == TYPE_UNKNOWN ) {
    /* no mask
     */
    if ( 0.99 < multiplier && multiplier < 1.01 ) {
      
      /* no mask && no multiplicative coefficient
       */

#define _INITMIN_WITHOUTMASK_WITHOUTMULTIPLICATIVE( TYPE ) {                      \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        max =  theInput[i] + height;                                              \
        theOutput[i] = ( max > theMax ) ? theMax : max;                           \
      }                                                                           \
}

      switch( theType ) {
      default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case SCHAR :
        _INITMIN_WITHOUTMASK_WITHOUTMULTIPLICATIVE( s8 )
        break;
      case UCHAR :
        _INITMIN_WITHOUTMASK_WITHOUTMULTIPLICATIVE( u8 )
        break;
      case SSHORT :
        _INITMIN_WITHOUTMASK_WITHOUTMULTIPLICATIVE( s16 )
        break;
      case USHORT :
        _INITMIN_WITHOUTMASK_WITHOUTMULTIPLICATIVE( u16 )
        break;
      }
      
    }
    else {
      /* no mask && multiplicative coefficient
       */

#define _INITMIN_WITHOUTMASK_WITHMULTIPLICATIVE( TYPE ) {                          \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;    \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput;  \
      for ( i=first; i<=last; i++ ) {                                              \
        max =  theInput[i] + height;                                               \
        if ( max > theMax ) max = theMax;                                          \
        else {                                                                     \
          if ( max < theMax - (int)( (theMax - theInput[i]) * multiplier + 0.5 ) ) \
            max = theMax - (int)( (theMax - theInput[i]) * multiplier + 0.5 );     \
        }                                                                          \
        theOutput[i] = max;                                                        \
      }                                                                            \
}

      switch( theType ) {
      default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case SCHAR :
        _INITMIN_WITHOUTMASK_WITHMULTIPLICATIVE( s8 )
        break;
      case UCHAR :
        _INITMIN_WITHOUTMASK_WITHMULTIPLICATIVE( u8 )
        break;
      case SSHORT :
        _INITMIN_WITHOUTMASK_WITHMULTIPLICATIVE( s16 )
        break;
      case USHORT :
        _INITMIN_WITHOUTMASK_WITHMULTIPLICATIVE( u16 )
        break;
      }
      
    }
  }

  else {
    /* mask
     */
    if ( 0.99 < multiplier && multiplier < 1.01 ) {
      /* mask && no multiplicative coefficient
       */

#define _INITMIN_WITHMASK_WITHOUTMULTIPLICATIVE( TYPEMASK, TYPE ) {               \
      TYPEMASK *theMask = (TYPEMASK*)bufMask;                                     \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        if ( theMask[i] != maskValue ) {                                          \
          theOutput[i] = theInput[i];                                             \
        }                                                                         \
        else {                                                                    \
          max =  theInput[i] + height;                                            \
          theOutput[i] = ( max > theMax ) ? theMax : max;                         \
        }                                                                         \
      }                                                                           \
}

      switch ( theMaskType ) {
      default : 
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such mask type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case UCHAR :
        switch( theType ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          }
      chunk->ret = -1;
      return( (void*)NULL );
        case SCHAR :
          _INITMIN_WITHMASK_WITHOUTMULTIPLICATIVE( u8, s8 )
          break;
        case UCHAR :
          _INITMIN_WITHMASK_WITHOUTMULTIPLICATIVE( u8, u8 )
            break;
        case SSHORT :
          _INITMIN_WITHMASK_WITHOUTMULTIPLICATIVE( u8, s16 )
            break;
        case USHORT :
          _INITMIN_WITHMASK_WITHOUTMULTIPLICATIVE( u8, u16 )
            break;
        }
        break;
      }

    }
    else {
      /* mask && multiplicative coefficient
       */

#define _INITMIN_WITHMASK_WITHMULTIPLICATIVE( TYPEMASK, TYPE ) {                  \
      TYPEMASK *theMask = (TYPEMASK*)bufMask;                                     \
      TYPE *theInput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theInput;   \
      TYPE *theOutput = (TYPE*)((_initAuxiliaryImageParam*)parameter)->theOutput; \
      for ( i=first; i<=last; i++ ) {                                             \
        if ( theMask[i] != maskValue ) {                                          \
          theOutput[i] = theInput[i];                                             \
        }                                                                         \
        else {                                                                    \
          max =  theInput[i] + height;                                               \
          if ( max > theMax ) max = theMax;                                          \
          else {                                                                     \
            if ( max < theMax - (int)( (theMax - theInput[i]) * multiplier + 0.5 ) ) \
              max = theMax - (int)( (theMax - theInput[i]) * multiplier + 0.5 );     \
          }                                                                          \
          theOutput[i] = max;                                                        \
        }                                                                         \
      }                                                                           \
}

      switch ( theMaskType ) {
      default : 
        if ( _verbose_ ) {
          fprintf( stderr, "%s: such mask type not handled yet\n", proc );
        }
        chunk->ret = -1;
        return( (void*)NULL );
      case UCHAR :
        switch( theType ) {
        default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          }
      chunk->ret = -1;
      return( (void*)NULL );
        case SCHAR :
          _INITMIN_WITHMASK_WITHMULTIPLICATIVE( u8, s8 )
          break;
        case UCHAR :
          _INITMIN_WITHMASK_WITHMULTIPLICATIVE( u8, u8 )
            break;
        case SSHORT :
          _INITMIN_WITHMASK_WITHMULTIPLICATIVE( u8, s16 )
            break;
        case USHORT :
          _INITMIN_WITHMASK_WITHMULTIPLICATIVE( u8, u16 )
            break;
        }
        break;
      }

    }
  }

  chunk->ret = 1;
  return( (void*)NULL );
}





/* initialization for regional maxima:
   we compute min( I - height, I * multiplier ), more precisely, we compute 
   min ( I - height, I_min + (I-I_min) * multiplier).
   This way, we lower the peaks either by substracting a constant value (height), or by 
   multiplying the peak height (w.r.t. the minimum image value) by an other constant value
   (assumed to be less than 1)
   Value less than the minimum image value are set to the minimum image value.

   initialization for regional minima:
   we compute max ( I + height, I_max - (I_max-I) * multiplier).
   This way, we fill the valleys either by adding a constant value (height), or by 
   multiplying the valley depth (w.r.t. the maximum image value) by an other constant value
   (assumed to be less than 1)
   Value larger than the maximum image value are set to the maximum image value.
*/

static int _initRegionalAuxiliaryImage( void *theInput, void *theOutput, bufferType theType, 
                                        void *theMask, bufferType theMaskType,
                                        int maskValue,
                                        int *theDim,
                                        enumRegionalExtremum extremumType,
                                        int height, double multiplier,
                                        int theMin, int theMax )
{
  char *proc = "_initRegionalMaxAuxiliaryImage";
  typeChunks chunks;
  int n;
  size_t first = 0;
  size_t last = theDim[0] * theDim[1] * theDim[2] - 1;
  _initAuxiliaryImageParam p;

  _initInitAuxiliaryImageParam( &p );

  p.theInput = theInput;
  p.theOutput = theOutput;
  p.theType = theType;
  p.theMask = theMask;
  p.theMaskType = theMaskType;
  p.maskValue = maskValue;
  p.height = height;
  p.multiplier = multiplier;
  p.theMin = theMin;
  p.theMax = theMax;

  initChunks( &chunks );

  if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute chunks\n", proc );
    return( -1 );
  }

  for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
    chunks.data[n].parameters = (void*)(&p);
  }

  switch ( extremumType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
    return( -1 );
  case _REGIONAL_MAX_ :
    if ( processChunks( &_initRegionalMaxAuxiliaryImageSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to initialize regional maxima computation\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
  case _REGIONAL_MIN_ :
    if ( processChunks( &_initRegionalMinAuxiliaryImageSubroutine, &chunks, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to initialize regional minima computation\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    break;
  }

  freeChunks( &chunks );

  return( 1 );
}










/*************************************************************
 *
 * list building
 * two procedures:
 * 1. the number of points for each value is calculated so that
 *    the exact number of points is allocated
 *    it avoids reallocation
 *    It is designed for the first assignment where all the points
 *    have to be put in lists
 * 2. the  addPointToList() function is used, reallocation may
 *    happen
 *
 ************************************************************/



static int _buildFirstLists( typePointList *thePointList,
                             void *theInput, void *theOutput, bufferType theType,
                             int *theDim,
                             enumRegionalExtremum extremumType,
                             int theMin, int theMax )
{
  char *proc = "_buildFirstLists";
  
  int *histo = NULL;
  int i, j;
  int v =  theDim[0]*theDim[1]*theDim[2];
  int theFirst = theMin;
  int theLast = theMax;
  int valueNotToIndex = 0;

  int x, y, z;
  int insidez, insidey;
  typePoint *pt;
  int dimx = theDim[0];
  int dimx1 = theDim[0]-1;
  int dimy = theDim[1];
  int dimy1 = theDim[1]-1;
  int dimz = theDim[2];
  int dimz1 = theDim[2]-1;



  /* the points with extremal value won't change
   */
  switch ( extremumType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
    return( -1 );
    case _REGIONAL_MAX_ :
      theLast = theMax - 1;
      valueNotToIndex = theMax;
      break;
    case _REGIONAL_MIN_ :
      theFirst = theMin + 1;
      valueNotToIndex = theMin;
      break;
  }
  


  /* here we allocate the point lists with the right number of points
     this will avoid multiple reallocations
  */

  /* histogram computation
   */
  
  histo = (int*)malloc( (theMax-theMin+1)*sizeof(int) );
  if ( histo == (int*)NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate histogram\n", proc );
    }
    return( -1 );
  }

  for ( i=0; i<theMax-theMin+1; i++ ) histo[i] = 0;

  switch ( theType ) {
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    }
    free( histo );
    return( -1 );
  case SCHAR :
    {
      char *theRef = (char *)theInput;
      char *theBuf = (char *)theOutput;
      for ( i=0; i<v; i++ ) {
        if ( theBuf[i] == theRef[i] ) continue;
        if ( theBuf[i] == valueNotToIndex ) continue;
        histo[ ((int)theBuf[i])-theMin ] ++;
      }
    }
    break;
  case UCHAR :
    {
      unsigned char *theRef = (unsigned char *)theInput;
      unsigned char *theBuf = (unsigned char *)theOutput;
      for ( i=0; i<v; i++ ) {
        if ( theBuf[i] == theRef[i] ) continue;
        if ( theBuf[i] == valueNotToIndex ) continue;
        histo[ ((int)theBuf[i])-theMin ] ++;
      }
    }
    break;
  case SSHORT :
    {
      short int *theRef = (short int *)theInput;
      short int *theBuf = (short int *)theOutput;
      for ( i=0; i<v; i++ ) {
        if ( theBuf[i] == theRef[i] ) continue;
        if ( theBuf[i] == valueNotToIndex ) continue;
        histo[ ((int)theBuf[i])-theMin ] ++;
      }
    }
    break;
  case USHORT :
    {
      unsigned short int *theRef = (unsigned short int *)theInput;
      unsigned short int *theBuf = (unsigned short int *)theOutput;
      for ( i=0; i<v; i++ ) {
        if ( theBuf[i] == theRef[i] ) continue;
        if ( theBuf[i] == valueNotToIndex ) continue;
        histo[ ((int)theBuf[i])-theMin ] ++;
      }
    }
    break;
  }
  


  /* list allocation
   */

  for ( j=theFirst, i=theFirst-theMin; j<=theLast; j++, i++ ) {

    /* just in case
     */
    if ( thePointList[i].data != (typePoint *)NULL ) {
      freePointList( &thePointList[i] );
    }
    initPointList( &thePointList[i] );

    if ( histo[i] == 0 ) continue;


    thePointList[i].data = (typePoint *)malloc( histo[i] * sizeof( typePoint ) );

    if ( thePointList[i].data == (typePoint *)NULL ) {
      free( histo );
      if ( _verbose_ ) {
        fprintf( stderr, "%s: unable to allocate list #%d\n", proc, i );
      }
      return( -1 );
    }

    thePointList[i].n_allocated_points = histo[i];
    thePointList[i].n_points = 0;

  }

  free( histo );



 /* fill the lists
   */

#define _FILL_FIRST_LISTS( TYPE ) {                                                                  \
  TYPE *theRef = (TYPE *)theInput;                                                                   \
  TYPE *theBuf = (TYPE *)theOutput;                                                                  \
  for ( i=0, z=0; z<dimz; z++ ) {                                                                    \
    insidez = ( 0 < z && z < dimz1 ) ? 1 : 0;                                                        \
    for ( y=0; y<dimy; y++ ) {                                                                       \
      insidey = ( insidez == 1 && 0 < y && y < dimy1 ) ? 1 : 0;                                      \
      for ( x=0; x<dimx; x++, i++ ) {                                                                \
        if ( theBuf[i] == theRef[i] ) continue;                                                      \
        if ( theBuf[i] == valueNotToIndex ) continue;                                                \
        pt = &( thePointList[ theBuf[i]-theMin ].data[ thePointList[ theBuf[i]-theMin ].n_points ]); \
        pt->x = x;   pt->y = y;   pt->z = z;   pt->index = i;                                        \
        pt->interior = ( insidey == 1 && 0 < x && x < dimx1 ) ? 1 : 0;                               \
        thePointList[ theBuf[i]-theMin ].n_points ++;                                                \
      }                                                                                              \
    }                                                                                                \
  }                                                                                                  \
}
 
  switch ( theType ) {
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    }
    return( -1 );
  case SCHAR :
    _FILL_FIRST_LISTS( s8 )
    break;
  case UCHAR :
    _FILL_FIRST_LISTS( u8 )
    break;
  case SSHORT :
    _FILL_FIRST_LISTS( s16 )
    break;
  case USHORT :
    _FILL_FIRST_LISTS( u16 )
    break;
  }

  return( 1 );
}





static int _buildLists( typePointList *thePointList,
                        void *theInput, void *theOutput, bufferType theType,
                        int *theDim,
                        enumRegionalExtremum extremumType,
                        int theMin, int theMax )
{
  char *proc = "_buildLists";
  
  int i;
  int theFirst = theMin;
  int theLast = theMax;
  int valueNotToIndex = 0;

  int x, y, z;
  int insidez, insidey;
  int interior;
  int dimx = theDim[0];
  int dimx1 = theDim[0]-1;
  int dimy = theDim[1];
  int dimy1 = theDim[1]-1;
  int dimz = theDim[2];
  int dimz1 = theDim[2]-1;



  /* the points with extremal value won't change
   */
  switch ( extremumType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
    return( -1 );
    case _REGIONAL_MAX_ :
      theLast = theMax - 1;
      valueNotToIndex = theMax;
      break;
    case _REGIONAL_MIN_ :
      theFirst = theMin + 1;
      valueNotToIndex = theMin;
      break;
  }
  

  /* set the number of points to 0
   */
  for ( i=0; i<theMax-theMin+1; i++ )
    thePointList[i].n_points = 0;


 /* fill the lists
   */

#define _FILL_LISTS( TYPE ) {                                                                        \
  TYPE *theRef = (TYPE *)theInput;                                                                   \
  TYPE *theBuf = (TYPE *)theOutput;                                                                  \
  for ( i=0, z=0; z<dimz; z++ ) {                                                                    \
    insidez = ( 0 < z && z < dimz1 ) ? 1 : 0;                                                        \
    for ( y=0; y<dimy; y++ ) {                                                                       \
      insidey = ( insidez == 1 && 0 < y && y < dimy1 ) ? 1 : 0;                                      \
      for ( x=0; x<dimx; x++, i++ ) {                                                                \
        if ( theBuf[i] == theRef[i] ) continue;                                                      \
        if ( theBuf[i] == valueNotToIndex ) continue;                                                \
        interior = ( insidey == 1 && 0 < x && x < dimx1 ) ? 1 : 0;                                                                                \
        if ( addPointToList( &thePointList[ theBuf[i]-theMin ], x, y, z, i, interior ) == -1 ) {     \
          if ( _verbose_ ) {                                                                             \
            fprintf( stderr, "%s: unable to add point (%d,%d,%d) of value %d to list\n", proc,       \
                     x, y, z, theBuf[i]);                                                            \
            fprintf( stderr, "   ... list[ %d ] has %lu points / %lu allocated\n",                   \
                     theBuf[i]-theMin, thePointList[ theBuf[i]-theMin ].n_points,                    \
                     thePointList[ theBuf[i]-theMin ].n_allocated_points );                          \
          }                                                                                          \
          return( -1 );                                                                              \
        }                                                                                            \
      }                                                                                              \
    }                                                                                                \
  }                                                                                                  \
}
 
  switch ( theType ) {
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    }
    return( -1 );
  case SCHAR :
    _FILL_LISTS( s8 )
    break;
  case UCHAR :
    _FILL_LISTS( u8 )
    break;
  case SSHORT :
    _FILL_LISTS( s16 )
    break;
  case USHORT :
    _FILL_LISTS( u16 )
    break;
  }

  return( 1 );
}










/**************************************************
 *
 * offset
 *
 **************************************************/



typedef struct typeOffset {
  int dx;
  int dy;
  int dz;
  int di;
} typeOffset;



typedef struct typeNeighborhood {
  int nneighbors;
  typeOffset neighbors[27];
} typeNeighborhood;



static void _defineNeighborsTobBeTested( typeNeighborhood *n, 
                                        int *theDim,
                                        int connectivity )
{
  int i = 0;

  for ( i=0; i<27; i++ ) {
    n->neighbors[i].dx = 0;
    n->neighbors[i].dy = 0;
    n->neighbors[i].dz = 0;
    n->neighbors[i].di = 0;
  }

  i = 0;

  /* 04-neighbors
   */
  n->neighbors[i].dy = -1;   i++;
  n->neighbors[i].dx = -1;   i++;
  n->neighbors[i].dx =  1;   i++;
  n->neighbors[i].dy =  1;   i++;

  switch ( connectivity ) {
  case 4 :
    break;
  case 6 :
    n->neighbors[i].dz = -1;   i++;
    n->neighbors[i].dz =  1;   i++;
    break;
  case 8 :
  case 18 :
  case 26 :
  default : 
    n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
    break;
  }
  
  switch ( connectivity ) {
  case 4 :
  case 8 :
  case 6 :
    break;
  case 26 :
  default : 
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   n->neighbors[i].dx =  1;   i++;
  case 18 :
    n->neighbors[i].dz = -1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz = -1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz = -1;   n->neighbors[i].dy =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy = -1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx = -1;   i++;
    n->neighbors[i].dz =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dx =  1;   i++;
    n->neighbors[i].dz =  1;   n->neighbors[i].dy =  1;   i++;
  }

  n->nneighbors = i;
  
  for ( i=0; i<n->nneighbors; i++ ) {
    n->neighbors[i].di = n->neighbors[i].dz * theDim[0] * theDim[1]
      + n->neighbors[i].dy * theDim[0]
      + n->neighbors[i].dx;
  }
}










/*************************************************************
 *
 * process lists
 *
 ************************************************************/



typedef struct _processListParam {
  typePointList *thePointList;
  void *theInput;
  void *theOutput; 
  int *theDim; 
  typeNeighborhood *neighborhood;

  int changes;
} _processListParam;



static void _initProcessListParam( _processListParam *p )
{
  p->thePointList = (typePointList *)NULL;
  p->theInput = (void *)NULL;
  p->theOutput = (void *)NULL;
  p->theDim = (int *)NULL;
  p->changes = 0;
}



/* dilate (X-connectivity)
   warning: in parallelism, buffer value are accessed outside the allocated range
   of each chunk -> we may have a different behavior than the sequential
   computation (but should be the same result)
   can be corrected if two buffers are used (one as input, the second one as output)
   warning: this may cause access to a data being written by an other chunk :(
*/





#define _PROCESSLISTMAX( TYPE ) {                                               \
  typePointList *thePointList = ((_processListParam*)parameter)->thePointList;  \
  TYPE *theRef = (TYPE *)(((_processListParam*)parameter)->theInput);           \
  TYPE *theBuf = (TYPE *)(((_processListParam*)parameter)->theOutput);          \
  int dimx = ((_processListParam*)parameter)->theDim[0];                        \
  int dimy = ((_processListParam*)parameter)->theDim[1];                        \
  int dimz = ((_processListParam*)parameter)->theDim[2];                        \
  typeNeighborhood *neigh = ((_processListParam*)parameter)->neighborhood;      \
  typeOffset *neighbors = neigh->neighbors;                                     \
  int nneighbors = neigh->nneighbors;                                           \
  typePoint *pt;                                                                \
  int x, y, z, i;                                                               \
  size_t p;                                                                     \
  int n, val;                                                                   \
  int c = 0;                                                                    \
                                                                                \
  for ( p=first; p<=last; p ++ ) {                                              \
    pt = &(thePointList->data[p]);                                              \
    i = pt->index;                                                              \
    val = theBuf[i];                                                            \
    if ( pt->interior ) {                                                       \
      for ( n=0; n<nneighbors; n++ )                                            \
        if ( val < theBuf[ i+neighbors[n].di ] )                                \
          val = theBuf[ i+neighbors[n].di ];                                    \
    }                                                                           \
    else {                                                                      \
      x = pt->x;   y = pt->y;    z = pt->z;                                     \
      for ( n=0; n<nneighbors; n++ ) {                                          \
        if ( x + neighbors[n].dx < 0 || dimx <= x + neighbors[n].dx )           \
          continue;                                                             \
        if ( y + neighbors[n].dy < 0 || dimy <= y + neighbors[n].dy )           \
          continue;                                                             \
        if ( z + neighbors[n].dz < 0 || dimz <= z + neighbors[n].dz )           \
          continue;                                                             \
        if ( val < theBuf[ i+neighbors[n].di ] )                                \
          val = theBuf[ i+neighbors[n].di ];                                    \
      }                                                                         \
    }                                                                           \
    if ( val > theRef[i] ) val = theRef[i];                                     \
    if ( val > theBuf[i] ) {                                                    \
      theBuf[i] = val;                                                          \
      c ++;                                                                     \
    }                                                                           \
  }                                                                             \
  ((_processListParam*)parameter)->changes = c;                                 \
  chunk->ret = 1;                                                               \
  return( (void*)NULL );                                                        \
}



#define _PROCESSLISTMIN( TYPE ) {                                               \
  typePointList *thePointList = ((_processListParam*)parameter)->thePointList;  \
  TYPE *theRef = (TYPE *)(((_processListParam*)parameter)->theInput);           \
  TYPE *theBuf = (TYPE *)(((_processListParam*)parameter)->theOutput);          \
  int dimx = ((_processListParam*)parameter)->theDim[0];                        \
  int dimy = ((_processListParam*)parameter)->theDim[1];                        \
  int dimz = ((_processListParam*)parameter)->theDim[2];                        \
  typeNeighborhood *neigh = ((_processListParam*)parameter)->neighborhood;      \
  typeOffset *neighbors = neigh->neighbors;                                     \
  int nneighbors = neigh->nneighbors;                                           \
  typePoint *pt;                                                                \
  int x, y, z, i;                                                               \
  size_t p;                                                                     \
  int n, val;                                                                   \
  int c = 0;                                                                    \
                                                                                \
  for ( p=first; p<=last; p ++ ) {                                              \
    pt = &(thePointList->data[p]);                                              \
    i = pt->index;                                                              \
    val = theBuf[i];                                                            \
    if ( pt->interior ) {                                                       \
      for ( n=0; n<neigh->nneighbors; n++ )                                     \
        if ( val > theBuf[ i+neighbors[n].di ] )                                \
          val = theBuf[ i+neighbors[n].di ];                                    \
    }                                                                           \
    else {                                                                      \
      x = pt->x;   y = pt->y;    z = pt->z;                                     \
      for ( n=0; n<nneighbors; n++ ) {                                          \
        if ( x + neighbors[n].dx < 0 || dimx <= x + neighbors[n].dx )           \
          continue;                                                             \
        if ( y + neighbors[n].dy < 0 || dimy <= y + neighbors[n].dy )           \
          continue;                                                             \
        if ( z + neighbors[n].dz < 0 || dimz <= z + neighbors[n].dz )           \
          continue;                                                             \
        if ( val > theBuf[ i+neighbors[n].di ] )                                \
          val = theBuf[ i+neighbors[n].di ];                                    \
      }                                                                         \
    }                                                                           \
    if ( val < theRef[i] ) val = theRef[i];                                     \
    if ( val < theBuf[i] ) {                                                    \
      theBuf[i] = val;                                                          \
      c ++;                                                                     \
    }                                                                           \
  }                                                                             \
  ((_processListParam*)parameter)->changes = c;                                 \
  chunk->ret = 1;                                                               \
  return( (void*)NULL );                                                        \
}



static void *_processListMaxSubroutine_s8( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMAX( s8 )
}



static void *_processListMaxSubroutine_u8( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMAX( u8 )
}



static void *_processListMaxSubroutine_s16( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMAX( s16 )
}



static void *_processListMaxSubroutine_u16( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMAX( u16 )
}



static void *_processListMinSubroutine_s8( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMIN( s8 )
}



static void *_processListMinSubroutine_u8( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMIN( u8 )
}



static void *_processListMinSubroutine_s16( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMIN( s16 )
}



static void *_processListMinSubroutine_u16( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  _PROCESSLISTMIN( u16 )
}







static int _processList ( typePointList *thePointList,
                          void *theInput,
                          void *theOutput,
                          bufferType theType,
                          int *theDim,
                          int theConnectivity,
                          enumRegionalExtremum extremumType,
                          int theMin, int theMax )
{
  char *proc = "_processList";
  int connectivity = theConnectivity;
  typeNeighborhood neighborhood;

  _chunk_callfunction _processListSubroutine = NULL;
  int n;
  typeChunks chunks; /* list of chunks */
  typeChunk chunk; /* one chunk, for sequential processing */
  size_t first, last;
  _processListParam aux;
  _processListParam *auxList = NULL;
  int use_parallelism = 0;
  int magic_number = 2;

  typePoint tmp;
  int i;
  int refValue = 0, val;
  int c, changes = 0;



  if ( thePointList->n_points <= 0 ) return( 0 );

  /*--------------------------------------------------
   *
   * connectivity based choices
   *
   --------------------------------------------------*/

  switch( connectivity ) {
  case 4 :
  case 8 :
  case 6 :
  case 18 :
  case 26 :
    break;
  default :
    connectivity = 26;
  }
    
  if ( theDim[2] == 1 ) {
    switch( connectivity ) {
    case 6 :
      connectivity = 4; break;
    case 18 :
    case 26 :
      connectivity = 8; break;
    default :
      break;
    }
  }
  _defineNeighborsTobBeTested( &neighborhood, theDim, connectivity );



  /*--------------------------------------------------
   *
   * subroutine
   *
   --------------------------------------------------*/

  switch( extremumType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
    return( -1 );
  case _REGIONAL_MAX_ :
    refValue = theMax;
    switch ( theType ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      return( -1 );
    case SCHAR :
      _processListSubroutine = &_processListMaxSubroutine_s8;
      break;
    case UCHAR :
      _processListSubroutine = &_processListMaxSubroutine_u8;
      break;
    case SSHORT :
      _processListSubroutine = &_processListMaxSubroutine_s16;
      break;
    case USHORT :
      _processListSubroutine = &_processListMaxSubroutine_u16;
      break;
    }
    break;
  case _REGIONAL_MIN_ :
    refValue = theMin;
    switch ( theType ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      return( -1 );
    case SCHAR :
      _processListSubroutine = &_processListMinSubroutine_s8;
      break;
    case UCHAR :
      _processListSubroutine = &_processListMinSubroutine_u8;
      break;
    case SSHORT :
      _processListSubroutine = &_processListMinSubroutine_s16;
      break;
    case USHORT :
      _processListSubroutine = &_processListMinSubroutine_u16;
      break;
    }
    break;
  }


  /*--------------------------------------------------
   *
   * parallelism initialization
   *
   --------------------------------------------------*/

  first = 0;
  last = thePointList->n_points-1;

  if ( thePointList->n_points > (size_t)(magic_number*getMaxChunks()) ) {
    use_parallelism = 1;

    initChunks( &chunks );
    if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute chunks\n", proc );
      return( -1 );
    }
    
    auxList = (_processListParam*)malloc( chunks.n_allocated_chunks * sizeof(_processListParam) );
    if ( auxList == (_processListParam*)NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary variables\n", proc );
      freeChunks( &chunks );
      return( -1 );
    }
    
    for ( n=0; n<chunks.n_allocated_chunks; n++ ) {
      _initProcessListParam( &(auxList[n]) );
      auxList[n].thePointList = thePointList;
      auxList[n].theInput = theInput;
      auxList[n].theOutput = theOutput;
      auxList[n].theDim = theDim;
      auxList[n].neighborhood = &neighborhood;

      chunks.data[n].parameters = (void*)(&auxList[n]);
    }
  }
    
  _initProcessListParam( &aux );
  aux.thePointList = thePointList;
  aux.theInput = theInput;
  aux.theOutput = theOutput;
  aux.theDim = theDim;
  aux.neighborhood = &neighborhood;


  /*--------------------------------------------------
   *
   * processing
   *
   --------------------------------------------------*/

  do {

    if ( use_parallelism == 1 ) {
      for ( n=0; n<chunks.n_allocated_chunks; n++ )
        auxList[n].changes = 0;
      if ( processChunks(  _processListSubroutine, &chunks, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute changes\n", proc );
        free( auxList );
        freeChunks( &chunks );
        return( -1 );
      }
    }
    else {
      aux.changes = 0;
      initChunk( &chunk );
      chunk.first = first;
      chunk.last = last;
      chunk.parameters = (void*)&aux;
      (* _processListSubroutine)( (void*)&chunk );
      if ( chunk.ret != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute changes\n", proc );
        return( -1 );
      }
    }


    /* number of changes at this iteration
     */
    c = 0;
    if ( use_parallelism == 1 ) {
      for ( n=0; n<chunks.n_allocated_chunks; n++ )
        c += auxList[n].changes;
    }
    else 
      c = aux.changes;

    if ( _verbose_ >= 4 ) {
      fprintf( stderr,"      changes = %8d\n", c );
    }

    changes += c;
    

    /* prepare next iteration if necessary
     */
    
#define _REORDERLIST_FIRST( TYPE ) {                       \
    TYPE *theRef = (TYPE *)theInput;                       \
    TYPE *theBuf = (TYPE *)theOutput;                      \
    for ( n=first; n<thePointList->n_points; n++ ) {       \
      i = thePointList->data[n].index;                     \
      val = theBuf[i];                                     \
      if ( val == theRef[i] || val == refValue ) {         \
        tmp = thePointList->data[first];                   \
        thePointList->data[first] = thePointList->data[n]; \
        thePointList->data[n] = tmp;                       \
        first ++;                                          \
      }                                                    \
    }                                                      \
}

#define _REORDERLIST_LAST( TYPE ) {                        \
    TYPE *theRef = (TYPE *)theInput;                       \
    TYPE *theBuf = (TYPE *)theOutput;                      \
    for ( n=first; n<(int)thePointList->n_points; ) {      \
      i = thePointList->data[n].index;                     \
      val = theBuf[i];                                     \
      if ( val == theRef[i] || val == refValue ) {         \
        tmp = thePointList->data[thePointList->n_points-1];                   \
        thePointList->data[thePointList->n_points-1] = thePointList->data[n]; \
        thePointList->data[n] = tmp;                       \
        thePointList->n_points --;                         \
      }                                                    \
      else {                                               \
        n ++;                                              \
      }                                                    \
    }                                                      \
}



    if ( c > 0 ) {
      /* put points which won't change any more 
         at the end of the list
      */
      switch( theType ) {
      default :
        free( auxList );
        freeChunks( &chunks );
        if ( _verbose_ )
          fprintf( stderr, "%s: such image type not handled yet (list re-ordering)\n", proc );
        return( -1 );
      case SCHAR :
        _REORDERLIST_LAST( s8 )
        break;
      case UCHAR :
        _REORDERLIST_LAST( u8 )
        break;
      case SSHORT :
        _REORDERLIST_LAST( s16 )
        break;
      case USHORT :
        _REORDERLIST_LAST( u16 )
        break;
      }

      if ( use_parallelism == 1 
           && (size_t)thePointList->n_points - first > (size_t)(magic_number*getMaxChunks()) ) {
        if ( buildEqualChunks( &(chunks.data[0]), first, (size_t)(thePointList->n_points-1),
                               chunks.n_allocated_chunks ) != 1 ) {
          free( auxList );
          freeChunks( &chunks );
          if ( _verbose_ )
            fprintf( stderr, "%s: unable to re-compute chunks\n", proc );
          return( -1 );
        }
      }
      else {
        if ( use_parallelism == 1 ) {
          free( auxList );
          freeChunks( &chunks );
        }
        use_parallelism = 0;
      }

    }
    
  } while ( c > 0 && first < (size_t)thePointList->n_points );
  
  if ( use_parallelism == 1 ) {
    free( auxList );
    freeChunks( &chunks );
  }

  return( changes );


  return( 1 );
}







/*************************************************************
 *
 * 
 *
 ************************************************************/




/* this function only extends the points from theOutput
   within the limits of theInput
*/
static int _processRegionalExtrema( void *theInput, void *theOutput, bufferType theType, 
                                    int *theDim,
                                    int connectivity,
                                    enumRegionalExtremum extremumType,
                                    int theMin, int theMax )
{
  char *proc = "_processRegionalExtrema";
  typePointList *thePointList = NULL;
  
  int changes, localchanges;
  int iteration = 0;
  int i, j;

  /* list allocation
     list length is theMax-theMin+1
     point of intensity i (in [theMin,theMax]) will be in list[i-theMin]
   */
  if ( allocPointList( &thePointList, theMin, theMax ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate list\n", proc );
    return( -1 );
  }


  do {

    changes = 0;

    /* build lists
     */

    if ( _verbose_ >= 4 )
      fprintf( stderr, "iteration #%d: entering list building\n", iteration );


    if ( iteration == 0 ) {
      if ( _buildFirstLists( thePointList,
                             theInput, theOutput, theType, theDim,
                             extremumType, theMin, theMax ) != 1 ) {
        for ( i=theMin; i<=theMax; i++ )
          freePointList( &thePointList[i-theMin] );
        free( thePointList );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to build list at first iteration %d\n", proc, iteration );
        return( -1 );
      }
    }
    else {
      if ( _buildLists( thePointList,
                        theInput, theOutput, theType, theDim,
                        extremumType, theMin, theMax ) != 1 ) {
        for ( i=theMin; i<=theMax; i++ )
          freePointList( &thePointList[i-theMin] );
        free( thePointList );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to build list at iteration %d\n", proc, iteration );
        return( -1 );
      }
    }

    if ( _verbose_ >= 4 )
      fprintf( stderr, "iteration #%d: list building done\n", iteration );


    /* check lists
     */
    if ( _verbose_ >= 5 || (_verbose_ >= 4 && iteration == 0) ) {
        fprintf( stderr, "==================================================\n" );
        fprintf( stderr, "%s: list statistics at iteration %d\n", proc, iteration );
        fprintf( stderr, "\t min, max Values = %d, %d\n", theMin, theMax );
        statsPointList( stderr, thePointList, theMin, theMax );
        fprintf( stderr, "==================================================\n" );
    }


    /* process lists
     */
    switch ( extremumType ) {
    default :
      for ( j=0; j<theMax-theMin+1; j++ )
        freePointList( &thePointList[j] );
      free( thePointList );
      if ( _verbose_ )
        fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
      return( -1 );

    case _REGIONAL_MAX_ :

      for ( i=theMax-1; i>=theMin; i-- ) {

        if ( thePointList[i-theMin].n_points == 0 ) continue;

        if ( _verbose_ >= 4 )
          fprintf( stderr, " - iteration #%d: list #%5d, n=%lu\n", iteration, i-theMin, thePointList[i-theMin].n_points );


        localchanges = _processList ( &(thePointList[i-theMin]), theInput, theOutput, theType, theDim,
                                      connectivity, extremumType, theMin, theMax );


        if ( localchanges < 0 ) {
          for ( j=0; j<theMax-theMin+1; j++ )
            freePointList( &thePointList[j] );
          free( thePointList );
          if ( _verbose_ )
            fprintf( stderr, "%s: error when processing list %d at iteration %d\n", proc, i, iteration );
          return( -1 );
        }

        changes += localchanges;

        if ( localchanges > 0 && _verbose_ >= 2 ) {
          fprintf( stderr,"   it. #%3d val. %05d: changes = %8d / %8d", iteration, i, localchanges, changes );
          if ( _verbose_ >= 3 )
            fprintf( stderr, "\n" );
          else
            fprintf( stderr, "\r" );
        }

      }
      break;
      
    case _REGIONAL_MIN_ :

      for ( i=theMin+1; i<=theMax; i++ ) {

        if ( thePointList[i-theMin].n_points == 0 ) continue;

        if ( _verbose_ >= 4 )
          fprintf( stderr, " - iteration #%d: list #%5d, n=%lu\n", iteration, i-theMin, thePointList[i-theMin].n_points );


        localchanges = _processList ( &(thePointList[i-theMin]), theInput, theOutput, theType, theDim,
                                      connectivity, extremumType, theMin, theMax );


        if ( localchanges < 0 ) {
          for ( j=0; j<theMax-theMin+1; j++ )
            freePointList( &thePointList[j] );
          free( thePointList );
          if ( _verbose_ )
            fprintf( stderr, "%s: error when processing list %d at iteration %d\n", proc, i, iteration );
          return( -1 );
        }

        changes += localchanges;

        if ( localchanges > 0 && _verbose_ >= 2 ) {
          fprintf( stderr,"   #%3d.%05d: changes = %8d / %8d", iteration, i, localchanges, changes );
          if ( _verbose_ >= 3 )
            fprintf( stderr, "\n" );
          else
            fprintf( stderr, "\r" );
        }
      }
      break;
    }

    if ( _verbose_ >= 2 ) fprintf( stderr, "\n" );

    if ( _verbose_ == 1 ) {
      fprintf( stderr, "iteration #%3d - changes = %9d\t\t\n", iteration, changes );
    }

    iteration ++;

  } while (changes > 0 );



  /* freeing the structures
   */
  for ( j=0; j<theMax-theMin+1; j++ )
    freePointList( &thePointList[j] );
  free( thePointList );

  return( 1 );

}




static int _regionalExtrema( void *theInput, void *theOutput, bufferType theType, 
                            int *theDim,
                            int height, double multiplier,
                            int connectivity,
                            enumRegionalExtremum extremumType )
{
  char *proc = "_regionalExtrema";

  int theInputMin, theInputMax;
  int theInitialMin, theInitialMax;
  int v = theDim[0]*theDim[1]*theDim[2];
  int i;
  void *theTmp = NULL;



  /* compute min and max values of input image
     these values are used for image initialization (ie substraction of h
     or multiplication by the coefficient), points below the min (for maxima
     computation) of above the max (for minima computation) are set to
     respectively the min and the max.
     This way, points of min (resp. max) values in the input image won't be processed.
   */
  if ( minMaxValues( theInput, theType, (void*)NULL, TYPE_UNKNOWN, theDim, &theInputMin, &theInputMax ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute min and max\n", proc );
    return( -1 );
  }
  if ( theInputMin == theInputMax ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: uniform input image (of value %d), no possible extrema\n", proc, theInputMin );
    return( -1 );
  }

  if ( _verbose_ >= 2 )
    fprintf( stderr, "%s:   input image has value in [%5d %5d]\n",
             proc, theInputMin, theInputMax );





  /* allocating the auxiliary image
   */
  if ( theInput == theOutput ) {
    switch ( theType ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      }
      return( -1 );
    case SCHAR :
      theTmp = malloc( v * sizeof( s8 ) );
      break;
    case UCHAR :
      theTmp = malloc( v * sizeof( u8 ) );
      break;
    case SSHORT :
      theTmp = malloc( v * sizeof( s16 ) );
      break;
    case USHORT :
      theTmp = malloc( v * sizeof( u16 ) );
      break;
    }
    if ( theTmp == NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
  }
  else {
    theTmp = theOutput;
  }
  
  

  /* initializing the auxiliary image
   */
  if ( _initRegionalAuxiliaryImage( theInput, theTmp, theType, 
                                    (void*)NULL, TYPE_UNKNOWN, -1,
                                    theDim,
                                    extremumType,
                                    height, multiplier,
                                    theInputMin, theInputMax ) != 1 ) {
    if ( theInput == theOutput ) free( theTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: error when initializing auxiliary image\n", proc );
    return( -1 );
  }



  /* compute min and max values of initial image
   */
  if ( minMaxValues( theTmp, theType, (void*)NULL, TYPE_UNKNOWN, theDim, &theInitialMin, &theInitialMax ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute min and max\n", proc );
    return( -1 );
  }

  if ( extremumType == _REGIONAL_MAX_ && theInitialMin != theInputMin ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird, the minimal value should be the same for the input (%d) and the initial (%d) images.\n", 
               proc, theInputMin, theInitialMin );
  }
  else if ( extremumType == _REGIONAL_MIN_ && theInitialMax != theInputMax ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird, the maximal value should be the same for the input (%d) and the initial (%d) images.\n", 
               proc, theInputMax, theInitialMax );
  }

  if ( _verbose_ >= 2 )
    fprintf( stderr, "%s: initial image has value in [%5d %5d]\n",
             proc, theInitialMin, theInitialMax );





  /* compute the extrema
   */
  if ( _processRegionalExtrema( theInput, theTmp, theType, 
                                theDim,
                                connectivity, extremumType,
                                theInitialMin, theInitialMax ) != 1 ) {
    if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: error when computing extrema\n", proc );
      return( -1 );
  }



#define _MAXIMA_OUTPUT( TYPE ) {                           \
  TYPE *theBuf = (TYPE*)theInput;                          \
  TYPE *tmpBuf = (TYPE*)theTmp;                            \
  TYPE *resBuf = (TYPE*)theOutput;                         \
  for ( i=0; i<v; i++ ) resBuf[i] = theBuf[i] - tmpBuf[i]; \
}

#define _MINIMA_OUTPUT( TYPE ) {                           \
  TYPE *theBuf = (TYPE*)theInput;                          \
  TYPE *tmpBuf = (TYPE*)theTmp;                            \
  TYPE *resBuf = (TYPE*)theOutput;                         \
  for ( i=0; i<v; i++ ) resBuf[i] = tmpBuf[i] - theBuf[i]; \
}

  
  /* computing the output
   */

  switch ( extremumType ) {

  default :
    if ( theInput == theOutput ) free( theTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
    return( -1 );

  case _REGIONAL_MAX_ :

    switch ( theType ) {
    default :
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      return( -1 );
    case SCHAR : 
      _MAXIMA_OUTPUT( s8 )
      break;
    case UCHAR : 
      _MAXIMA_OUTPUT( s8 )
      break;
    case SSHORT : 
      _MAXIMA_OUTPUT( s16 )
      break;    
    case USHORT : 
      _MAXIMA_OUTPUT( u16 )
      break;
    }
    break;

  case _REGIONAL_MIN_ :

    switch ( theType ) {
    default :
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      return( -1 );
    case SCHAR : 
      _MINIMA_OUTPUT( s8 )
      break;
    case UCHAR : 
      _MINIMA_OUTPUT( s8 )
      break;
    case SSHORT : 
      _MINIMA_OUTPUT( s16 )
      break;    
    case USHORT : 
      _MINIMA_OUTPUT( u16 )
      break;
    }
    break;

  }

  if ( theInput == theOutput ) free( theTmp );

  return( 1 );
}









static int _regionalHierarchicalExtrema( void *theInput, void *theOutput, bufferType theType, 
                                         int *theDim,
                                         int heightmin, int heightmax,
                                         int connectivity,
                                         enumRegionalExtremum extremumType )
{
  char *proc = "_regionalHierarchicalExtrema";

  int theInputMin, theInputMax;
  int theInitialMin, theInitialMax;
  
  int v = theDim[0]*theDim[1]*theDim[2];
  int height, i;
  void *theTmp = NULL;

  void *theRes = NULL;
  bufferType theResType = TYPE_UNKNOWN;

  void *theMask = NULL;
  bufferType theMaskType = TYPE_UNKNOWN;

  
  if ( heightmin > heightmax ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird height range =[%d %d]\n", proc, heightmin, heightmax);
    return( -1 );
  }



  /* compute min and max values of input image
   */
  if ( minMaxValues( theInput, theType, (void*)NULL, TYPE_UNKNOWN, theDim, &theInputMin, &theInputMax ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to compute min and max\n", proc );
    return( -1 );
  }
  if ( theInputMin == theInputMax ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: uniform input image (of value %d), no possible extrema\n", proc, theInputMin );
    return( -1 );
  }

  if ( _verbose_ >= 2 )
    fprintf( stderr, "%s:   input image has value in [%5d %5d]\n",
             proc, theInputMin, theInputMax );





  /* allocating the auxiliary image for computation
   */
  if ( theInput == theOutput ) {
    switch ( theType ) {
    default :
      if ( _verbose_ ) {
        fprintf( stderr, "%s: such image type not handled yet\n", proc );
      }
      return( -1 );
    case SCHAR :
      theTmp = malloc( v * sizeof( s8 ) );
      break;
    case UCHAR :
      theTmp = malloc( v * sizeof( u8 ) );
      break;
    case SSHORT :
      theTmp = malloc( v * sizeof( s16 ) );
      break;
    case USHORT :
      theTmp = malloc( v * sizeof( u16 ) );
      break;
    }
    if ( theTmp == NULL ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate auxiliary image\n", proc );
      return( -1 );
    }
  }
  else {
    theTmp = theOutput;
  }
  
  
  /* allocating the result image 
   */
  if ( heightmax - heightmin <= 255 ) {
    theRes = malloc( v * sizeof( u8 ) );
    if ( theRes == NULL ) {
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate result image\n", proc );
      return( -1 );
    }
    theResType = UCHAR;
  }
  else {
    if ( theInput == theOutput ) free( theTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: height range =[%d %d] too large\n", proc, heightmin, heightmax);
    return( -1 );
  }
  
  switch ( theResType ) {
  default : 
    free( theRes );
    if ( theInput == theOutput ) free( theTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: such result image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *resBuf = (u8*)theRes;
      for ( i=0; i<v; i++ )
        resBuf[i] = 0;
    }
    break;
  }



  /* loop over the heights
   */

  for ( height=heightmax; height>=heightmin; height-- ) {

    if ( _verbose_ )
      fprintf( stderr, "%s: processing height = %d   \n", proc, height );

    /* initializing the auxiliary image
     */

    theMask = (void*)NULL;
    theMaskType = TYPE_UNKNOWN;

    if ( height < heightmax ) {
      theMask = theRes;
      theMaskType = theResType;
    }

    if ( _initRegionalAuxiliaryImage( theInput, theTmp, theType, 
                                      theMask, theMaskType, height+1,
                                      theDim,
                                      extremumType,
                                      height, 1.0,
                                      theInputMin, theInputMax ) != 1 ) {
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: error when initializing auxiliary image\n", proc );
      return( -1 );
    }



    /* compute min and max values
     */
    if ( minMaxValues( theTmp, theType, theMask, theMaskType, theDim, &theInitialMin, &theInitialMax ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to compute min and max\n", proc );
      return( -1 );
    }
    
    if ( theInitialMin == theInitialMax ) {
      /* a subtraction should be done here
       */
      free( theRes );
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: uniform image (of value %d), no possible extrema\n", proc, theInitialMin );
      return( -1 );
    }
    
    if ( extremumType == _REGIONAL_MAX_ && theInitialMin != theInputMin ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: weird, the minimal value should be the same for the input (%d) and the initial (%d) images.\n",
                 proc, theInputMin, theInitialMin );
    }
    else if ( extremumType == _REGIONAL_MIN_ && theInitialMax != theInputMax ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: weird, the maximal value should be the same for the input (%d) and the initial (%d) images.\n",
                 proc, theInputMax, theInitialMax );
    }

    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: initial image has value in [%5d %5d]\n",
               proc, theInitialMin, theInitialMax );
    

    
    /* compute the extrema
     */
    if ( _processRegionalExtrema( theInput, theTmp, theType, 
                                  theDim,
                                  connectivity, extremumType,
                                  theInitialMin, theInitialMax ) != 1 ) {
      free( theRes );
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: error when computing extrema\n", proc );
      return( -1 );
    }
    
    
#define _MAXIMA_HIERARCHICAL_OUTPUT( TYPERES, TYPE ) { \
  TYPE *theBuf = (TYPE*)theInput;               \
  TYPE *tmpBuf = (TYPE*)theTmp;                 \
  TYPERES *resBuf = (TYPERES*)theRes;           \
  for ( i=0; i<v; i++ )                         \
    if ( theBuf[i] - tmpBuf[i] > 0 )            \
    resBuf[i] = height;                         \
}

#define _MINIMA_HIERARCHICAL_OUTPUT( TYPERES, TYPE ) { \
  TYPE *theBuf = (TYPE*)theInput;               \
  TYPE *tmpBuf = (TYPE*)theTmp;                 \
  TYPERES *resBuf = (TYPERES*)theRes;           \
  for ( i=0; i<v; i++ )                         \
    if ( tmpBuf[i] - theBuf[i] > 0 )            \
    resBuf[i] = height;                         \
}

    /* computing the output
     */
    switch ( theResType ) {
    default : 
      free( theRes );
      if ( theInput == theOutput ) free( theTmp );
      if ( _verbose_ )
        fprintf( stderr, "%s: such result image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :

      switch ( extremumType ) {
      default :
        free( theRes );
        if ( theInput == theOutput ) free( theTmp );
        if ( _verbose_ )
          fprintf( stderr, "%s: such extremum type not handled yet\n", proc );
        return( -1 );

      case _REGIONAL_MAX_ :
        switch ( theType ) {
        default :
          free( theRes );
          if ( theInput == theOutput ) free( theTmp );
          if ( _verbose_ )
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          return( -1 );
        case SCHAR :
          _MAXIMA_HIERARCHICAL_OUTPUT( u8, s8 )
          break;
        case UCHAR :
          _MAXIMA_HIERARCHICAL_OUTPUT( u8, u8 )
          break;
        case SSHORT :
          _MAXIMA_HIERARCHICAL_OUTPUT( u8, s16 )
          break;
        case USHORT :
          _MAXIMA_HIERARCHICAL_OUTPUT( u8, u16 )
          break;
        }
        break;

      case _REGIONAL_MIN_ :
        switch ( theType ) {
        default :
          free( theRes );
          if ( theInput == theOutput ) free( theTmp );
          if ( _verbose_ )
            fprintf( stderr, "%s: such image type not handled yet\n", proc );
          return( -1 );
        case SCHAR :
          _MINIMA_HIERARCHICAL_OUTPUT( u8, s8 )
          break;
        case UCHAR :
          _MINIMA_HIERARCHICAL_OUTPUT( u8, u8 )
          break;
        case SSHORT :
          _MINIMA_HIERARCHICAL_OUTPUT( u8, s16 )
          break;
        case USHORT :
          _MINIMA_HIERARCHICAL_OUTPUT( u8, u16 )
          break;
        }
        break;

      } /* end of  switch ( extremumType ) */
      break;
    } /* switch ( theResType ) */

  } /* end of for ( height=heightmax; height>=heightmin; height-- ) */



  /* copy result
   */
    if ( ConvertBuffer( theRes, theResType, theOutput, theType, (size_t)v ) != 1 ) {
    free( theRes );
    if ( theInput == theOutput ) free( theTmp );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to copy/convert the result\n", proc );
    return( -1 );
  }

  free( theRes );
  if ( theInput == theOutput ) free( theTmp );

  return( 1 );
}





int regionalMaxima( void *theInput, void *theOutput, 
                 bufferType theType, int *theDim,
                 int height, double multiplier,
                 int connectivity )
{
  return( _regionalExtrema( theInput, theOutput, theType,  theDim, 
                            height, multiplier,
                            connectivity, _REGIONAL_MAX_ ) );
}





int regionalMinima( void *theInput, void *theOutput, 
                    bufferType theType, int *theDim,
                    int height, double multiplier,
                    int connectivity )
{
  return( _regionalExtrema( theInput, theOutput, theType, theDim, 
                            height, multiplier,
                            connectivity, _REGIONAL_MIN_ ) );
}






int regionalHierarchicalMaxima( void *theInput, void *theOutput, 
                 bufferType theType, int *theDim,
                 int heightmin, int heightmax,
                 int connectivity )
{
  return( _regionalHierarchicalExtrema( theInput, theOutput, theType, theDim, 
                                        heightmin, heightmax,
                                        connectivity, _REGIONAL_MAX_ ) );
}





int regionalHierarchicalMinima( void *theInput, void *theOutput, 
                    bufferType theType, int *theDim,
                    int heightmin, int heightmax,
                    int connectivity )
{
  return( _regionalHierarchicalExtrema( theInput, theOutput, theType, theDim, 
                                        heightmin, heightmax,
                                        connectivity, _REGIONAL_MIN_ ) );
}
