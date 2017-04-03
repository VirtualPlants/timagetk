/*************************************************************************
 * convolution.c -
 *
 * $Id: convolution.c,v 1.7 2003/07/04 08:16:47 greg Exp $
 *
 * Copyright (c) INRIA 2002
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Sep 26 21:27:16 MEST 2002
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <convolution1D.h>
#include <convolution.h>
#include <convert.h>

static int _verbose_ = 1;
static int _width_ = 2;


static int _convolution( void *bufferIn,
                         bufferType typeIn,
                         void *bufferOut,
                         bufferType typeOut,
                         int *bufferDims,
                         double *mask,
                         int *maskDims,
                         int *maskCenter );





double _buildGaussianMaskFromFwhm( double *mask, int fwhm )
{
  double s = (double)fwhm / sqrt( 8.0 * log ( 2.0 ) );
  double x, sum = 0.0;
  int i;

  for ( i=0; i<=2*_width_*fwhm; i++ ) {
    x = (double)(i-_width_*fwhm);
    mask[i] = exp( -0.5 * ( x / s ) * ( x / s ) );
    sum += mask[i];
  }

  for ( i=0; i<=2*_width_*fwhm; i++ ) mask[i] /= sum;
  return( s );
}





int ConvolutionWithFwhm( void *bufferIn,  bufferType typeIn,
                         void *bufferOut, bufferType typeOut,
                         int *bufferDims,
                         int *fwhm )
{

  int maxfwhm = 0;
  double *mask = NULL;
  double sigma;

  float *tmp;
  float *tmp1Buffer = NULL;
  float *tmp2Buffer = NULL;

  float *res1Buffer = NULL;
  float *res2Buffer = NULL;

  int maskDims[3];
  int maskCenter[3];

  if ( bufferDims[0] > 1 && maxfwhm < fwhm[0] ) maxfwhm = fwhm[0];
  if ( bufferDims[1] > 1 && maxfwhm < fwhm[1] ) maxfwhm = fwhm[1];
  if ( bufferDims[2] > 1 && maxfwhm < fwhm[2] ) maxfwhm = fwhm[2];
  
  if ( maxfwhm == 0 ) return( 0 );

  mask = (double*)malloc( (2*_width_*maxfwhm+1) * sizeof( double) );
  if ( mask == NULL ) return( 0 );

  res1Buffer = (float*)malloc( bufferDims[0] * bufferDims[1]
                               * bufferDims[2] * sizeof(float) );
  if ( res1Buffer == NULL ) {
    free( mask );
    return( 0 );
  }
  if ( typeOut == FLOAT && bufferOut != bufferIn ) {
    res2Buffer = (float*)bufferOut;
  }
  else {
    res2Buffer = (float*)malloc( bufferDims[0] * bufferDims[1]
                                 * bufferDims[2] * sizeof(float) );
    if ( res2Buffer == NULL ) {
      free( mask );
      free( res1Buffer );
      return( 0 );
    }
  }

  tmp1Buffer = res1Buffer;
  tmp2Buffer = res2Buffer;



  if ( ConvertBuffer( bufferIn, typeIn, tmp1Buffer, FLOAT,
                      bufferDims[0] * bufferDims[1] * bufferDims[2] ) != 1 ) {
    if ( typeOut != FLOAT || bufferOut == bufferIn ) free( res2Buffer );
    free( res1Buffer );
    free( mask );
    return( 0 );
  }



  if ( bufferDims[0] > 1 && fwhm[0] > 0 ) {
    sigma = _buildGaussianMaskFromFwhm( mask, fwhm[0] );    
    fprintf( stderr, "sigma X = %f\n", sigma );
    maskDims[0] = 2*_width_*fwhm[0]+1;
    maskDims[1] = 1;
    maskDims[2] = 1;
    maskCenter[0] = _width_*fwhm[0];
    maskCenter[1] = 0;
    maskCenter[2] = 0;
    _convolution( tmp1Buffer, FLOAT, tmp2Buffer, FLOAT, bufferDims,
                  mask, maskDims, maskCenter );
    tmp = tmp2Buffer;   tmp2Buffer = tmp1Buffer;   tmp1Buffer = tmp;
  }



  if ( bufferDims[1] > 1 && fwhm[1] > 0 ) {
    sigma = _buildGaussianMaskFromFwhm( mask, fwhm[1] );    
    fprintf( stderr, "sigma Y = %f\n", sigma );
    maskDims[1] = 2*_width_*fwhm[1]+1;
    maskDims[0] = 1;
    maskDims[2] = 1;
    maskCenter[1] = _width_*fwhm[1];
    maskCenter[0] = 0;
    maskCenter[2] = 0;
    _convolution( tmp1Buffer, FLOAT, tmp2Buffer, FLOAT, bufferDims,
                  mask, maskDims, maskCenter );
    tmp = tmp2Buffer;   tmp2Buffer = tmp1Buffer;   tmp1Buffer = tmp;
  }



  if ( bufferDims[2] > 1 && fwhm[2] > 0 ) {
    sigma = _buildGaussianMaskFromFwhm( mask, fwhm[2] );    
    fprintf( stderr, "sigma Z = %f\n", sigma );
    maskDims[2] = 2*_width_*fwhm[2]+1;
    maskDims[1] = 1;
    maskDims[0] = 1;
    maskCenter[2] = _width_*fwhm[2];
    maskCenter[1] = 0;
    maskCenter[0] = 0;
    _convolution( tmp1Buffer, FLOAT, tmp2Buffer, FLOAT, bufferDims,
                  mask, maskDims, maskCenter );
    tmp = tmp2Buffer;   tmp2Buffer = tmp1Buffer;   tmp1Buffer = tmp;
  }



  if ( ConvertBuffer( tmp1Buffer, FLOAT, bufferOut, typeOut,
                      bufferDims[0] * bufferDims[1] * bufferDims[2] ) != 1 ) {
    if ( typeOut != FLOAT || bufferOut == bufferIn ) free( res2Buffer );
    free( res1Buffer );
    free( mask );
    return( 0 );
  }



  if ( typeOut != FLOAT || bufferOut == bufferIn ) free( res2Buffer );
  free( res1Buffer );
  free( mask );
  return( 1 );
}





int Convolution( void *bufferIn,  bufferType typeIn,
                 void *bufferOut, bufferType typeOut,
                 int *bufferDims,
                 void *mask,      bufferType typeMask,
                 int *maskDims,   int *maskCenter )
{
  float *bufferRes = NULL;
  double *bufferMask = NULL;
  int ret;


  if ( typeOut == FLOAT && bufferOut != bufferIn ) {
    bufferRes = (float*)bufferOut;
  }
  else {
    bufferRes = (float*)malloc( bufferDims[0] * bufferDims[1]
                                * bufferDims[2] * sizeof(float) );
    if ( bufferRes == NULL ) return( 0 );
  }

  if ( typeMask == DOUBLE ) {
    bufferMask = (double*)mask;
  }
  else {
    bufferMask = (double*)malloc( maskDims[0] * maskDims[1]
                                  * maskDims[2] * sizeof(double) );
    if ( bufferMask == NULL ) {
      if ( typeOut != FLOAT || bufferOut == bufferIn ) free( bufferRes );
      return( 0 );
    }
    
    if ( ConvertBuffer( mask, typeMask, bufferMask, DOUBLE, 
                        maskDims[0] * maskDims[1] * maskDims[2] ) != 1 ) {
      if ( typeMask != DOUBLE ) free( bufferMask );
      if ( typeOut != FLOAT || bufferOut == bufferIn ) free( bufferRes );
      return( 0 );
    }
  }

  ret = _convolution( bufferIn, typeIn, bufferRes, FLOAT, bufferDims,
                      bufferMask, maskDims, maskCenter );
  
  if ( ret == 1 ) {
    if ( ConvertBuffer( bufferRes, FLOAT, bufferOut, typeOut, 
                        bufferDims[0] * bufferDims[1] * bufferDims[2] )!= 1 ) {
      if ( typeMask != DOUBLE ) free( bufferMask );
      if ( typeOut != FLOAT || bufferOut == bufferIn ) free( bufferRes );
      return( 0 );
    }
  }

  if ( typeMask != DOUBLE ) free( bufferMask );
  if ( typeOut != FLOAT || bufferOut == bufferIn ) free( bufferRes );

  return( ret );
}











/*
 *
 * 
 *
 *
 */

int SeparableConvolution( void *bufferIn,
                          bufferType typeIn,
                          void *bufferOut,
                          bufferType typeOut,
                          int *bufferDims,
                          int *borderLengths,
                          double **convolutionMasks,
                          int *convolutionMaskLength )
{
  char *proc = "SeparableConvolution";
  register int dimx, dimxXdimy;
  int dimy, dimz;
  register int x, y, z;
  /* 
   *obviously, we need to perform the computation 
   * with float or double values. For this reason,
   * we allocate an auxiliary buffer if the output buffer
   * is not of type float or double.
   */
  void *bufferToBeProcessed = (void*)NULL;
  bufferType typeToBeProcessed = TYPE_UNKNOWN;
  void *bufferResult = (void*)NULL;
  bufferType typeResult = TYPE_UNKNOWN;
  /*
   * lines' lengths
   */
  int lengthX = 0;
  int lengthY = 0;
  int lengthZ = 0;
  int maxLengthline = 0;
  int borderXlength = 0;
  int borderYlength = 0;
  int borderZlength = 0;
  /*
   * 1D arrays for computations.
   */
  double *theLine = (double*)NULL;
  double *resLine = (double*)NULL;
  double *tmpLine = (double*)NULL;
  /*
   * pointers for computations;
   */
  register r32 *r32firstPoint = (r32*)NULL;
  register r64 *r64firstPoint = (r64*)NULL;
  register r32 *r32_pt = (r32*)NULL;
  register r64 *r64_pt = (r64*)NULL;
  register double *dbl_pt1 = (double*)NULL;
  register double *dbl_pt2 = (double*)NULL;
  register double dbl_first = 0.0;
  register double dbl_last = 0.0;
  int offsetLastPoint = 0;
  int offsetNextFirstPoint = 0;
  register r32 *r32firstPointResult = (r32*)NULL;
  register r64 *r64firstPointResult = (r64*)NULL;
  double *theLinePlusBorder = (double*)NULL;
  double *resLinePlusBorder = (double*)NULL;

  int half;

  /* 
   * We check the buffers' dimensions.
   */
  dimx = bufferDims[0];   dimy = bufferDims[1];   dimz = bufferDims[2];
  dimxXdimy = dimx * dimy;
  if ( (dimx <= 0) || (dimy <= 0) || (dimz <= 0) ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: improper buffer's dimension.\n", proc );
    return( -1 );
  }
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
      if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
        free( bufferResult );
      return( 0 );
    }
    bufferToBeProcessed = bufferResult;
    typeToBeProcessed = typeResult;
  }



  /*
   * Estimation of the lines' length along each direction.
   */
  if ( borderLengths != NULL ) {
    borderXlength = borderLengths[0];
    borderYlength = borderLengths[1];
    borderZlength = borderLengths[2];
    if ( borderXlength < 0 ) borderXlength = 0;
    if ( borderYlength < 0 ) borderYlength = 0;
    if ( borderZlength < 0 ) borderZlength = 0;
  }

  /*
   * Tue Jul  6 19:15:15 MET DST 1999 (gregoire Malandain)
   * changes 3 x dimx -> dimx, dimy, dimz
   */
  lengthX = dimx + 2 * borderXlength;
  lengthY = dimy + 2 * borderYlength;
  lengthZ = dimz + 2 * borderZlength;
  maxLengthline = lengthX;
  if ( maxLengthline < lengthY ) maxLengthline = lengthY;
  if ( maxLengthline < lengthZ ) maxLengthline = lengthZ;
  if ( maxLengthline <= 0 ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, " Error in %s: unable to deal with dimensions = 0.\n", proc );
    if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
      free( bufferResult );
    return( -1 );
  }
  /*
   * Allocations of work arrays. 
   * We will use them to process each line.
   */
  theLine = (double*)malloc( 3 * maxLengthline * sizeof(double) );
  if ( theLine == (double*)NULL ) {
    if ( _verbose_ > 0 )
      fprintf( stderr, " Fatal error in %s: unable to allocate auxiliary work arrays.\n", proc );
    if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
      free( bufferResult );
    return( -1 );
  }
  resLine = theLine + maxLengthline;
  tmpLine = resLine + maxLengthline;

  /*
   * From now,
   * typeToBeProcessed is either FLOAT or DOUBLE
   * so is typeResult.
   */

  

  /*
   * Processing along X.
   */
  if ( dimx > 1 
       && convolutionMaskLength[0] > 1 
       && convolutionMasks[0] != NULL ) {

    if ( _verbose_ != 0 )
      fprintf( stderr, " %s: processing along X.\n", proc );

    half = (int)( convolutionMaskLength[0]/2 );
    if ( 2*half+1 != convolutionMaskLength[0] ) {
      if ( _verbose_ != 0 )
        fprintf( stderr, " %s: convolution mask length (along X) should be odd\n",
                 proc );
      return( -1 );
    }

    r64firstPoint = (r64*)bufferToBeProcessed;
    r32firstPoint = (r32*)bufferToBeProcessed;

    r64firstPointResult = (r64*)bufferResult;
    r32firstPointResult = (r32*)bufferResult;

    offsetLastPoint = borderXlength + dimx - 1;

    theLinePlusBorder = theLine + borderXlength;
    resLinePlusBorder = resLine + borderXlength;

    /*
     * There are dimz*dimy X lines to be processed.
     */
    for ( z=0; z<dimz; z++ )
    for ( y=0; y<dimy; y++ ) {
      /*
       * Acquiring a X line.
       */ 
      dbl_pt1 = theLinePlusBorder;
      switch ( typeToBeProcessed ) {
      case DOUBLE :
        (void)memcpy( (void*)dbl_pt1, (void*)r64firstPoint, dimx * sizeof(r64) );
        r64firstPoint += dimx;
        break;
      case FLOAT :
      default :
        for ( x=0; x<dimx; x++, dbl_pt1++, r32firstPoint++ ) *dbl_pt1 = *r32firstPoint;
      }
      /*
       * Adding points at both ends of the line.
       */
      if ( borderXlength > 0 ) {
        dbl_pt1 = theLine + borderXlength;   dbl_first = *dbl_pt1;
        dbl_pt2 = theLine + offsetLastPoint; dbl_last  = *dbl_pt2;
        for ( x=0; x<borderXlength; x++ ) {
          *--dbl_pt1 = dbl_first;
          *++dbl_pt2 = dbl_last;
        }
      }
      /*
       * Processing the line.
       */
      if ( compute1DConvolutionWithMask( theLine, DOUBLE, resLine, DOUBLE, lengthX,
                           convolutionMasks[0], half ) != 1 ) {
        if ( _verbose_ != 0 )
          fprintf(stderr," Error in %s: unable to process X line (y=%d,z=%d).\n", proc, y, z);
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        free( (void*)theLine );
        return( -1 );
      }
      /*
       * Copy the result into the buffer bufferResult.
       */
      dbl_pt1 = resLinePlusBorder;
      switch ( typeResult ) {
      case DOUBLE :
        (void)memcpy( (void*)r64firstPointResult, (void*)dbl_pt1, dimx * sizeof(r64) );
        r64firstPointResult += dimx;
        break;
      case FLOAT :
      default :
        for ( x=0; x<dimx; x++, dbl_pt1++, r32firstPointResult++ )
          *r32firstPointResult = (r32)(*dbl_pt1);
      }
    }
    
    /*
     * The next buffer to be processed is the buffer
     * bufferResult.
     */
    bufferToBeProcessed = bufferResult;
    typeToBeProcessed = typeResult;
    
  } /* end of Processing along X. */
  
  /*
   * Processing along Y.
   */
  if ( dimy > 1 
       && convolutionMaskLength[1] > 1 
       && convolutionMasks[1] != NULL ) {

    if ( _verbose_ != 0 )
      fprintf( stderr, " %s: processing along Y.\n", proc );

    half = (int)( convolutionMaskLength[1]/2 );
    if ( 2*half+1 != convolutionMaskLength[1] ) {
      if ( _verbose_ != 0 )
        fprintf( stderr, " %s: convolution mask length (along Y) should be odd\n",
                 proc );
      return( -1 );
    }

    r64firstPoint = (r64*)bufferToBeProcessed;
    r32firstPoint = (r32*)bufferToBeProcessed;

    r64firstPointResult = (r64*)bufferResult;
    r32firstPointResult = (r32*)bufferResult;

    offsetLastPoint = borderYlength + dimy - 1;
    offsetNextFirstPoint = dimx * dimy - dimx;

    theLinePlusBorder = theLine + borderYlength;
    resLinePlusBorder = resLine + borderYlength;

    /*
     * There are dimz*dimx Y lines to be processed.
     */
    for ( z=0; z<dimz; z++ ) {
      for ( x=0; x<dimx; x++ ) {
      /*
       * Acquiring a Y line.
       */ 
        dbl_pt1 = theLinePlusBorder;
        switch ( typeToBeProcessed ) {
        case DOUBLE :
          r64_pt = r64firstPoint;
          for ( y=0; y<dimy; y++, dbl_pt1++, r64_pt += dimx ) *dbl_pt1 = *r64_pt;
          /*
           * Going to the first point of the next Y line
           */
          r64firstPoint ++;
          break;
        case FLOAT :
        default :
          r32_pt = r32firstPoint;
          for ( y=0; y<dimy; y++, dbl_pt1++, r32_pt += dimx ) *dbl_pt1 = *r32_pt;
          r32firstPoint ++;
        }
        /*
         * Adding points at both ends of the line.
         */
        if ( borderYlength > 0 ) {
          dbl_pt1 = theLine + borderYlength;   dbl_first = *dbl_pt1;
          dbl_pt2 = theLine + offsetLastPoint; dbl_last  = *dbl_pt2;
          for ( y=0; y<borderYlength; y++ ) {
            *--dbl_pt1 = dbl_first;
            *++dbl_pt2 = dbl_last;
          }
        }
        /*
         * Processing the line.
         */
      if ( compute1DConvolutionWithMask( theLine, DOUBLE, resLine, DOUBLE, lengthY,
                           convolutionMasks[1], half ) != 1 ) {
          if ( _verbose_ != 0 )
            fprintf(stderr," Error in %s: unable to process Y line (x=%d,z=%d).\n", proc, x, z);
          if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
            free( bufferResult );
          free( (void*)theLine );
          return( -1 );
        }
        /*
         * Copy the result into the buffer bufferResult.
         */
        dbl_pt1 = resLinePlusBorder;
        switch ( typeResult ) {
        case DOUBLE :
          r64_pt = r64firstPointResult;
          for ( y=0; y<dimy; y++, dbl_pt1++, r64_pt += dimx ) *r64_pt = *dbl_pt1;
          r64firstPointResult ++;
          break;
        case FLOAT :
        default :
          r32_pt = r32firstPointResult;
          for ( y=0; y<dimy; y++, dbl_pt1++, r32_pt += dimx ) *r32_pt = *dbl_pt1;
          r32firstPointResult ++;
        }
      }
      /*
       * Going to the first point of the next Y line
       * which is the first Y line of the next slice.
       *
       * The pointer r[32,64]firstPoint[Result] has
       * already been increased by dimx. To reach
       * the first point of the next slice, we
       * have to increase it by (dimx*dimy)-dimx.
       */
      switch ( typeToBeProcessed ) {
      case DOUBLE :
        r64firstPoint += offsetNextFirstPoint;
        break;
      case FLOAT :
      default :
        r32firstPoint += offsetNextFirstPoint;
      }
      switch ( typeResult ) {
      case DOUBLE :
        r64firstPointResult += offsetNextFirstPoint;
        break;
      case FLOAT :
      default :
        r32firstPointResult += offsetNextFirstPoint;
      }
    }
    
    /*
     * The next buffer to be processed is the buffer
     * bufferResult.
     */
    bufferToBeProcessed = bufferResult;
    typeToBeProcessed = typeResult;
  
  } /* end of Processing along Y. */
  

  /*
   * Processing along Z.
   */
  if ( dimz > 1 
       && convolutionMaskLength[2] > 1 
       && convolutionMasks[2] != NULL ) {

    if ( _verbose_ != 0 )
      fprintf( stderr, " %s: processing along Z.\n", proc );
    
    half = (int)( convolutionMaskLength[2]/2 );
    if ( 2*half+1 != convolutionMaskLength[2] ) {
      if ( _verbose_ != 0 )
        fprintf( stderr, " %s: convolution mask length (along Z) should be odd\n",
                 proc );
      return( -1 );
    }

    r64firstPoint = (r64*)bufferToBeProcessed;
    r32firstPoint = (r32*)bufferToBeProcessed;

    offsetLastPoint = borderZlength + dimz - 1;

    r64firstPointResult = (r64*)bufferResult;
    r32firstPointResult = (r32*)bufferResult;

    offsetLastPoint = borderZlength + dimz - 1;

    theLinePlusBorder = theLine + borderYlength;
    resLinePlusBorder = resLine + borderYlength;

    /*
     * There are dimy*dimx Z lines to be processed.
     */
    for ( y=0; y<dimy; y++ )
    for ( x=0; x<dimx; x++ ) {
      /*
       * Acquiring a Z line.
       */ 
      dbl_pt1 = theLinePlusBorder;
      switch ( typeToBeProcessed ) {
      case DOUBLE :
        r64_pt = r64firstPoint;
        for ( z=0; z<dimz; z++, dbl_pt1++, r64_pt += dimxXdimy ) *dbl_pt1 = *r64_pt;
        /*
         * Going to the first point of the next Z line
         */
        r64firstPoint ++;
        break;
      case FLOAT :
      default :
        r32_pt = r32firstPoint;
        for ( z=0; z<dimz; z++, dbl_pt1++, r32_pt += dimxXdimy ) *dbl_pt1 = *r32_pt;
        r32firstPoint ++;
      }
      /*
       * Adding points at both ends of the line.
       */
      if ( borderZlength > 0 ) {
        dbl_pt1 = theLine + borderZlength;   dbl_first = *dbl_pt1;
        dbl_pt2 = theLine + offsetLastPoint; dbl_last  = *dbl_pt2;
        for ( z=0; z<borderZlength; z++ ) {
          *--dbl_pt1 = dbl_first;
          *++dbl_pt2 = dbl_last;
        }
      }
      /*
       * Processing the line.
       */
      if ( compute1DConvolutionWithMask( theLine, DOUBLE, resLine, DOUBLE, lengthZ,
                           convolutionMasks[2], half ) != 1 ) {
        if ( _verbose_ != 0 )
          fprintf(stderr," Error in %s: unable to process Z line (x=%d,y=%d).\n", proc, x, y);
        if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
          free( bufferResult );
        free( (void*)theLine );
        return( -1 );
      }
      
      /*
       * Copy the result into the buffer bufferResult.
       */
      dbl_pt1 = resLinePlusBorder;
      switch ( typeResult ) {
      case DOUBLE :
        r64_pt = r64firstPointResult;
        for ( z=0; z<dimz; z++, dbl_pt1++, r64_pt += dimxXdimy ) *r64_pt = *dbl_pt1;
        r64firstPointResult ++;
        break;
      case FLOAT :
      default :
        r32_pt = r32firstPointResult;
        for ( z=0; z<dimz; z++, dbl_pt1++, r32_pt += dimxXdimy ) *r32_pt = *dbl_pt1;
        r32firstPointResult ++;
      }
    }

  } /* end of Processing along Z. */
  



  /*
   * From bufferResult to bufferOut
   */
  if ( ConvertBuffer( bufferResult, typeResult, bufferOut, typeOut, (dimx*dimy*dimz) ) != 1 ) {
    if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
      free( bufferResult );
    return( 0 );
  }

  /*
   * Releasing the buffers.
   */
  if ( (typeOut != FLOAT) && (typeOut != DOUBLE) )
    free( bufferResult );
  free( (void*)theLine );
  
  return( 1 );
}









void Convolution_verbose ( )
{
  _verbose_ = 1;
}





void Convolution_noverbose ( )
{
  _verbose_ = 0;
}





static int _convolution( void *bufferIn,
                         bufferType typeIn,
                         void *bufferOut,
                         bufferType typeOut,
                         int *bufferDims,
                         double *mask,
                         int *maskDims,
                         int *maskCenter )
{
  int dimx = bufferDims[0];
  int dimy = bufferDims[1];
  int dimz = bufferDims[2];

  int mdimx = maskDims[0];
  int mdimy = maskDims[1];
  int mdimz = maskDims[2];

  int cx = maskCenter[0];
  int cy = maskCenter[1];
  int cz = maskCenter[2];

  int loffx = - cx;
  int roffx = mdimx - 1 - cx;
  int loffy = - cy;
  int roffy = mdimy - 1 - cy;
  int loffz = - cz;
  int roffz = mdimz - 1 - cz;

  int lx = 0;
  int rx = 0;
  int ly = 0;
  int ry = 0;
  int lz = 0;
  int rz = 0;

  double val;

  int *offset = NULL;

  int ind;
  int i, j, k;
  int x, y, z;
  int iz, iy, ix;


  offset = (int*)malloc( mdimx * mdimy * mdimz * sizeof( int ) );
  if ( offset == NULL ) return( 0 );

  for ( ind=0, z=0; z<mdimz; z++ )
  for ( y=0; y<mdimy; y++ )
  for ( x=0; x<mdimx; x++, ind++ )
    offset[ ind ] = (z+loffz)*dimy*dimx + (y+loffy)*dimx + x+loffx;

  
  switch ( typeOut ) {

  default :
    free( offset );
    return( 0 );

  case FLOAT :
    {
      float* resBuf = (float*)bufferOut;

      switch ( typeIn ) {
      default :
        free( offset );
        return( 0 );

      case UCHAR :
        {
          u8* theBuf = (u8*)bufferIn;

          for ( ind=0, z=0; z<dimz; z++ ) {

            if ( mdimz > 1 ) {
              lz = loffz;
              rz = roffz;
              if ( z+loffz < 0 )     lz = - z;
              if ( z+roffz >= dimz ) rz = dimz-1-z;
              lz += cz;
              rz += cz;
            }

            for ( y=0; y<dimy; y++ ) {

              if ( mdimy > 1 ) {
                ly = loffy;
                ry = roffy;
                if ( y+loffy < 0 )     ly = - y;
                if ( y+roffy >= dimy ) ry = dimy-1-y;
                ly += cy;
                ry += cy;
              }


              for ( x=0; x<dimx; x++, ind++ ) {

                if ( mdimx > 1 ) {
                  lx = loffx;
                  rx = roffx;
                  if ( x+loffx < 0 )     lx = - x;
                  if ( x+roffx >= dimx ) rx = dimx-1-x;
                  lx += cx;
                  rx += cx;
                }

                val = 0;
                for ( iz = lz *mdimy, k = lz; k <= rz; iz += mdimy, k ++ )
                  for ( iy = (iz+ly)*mdimx, j = ly; j <= ry; iy += mdimx, j ++ )
                    for ( ix = iy + lx, i = lx; i <= rx; ix++, i ++ )
                      val += mask[ ix ] * theBuf[ ind + offset[ix] ];

                resBuf[ ind ] = val;
              }
            }
          } /* for ( ind=0, z=0; z<dimz; z++ ) */
        }
        break;

      case USHORT :
        {
          u16* theBuf = (u16*)bufferIn;

          for ( ind=0, z=0; z<dimz; z++ ) {

            if ( mdimz > 1 ) {
              lz = loffz;
              rz = roffz;
              if ( z+loffz < 0 )     lz = - z;
              if ( z+roffz >= dimz ) rz = dimz-1-z;
              lz += cz;
              rz += cz;
            }

            for ( y=0; y<dimy; y++ ) {

              if ( mdimy > 1 ) {
                ly = loffy;
                ry = roffy;
                if ( y+loffy < 0 )     ly = - y;
                if ( y+roffy >= dimy ) ry = dimy-1-y;
                ly += cy;
                ry += cy;
              }


              for ( x=0; x<dimx; x++, ind++ ) {

                if ( mdimx > 1 ) {
                  lx = loffx;
                  rx = roffx;
                  if ( x+loffx < 0 )     lx = - x;
                  if ( x+roffx >= dimx ) rx = dimx-1-x;
                  lx += cx;
                  rx += cx;
                }

                val = 0;
                for ( iz = lz *mdimy, k = lz; k <= rz; iz += mdimy, k ++ )
                  for ( iy = (iz+ly)*mdimx, j = ly; j <= ry; iy += mdimx, j ++ )
                    for ( ix = iy + lx, i = lx; i <= rx; ix++, i ++ )
                      val += mask[ ix ] * theBuf[ ind + offset[ix] ];

                resBuf[ ind ] = val;
              }
            }
          } /* for ( ind=0, z=0; z<dimz; z++ ) */
        }
        break;

      case FLOAT :
        {
          float* theBuf = (float*)bufferIn;

          for ( ind=0, z=0; z<dimz; z++ ) {

            if ( mdimz > 1 ) {
              lz = loffz;
              rz = roffz;
              if ( z+loffz < 0 )     lz = - z;
              if ( z+roffz >= dimz ) rz = dimz-1-z;
              lz += cz;
              rz += cz;
            }

            for ( y=0; y<dimy; y++ ) {

              if ( mdimy > 1 ) {
                ly = loffy;
                ry = roffy;
                if ( y+loffy < 0 )     ly = - y;
                if ( y+roffy >= dimy ) ry = dimy-1-y;
                ly += cy;
                ry += cy;
              }

              if ( mdimx == 1 ) {

                for ( x=0; x<dimx; x++, ind++ ) {
                  val = 0;
                  for ( iz = lz *mdimy, k = lz; k <= rz; iz += mdimy, k ++ )
                    for ( iy = (iz+ly)*mdimx, j = ly; j <= ry; iy += mdimx, j ++ )
                      val += mask[ iy ] * theBuf[ ind + offset[iy] ];
                  resBuf[ ind ] = val;
                }

              }
              else {

                for ( x=0; x<dimx; x++, ind++ ) {

                  if ( mdimx > 1 ) {
                    lx = loffx;
                    rx = roffx;
                    if ( x+loffx < 0 )     lx = - x;
                    if ( x+roffx >= dimx ) rx = dimx-1-x;
                    lx += cx;
                    rx += cx;
                  }

                  val = 0;
                  for ( iz = lz *mdimy, k = lz; k <= rz; iz += mdimy, k ++ )
                    for ( iy = (iz+ly)*mdimx, j = ly; j <= ry; iy += mdimx, j ++ )
                      for ( ix = iy + lx, i = lx; i <= rx; ix++, i ++ )
                        val += mask[ ix ] * theBuf[ ind + offset[ix] ];

                  resBuf[ ind ] = val;
                }
              }
            }
          } /* for ( ind=0, z=0; z<dimz; z++ ) */
        }
        break;

      case DOUBLE :
        {
          double* theBuf = (double*)bufferIn;

          for ( ind=0, z=0; z<dimz; z++ ) {

            if ( mdimz > 1 ) {
              lz = loffz;
              rz = roffz;
              if ( z+loffz < 0 )     lz = - z;
              if ( z+roffz >= dimz ) rz = dimz-1-z;
              lz += cz;
              rz += cz;
            }

            for ( y=0; y<dimy; y++ ) {

              if ( mdimy > 1 ) {
                ly = loffy;
                ry = roffy;
                if ( y+loffy < 0 )     ly = - y;
                if ( y+roffy >= dimy ) ry = dimy-1-y;
                ly += cy;
                ry += cy;
              }


              for ( x=0; x<dimx; x++, ind++ ) {

                if ( mdimx > 1 ) {
                  lx = loffx;
                  rx = roffx;
                  if ( x+loffx < 0 )     lx = - x;
                  if ( x+roffx >= dimx ) rx = dimx-1-x;
                  lx += cx;
                  rx += cx;
                }

                val = 0;
                for ( iz = lz *mdimy, k = lz; k <= rz; iz += mdimy, k ++ )
                  for ( iy = (iz+ly)*mdimx, j = ly; j <= ry; iy += mdimx, j ++ )
                    for ( ix = iy + lx, i = lx; i <= rx; ix++, i ++ )
                      val += mask[ ix ] * theBuf[ ind + offset[ix] ];

                resBuf[ ind ] = val;
              }
            }
          } /* for ( ind=0, z=0; z<dimz; z++ ) */
        }
        break;

      }

    }
    break;
    /* end of case typeOut = FLOAT */
  }





  
  free( offset );
  return( 1 );
}
