/*************************************************************************
 * linearFiltering-gradient.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Dec 26 18:28:04 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */



/* WARNING, this file is not aimed to be computed
 * it is included from linearFiltering.c
 */



int gradientModulus2D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientModulus2D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *auxBuf = NULL;
  float *tmpBuf = NULL;
  float *grdBuf = NULL;
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
    grdBuf  = tmpBuf;
    grdBuf += dimx*dimy*dimz;
  } else {
    grdBuf  = (float*)bufferOut;
  }



  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];
    
  /* derivative along X 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)grdBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X derivative (2D)\n", proc );
    free( auxBuf );
    return( -1 );
  }
  
  /* derivative along Y 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_1;
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
    grdBuf[i] = (float)sqrt( grdBuf[i]*grdBuf[i] + tmpBuf[i]*tmpBuf[i] );
  
  

  if ( grdBuf != bufferOut ) {
    if ( ConvertBuffer( grdBuf, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( auxBuf );
      return( -1 );
    }
  }

  free( auxBuf );

  return( 1 );
}





int gradientModulus3D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientModulus3D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *auxBuf = NULL;
  float *tmpBuf = NULL;
  float *grdBuf = NULL;
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
    grdBuf  = tmpBuf;
    grdBuf += dimx*dimy*dimz;
  } else {
    grdBuf  = (float*)bufferOut;
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
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)tmpBuf, FLOAT, (void*)grdBuf, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X derivative (3D)\n", proc );
    free( auxBuf );
    return( -1 );
  }
  
  /* derivative along Y 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_1;
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
    grdBuf[i] = grdBuf[i]*grdBuf[i] + tmpBuf[i]*tmpBuf[i];



  /* derivative along Z 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = DERIVATIVE_1;
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
    grdBuf[i] = (float)sqrt( grdBuf[i] + tmpBuf[i]*tmpBuf[i] );

  

  if ( grdBuf != bufferOut ) {
    if ( ConvertBuffer( grdBuf, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( auxBuf );
      return( -1 );
    }
  }

  free( auxBuf );

  return( 1 );
}





int gradientModulus( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( gradientModulus2D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
  else
    return( gradientModulus3D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
}
