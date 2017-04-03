/*************************************************************************
 * linearFiltering-laplacian.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Dec 26 22:19:17 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */



/* WARNING, this file is not aimed to be computed
 * it is included from linearFiltering.c
 */



int laplacian2D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "laplacian2D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *auxBuf = NULL;
  float *tmpBuf = NULL;
  float *lapBuf = NULL;
  long int i;

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];

  sizeAuxBuf = dimx*dimy*dimz;
  if ( typeOut != FLOAT || bufferIn == bufferOut )
    sizeAuxBuf *= 2;


  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }
  tmpBuf = auxBuf;
  if ( typeOut != FLOAT || bufferIn == bufferOut ) {
    lapBuf  = tmpBuf;
    lapBuf += dimx*dimy*dimz;
  } else {
    lapBuf  = (float*)bufferOut;
  }



  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];
    
  /* derivative along X 
   */
  filter[0].derivative = DERIVATIVE_2;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)lapBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X derivative (2D)\n", proc );
    free( auxBuf );
    return( -1 );
  }
  
  /* derivative along Y 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_2;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)tmpBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Y derivative (2D)\n", proc );
    free( auxBuf );
    return( -1 );
  }



  sizeAuxBuf = dimx*dimy*dimz;
#ifdef _OPENMP
#pragma omp parallel for 
#endif 
  for ( i = 0; i < (long int)sizeAuxBuf; i++ )
    lapBuf[i] += tmpBuf[i];

  

  if ( lapBuf != bufferOut ) {
    if ( ConvertBuffer( lapBuf, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( auxBuf );
      return( -1 );
    }
  }

  free( auxBuf );

  return( 1 );
}





int laplacian3D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "laplacian3D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *auxBuf = NULL;
  float *tmpBuf = NULL;
  float *lapBuf = NULL;
  long int i;

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];

  sizeAuxBuf = dimx*dimy*dimz;
  if ( typeOut != FLOAT || bufferIn == bufferOut )
    sizeAuxBuf *= 2;


  /* allocation des buffers de calcul
   */
  auxBuf = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( auxBuf == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }
  tmpBuf = auxBuf;
  if ( typeOut != FLOAT || bufferIn == bufferOut ) {
    lapBuf  = tmpBuf;
    lapBuf += dimx*dimy*dimz;
  } else {
    lapBuf  = (float*)bufferOut;
  }



  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];
  
  /* smoothing along Z 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = DERIVATIVE_0;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)tmpBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Z smoothing (3D)\n", proc );
    free( auxBuf );
    return( -1 );
  }
  
  /* derivative along X 
   */
  filter[0].derivative = DERIVATIVE_2;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)tmpBuf, FLOAT, (void*)lapBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X derivative (3D)\n", proc );
    free( auxBuf );
    return( -1 );
  }
  
  /* derivative along Y 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_2;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)tmpBuf, FLOAT, (void*)tmpBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Y derivative (3D)\n", proc );
    free( auxBuf );
    return( -1 );
  }



  sizeAuxBuf = dimx*dimy*dimz;
#ifdef _OPENMP
#pragma omp parallel for 
#endif 
  for ( i = 0; i < (long int)sizeAuxBuf; i++ )
    lapBuf[i] += tmpBuf[i];



  /* derivative along Z 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = DERIVATIVE_2;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)tmpBuf, FLOAT,
                                  bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Z derivative (3D)\n", proc );
    free( auxBuf );
    return( -1 );
  }



#ifdef _OPENMP
#pragma omp parallel for 
#endif 
  for ( i = 0; i < (long int)sizeAuxBuf; i++ )
    lapBuf[i] += tmpBuf[i];

  

  if ( lapBuf != bufferOut ) {
    if ( ConvertBuffer( lapBuf, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( auxBuf );
      return( -1 );
    }
  }

  free( auxBuf );

  return( 1 );
}





int laplacian( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( laplacian2D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
  else
    return( laplacian3D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
}
