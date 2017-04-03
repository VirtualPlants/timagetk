/*************************************************************************
 * local-operation.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Sun Feb 17 20:29:10 CET 2013
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convert.h>
#include <local-operation.h>





static int _verbose_ = 1;



void setVerboseInLocalOperation( int v )
{
  _verbose_ = v;
}

void incrementVerboseInLocalOperation(  )
{
  _verbose_ ++;
}

void decrementVerboseInLocalOperation(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}



typedef enum {
  _MAX_,
  _MIN_,
  _QUANTILE_,
  _MEAN_,
  _ROBUST_MEAN_,
  _SQRSUM_,
  _SUM_,
  _STDDEV_,
  _VAR_
} enumFiltering;


typedef struct {
  i32 v;
  double r;
} typei32Point;

typedef struct {
  r32 v;
  double r;
} typer32Point;

typedef struct {
  r64 v;
  double r;
} typer64Point;




#define _EXTRACT_NEIGHBORHOOD_( TAB ) {                         \
  iLength = 0;                                                  \
  /*--- saisie du voisinage ---*/				\
  if ( (z + nOffsets[2] < 0) || (z + pOffsets[2] >= dimz) ||	\
       (y + nOffsets[1] < 0) || (y + pOffsets[1] >= dimy) ||	\
       (x + nOffsets[0] < 0) || (x + pOffsets[0] >= dimx) ) {	\
    /*--- borders ---*/					        \
    for ( k = nOffsets[2]; k <= pOffsets[2]; k ++ ) {		\
      if ( (z + k < 0) || (z + k >= dimz) ) continue;		\
      for ( j = nOffsets[1]; j <= pOffsets[1]; j ++ ) {		\
        if ( (y + j < 0) || (y + j >= dimy) ) continue;		\
        for ( i = nOffsets[0]; i <= pOffsets[0]; i ++ ) {	\
          if ( (x + i < 0) || (x + i >= dimx) ) continue;	\
          TAB[ iLength ++ ].v = theBuf[(z+k)*dimxy+(y+j)*dimx+(x+i)]; \
        }							\
      }								\
    }								\
    iQuant = (int)((iLength-1) * q + 0.5);			\
  } else { 							\
    /*--- deep inside the image ---*/				\
    for ( k = nOffsets[2]; k <= pOffsets[2]; k ++ )		\
    for ( j = nOffsets[1]; j <= pOffsets[1]; j ++ )		\
    for ( i = nOffsets[0]; i <= pOffsets[0]; i ++ ) {		\
      TAB[ iLength ++ ].v = theBuf[(z+k)*dimxy+(y+j)*dimx+(x+i)]; \
    }						        	\
    iQuant = quantile;						\
  }								\
}





#define _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB ) {			\
  iLength = 0;                                                  \
  /*--- saisie du voisinage ---*/				\
  if ( (z + nOffsets[2] < 0) || (z + pOffsets[2] >= dimz) ||	\
       (y + nOffsets[1] < 0) || (y + pOffsets[1] >= dimy) ||	\
       (x + nOffsets[0] < 0) || (x + pOffsets[0] >= dimx) ) {	\
    for ( b=0; b<dimb; b++ ) {                                  \
      theBuf = (ITYPE*)(bufferIn[b]);                           \
      /*--- borders ---*/					\
      for ( k = nOffsets[2]; k <= pOffsets[2]; k ++ ) {		\
        if ( (z + k < 0) || (z + k >= dimz) ) continue;		\
        for ( j = nOffsets[1]; j <= pOffsets[1]; j ++ ) {	\
          if ( (y + j < 0) || (y + j >= dimy) ) continue;	\
          for ( i = nOffsets[0]; i <= pOffsets[0]; i ++ ) {	\
            if ( (x + i < 0) || (x + i >= dimx) ) continue;	\
            TAB[ iLength ++ ].v = theBuf[(z+k)*dimxy+(y+j)*dimx+(x+i)]; \
          }							\
        }						     	\
      }                                                         \
    }								\
    iQuant = (int)((iLength-1) * q + 0.5);			\
  } else { 							\
    for ( b=0; b<dimb; b++ ) {                                  \
      theBuf = (ITYPE*)(bufferIn[b]);                           \
      /*--- deep inside the image ---*/				\
      for ( k = nOffsets[2]; k <= pOffsets[2]; k ++ )		\
      for ( j = nOffsets[1]; j <= pOffsets[1]; j ++ )		\
      for ( i = nOffsets[0]; i <= pOffsets[0]; i ++ ) {		\
        TAB[ iLength ++ ].v = theBuf[(z+k)*dimxy+(y+j)*dimx+(x+i)]; \
      }                                                         \
    }						        	\
    iQuant = quantile;						\
  }								\
}





#define _COMPUTE_QUANTILE_( TAB, TMP ) {                             \
  /*--- computing the median value 				   \
        without ordering the values ---*/			   \
  iLeft = 0;   iRight = iLength - 1;				   \
  do {								   \
    /* swap left et (left+right)/2 */				   \
    j = (iLeft+iRight)/2;					   \
    TMP = TAB[iLeft];   TAB[iLeft] = TAB[j];   TAB[j] = TMP;	   \
    /* cut TAB into two */					   \
    iLast = iLeft;						   \
    for ( i = iLeft+1; i <= iRight; i++ ) {			   \
      if ( TAB[i].v < TAB[iLeft].v ) {				   \
        iLast ++;						   \
        TMP = TAB[i];   TAB[i] = TAB[iLast];   TAB[iLast] = TMP;   \
      }								   \
    }								   \
    TMP = TAB[iLeft];   TAB[iLeft] = TAB[iLast]; TAB[iLast] = TMP; \
								   \
    if ( iLast >  iQuant ) iRight = iLast - 1;			   \
    if ( iLast <  iQuant ) iLeft  = iLast + 1;			   \
  } while ( iLast != iQuant );					   \
  resBuf[z*dimxy+y*dimx+x] = TAB[iQuant].v;		     	   \
}



#define _COMPUTE_MIN_( TAB, TMP ) {	       \
  TMP = TAB[0];                                \
  for ( iLeft=1; iLeft<iLength; iLeft++ ) {    \
    if ( TMP.v > TAB[iLeft].v ) TMP = TAB[iLeft];  \
  }                                            \
  resBuf[z*dimxy+y*dimx+x] = TMP.v;	       \
}



#define _COMPUTE_MAX_( TAB, TMP ) {	       \
  TMP = TAB[0];                                \
  for ( iLeft=1; iLeft<iLength; iLeft++ ) {    \
    if ( TMP.v < TAB[iLeft].v ) TMP = TAB[iLeft];  \
  }                                            \
  resBuf[z*dimxy+y*dimx+x] = TMP.v;	       \
}



#define _COMPUTE_MEAN_( TAB, TMP ) {	       \
  TMP.v = 0;                                   \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    TMP.v += TAB[iLeft].v;		       \
  }                                            \
  TMP.v /= (double)iLength;                    \
  resBuf[z*dimxy+y*dimx+x] = TMP.v;	       \
}



#define _COMPUTE_SUM_( TAB, TMP ) {	       \
  TMP.v = 0;                                   \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    TMP.v += TAB[iLeft].v;		       \
  }                                            \
  resBuf[z*dimxy+y*dimx+x] = TMP.v;	       \
}



#define _COMPUTE_SQRSUM_( TAB, TMP ) {	       \
  TMP.v = 0;                                   \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    TMP.v += TAB[iLeft].v*TAB[iLeft].v;	       \
  }                                            \
  resBuf[z*dimxy+y*dimx+x] = TMP.v;	       \
}



#define _COMPUTE_STDDEV_( TAB, TMP, SQR ) {    \
  TMP.v = SQR.v = 0;                           \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    TMP.v += TAB[iLeft].v;		       \
    SQR.v += (double)TAB[iLeft].v * (double)TAB[iLeft].v; \
  }                                            \
  TMP.v /= (double)iLength;                    \
  SQR.v /= (double)iLength;                    \
  resBuf[z*dimxy+y*dimx+x] = sqrt( SQR.v - TMP.v*TMP.v ); \
}



#define _COMPUTE_VAR_( TAB, TMP, SQR ) {       \
  TMP.v = SQR.v = 0;                           \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    TMP.v += TAB[iLeft].v;		       \
    SQR.v += (double)TAB[iLeft].v * (double)TAB[iLeft].v; \
  }                                            \
  TMP.v /= (double)iLength;                    \
  SQR.v /= (double)iLength;                    \
  resBuf[z*dimxy+y*dimx+x] = SQR.v - TMP.v*TMP.v; \
}



#define _COMPUTE_ROBUST_MEAN_( TAB, TMP, SUM, SUN ) {	\
  SUM.v = 0;                                   \
  for ( iLeft=0; iLeft<iLength; iLeft++ ) {    \
    SUM.v += TAB[iLeft].v;		       \
  }                                            \
  SUM.v /= (double)iLength;                    \
  iFrac = (int)(iLength * lts_fraction + 0.5 ); \
  if ( iFrac == iLength || iFrac <= 1 ) {      \
    resBuf[z*dimxy+y*dimx+x] = SUM.v;          \
    continue;                                  \
  }                                            \
  n = 0;                                       \
  do {                                         \
    SUN = SUM;                                 \
    for ( iLeft=0; iLeft<iLength; iLeft++ )    \
      TAB[iLeft].r = fabs( TAB[iLeft].v - SUM.v ) ; \
    iLeft = 0;   iRight = iLength - 1;				   \
    do {							   \
      j = (iLeft+iRight)/2;					   \
      TMP = TAB[iLeft];   TAB[iLeft] = TAB[j];   TAB[j] = TMP;	   \
      iLast = iLeft;						   \
      for ( i = iLeft+1; i <= iRight; i++ ) {			   \
        if ( TAB[i].r < TAB[iLeft].r ) {			   \
          iLast ++;						   \
          TMP = TAB[i];   TAB[i] = TAB[iLast];   TAB[iLast] = TMP; \
        }							   \
      }								   \
      TMP = TAB[iLeft];   TAB[iLeft] = TAB[iLast]; TAB[iLast] = TMP; \
      if ( iLast >  iFrac ) iRight = iLast - 1;			   \
      if ( iLast <  iFrac ) iLeft  = iLast + 1;			   \
    } while ( iLast != iFrac );					   \
    SUM.v = 0;                                 \
    for ( iLeft=0; iLeft<iFrac; iLeft++ ) {    \
      SUM.v += TAB[iLeft].v;		       \
    }                                          \
    SUM.v /= (double)iFrac;                    \
    n ++;                                      \
  } while ( n<maxiterations && fabs(SUM.v - SUN.v) > acc*fabs(SUM.v) ); \
  resBuf[z*dimxy+y*dimx+x] = SUM.v;	       \
}



#define _QUANTILE_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {  \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_QUANTILE_( TAB, TMP )                    \
    }                                                   \
}



#define _MIN_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {     \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_MIN_( TAB, TMP )                         \
    }                                                   \
}



#define _MAX_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {     \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_MAX_( TAB, TMP )                         \
    }                                                   \
}



#define _MEAN_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {    \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_MEAN_( TAB, TMP )                        \
    }                                                   \
}



#define _SQRSUM_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {  \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_SQRSUM_( TAB, TMP )                      \
    }                                                   \
}



#define _SUM_FILTERING_( ITYPE, OTYPE, TAB, TMP ) {     \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_SUM_( TAB, TMP )                         \
    }                                                   \
}



#define _STDDEV_FILTERING_( ITYPE, OTYPE, TAB, TMP, SQR ) { \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	        \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_STDDEV_( TAB, TMP, SQR )	        	\
    }                                                   \
}



#define _VAR_FILTERING_( ITYPE, OTYPE, TAB, TMP, SQR ) { \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);              \
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_VAR_( TAB, TMP, SQR )			\
    }                                                   \
}


#define _ROBUST_MEAN_FILTERING_( ITYPE, OTYPE, TAB, TMP, S, T ) { \
    ITYPE *theBuf = (ITYPE*)(bufferIn[0]);	   	\
    OTYPE *resBuf = (OTYPE*)bufferAux;		        \
							\
    for ( z = 0; z < dimz; z ++ )	          	\
    for ( y = 0; y < dimy; y ++ )		        \
    for ( x = 0; x < dimx; x ++ ) {                     \
      if ( dimb > 0 ) {                                 \
        _EXTRACT_NEIGHBORHOODS_( ITYPE, TAB )		\
      } else {                                          \
        _EXTRACT_NEIGHBORHOOD_( TAB )                   \
      }                                                 \
      _COMPUTE_ROBUST_MEAN_( TAB, TMP, S, T )		\
    }                                                   \
}





static int localFiltering( void **bufferIn,
			   bufferType typeIn,
			   int dimb,
			   void *bufferOut,
			   bufferType typeOut,
			   int *bufferDims,
			   int *windowDims,
			   enumFiltering operation,
			   double q,
			   double lts_fraction )
{
  char *proc = "localFiltering";
  void *bufferAux = (void*)NULL;
  bufferType typeAux = TYPE_UNKNOWN;
  size_t v = bufferDims[2] * bufferDims[1] * bufferDims[0];

  int wDims[3], nOffsets[3], pOffsets[3];
  int length, quantile;
  int iRight, iLeft, iLast, iQuant, iLength, iFrac;
  
  typer64Point sum, sum2, sumsqr;

  int dimz = bufferDims[2];
  int dimy = bufferDims[1];
  int dimx = bufferDims[0];
  int dimxy = dimx * dimy;
  int b, x, y, z, i, j, k, n;

  int maxiterations = 10;
  double acc = 1e-4;

  typei32Point tmpInt, *tabInt = (typei32Point*)NULL;
  typer32Point tmpFlo, *tabFlo = (typer32Point*)NULL;
  


  /* window dimension and
     offsets
  */
  wDims[0] = windowDims[0];
  wDims[1] = windowDims[1];
  wDims[2] = windowDims[2];
  if ( wDims[0] <= 0 ) wDims[0] = 1; 
  if ( wDims[1] <= 0 ) wDims[1] = 1; 
  if ( wDims[2] <= 0 ) wDims[2] = 1; 
  if ( wDims[0] > bufferDims[0] ) wDims[0] = bufferDims[0];
  if ( wDims[1] > bufferDims[1] ) wDims[1] = bufferDims[1];
  if ( wDims[2] > bufferDims[2] ) wDims[2] = bufferDims[2];
  
  nOffsets[0] = -(int)(wDims[0] / 2); pOffsets[0] = wDims[0] - 1 + nOffsets[0];
  nOffsets[1] = -(int)(wDims[1] / 2); pOffsets[1] = wDims[1] - 1 + nOffsets[1];
  nOffsets[2] = -(int)(wDims[2] / 2); pOffsets[2] = wDims[2] - 1 + nOffsets[2];

  length = dimb * wDims[2] * wDims[1] * wDims[0];
  quantile = (int)((length-1) * q + 0.5);



  /* allocation of an auxiliary buffer ?
   */
  switch ( operation ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such operation not handled yet\n", proc );
    return( -1 );
  case _MAX_ :
  case _MIN_ :
  case _QUANTILE_ :
    typeAux = typeIn;
    break;
  case _MEAN_ :
  case _ROBUST_MEAN_ :
  case _SQRSUM_ :
  case _SUM_ :
  case _STDDEV_ :
  case _VAR_ :
    typeAux = FLOAT;
    break;
  }

  if ( bufferOut == bufferIn || typeOut != typeAux ) {
    switch( typeAux ) {
    default :
      if ( _verbose_ )
	fprintf( stderr, "%s: such image type not handled yet\n", proc );
      return( -1 );
    case UCHAR :
    case SCHAR :
      break;
    case USHORT :
    case SSHORT :
      v *= sizeof( u16 );
      break;
    case UINT :
    case SINT :
      v *= sizeof( u32 );
      break;
    case FLOAT :
      v *= sizeof( r32 );
      break;
    }
    bufferAux = (void*)malloc( v );
    if ( bufferAux == NULL ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      return( -1 );
    }
  }
  else {
    bufferAux = bufferOut;
  }

  

  /* allocation of an auxiliary array
   */
  switch( typeIn ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
  case SINT :
    tabInt = (typei32Point*)malloc( length * sizeof(typei32Point) );
    if ( tabInt == (typei32Point*)NULL ) {
      if ( bufferAux != bufferOut ) free( bufferAux );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      return( -1 );
    }
    break;
  case FLOAT :
    tabFlo = (typer32Point*)malloc( length * sizeof(typer32Point) );
    if ( tabFlo == (typer32Point*)NULL ) {
      if ( bufferAux != bufferOut ) free( bufferAux );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
      return( -1 );
    }
    break;
  }


#define _DEALLOCATIONS_ {                              \
  if ( tabInt != (typei32Point*)NULL ) free( tabInt ); \
  if ( tabFlo != (typer32Point*)NULL ) free( tabFlo ); \
  if ( bufferAux != bufferOut ) free( bufferAux );     \
}
  

  switch ( typeIn ) {

  default :
    _DEALLOCATIONS_;
    if ( _verbose_ )
      fprintf( stderr, "%s: such input image type not handled yet\n", proc );
    return( -1 );

  case UCHAR :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = UCHAR)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case UCHAR :
	_MAX_FILTERING_( u8, u8, tabInt, tmpInt )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case UCHAR :
	_MIN_FILTERING_( u8, u8, tabInt, tmpInt )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case UCHAR :
	_QUANTILE_FILTERING_( u8, u8, tabInt, tmpInt )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( u8, r32, tabInt, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( u8, r32, tabInt, tmpInt, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( u8, r32, tabInt, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( u8, r32, tabInt, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( u8, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( u8, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    }

    break;

  case SCHAR :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = SCHAR)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case SCHAR :
	_MAX_FILTERING_( s8, s8, tabInt, tmpInt )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case SCHAR :
	_MIN_FILTERING_( s8, s8, tabInt, tmpInt )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case SCHAR :
	_QUANTILE_FILTERING_( s8, s8, tabInt, tmpInt )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( s8, r32, tabInt, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( s8, r32, tabInt, tmpInt, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( s8, r32, tabInt, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( s8, r32, tabInt, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( s8, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( s8, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    }

    break;
    
  case USHORT :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = USHORT)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case USHORT :
	_MAX_FILTERING_( u16, u16, tabInt, tmpInt )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case USHORT :
	_MIN_FILTERING_( u16, u16, tabInt, tmpInt )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case USHORT :
	_QUANTILE_FILTERING_( u16, u16, tabInt, tmpInt )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( u16, r32, tabInt, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( u16, r32, tabInt, tmpInt, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( u16, r32, tabInt, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( u16, r32, tabInt, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( u16, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( u16, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    }

    break;

  case SSHORT :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = SSHORT)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case SSHORT :
	_MAX_FILTERING_( s16, s16, tabInt, tmpInt )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case SSHORT :
	_MIN_FILTERING_( s16, s16, tabInt, tmpInt )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case SSHORT :
	_QUANTILE_FILTERING_( s16, s16, tabInt, tmpInt )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( s16, r32, tabInt, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( s16, r32, tabInt, tmpInt, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( s16, r32, tabInt, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( s16, r32, tabInt, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( s16, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( s16, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    }

    break;
    
  case SINT :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = INT)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case SINT :
	_MAX_FILTERING_( s32, s32, tabInt, tmpInt )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case SINT :
	_MIN_FILTERING_( s32, s32, tabInt, tmpInt )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case SINT :
	_QUANTILE_FILTERING_( s32, s32, tabInt, tmpInt )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( s32, r32, tabInt, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( s32, r32, tabInt, tmpInt, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( s32, r32, tabInt, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( s32, r32, tabInt, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( s32, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( s32, r32, tabInt, sum, sumsqr )
	break;
      }
      break;
    }

    break;
    
  case FLOAT :
    
    switch ( operation ) {
    default :
      _DEALLOCATIONS_;
      if ( _verbose_ )
	fprintf( stderr, "%s: such operation not handled yet (typeIn = FLOAT)\n", proc );
      return( -1 );
    case _MAX_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (max)\n", proc );
	return( -1 );
      case FLOAT :
	_MAX_FILTERING_( r32, r32, tabFlo, tmpFlo )
	break;
      }
      break;
    case _MIN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (min)\n", proc );
	return( -1 );
      case FLOAT :
	_MIN_FILTERING_( r32, r32, tabFlo, tmpFlo )
	break;
      }
      break;
    case _QUANTILE_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (quantile)\n", proc );
	return( -1 );
      case FLOAT :
	_QUANTILE_FILTERING_( r32, r32, tabFlo, tmpFlo )
	break;
      }
      break;
    case _MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (mean)\n", proc );
	return( -1 );
      case FLOAT :
	_MEAN_FILTERING_( r32, r32, tabFlo, sum )
	break;
      }
      break;
    case _ROBUST_MEAN_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (robust mean)\n", proc );
	return( -1 );
      case FLOAT :
	_ROBUST_MEAN_FILTERING_( r32, r32, tabFlo, tmpFlo, sum, sum2 )
	break;
      }
      break;
    case _SQRSUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SQRSUM_FILTERING_( r32, r32, tabFlo, sum )
	break;
      }
      break;
    case _SUM_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (sum)\n", proc );
	return( -1 );
      case FLOAT :
	_SUM_FILTERING_( r32, r32, tabFlo, sum )
	break;
      }
      break;
    case _STDDEV_ :
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (std dev)\n", proc );
	return( -1 );
      case FLOAT :
	_STDDEV_FILTERING_( r32, r32, tabFlo, sum, sumsqr )
	break;
      }
      break;
    case _VAR_ : 
      switch ( typeAux ) {
      default :
	_DEALLOCATIONS_;
	if ( _verbose_ ) fprintf( stderr, "%s: such output image type not handled yet (var)\n", proc );
	return( -1 );
      case FLOAT :
	_VAR_FILTERING_( r32, r32, tabFlo, sum, sumsqr )
	break;
      }
      break;
    }

    break;

  }



  /* conversion (if required)
   */
  if ( bufferAux != bufferOut ) {
    if ( ConvertBuffer( bufferAux, typeAux, bufferOut, typeOut, 
			bufferDims[2] * bufferDims[1] * bufferDims[0] ) != 1 ) {
      _DEALLOCATIONS_;
      if ( _verbose_ ) fprintf( stderr, "%s: unable to convert buffer\n", proc );
      return( -1 );
    }
  }

  _DEALLOCATIONS_;

  return( 1 );
}











/**************************************************
 *
 * operation on one buffer
 *
 **************************************************/

int medianFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _QUANTILE_, 0.5, 1.0 ) );
}





int quantileFiltering( void *bufferIn,
		       bufferType typeIn,
		       void *bufferOut,
		       bufferType typeOut,
		       int *bufferDims,
		       int *windowDims,
		       double quantile )
{
  char *proc = "quantileFiltering";

  if ( quantile < 0 || quantile > 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: quantile value have to be in [0,1]\n", proc );
    return( -1 );
  }
    
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _QUANTILE_, quantile, 1.0 ) );
}





int minFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MIN_, 0.5, 1.0 ) );
}





int maxFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MAX_, 0.5, 1.0 ) );
}



int meanFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MEAN_, 0.5, 1.0 ) );
}



int sumFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _SUM_, 0.5, 1.0 ) );
}



int sumSquaresFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _SQRSUM_, 0.5, 1.0 ) );
}



int stddevFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _STDDEV_, 0.5, 1.0 ) );
}



int varFiltering( void *bufferIn,
		     bufferType typeIn,
		     void *bufferOut,
		     bufferType typeOut,
		     int *bufferDims,
		     int *windowDims )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _VAR_, 0.5, 1.0 ) );
}



int robustMeanFiltering( void *bufferIn,
			 bufferType typeIn,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims,
			 double lts_fraction )
{
  return( localFiltering( &bufferIn, typeIn, 1,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _ROBUST_MEAN_, 0.5, lts_fraction ) );
}






/**************************************************
 *
 * operation on an array of buffer
 *
 **************************************************/

int medianFilteringBuffers( void **bufferIn,
			    bufferType typeIn,
			    int nBuffers,
			    void *bufferOut,
			    bufferType typeOut,
			    int *bufferDims,
			    int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _QUANTILE_, 0.5, 1.0 ) );
}





int quantileFilteringBuffers( void **bufferIn,
			      bufferType typeIn,
			      int nBuffers,
			      void *bufferOut,
			      bufferType typeOut,
			      int *bufferDims,
			      int *windowDims,
			      double quantile )
{
  char *proc = "quantileFiltering";

  if ( quantile < 0 || quantile > 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: quantile value have to be in [0,1]\n", proc );
    return( -1 );
  }
    
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _QUANTILE_, quantile, 1.0 ) );
}





int minFilteringBuffers( void **bufferIn,
			 bufferType typeIn,
			 int nBuffers,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MIN_, 0.5, 1.0 ) );
}





int maxFilteringBuffers( void **bufferIn,
			 bufferType typeIn,
			 int nBuffers,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MAX_, 0.5, 1.0 ) );
}



int meanFilteringBuffers( void **bufferIn,
			  bufferType typeIn,
			  int nBuffers,
			  void *bufferOut,
			  bufferType typeOut,
			  int *bufferDims,
			  int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _MEAN_, 0.5, 1.0 ) );
}



int sumFilteringBuffers( void **bufferIn,
			 bufferType typeIn,
			 int nBuffers,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _SUM_, 0.5, 1.0 ) );
}



int sumSquaresFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _SQRSUM_, 0.5, 1.0 ) );
}



int stddevFilteringBuffers( void **bufferIn,
			    bufferType typeIn,
			    int nBuffers,
			    void *bufferOut,
			    bufferType typeOut,
			    int *bufferDims,
			    int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _STDDEV_, 0.5, 1.0 ) );
}



int varFilteringBuffers( void **bufferIn,
			 bufferType typeIn,
			 int nBuffers,
			 void *bufferOut,
			 bufferType typeOut,
			 int *bufferDims,
			 int *windowDims )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _VAR_, 0.5, 1.0 ) );
}



int robustMeanFilteringBuffers( void **bufferIn,
				bufferType typeIn,
				int nBuffers,
				void *bufferOut,
				bufferType typeOut,
				int *bufferDims,
				int *windowDims,
				double lts_fraction )
{
  return( localFiltering( bufferIn, typeIn, nBuffers,
			  bufferOut, typeOut,
			  bufferDims, windowDims, 
			  _ROBUST_MEAN_, 0.5, lts_fraction ) );
}
