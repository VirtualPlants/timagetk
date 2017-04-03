/*************************************************************************
 * linearFiltering-hessian.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Dec 26 22:52:20 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 */



/* WARNING, this file is not aimed to be computed
 * it is included from linearFiltering.c
 */



int gradientHessianGradient2D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientHessianGradient2D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *theXX = NULL;
  float *theYY = NULL;
  float *theXY = NULL;
  float *theX  = NULL;
  float *theY  = NULL;
  float *theH  = NULL;
  double g;
  long int i;

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];


  /* we could spare one buffer,
     but who cares?
  */
  sizeAuxBuf = (size_t)5 * dimx*dimy*dimz;
  if ( typeOut != FLOAT || bufferIn == bufferOut )
    sizeAuxBuf += dimx*dimy*dimz;


  /* allocation des buffers de calcul
   */
  theXX = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( theXX == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  sizeAuxBuf = dimx*dimy*dimz;
  theYY = theXY = theX = theY = theH = theXX;
  theYY +=   sizeAuxBuf;
  theXY += 2*sizeAuxBuf;
  theX  += 3*sizeAuxBuf;
  theY  += 4*sizeAuxBuf;
  
  if ( typeOut != FLOAT || bufferIn == bufferOut ) {
    theH  += 5*sizeAuxBuf;
  } else {
    theH  = (float*)bufferOut;
  }



  /* filtering
   */
  filter[0] = theFilter[0];
  filter[1] = theFilter[1];
  filter[2] = theFilter[2];
    
  /* smoothing along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theXX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Y smoothing (2D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 1st derivative along X 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXX, FLOAT, (void*)theX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X 1st derivative (2D)\n", proc );
    free( theXX );
    return( -1 );
  }
  
  /* 2nd derivative along X 
   */
  filter[0].derivative = DERIVATIVE_2;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXX, FLOAT, (void*)theXX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X 2nd derivative (2D)\n", proc );
    free( theXX );
    return( -1 );
  }


   
  /* smoothing along X 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theYY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute X smoothing (2D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 1st derivative along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_1;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYY, FLOAT, (void*)theY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Y 1st derivative (2D)\n", proc );
    free( theXX );
    return( -1 );
  }
  
  /* 2nd derivative along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_2;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYY, FLOAT, (void*)theYY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute Y 2nd derivative (2D)\n", proc );
    free( theXX );
    return( -1 );
  }



  /* 2nd derivative along X and Y 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = DERIVATIVE_1;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theXY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute XY 2nd derivative (2D)\n", proc );
    free( theXX );
    return( -1 );
  }



  sizeAuxBuf = dimx*dimy*dimz;
#ifdef _OPENMP
#pragma omp parallel for private( g )
#endif 
  for ( i = 0; i < (long int)sizeAuxBuf; i++ ) {
    theH[i] = theX[i] * ( theXX[i] * theX[i] + theXY[i] * theY[i] ) 
      + theY[i] * ( theXY[i] * theX[i] + theYY[i] * theY[i] );
    if ( 0 ) {
      g = theX[i] * theX[i] + theY[i] * theY[i];
      if ( g > 1e-10 ) theH[i] /= g;
    }
  }
  
  

  if ( theH != bufferOut ) {
    if ( ConvertBuffer( theH, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( theXX );
      return( -1 );
    }
  }

  free( theXX );

  return( 1 );
}





int gradientHessianGradient3D( void *bufferIn,
                       bufferType typeIn,
                       void *bufferOut,
                       bufferType typeOut,
                       int *bufferDims,
                       int *borderLengths,
                       typeFilteringCoefficients *theFilter )
{
  char *proc = "gradientHessianGradient3D";
  size_t dimx, dimy, dimz;
  size_t sizeAuxBuf = 0;
  typeFilteringCoefficients filter[3];

  float *theXX = NULL;
  float *theYY = NULL;
  float *theZZ = NULL;
  float *theXY = NULL;
  float *theXZ = NULL;
  float *theYZ = NULL;
  float *theX  = NULL;
  float *theY  = NULL;
  float *theZ  = NULL;
  float *theH  = NULL;
  double g;

  long int i;

  dimx = bufferDims[0];
  dimy = bufferDims[1];
  dimz = bufferDims[2];

  
  /* we could spare one buffer,
     but who cares?
  */
  sizeAuxBuf = (size_t)9 * dimx*dimy*dimz;
  if ( typeOut != FLOAT || bufferIn == bufferOut )
    sizeAuxBuf += dimx*dimy*dimz;


  /* allocation des buffers de calcul
   */
  theXX = (float*)malloc( sizeAuxBuf * sizeof(float) );
  if ( theXX == NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, "%s: unable to allocate auxiliary buffer\n", proc );
    return( -1 );
  }

  sizeAuxBuf = dimx*dimy*dimz;
  theYY = theZZ = theXY = theXZ = theYZ = theX = theY = theZ = theH = theXX;
  theYY +=   sizeAuxBuf;
  theZZ += 2*sizeAuxBuf;
  theXY += 3*sizeAuxBuf;
  theXZ += 4*sizeAuxBuf;
  theYZ += 5*sizeAuxBuf;
  theX  += 6*sizeAuxBuf;
  theY  += 7*sizeAuxBuf;
  theZ  += 8*sizeAuxBuf;
  
  if ( typeOut != FLOAT || bufferIn == bufferOut ) {
    theH  += 9*sizeAuxBuf;
  } else {
    theH  = (float*)bufferOut;
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
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theXY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (.,.,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* smoothing along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXY, FLOAT, (void*)theXX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (.,0,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 1st derivative along X 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXX, FLOAT, (void*)theX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (1,0,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }
  
  /* 2nd derivative along X 
   */
  filter[0].derivative = DERIVATIVE_2;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXX, FLOAT, (void*)theXX, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (2,0,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* smoothing along X 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXY, FLOAT, (void*)theYY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,.,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 1st derivative along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_1;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYY, FLOAT, (void*)theY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,1,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }
  
  /* 2nd derivative along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_2;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYY, FLOAT, (void*)theYY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,2,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 2nd derivative along X and Y 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = DERIVATIVE_1;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXY, FLOAT, (void*)theXY, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (1,1,0) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }



  /* 1st derivative along Z 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = DERIVATIVE_1;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theYZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (.,.,1) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* smoothing along Y 
   */
  filter[0].derivative = NODERIVATIVE;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYZ, FLOAT, (void*)theXZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (.,0,1) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* smoothing along X 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXZ, FLOAT, (void*)theZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,0,1) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* 1st derivative along X 
   */
  filter[0].derivative = DERIVATIVE_1;
  filter[1].derivative = NODERIVATIVE;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theXZ, FLOAT, (void*)theXZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (1,0,1) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  /* smoothing along X, 1st derivative along Y 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_1;
  filter[2].derivative = NODERIVATIVE;
  if ( separableLinearFiltering( (void*)theYZ, FLOAT, (void*)theYZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,1,1) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }


  /* 2nd derivative along Z 
   */
  filter[0].derivative = DERIVATIVE_0;
  filter[1].derivative = DERIVATIVE_0;
  filter[2].derivative = DERIVATIVE_2;
  if ( separableLinearFiltering( bufferIn, typeIn, (void*)theZZ, FLOAT,
                                 bufferDims, borderLengths, filter ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute (0,0,2) filtering (3D)\n", proc );
    free( theXX );
    return( -1 );
  }

  

  sizeAuxBuf = dimx*dimy*dimz;
#ifdef _OPENMP
#pragma omp parallel for private( g )
#endif 
  for ( i = 0; i < (long int)sizeAuxBuf; i++ ) {
    theH[i] = theX[i] * ( theXX[i] * theX[i] + theXY[i] * theY[i] + theXZ[i] * theZ[i] ) 
      + theY[i] * ( theXY[i] * theX[i] + theYY[i] * theY[i] + theYZ[i] * theZ[i] )
      + theZ[i] * ( theXZ[i] * theX[i] + theYZ[i] * theY[i] + theZZ[i] * theZ[i] );
    if ( 0 ) {
      g = theX[i] * theX[i] + theY[i] * theY[i] + theZ[i] * theZ[i];
      if ( g > 1e-10 ) theH[i] /= g;
    }
  }
  
  

  if ( theH != bufferOut ) {
    if ( ConvertBuffer( theH, FLOAT, bufferOut, typeOut, sizeAuxBuf ) != 1 ) {
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to convert buffer\n", proc );
      free( theXX );
      return( -1 );
    }
  }

  free( theXX );

  return( 1 );
}





int gradientHessianGradient( void *bufferIn,
                             bufferType typeIn,
                             void *bufferOut,
                             bufferType typeOut,
                             int *bufferDims,
                             int *borderLengths,
                             typeFilteringCoefficients *theFilter )
{
  if ( bufferDims[2] == 1 ) 
    return( gradientHessianGradient2D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
  else
    return( gradientHessianGradient3D( bufferIn, typeIn, bufferOut, typeOut,
                               bufferDims, borderLengths, theFilter ) );
}
