/*************************************************************************
 * linearFiltering.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Nov 28 11:49:01 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convert.h>
#include <chunks.h>

#include <convolution1D.h>
#include <recline.h>

#include <linearFiltering.h>



static int _verbose_ = 1;
static int _debug_ = 0;



void setVerboseInLinearFiltering( int v )
{
  _verbose_ = v;
}

void incrementVerboseInLinearFiltering(  )
{
  _verbose_ ++;
}

void decrementVerboseInLinearFiltering(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}

void setDebugInLinearFiltering( int v )
{
  _debug_ = v;
}

void incrementDebugInLinearFiltering(  )
{
  _debug_ ++;
}

void decrementDebugInLinearFiltering(  )
{
  _debug_ --;
  if ( _debug_ < 0 ) _debug_ = 0;
}



/* structure for parallelism
 */

typedef struct typeLinearFilter {

  /* images
   */

  void *bufferIn;
  bufferType typeIn;
  void *bufferOut;
  bufferType typeOut;

  size_t dimx;
  size_t dimy;
  size_t dimz;


  /* filter
   */
  typeFilteringCoefficients filter;


  /* processing
   */

  int borderLength;

} typeLinearFilter;

/*
 *
 * 
 *
 *
 */

static void _initTypeLinearFilter( typeLinearFilter *p )
{
  p->bufferIn = (void*)NULL;
  p->typeIn = TYPE_UNKNOWN;
  p->bufferOut = (void*)NULL;
  p->typeOut = TYPE_UNKNOWN;

  p->dimx = 0;
  p->dimy = 0;
  p->dimz = 0;

  initFilteringCoefficients( &(p->filter) );

  p->borderLength = 0;
}



static int _testFilteringCoefficients( int dim, int border, typeFilteringCoefficients *c )
{
  char *proc = "testFilteringCoefficients";

  if ( c->parameters == NULL ) {
    if ( _verbose_ >= 2 ) {
      fprintf( stderr, "%s: NULL parameters\n", proc );
    }
    return( 0 );
  }

  switch( c->type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ >= 2 )
      fprintf( stderr, "%s: unknown filter\n", proc );
    return( 0 );
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    if ( dim <= 4 ) {
      if ( _verbose_ >= 2 )
        fprintf( stderr, "%s: too small dimension\n", proc );
      return( 0 );
    }
    break;
  case GAUSSIAN_CONVOLUTION :
    if ( dim + 2 * border < ((type1DConvolutionMask*)(c->parameters))->length ) {
      if ( _verbose_ >= 2 ) {
        fprintf( stderr, "%s: too small dimension\n", proc );
        fprintf( stderr, "\t dimension = %d, border = %d, maks length = %d\n",
                 dim, border, ((type1DConvolutionMask*)(c->parameters))->length );
      }
      return( 0 );
    }
  }
  
  return( 1 );

}





/************************************************************
 *
 * 
 *
 ************************************************************/

#include "linearFiltering-gradient.c"
#include "linearFiltering-hessian.c"
#include "linearFiltering-laplacian.c"



/************************************************************
 *
 * filtering stuff
 * here, we find the 3 directional (along X, Y and Z) filtering
 *
 ************************************************************/

static void *_linearFilteringAlongX( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  char *proc = "_linearFilteringAlongX";
  typeLinearFilter *p = (typeLinearFilter*)parameter;
  size_t i, k;
  int x, y, z;
  int dimx = p->dimx;
  int dimy = p->dimy;
  int length;
  double *allocLine, *theLine, *resLine, *auxLine;

  type1DConvolutionMask *mask = NULL;
  RFcoefficientType *RFC = NULL;

  length = dimx + 2 * p->borderLength;
  allocLine = (double*)malloc( 3 * length * sizeof(double) );
  if ( allocLine == (double*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    chunk->ret = 0;
    return( (void*)NULL );

  }
  theLine = allocLine;
  resLine = theLine; resLine += length;
  auxLine = resLine; auxLine += length;

  switch ( p->filter.type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ )
      fprintf( stderr, "%s: filter type not handled yet\n", proc );
    free( allocLine );
    chunk->ret = 0;
    return( (void*)NULL );
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    RFC = (RFcoefficientType *)p->filter.parameters;
    break;
  case GAUSSIAN_CONVOLUTION :
    mask = (type1DConvolutionMask*)p->filter.parameters;
    break;
  }

  
  /* first and last represented the range of indices
     i=z*dimy+y in [0 dimy*dimz-1]
  */

  z = first / dimy;
  y = first % dimy;
  for ( i=first; i<=last; z++, y=0 )
  for ( ; i<=last && y<dimy; y++, i++ ) {

    /* acquiring a line
     */
    switch ( p->typeIn ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such input image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );
      
    case FLOAT :
      { 
      float *theBuf = (float*)p->bufferIn;
      for ( k=(z*dimy+y)*dimx, x=0; x<dimx; k++, x++ )
        theLine[p->borderLength + x] = theBuf[ k ];
      }
      break;
    }
    
    for ( x=0; x<p->borderLength; x++ ) {
      theLine[x] = theLine[ p->borderLength ];
      theLine[p->borderLength + dimx+x] = theLine[p->borderLength + dimx-1];
    }
    
    /* processing the line
     */
    switch ( p->filter.type ) {
    default :
    case UNKNOWN_FILTER :
    case ALPHA_DERICHE :
    case GAUSSIAN_DERICHE :
    case GAUSSIAN_FIDRICH :
    case GAUSSIAN_YOUNG_1995 :
    case GAUSSIAN_YOUNG_2002 :
    case GABOR_YOUNG_2002 :
      if ( RecursiveFilter1D( RFC, theLine, resLine, auxLine, resLine, length ) == 0 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when computing recursive filtering\n", proc );
        free( allocLine );
        chunk->ret = 0;
        return( (void*)NULL );
      }
      break;
    case GAUSSIAN_CONVOLUTION :
      _compute1DDoubleConvolution( theLine, resLine, length,
                                   mask->data, mask->halflength );
      break;
    }
    
    /* copying the line
     */
    switch ( p->typeOut ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );
      
    case FLOAT :
      {
        float *resBuf = (float*)p->bufferOut;
        for ( k=(z*dimy+y)*dimx, x=0; x<dimx; k++, x++ )
          resBuf[ k ] = resLine[p->borderLength + x];
      }
      break;
    }
    
  }

  free( allocLine );
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_linearFilteringAlongY( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  char *proc = "_linearFilteringAlongY";
  typeLinearFilter *p = (typeLinearFilter*)parameter;
  size_t i, k;
  int x, y, z;
  int dimx = p->dimx;
  int dimy = p->dimy;
  int length;
  double *allocLine, *theLine, *resLine, *auxLine;

  type1DConvolutionMask *mask = NULL;
  RFcoefficientType *RFC = NULL;

  length = dimy + 2 * p->borderLength;
  allocLine = (double*)malloc( 3 * length * sizeof(double) );
  if ( allocLine == (double*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    chunk->ret = 0;
    return( (void*)NULL );
  }
  theLine = allocLine;
  resLine = theLine; resLine += length;
  auxLine = resLine; auxLine += length;

  switch ( p->filter.type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ )
      fprintf( stderr, "%s: filter type not handled yet\n", proc );
    free( allocLine );
    chunk->ret = 0;
    return( (void*)NULL );
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    RFC = (RFcoefficientType *)p->filter.parameters;
    break;
  case GAUSSIAN_CONVOLUTION :
    mask = (type1DConvolutionMask*)p->filter.parameters;
    break;
  }

  
  /* first and last represented the range of indices
     i=z*dimx+x in [0 dimx*dimz-1]
  */

  z = first / dimx;
  x = first % dimx;
  for ( i=first; i<=last; z++, x=0 )
  for ( ; i<=last && x<dimx; x++, i++ ) {

    /* acquiring a line
     */
    switch ( p->typeIn ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such input image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );
      
    case FLOAT :
      { 
      float *theBuf = (float*)p->bufferIn;
      for ( k=(z*dimy*dimx)+x, y=0; y<dimy; k+=dimx, y++ )
        theLine[p->borderLength + y] = theBuf[ k ];
      }
      break;
    }
 
    for ( y=0; y<p->borderLength; y++ ) {
      theLine[y] = theLine[ p->borderLength ];
      theLine[p->borderLength + dimy+y] = theLine[p->borderLength + dimy-1];
    }

    /* processing the line
     */
    switch ( p->filter.type ) {
    default :
    case UNKNOWN_FILTER :
    case ALPHA_DERICHE :
    case GAUSSIAN_DERICHE :
    case GAUSSIAN_FIDRICH :
    case GAUSSIAN_YOUNG_1995 :
    case GAUSSIAN_YOUNG_2002 :
    case GABOR_YOUNG_2002 :
      if ( RecursiveFilter1D( RFC, theLine, resLine, auxLine, resLine, length ) == 0 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when computing recursive filtering\n", proc );
        free( allocLine );
        chunk->ret = 0;
        return( (void*)NULL );
      }
      break;
    case GAUSSIAN_CONVOLUTION :
      _compute1DDoubleConvolution( theLine, resLine, length,
                                   mask->data, mask->halflength );
      break;
    }

    /* copying the line
     */
    switch ( p->typeOut ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );

    case FLOAT :
      {
        float *resBuf = (float*)p->bufferOut;
        for ( k=(z*dimy*dimx)+x, y=0; y<dimy; k+=dimx, y++ )
          resBuf[ k ] = resLine[p->borderLength + y];
      }
      break;
    }

  }
  
  free( allocLine );
  chunk->ret = 1;
  return( (void*)NULL );
}





static void *_linearFilteringAlongZ( void *par )
{
  typeChunk *chunk = (typeChunk *)par;
  void *parameter = chunk->parameters;
  size_t first = chunk->first;
  size_t last = chunk->last;

  char *proc = "_linearFilteringAlongZ";
  typeLinearFilter *p = (typeLinearFilter*)parameter;
  size_t i, k;
  int x, y, z;
  int dimx = p->dimx;
  int dimy = p->dimy;
  int dimz = p->dimz;
  int length;
  double *allocLine, *theLine, *resLine, *auxLine;

  type1DConvolutionMask *mask = NULL;
  RFcoefficientType *RFC = NULL;

  length = dimz + 2 * p->borderLength;
  allocLine = (double*)malloc( 3 * length * sizeof(double) );
  if ( allocLine == (double*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    chunk->ret = 0;
    return( (void*)NULL );
  }
  theLine = allocLine;
  resLine = theLine; resLine += length;
  auxLine = resLine; auxLine += length;

  switch ( p->filter.type ) {
  default :
  case UNKNOWN_FILTER :
    if ( _verbose_ )
      fprintf( stderr, "%s: filter type not handled yet\n", proc );
    free( allocLine );
    chunk->ret = 0;
    return( (void*)NULL );
  case ALPHA_DERICHE :
  case GAUSSIAN_DERICHE :
  case GAUSSIAN_FIDRICH :
  case GAUSSIAN_YOUNG_1995 :
  case GAUSSIAN_YOUNG_2002 :
  case GABOR_YOUNG_2002 :
    RFC = (RFcoefficientType *)p->filter.parameters;
    break;
  case GAUSSIAN_CONVOLUTION :
    mask = (type1DConvolutionMask*)p->filter.parameters;
    break;
  }

  
  /* first and last represented the range of indices
     i=y*dimx+x in [0 dimx*dimy-1]
  */
  y = first / dimx;
  x = first % dimx;
  for ( i=first; i<=last; y++, x=0 )
  for ( ; i<=last && x<dimx; x++, i++ ) {

    /* acquiring a line
     */
    switch ( p->typeIn ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such input image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );
      
    case FLOAT :
      { 
      float *theBuf = (float*)p->bufferIn;
      for ( k=y*dimx+x, z=0; z<dimz; k+=dimx*dimy, z++ )
        theLine[p->borderLength + z] = theBuf[ k ];
      }
      break;
    }
    
    for ( z=0; z<p->borderLength; z++ ) {
      theLine[z] = theLine[ p->borderLength ];
      theLine[p->borderLength + dimz+z] = theLine[p->borderLength + dimz-1];
    }
    
    /* processing the line
     */
    switch ( p->filter.type ) {
    default :
    case UNKNOWN_FILTER :
    case ALPHA_DERICHE :
    case GAUSSIAN_DERICHE :
    case GAUSSIAN_FIDRICH :
    case GAUSSIAN_YOUNG_1995 :
    case GAUSSIAN_YOUNG_2002 :
    case GABOR_YOUNG_2002 :
      if ( RecursiveFilter1D( RFC, theLine, resLine, auxLine, resLine, length ) == 0 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when computing recursive filtering\n", proc );
        free( allocLine );
        chunk->ret = 0;
        return( (void*)NULL );
      }
      break;
    case GAUSSIAN_CONVOLUTION :
      _compute1DDoubleConvolution( theLine, resLine, length,
                                   mask->data, mask->halflength );
      break;
    }

    /* copying the line
     */
    switch ( p->typeOut ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: such output image type not handled yet\n", proc );
      free( allocLine );
      chunk->ret = 0;
      return( (void*)NULL );

    case FLOAT :
      {
        float *resBuf = (float*)p->bufferOut;
        for ( k=y*dimx+x, z=0; z<dimz; k+=dimx*dimy, z++ )
          resBuf[ k ] = resLine[p->borderLength + z];
      }
      break;
    }

  }
  
  free( allocLine );
  chunk->ret = 1;
  return( (void*)NULL );
}





/*
 *
 * 
 *
 *
 */

int separableLinearFiltering( void *bufferIn,
                              bufferType typeIn,
                              void *bufferOut,
                              bufferType typeOut,
                              int *bufferDims,
                              int *borderLengths,
                              typeFilteringCoefficients *filter )
{
  char *proc = "separableLinearFiltering";
  size_t dimx, dimy, dimz;

  void *bufferToBeProcessed = (void*)NULL;
  bufferType typeToBeProcessed = TYPE_UNKNOWN;
  void *bufferResult = (void*)NULL;
  bufferType typeResult = TYPE_UNKNOWN;

  typeChunks chunks;
  size_t first, last;
  typeLinearFilter parameters;
  int n;


  



  /* 
   * We check the buffers' dimensions.
   */
  dimx = bufferDims[0];   dimy = bufferDims[1];   dimz = bufferDims[2];
  if ( (dimx <= 0) || (dimy <= 0) || (dimz <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }



  /************************************************************* 
   * obviously, we need to perform the computation 
   * with float or double values. For this reason,
   * we allocate an auxiliary buffer if the output buffer
   * is not of type float or double.
   */

  /*
   * We check the pointers.
   */
  if ( (bufferIn == (void*)NULL) || (bufferOut == (void*)NULL) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: NULL pointer on buffer.\n", proc );
    return( -1 );
  }

  /* 
   * May we use the buffer bufferOut as the bufferResult?
   * If its type is FLOAT or DOUBLE, then yes.
   * If not, we have to allocate an auxiliary buffer.
   */
  if ( (typeOut == FLOAT) || (typeOut == DOUBLE) ) {
    bufferResult = bufferOut;
    typeResult = typeOut;
  } else {
    bufferResult = (void*)malloc( (dimx*dimy*dimz) * sizeof(r32) );
    if ( bufferResult == (void*)NULL ) {
      if ( _verbose_ > 0 )
        fprintf( stderr, " Fatal error in %s: unable to allocate auxiliary buffer.\n", proc );
      return( -1 );
    }
    typeResult = FLOAT;
  }
  
  /* 
   * May we consider the buffer bufferIn as the bufferToBeProcessed?
   * If its type is FLOAT or DOUBLE, then yes.
   * If not, we convert it into the buffer bufferResult, and this
   * last buffer is the bufferToBeProcessed.
   */

  if ( (typeIn == FLOAT) || (typeIn == DOUBLE) ) {
    bufferToBeProcessed = bufferIn;
    typeToBeProcessed = typeIn;
  } else {
    if ( ConvertBuffer( bufferIn, typeIn, bufferResult, typeResult, (dimx*dimy*dimz) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }
    bufferToBeProcessed = bufferResult;
    typeToBeProcessed = typeResult;
  }



  _initTypeLinearFilter( &parameters );
  parameters.dimx = dimx;
  parameters.dimy = dimy;
  parameters.dimz = dimz;



  /************************************************************
   *
   * processing along X
   *
   ************************************************************/
  
  if ( dimx > 1 
       && filter[0].type != UNKNOWN_FILTER 
       && filter[0].derivative != NODERIVATIVE
       && filter[0].coefficient > 0.0 ) {
    
    parameters.bufferIn = bufferToBeProcessed;
    parameters.typeIn = typeToBeProcessed;
    parameters.bufferOut = bufferResult;
    parameters.typeOut = typeResult;
    parameters.borderLength = borderLengths[0];
    parameters.filter = filter[0];

    
    
    if ( buildFilteringCoefficients( &(parameters.filter) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when building filter\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }

    if ( _debug_ )
      printFilteringCoefficients( stderr, &(parameters.filter), "filter coefficients along X" );

    if ( _testFilteringCoefficients( dimx, borderLengths[0], &(parameters.filter) ) != 1 ) {
      
      if ( _verbose_ >=2 )
        fprintf( stderr, "%s: tests failed for filtering along X\n", proc );

    }
    else {

      first = 0;
      last = dimy*dimz-1;
      
      initChunks( &chunks );
      if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when building chunks\n", proc );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }
      
      for ( n=0; n<chunks.n_allocated_chunks; n++ ) 
        chunks.data[n].parameters = (void*)(&parameters);
      /* ... */
      
      if ( processChunks( &_linearFilteringAlongX, &chunks, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute X filtering\n", proc );
        freeChunks( &chunks );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }
      
      freeChunks( &chunks );
      bufferToBeProcessed = bufferResult;
      typeToBeProcessed = typeResult;
    }

    freeFilteringCoefficients( &(parameters.filter) );

  }


  /************************************************************
   *
   * processing along Y
   *
   ************************************************************/
  if ( dimy > 1 
       && filter[1].type != UNKNOWN_FILTER 
       && filter[1].derivative != NODERIVATIVE
       && filter[1].coefficient > 0.0 ) {

    parameters.bufferIn = bufferToBeProcessed;
    parameters.typeIn = typeToBeProcessed;
    parameters.bufferOut = bufferResult;
    parameters.typeOut = typeResult;
    parameters.borderLength = borderLengths[1];
    parameters.filter = filter[1];

    
    
    if ( buildFilteringCoefficients( &(parameters.filter) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when building filter\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }

    if ( _debug_ )
      printFilteringCoefficients( stderr, &(parameters.filter), "filter coefficients along Y" );

    if ( _testFilteringCoefficients( dimy, borderLengths[1], &(parameters.filter) ) != 1 ) {
      
      if ( _verbose_ >=2 )
        fprintf( stderr, "%s: tests failed for filtering along Y\n", proc );

    }
    else {

      first = 0;
      last = dimx*dimz-1;

      initChunks( &chunks );
      if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when building chunks\n", proc );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }
  
      for ( n=0; n<chunks.n_allocated_chunks; n++ ) 
        chunks.data[n].parameters = (void*)(&parameters);
      /* ... */
    
      if ( processChunks( &_linearFilteringAlongY, &chunks, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute Y filtering\n", proc );
        freeChunks( &chunks );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }

      freeChunks( &chunks );
      bufferToBeProcessed = bufferResult;
      typeToBeProcessed = typeResult;
    }

    freeFilteringCoefficients( &(parameters.filter) );

  }



  /************************************************************
   *
   * processing along Z
   *
   ************************************************************/
  if ( dimz > 1 
       && filter[2].type != UNKNOWN_FILTER 
       && filter[2].derivative != NODERIVATIVE
       && filter[2].coefficient > 0.0 ) {

    parameters.bufferIn = bufferToBeProcessed;
    parameters.typeIn = typeToBeProcessed;
    parameters.bufferOut = bufferResult;
    parameters.typeOut = typeResult;
    parameters.borderLength = borderLengths[2];
    parameters.filter = filter[2];

    
    
    if ( buildFilteringCoefficients( &(parameters.filter) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when building filter\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }

    if ( _debug_ )
      printFilteringCoefficients( stderr, &(parameters.filter), "filter coefficients along Z" );

    if ( _testFilteringCoefficients( dimz, borderLengths[2], &(parameters.filter) ) != 1 ) {
      
      if ( _verbose_ >=2 )
        fprintf( stderr, "%s: tests failed for filtering along Z\n", proc );

    }
    else {

      first = 0;
      last = dimx*dimy-1;

      initChunks( &chunks );
      if ( buildChunks( &chunks, first, last, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: error when building chunks\n", proc );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }
      
      for ( n=0; n<chunks.n_allocated_chunks; n++ ) 
        chunks.data[n].parameters = (void*)(&parameters);
      /* ... */
    
      if ( processChunks( &_linearFilteringAlongZ, &chunks, proc ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to compute Z filtering\n", proc );
        freeFilteringCoefficients( &(parameters.filter) );
        freeChunks( &chunks );
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        return( -1 );
      }
      
      freeChunks( &chunks );
    }

    freeFilteringCoefficients( &(parameters.filter) );

  }



  /************************************************************
   *
   * processings have been done
   *
   ************************************************************/

  /* case where (typeIn == FLOAT) || (typeIn == DOUBLE)
     bufferIn has not been converted into bufferResult
  */
  if ( bufferIn == bufferToBeProcessed && bufferIn != bufferOut ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: no filtering was done, copy the input buffer\n", proc );
    }
    if ( ConvertBuffer( bufferIn, typeIn, bufferOut, typeOut, (dimx*dimy*dimz) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }
  }
  else {
    if ( ConvertBuffer( bufferResult, typeResult, bufferOut, typeOut, (dimx*dimy*dimz) ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( -1 );
    }
  }

  /*
   * Releasing the buffers.
   */
  if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
    free( bufferResult );
  
  return( 1 );
}







