/****************************************************
 * histogram.c - 
 *
 * $Id$
 *
 * Copyright (c) INRIA 2013, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Wed Feb  6 12:06:18 CET 2013
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

#include <fcntl.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include <math.h>

#include <convert.h> /* for printType() */
#include <histogram.h>





static int _verbose_ = 1;
static int _debug_ = 0;




void setVerboseInHistogram( int v )
{
  _verbose_ = v;
}

void incrementVerboseInHistogram(  )
{
  _verbose_ ++;
}

void decrementVerboseInHistogram(  )
{
  _verbose_ --;
  if ( _verbose_ < 0 ) _verbose_ = 0;
}


/**********************************************************************
 *
 *
 *
 **********************************************************************/


static int _minMeanMaxStddevValues( void *theIm, bufferType theType, 
                                    int *theDim,
                                    unionValues *theMin, double *theMean,
                                    unionValues *theMax, double *theStandardDeviation );
static int _minMeanMaxStddevValuesWithMask( void *theIm, bufferType theType, 
                                    void *theMask, bufferType theMaskType,
                                    int *theDim,
                                    unionValues *theMin, double *theMean,
                                    unionValues *theMax, double *theStandardDeviation );










/**********************************************************************
 *
 * histogram header construction tools
 *
 **********************************************************************/



bufferType typeHistogramIndex( float binlength, bufferType typeImage )
{
  switch( typeImage ) {
  default :
    return( TYPE_UNKNOWN );
  case UCHAR :
  case SCHAR :
  case USHORT :
  case SSHORT :
  case SINT :
  case UINT :
    if ( binlength <= 0.0 || ( 0.999999 < binlength && binlength < 1.000001 ) )
      return( SINT );
    return( FLOAT );
  case FLOAT :
  case DOUBLE :
    return( FLOAT );
  }
}





void initHistogramHeader( typeHistogramHeader *h ) 
{
  h->min.val_u64 = 0;
  h->max.val_u64 = 0;
  h->binlength = -1;
  h->typeIndex = TYPE_UNKNOWN;
  h->dim = 0;
  h->index = (void*)NULL;
}





void freeHistogramHeader( typeHistogramHeader *h ) 
{
  if ( h->index != (void*)NULL ) free( h->index );
  initHistogramHeader( h );
}





/* binlength <= 0.0 means binlength = 1
 */

int allocHistogramHeader( typeHistogramHeader *h, unionValues *min, unionValues *max,
                          float binlength, bufferType typeImage )
{
  char *proc = "allocHistogramHeader";
  bufferType typeIndex = typeHistogramIndex( binlength, typeImage ); 
  int n, dim = 0;
  
  switch ( typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    {
      int theMin = 0; 
      int theMax = 0;
      int i;
      s32 *index = (s32*)NULL;

      switch ( typeImage ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such data type not handled yet\n", proc );
        return( -1 );
      case SCHAR :
        theMin = min->val_s8;
        theMax = max->val_s8;
        break;
      case UCHAR :
        theMin = min->val_u8;
        theMax = max->val_u8;
        break;
      case SSHORT :
        theMin = min->val_s16;
        theMax = max->val_s16;
        break;
      case USHORT :
        theMin = min->val_u16;
        theMax = max->val_u16;
        break;
      case SINT :
        theMin = min->val_s32;
        theMax = max->val_s32;
        break;
      case UINT :
        theMin = min->val_u32;
        theMax = max->val_u32;
        break;
      }
      dim = theMax - theMin + 1;
      index = (s32*)malloc( dim*sizeof(s32) );
      if ( index == (s32*)NULL ) {
        fprintf( stderr, "%s: unable to allocate index\n", proc );
        return( -1 );
      }
      for ( n=0, i=theMin; n<dim; n++, i++ ) index[n] = i;
      
      h->min.val_s32 = theMin;
      h->max.val_s32 = theMax;

      h->binlength = 1;
      h->typeIndex = typeIndex;
      h->dim = dim;
      h->index = (void*)index;
    }
    break;


  case FLOAT :
    {
      float theMin = 0; 
      float theMax = 0;
      float i;
      r32 *index = (r32*)NULL;


      switch ( typeImage ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such data type not handled yet\n", proc );
        return( -1 );
      case SCHAR :
        theMin = min->val_s8;
        theMax = max->val_s8;
        break;
      case UCHAR :
        theMin = min->val_u8;
        theMax = max->val_u8;
        break;
      case SSHORT :
        theMin = min->val_s16;
        theMax = max->val_s16;
        break;
      case USHORT :
        theMin = min->val_u16;
        theMax = max->val_u16;
        break;
      case SINT :
        theMin = min->val_s32;
        theMax = max->val_s32;
        break;
      case UINT :
        theMin = min->val_u32;
        theMax = max->val_u32;
        break;
      case FLOAT :
        theMin = min->val_r32;
        theMax = max->val_r32;
        break;
      case DOUBLE :
        theMin = min->val_r64;
        theMax = max->val_r64;
        break;
      }

      h->min.val_r32 = theMin;
      h->max.val_r32 = theMax;

      h->binlength = ( binlength <= 0 ) ? 1.0 : binlength;
      if ( (theMax - theMin)/h->binlength > (int)((theMax - theMin)/h->binlength) )
        dim = (int)( (theMax - theMin)/h->binlength ) + 2;
      else
        dim = (int)( (theMax - theMin)/h->binlength ) + 1;

      index = (r32*)malloc( dim*sizeof(r32) );
      if ( index == (r32*)NULL ) {
        fprintf( stderr, "%s: unable to allocate index\n", proc );
        return( -1 );
      }

      for ( n=0, i=theMin; n<dim; n++, i+=h->binlength ) index[n] = i;
      h->typeIndex = typeIndex;
      h->dim = dim;
      h->index = (void*)index;
    }
    break;
  }

  return( 1 );
}










/**********************************************************************
 *
 * histogram construction tools
 *
 **********************************************************************/



void initHistogram( typeHistogram *h ) 
{
  initHistogramHeader( &(h->xaxis) );
  initHistogramHeader( &(h->yaxis) );
  h->typeHisto = TYPE_UNKNOWN;
  h->data = (void*)NULL;
}





void freeHistogram( typeHistogram *h ) 
{
  freeHistogramHeader( &(h->xaxis) );
  freeHistogramHeader( &(h->yaxis) );
  if ( h->data != (void*)NULL ) free( h->data );
  initHistogram( h );
}





int allocHistogramData( typeHistogram *h, bufferType typeHisto )
{
  char *proc = "alloc1DHistogramData";
  size_t size = 1;

  if( h->xaxis.dim <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no index values?\n", proc );
    return( -1 );
  }
  
  size *= h->xaxis.dim;

  if ( h->yaxis.dim > 1 ) 
    size *= h->yaxis.dim;

  switch( typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );
  case SINT :
    size *= sizeof( s32 );
    break;
  case UINT :
    size *= sizeof( u32 );
    break;
  case FLOAT :
    size *= sizeof( r32 );
    break;
  case DOUBLE :
    size *= sizeof( r64 );
    break;
  }


  h->data = (void*)malloc( size );

  if ( h->data == (void*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }

  (void)memset( h->data, 0, size );

  h->typeHisto = typeHisto;

  return( 1 );
}





void zeroHistogram( typeHistogram *h ) 
{
  char *proc = "zeroHistogram";
  size_t size = 1;

  size *= h->xaxis.dim;

  if ( h->yaxis.dim > 1 ) 
    size *= h->yaxis.dim;

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet (initialisation step)\n", proc );
    return;
  case SINT :
    (void)memset( h->data, 0, size * sizeof( s32 ) );
    break;
  case UINT :
    (void)memset( h->data, 0, size * sizeof( u32 ) );
    break;
  case FLOAT :
    (void)memset( h->data, 0, size * sizeof( r32 ) );
    break;
  case DOUBLE :
    (void)memset( h->data, 0, size * sizeof( r64 ) );
    break;
  }
}





int allocHistogramFromHistogram( typeHistogram *h,
                                 typeHistogram *template,
                                 bufferType typeHisto )
{
  char *proc = "allocHistogramFromHistogram";

  if ( allocHistogramHeader( &(h->xaxis), &(template->xaxis.min), &(template->xaxis.max),
                             template->xaxis.binlength, template->xaxis.typeIndex ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram x index\n", proc );
    return( -1 );
  }

  if ( template->yaxis.dim > 1 ) {
    if ( allocHistogramHeader( &(h->yaxis), &(template->yaxis.min), &(template->yaxis.max),
                               template->yaxis.binlength, template->yaxis.typeIndex ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate histogram x index\n", proc );
      return( -1 );
    }
  }

  if ( allocHistogramData( h, typeHisto ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }
  
  return( 1 );
}










/**********************************************************************
 *
 * 1D histogram construction tools
 *
 **********************************************************************/



int alloc1DHistogramFromType( typeHistogram *h, bufferType typeImage )
{
  char *proc = "alloc1DHistogramFromType";
  unionValues min, max;
  
  switch ( typeImage ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case SCHAR :
    min.val_s8 = -128;
    max.val_s8 = 127;
    break;
  case UCHAR :
    min.val_u8 = 0;
    max.val_u8 = 255;
    break;
  case SSHORT :
    min.val_s16 = -32768;
    max.val_s16 = 32767;
    break;
  case USHORT :
    min.val_u16 = 0;
    max.val_u16 = 32767;
    break;
  }

  if ( allocHistogramHeader( &(h->xaxis), &min, &max, -1.0, typeImage ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }

  if ( allocHistogramData( h, FLOAT ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }

  return( 1 );
}





int alloc1DHistogramFromImage( typeHistogram *h, 
                               void *theIm,
                               bufferType theType,
                               void *theMask,
                               bufferType theMaskType,
                               double *mat,
                               int *theDim )
{
  return( alloc1DHistogramFromImageAndBinLength( h, theIm, theType, 
                                                 theMask, theMaskType,
                                                 mat, theDim, -1.0 ) );
}





int alloc1DHistogramFromImageAndBinLength( typeHistogram *h, 
                                           void *theIm,
                                           bufferType theType,
                                           void *theMask,
                                           bufferType theMaskType,
                                           double *mat,
                                           int *theDim,
                                           float binlength)
{
  char *proc = "alloc1DHistogramFromImageAndBinLength";
  unionValues min, max;
  double mean, stddev;
  bufferType typeHisto = UINT;
  
  if ( _minMeanMaxStddevValuesWithMask( theIm, theType, theMask, theMaskType, theDim, &min, &mean, &max, &stddev ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute first order statistics\n", proc );
    return( -1 );
  }

  /* index type is SINT (signed int) for integer valued images if binlength is 1.0 or negative
   */
  if ( allocHistogramHeader( &(h->xaxis), &min, &max, binlength, theType ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }

  if ( mat != (double*)NULL ) 
    typeHisto = FLOAT;

  if ( allocHistogramData( h, typeHisto ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }
  
  if ( fill1DHistogramFromImage( h, theIm, theType, 
                                 theMask, theMaskType, mat, theDim  ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to fill histogram from buffer\n", proc );
    return( -1 );
  }

  return( 1 );
}





int alloc1DDifferenceHistogramFromJointHistogram( typeHistogram *hdiff,
                                                  typeHistogram *hjoint )
{
  char *proc = "alloc1DDifferenceHistogramFromJointHistogram";
  int xmin, xmax;
  int ymin, ymax;
  unionValues min, max;

  switch( hjoint->xaxis.typeIndex ) { 
  default : 
    if ( _verbose_ )
      fprintf( stderr, "%s: x index type not handled yet\n", proc );  
    return( -1 );  
  case SINT :
    xmin = hjoint->xaxis.min.val_s32;
    xmax = hjoint->xaxis.max.val_s32;
    switch( hjoint->xaxis.typeIndex ) { 
    default : 
      if ( _verbose_ )
        fprintf( stderr, "%s: y index type not handled yet\n", proc );
      return( -1 );  
    case SINT :
      ymin = hjoint->yaxis.min.val_s32;
      ymax = hjoint->yaxis.max.val_s32;
      min.val_s32 = ymin - xmax;
      max.val_s32 = ymax - xmin;

      if ( allocHistogramHeader( &(hdiff->xaxis), &min, &max, -1.0, SINT ) != 1 ) {
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to allocate histogram x index\n", proc );
        return( -1 );
      }

      break;
    }
    break;
  }

  if ( allocHistogramData( hdiff, hjoint->typeHisto ) != 1 ) {
    freeHistogram( hdiff );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate difference histogram data\n", proc );
    return( -1 );
  }

  if ( fill1DDifferenceHistogram( hdiff, hjoint ) != 1 ) {
    freeHistogram( hdiff );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to fill difference histogram from joint histogram\n", proc );
    return( -1 );
  }

  return( 1 );
}










/**********************************************************************
 *
 * 2D histogram construction tools
 *
 **********************************************************************/



int alloc2DHistogramFromTypes( typeHistogram *h, bufferType typeImage1, bufferType typeImage2 )
{
  char *proc = "alloc2DHistogramFromTypes";
  unionValues xmin, xmax;
  unionValues ymin, ymax;
  
  switch ( typeImage1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case SCHAR :
    xmin.val_s8 = -128;
    xmax.val_s8 = 127;
    break;
  case UCHAR :
    xmin.val_u8 = 0;
    xmax.val_u8 = 255;
    break;
  case SSHORT :
    xmin.val_s16 = -32768;
    xmax.val_s16 = 32767;
    break;
  case USHORT :
    xmin.val_u16 = 0;
    xmax.val_u16 = 32767;
    break;
  }

  switch ( typeImage2 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case SCHAR :
    ymin.val_s8 = -128;
    ymax.val_s8 = 127;
    break;
  case UCHAR :
    ymin.val_u8 = 0;
    ymax.val_u8 = 255;
    break;
  case SSHORT :
    ymin.val_s16 = -32768;
    ymax.val_s16 = 32767;
    break;
  case USHORT :
    ymin.val_u16 = 0;
    ymax.val_u16 = 32767;
    break;
  }

  if ( allocHistogramHeader( &(h->xaxis), &xmin, &xmax, -1.0, typeImage1 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }

  if ( allocHistogramHeader( &(h->xaxis), &ymin, &ymax, -1.0, typeImage2 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }

  if ( allocHistogramData( h, FLOAT ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }

  return( 1 );
}





int alloc2DHistogramFromHistogramList( typeHistogram *h, 
                                       typeHistogramList *hl )
{
  char *proc= "alloc2DHistogramFromHistogramList";
  int i;
  unionValues min;
  unionValues max;

  /* some test
   */

  for ( i=0; i<hl->n; i++ ) {
    if ( hl->data[i].yaxis.dim > 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram #%d is 2D\n", proc, i );
      return( -1 );
    }
    if ( hl->data[i].xaxis.binlength <= 0.999999
         || 1.000001 <= hl->data[i].xaxis.binlength ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram #%d has a binlength=%f\n", proc,
                 i, hl->data[i].xaxis.binlength  );
      return( -1 );
    }
  }

  for ( i=1; i<hl->n; i++ ) {
    if ( hl->data[i].xaxis.typeIndex != hl->data[0].xaxis.typeIndex ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: index type of histogram #%d is different from #0\n", proc, i );
      return( -1 );
    }
     if ( hl->data[i].typeHisto != hl->data[0].typeHisto ) {
       if ( _verbose_ )
        fprintf( stderr, "%s: type of histogram #%d is different from #0\n", proc, i );
       return( -1 );
     }
  }



  /* xaxis properties
   */

  switch( hl->data[0].xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    min.val_s32 = hl->data[0].xaxis.min.val_s32;
    max.val_s32 = hl->data[0].xaxis.max.val_s32;
    for ( i=1; i<hl->n; i++ ) {
      if ( min.val_s32 > hl->data[i].xaxis.min.val_s32 )
        min.val_s32 = hl->data[i].xaxis.min.val_s32;
      if ( max.val_s32 < hl->data[i].xaxis.max.val_s32 )
        max.val_s32 = hl->data[i].xaxis.max.val_s32;
    }
    if ( allocHistogramHeader( &(h->xaxis), &min, &max, 1.0, SINT ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: unable to allocate histogram xaxis\n", proc );
      return( -1 );
    }
    break;
  }

  min.val_s32 = 1;
  max.val_s32 = hl->n;
  if ( allocHistogramHeader( &(h->yaxis), &min, &max, 1.0, SINT ) != 1 ) {
    freeHistogramHeader(  &(h->xaxis) );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram yaxis\n", proc );
    return( -1 );
  }

  if ( allocHistogramData( h, hl->data[0].typeHisto ) != 1 ) {
    freeHistogramHeader(  &(h->yaxis) );
    freeHistogramHeader(  &(h->xaxis) );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }

  switch ( h->typeHisto ) {
  default :
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: such histogram type not handled yet\n", proc );
    return( -1 );
  case SINT :
    {
      s32 *buf = (s32*)h->data;
      for ( i=0; i<hl->n; i++ ) {
        buf += hl->data[i].xaxis.min.val_s32 - h->xaxis.min.val_s32;
        memcpy( buf, hl->data[i].data, hl->data[i].xaxis.dim * sizeof( s32 ) );
        buf += h->xaxis.max.val_s32 - hl->data[i].xaxis.max.val_s32;
      }
    }
    break;
  case UINT :
    {
      u32 *buf = (u32*)h->data;
      for ( i=0; i<hl->n; i++ ) {
        buf += hl->data[i].xaxis.min.val_u32 - h->xaxis.min.val_u32;
        memcpy( buf, hl->data[i].data, hl->data[i].xaxis.dim * sizeof( u32 ) );
        buf += hl->data[i].xaxis.dim;
        buf += h->xaxis.max.val_u32 - hl->data[i].xaxis.max.val_u32;
      }
    }
    break;
  }


  return( 1 );
}






int alloc2DHistogramFromImages( typeHistogram *h, 
                                void *theIm1,
                                bufferType theType1,
                                void *theMask1,
                                bufferType theMaskType1,
                                double *mat1,
                                void *theIm2,
                                bufferType theType2,
                                void *theMask2,
                                bufferType theMaskType2,
                                double *mat2,
                                int *theDim )
{
  return( alloc2DHistogramFromImagesAndBinLength( h, theIm1, theType1, theMask1, theMaskType1, mat1, 
                                                  theIm2, theType2, theMask2, theMaskType2, mat2, theDim, -1.0, -1.0 ) );
}





int alloc2DHistogramFromImagesAndBinLength( typeHistogram *h, 
                                           void *theIm1,
                                           bufferType theType1,
                                           void *theMask1,
                                           bufferType theMaskType1,
                                           double *mat1,
                                           void *theIm2,
                                           bufferType theType2,
                                           void *theMask2,
                                           bufferType theMaskType2,
                                           double *mat2,
                                           int *theDim,
                                           float binlength1,
                                           float binlength2 )
{
  char *proc = "alloc2DHistogramFromImagesAndBinLength";
  unionValues xmin, xmax;
  unionValues ymin, ymax;
  double mean, stddev;
  bufferType typeHisto = UINT;
  
  if ( _minMeanMaxStddevValuesWithMask( theIm1, theType1, theMask1, theMaskType1, theDim, &xmin, &mean, &xmax, &stddev ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute first order statistics\n", proc );
    return( -1 );
  }

  /* index type is SINT (signed int) for integer valued images if binlength is 1.0 or negative
   */
  if ( allocHistogramHeader( &(h->xaxis), &xmin, &xmax, binlength1, theType1 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }

  if ( _minMeanMaxStddevValuesWithMask( theIm2, theType2, theMask2, theMaskType2, theDim, &ymin, &mean, &ymax, &stddev ) != 1 ) {
    if ( _verbose_ ) 
      fprintf( stderr, "%s: unable to compute first order statistics\n", proc );
    return( -1 );
  }

  /* index type is SINT (signed int) for integer valued images if binlength is 1.0 or negative
   */
  if ( allocHistogramHeader( &(h->yaxis), &ymin, &ymax, binlength2, theType2 ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram header\n", proc );
    return( -1 );
  }


  if ( mat1 != (double*)NULL || mat2 != (double*)NULL ) 
    typeHisto = FLOAT;

  if ( allocHistogramData( h, typeHisto ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }
  
  if ( fill2DHistogramFromImages( h, 
                                 theIm1, theType1, theMask1, theMaskType1, mat1,
                                 theIm2, theType2, theMask2, theMaskType2, mat2, theDim  ) != 1 ) {
    freeHistogram( h );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to fill histogram from buffer\n", proc );
    return( -1 );
  }

  return( 1 );
}










/**********************************************************************
 *
 * histogram list construction tools
 *
 **********************************************************************/



void initHistogramList( typeHistogramList *hl )
{
  hl->data = (typeHistogram*)NULL;
  hl->n = 0;
}





void freeHistogramList( typeHistogramList *hl )
{
  int i;
  
  if ( hl->data != (typeHistogram*)NULL )
    for ( i=0; i<hl->n; i++ ) {
      freeHistogram( &(hl->data[i]) );
      initHistogram( &(hl->data[i]) );
    }
  initHistogramList( hl );
}





int allocHistogramList( typeHistogramList *hl, int n )
{
  char *proc = "allocHistogramList";
  int i;
  
  if ( n <= 0 ) {
    fprintf( stderr, "%s: negative dimension\n", proc );
    return( -1 );
  }

  hl->data = (typeHistogram*)malloc( n * sizeof(typeHistogram) );
  if ( hl->data == (typeHistogram*)NULL ) {
    fprintf( stderr, "%s: unable to allocate data\n", proc );
    return( -1 );
  }
  
  for ( i=0; i<n; i++ ) {
    initHistogram( &(hl->data[i]) );
  }
  hl->n = n;

  return( 1 );
}










/**********************************************************************
 *
 * histogram filling tools
 *
 **********************************************************************/



typedef struct {
  int i;
  int j;
  int k;
  double weight;
} typeWeightedContribution;





typedef struct {
  typeWeightedContribution contribution[8];
  int n;
} typeWeightedContributions;





/* determine les voisins et leur ponderation dans 
   une interpolation lineaire pour une matrice 4x4
   en coordonnees homogenes (unite = voxel !)
*/
static void _compute2DContributions( typeWeightedContributions *c,
                                     int i, int j, int k,
                                     double *mat,
                                     int *theDim )
{
  double x, y;
  double dx, dy, dxdy;
  double v;
  int ix, iy;
  
  c->n = 0;
  
  if ( mat == (double*)NULL ) {
    if ( i < 0 || i >= theDim[0] ) return;
    if ( j < 0 || j >= theDim[1] ) return;
    if ( k < 0 || k >= theDim[2] ) return;
    c->contribution[ c->n ].i = i;
    c->contribution[ c->n ].j = j;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = 1;
    c->n ++;
    return;
  }

  /* point hors image
   */
  x = mat[0] * i +  mat[1] * j +              mat[3];
  if ((x <= -0.5) || ( x >= (double)theDim[0] -0.5)) return;
  y = mat[4] * i +  mat[5] * j +              mat[7];
  if ((y <= -0.5) || ( y >= (double)theDim[1] -0.5)) return;

  if ( k < 0 || k >= theDim[2] ) return;

  ix = (int)x;      
  iy = (int)y;
  
  /* are we on the border or not ? */
  if ( (x > 0.0) && (ix < theDim[0]-1) &&
       (y > 0.0) && (iy < theDim[1]-1) ) {
    dx = x - ix;
    dy = y - iy;
    dxdy = dx*dy;
    /* we have
       v[5]=dxdy;         coefficient of tbuf(ix+1,iy+1)
       v[4]=dx-dxdy;      coefficient of tbuf(ix+1,iy  )
       v[1]=dy-dxdy;      coefficient of tbuf(ix  ,iy+1)
       v[0]=1-dx-dy+dxdy; coefficient of tbuf(ix  ,iy  )
    */
    v = dy-dxdy;
    
    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = 1-dx-v;
    c->n ++;
    
    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = dx-dxdy;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = v;
    c->n ++;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = dxdy;
    c->n ++;

    return;
  }

  /* here, we are sure we are on some border */
  if ( (x < 0.0) || (ix == theDim[0]-1) ) {

    /* we just look at y */
    if ( (y < 0.0) || (iy == theDim[1]-1) ) {

      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = k;
      c->contribution[ c->n ].weight = 1;
      c->n ++;

      return;
    }

    dy = y - iy;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = 1-dy;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = dy;
    c->n ++;

    return;
  }

  c->contribution[ c->n ].i = ix;
  c->contribution[ c->n ].j = iy;
  c->contribution[ c->n ].k = k;
  c->contribution[ c->n ].weight = 1-dx;
  c->n ++;

  c->contribution[ c->n ].i = ix+1;
  c->contribution[ c->n ].j = iy;
  c->contribution[ c->n ].k = k;
  c->contribution[ c->n ].weight = dx;
  c->n ++;
  
  return;
}





static void _compute3DContributions( typeWeightedContributions *c,
                                     int i, int j, int k,
                                     double *mat,
                                     int *theDim )
{
  double x, y, z;
  double dx, dy, dz, dxdy, dxdz, dydz, dxdydz;
  double v6, v5, v4;
  int ix, iy, iz;
  
  c->n = 0;

  if ( mat == (double*)NULL ) {
    if ( i < 0 || i >= theDim[0] ) return;
    if ( j < 0 || j >= theDim[1] ) return;
    if ( k < 0 || k >= theDim[2] ) return;
    c->contribution[ c->n ].i = i;
    c->contribution[ c->n ].j = j;
    c->contribution[ c->n ].k = k;
    c->contribution[ c->n ].weight = 1;
    c->n ++;
    return;
  }

  /* point hors image
   */
  x = mat[0] * i +  mat[1] * j + mat[2] * k + mat[3];
  if ((x <= -0.5) || ( x >= (double)theDim[0] -0.5)) return;
  y = mat[4] * i +  mat[5] * j + mat[6] * k + mat[7];
  if ((y <= -0.5) || ( y >= (double)theDim[1] -0.5)) return;
  z = mat[8] * i +  mat[9] * j + mat[10] * k + mat[11];
  if ((z <= -0.5) || ( z >= (double)theDim[2] -0.5)) return;

  ix = (int)x;      
  iy = (int)y;
  iz = (int)z;

  /* are we on the border or not ? */
  if ( (x > 0.0) && (ix < theDim[0]-1) &&
       (y > 0.0) && (iy < theDim[1]-1) &&
       (z > 0.0) && (iz < theDim[2]-1) ) {
    /* the corresponding point is in the box defined 
       by (ix[+1],iy[+1],iz[+1]) */
    dx = x - ix;
    dy = y - iy;
    dz = z - iz;
    dxdy = dx*dy;
    dxdz = dx*dz;
    dydz = dy*dz;
    dxdydz = dxdy*dz;

    /* we have
       v[7]=dxdydz;            coefficient of tbuf(ix+1,iy+1,iz+1)
       v[6]=dxdz-dxdydz;       coefficient of tbuf(ix+1,iy,  iz+1)
       v[5]=dxdy-dxdydz;       coefficient of tbuf(ix+1,iy+1,iz  )
       v[4]=dx-dxdy-v[6];      coefficient of tbuf(ix+1,iy  ,iz  )
       v[3]=dydz-dxdydz;       coefficient of tbuf(ix  ,iy+1,iz+1)
       v[2]=dz-dydz-v[6];      coefficient of tbuf(ix  ,iy  ,iz+1)
       v[1]=dy-dydz-v[5];      coefficient of tbuf(ix  ,iy+1,iz  )
       v[0]=1-dy-dz+dydz-v[4]; coefficient of tbuf(ix  ,iy  ,iz  )
    */
    v6 = dxdz-dxdydz;
    v5 = dxdy-dxdydz;
    v4 = dx-dxdy-v6;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = dxdydz;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = dydz-dxdydz;
    c->n ++;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = v6;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = dz-dydz-v6;
    c->n ++;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = v5;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = dy-dydz-v5;
    c->n ++;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = v4;
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = 1-dy-dz+dydz-v4;
    c->n ++;
    
    return;
  }

  /* here, we are sure we are on some border */

  if ( (x < 0.0) || (ix == theDim[0]-1) ) {
    if ( (y < 0.0) || (iy == theDim[1]-1) ) {
      if ( (z < 0.0) || (iz == theDim[2]-1) ) {

        c->contribution[ c->n ].i = ix;
        c->contribution[ c->n ].j = iy;
        c->contribution[ c->n ].k = iz;
        c->contribution[ c->n ].weight = 1;
        c->n ++;

        return;
      }
      
      dz = z - iz;
      
      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = iz;
      c->contribution[ c->n ].weight = 1-dz;
      c->n ++;
      
      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = iz+1;
      c->contribution[ c->n ].weight = dz;
      c->n ++;

      return;
    }

    dy = y - iy;

    if ( (z < 0.0) || (iz ==theDim[2]-1) ) {

      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = iz;
      c->contribution[ c->n ].weight = 1-dy;
      c->n ++;

      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy+1;
      c->contribution[ c->n ].k = iz;
      c->contribution[ c->n ].weight = dy;
      c->n ++;

      return;
    }
    
    dz = z - iz;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = (1-dy)*(1-dz);
    c->n ++;
      
    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = dy*(1-dz);
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = (1-dy)*dz;
    c->n ++;
    
    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy+1;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = dy*dz;
    c->n ++;

    return;
  }

  /* here we are sure that the border is either
     along the Y or the Z axis */
  dx = x - ix;

  if ( (y < 0.0) || (iy == theDim[1]-1) ) {
    if ( (z < 0.0) || (iz == theDim[2]-1) ) {
      
      c->contribution[ c->n ].i = ix;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = iz;
      c->contribution[ c->n ].weight = 1-dx;
      c->n ++;

      c->contribution[ c->n ].i = ix+1;
      c->contribution[ c->n ].j = iy;
      c->contribution[ c->n ].k = iz;
      c->contribution[ c->n ].weight = dx;
      c->n ++;
      
      return; 
    }

    dz = z - iz;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = (1-dx)*(1-dz);
    c->n ++;
      
    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz;
    c->contribution[ c->n ].weight = dx*(1-dz);
    c->n ++;

    c->contribution[ c->n ].i = ix;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = (1-dx)*dz;
    c->n ++;

    c->contribution[ c->n ].i = ix+1;
    c->contribution[ c->n ].j = iy;
    c->contribution[ c->n ].k = iz+1;
    c->contribution[ c->n ].weight = dx*dz;
    c->n ++;

    return;
  }

  dy = y - iy;

  c->contribution[ c->n ].i = ix;
  c->contribution[ c->n ].j = iy;
  c->contribution[ c->n ].k = iz;
  c->contribution[ c->n ].weight = (1-dx)*(1-dy);
  c->n ++;
  
  c->contribution[ c->n ].i = ix+1;
  c->contribution[ c->n ].j = iy;
  c->contribution[ c->n ].k = iz;
  c->contribution[ c->n ].weight = dx*(1-dy);
  c->n ++;
  
  c->contribution[ c->n ].i = ix;
  c->contribution[ c->n ].j = iy+1;
  c->contribution[ c->n ].k = iz;
  c->contribution[ c->n ].weight = (1-dx)*dy;
  c->n ++;
  
  c->contribution[ c->n ].i = ix+1;
  c->contribution[ c->n ].j = iy+1;
  c->contribution[ c->n ].k = iz;
  c->contribution[ c->n ].weight = dx*dy;
  c->n ++;
  
  return;
}








/**********************************************************************
 *
 * 1D histogram filling tools
 *
 **********************************************************************/



int fill1DHistogramFromImage( typeHistogram *h, 
                              void *theIm,
                              bufferType theType,
                              void *theMask,
                              bufferType theMaskType,
                              double *mat,
                              int *theDim )
{
  char *proc = "fill1DHistogramFromImage";
  size_t v = (size_t)theDim[0]*(size_t)theDim[1]*(size_t)theDim[2];

  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  if ( mat == (double*) NULL ) {
    if ( theMask == (void*)NULL ) 
      return( fill1DHistogramFromBuffer( h, theIm, theType, v ) );
    return( fill1DHistogramFromMaskedBuffer( h, theIm, theType, theMask, theMaskType, v ) );  
  }
  
  if ( theMask == (void*)NULL ) 
    return( fill1DHistogramFromResampledImage( h, theIm, theType, mat, theDim ) );
  
  return( fill1DHistogramFromMaskedResampledImage( h, theIm, theType, theMask, theMaskType, mat, theDim ) );

  return( -1 );
}





int fill1DHistogramFromBuffer( typeHistogram *h, 
                               void *theIm,
                               bufferType theType,
                               size_t v )
{
  char *proc = "fill1DHistogramFromBuffer";
  size_t n;

  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  zeroHistogram( h );

#define _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEINDEX min = h->xaxis.min.val_s32;                    \
  TYPEINDEX max = h->xaxis.max.val_s32;                    \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF *theBuf = (TYPEBUF*)theIm;                       \
  for ( n=0; n<v; n++, theBuf++ ) {                        \
    if ( (TYPEINDEX)*theBuf < min ) theHisto[0] ++;        \
    else if ( (TYPEINDEX)*theBuf > max ) theHisto[h->xaxis.dim-1] ++; \
    else theHisto[ (int)(*theBuf)-min ] ++;                \
  }                                                        \
}

#define _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEINDEX min = h->xaxis.min.val_r32;                    \
  TYPEINDEX max = h->xaxis.max.val_r32;                    \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF *theBuf = (TYPEBUF*)theIm;                       \
  for ( n=0; n<v; n++, theBuf++ ) {                        \
    if ( *theBuf < min ) theHisto[0] ++;                   \
    else if ( *theBuf > max ) theHisto[h->xaxis.dim-1] ++; \
    else theHisto[ (int)(((float)(*theBuf)-min)/h->xaxis.binlength + 0.5) ] ++; \
  }                                                        \
}

#define _FILL1DHISTO_INDEXS32_BUFFER( TYPEINDEX, TYPEHISTO ) {      \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, u8 );              \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, s8 );              \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, u16 );             \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, s16 );             \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, u32 );             \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXS32( TYPEINDEX, TYPEHISTO, s32 );             \
    break;                                                          \
  }                                                                 \
}

#define _FILL1DHISTO_INDEXR32_BUFFER( TYPEINDEX, TYPEHISTO ) {      \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, u8 );              \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, s8 );              \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, u16 );             \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, s16 );             \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, u32 );             \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, s32 );             \
    break;                                                          \
  case FLOAT :                                                      \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, r32 );             \
    break;                                                          \
  case DOUBLE :                                                     \
    _FILL1DHISTO_INDEXR32( TYPEINDEX, TYPEHISTO, r64 );             \
    break;                                                          \
  }                                                                 \
}

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    switch( h->typeHisto ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram type not handled yet\n", proc );
      return( -1 );
      break;
    case SINT :
      _FILL1DHISTO_INDEXS32_BUFFER( s32, s32 );
      break;
    case UINT :
      _FILL1DHISTO_INDEXS32_BUFFER( s32, u32 );
      break;
    case FLOAT :
      _FILL1DHISTO_INDEXS32_BUFFER( s32, r32 );
      break;
    case DOUBLE :
      _FILL1DHISTO_INDEXS32_BUFFER( s32, r64 );
      break;
    }
    break;
  case FLOAT :
    switch( h->typeHisto ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram type not handled yet\n", proc );
      return( -1 );
      break;
    case SINT :
      _FILL1DHISTO_INDEXR32_BUFFER( r32, s32 );
      break;
    case UINT :
      _FILL1DHISTO_INDEXR32_BUFFER( r32, u32 );
      break;
    case FLOAT :
      _FILL1DHISTO_INDEXR32_BUFFER( r32, r32 );
      break;
    case DOUBLE :
      _FILL1DHISTO_INDEXR32_BUFFER( r32, r64 );
      break;
    }
    break;
  }

  return( 1 );
}





int fill1DHistogramFromMaskedBuffer( typeHistogram *h, 
                                     void *theIm,
                                     bufferType theType,
                                     void *theMaskIm,
                                     bufferType theMaskType,
                                     size_t v )
{
  char *proc = "fill1DHistogramFromMaskedBuffer";
  size_t n;

  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );
  
  zeroHistogram( h );

#define _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEMASK *theMask = (TYPEMASK*)theMaskIm;                \
  TYPEINDEX min = h->xaxis.min.val_s32;                    \
  TYPEINDEX max = h->xaxis.max.val_s32;                    \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF *theBuf = (TYPEBUF*)theIm;                       \
  for ( n=0; n<v; n++, theBuf++, theMask++ ) {             \
    if ( *theMask <= 0 ) continue;                         \
    if ( (TYPEINDEX)*theBuf < min ) theHisto[0] ++;        \
    else if ( (TYPEINDEX)*theBuf > max ) theHisto[h->xaxis.dim-1] ++; \
    else theHisto[ (int)(*theBuf)-min ] ++;                \
  }                                                        \
}

#define _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEMASK *theMask = (TYPEMASK*)theMaskIm;                \
  TYPEINDEX min = h->xaxis.min.val_r32;                    \
  TYPEINDEX max = h->xaxis.max.val_r32;                    \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF *theBuf = (TYPEBUF*)theIm;                       \
  for ( n=0; n<v; n++, theBuf++, theMask++ ) {             \
    if ( *theMask <= 0 ) continue;                         \
    if ( *theBuf < min ) theHisto[0] ++;                   \
    else if ( *theBuf > max ) theHisto[h->xaxis.dim-1] ++; \
    else theHisto[ (int)(((float)(*theBuf)-min)/h->xaxis.binlength + 0.5) ] ++; \
  }                                                        \
}

#define _FILL1DHISTO_INDEXS32_MASKEDBUFFER( TYPEMASK, TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u8 ); \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s8 ); \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u16 ); \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s16 ); \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u32 ); \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s32 ); \
    break;                                                          \
  }                                                                 \
}

#define _FILL1DHISTO_INDEXR32_MASKEDBUFFER( TYPEMASK, TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u8 ); \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s8 ); \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u16 ); \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s16 ); \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, u32 ); \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, s32 ); \
    break;                                                          \
  case FLOAT :                                                      \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, r32 ); \
    break;                                                          \
  case DOUBLE :                                                     \
    _FILL1DHISTO_INDEXR32WITHMASK( TYPEMASK, TYPEINDEX, TYPEHISTO, r64 ); \
    break;                                                          \
  }                                                                 \
}




  switch( theMaskType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: mask type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch( h->xaxis.typeIndex ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: index type not handled yet\n", proc );
      return( -1 );
    case SINT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case SINT :
        _FILL1DHISTO_INDEXS32_MASKEDBUFFER( u8, s32, s32 );
        break;
      case UINT :
        _FILL1DHISTO_INDEXS32_MASKEDBUFFER( u8, s32, u32 );
        break;
      case FLOAT :
        _FILL1DHISTO_INDEXS32_MASKEDBUFFER( u8, s32, r32 );
        break;
      case DOUBLE :
        _FILL1DHISTO_INDEXS32_MASKEDBUFFER( u8, s32, r64 );
        break;
      }
      break;
    case FLOAT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case SINT :
        _FILL1DHISTO_INDEXR32_MASKEDBUFFER( u8, r32, s32 );
        break;
      case UINT :
        _FILL1DHISTO_INDEXR32_MASKEDBUFFER( u8, r32, u32 );
        break;
      case FLOAT :
        _FILL1DHISTO_INDEXR32_MASKEDBUFFER( u8, r32, r32 );
        break;
      case DOUBLE :
        _FILL1DHISTO_INDEXR32_MASKEDBUFFER( u8, r32, r64 );
        break;
      }
      break;
    }
    /* end of case theMaskType == UCHAR */
    break;
  }
  
  return( 1 );
}





int fill1DHistogramFromResampledImage( typeHistogram *h, 
                                       void *theIm,
                                       bufferType theType,
                                       double *mat,
                                       int *theDim )
{
  char *proc = "fill1DHistogramFromResampledImage";
  int i, j, k, n;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int dimxy = dimx*dimy;
  typeWeightedContributions c;
  
  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  zeroHistogram( h );

#define _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEINDEX min = h->xaxis.min.val_s32;                                  \
  TYPEINDEX max = h->xaxis.max.val_s32;                                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;                             \
  TYPEBUF v, *theBuf = (TYPEBUF*)theIm;                                  \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute2DContributions( &c, i, j, 0, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        v = theBuf[ c.contribution[n].j * dimx                           \
                    + c.contribution[n].i ];                             \
        if ( (TYPEINDEX)v < min ) theHisto[0] += c.contribution[n].weight; \
        else if ( (TYPEINDEX)v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(v)-min ] += c.contribution[n].weight;       \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( k=0; k<dimz; k++ )                                             \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute3DContributions( &c, i, j, k, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        v = theBuf[ c.contribution[n].k * dimxy                          \
                    + c.contribution[n].j * dimx                         \
                    + c.contribution[n].i ];                             \
        if ( (TYPEINDEX)v < min ) theHisto[0] += c.contribution[n].weight; \
        else if ( (TYPEINDEX)v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(v)-min ] += c.contribution[n].weight;       \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

#define _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEINDEX min = h->xaxis.min.val_r32;                                  \
  TYPEINDEX max = h->xaxis.max.val_r32;                                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;                             \
  TYPEBUF v, *theBuf = (TYPEBUF*)theIm;                                  \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute2DContributions( &c, i, j, 0, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        v = theBuf[ c.contribution[n].j * dimx                           \
                    + c.contribution[n].i ];                             \
        if ( v < min ) theHisto[0] += c.contribution[n].weight;          \
        else if ( v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(((float)(v)-min)/h->xaxis.binlength + 0.5) ] += c.contribution[n].weight; \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( k=0; k<dimz; k++ )                                             \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute3DContributions( &c, i, j, k, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        v = theBuf[ c.contribution[n].k * dimxy                          \
                    + c.contribution[n].j * dimx                         \
                    + c.contribution[n].i ];                             \
        if ( v < min ) theHisto[0] += c.contribution[n].weight;          \
        else if ( v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(((float)(v)-min)/h->xaxis.binlength + 0.5) ] += c.contribution[n].weight; \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

#define _FILL1DHISTO_INDEXS32_TRSFEDIMAGE( TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, u8 );      \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, s8 );      \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, u16 );     \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, s16 );     \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, u32 );     \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHTRSF( TYPEINDEX, TYPEHISTO, s32 );     \
    break;                                                          \
  }                                                                 \
}

#define _FILL1DHISTO_INDEXR32_TRSFEDIMAGE( TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, u8 );      \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, s8 );      \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, u16 );     \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, s16 );     \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, u32 );     \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, s32 );     \
    break;                                                          \
  case FLOAT :                                                      \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, r32 );     \
    break;                                                          \
  case DOUBLE :                                                     \
    _FILL1DHISTO_INDEXR32WITHTRSF( TYPEINDEX, TYPEHISTO, r64 );     \
    break;                                                          \
  }                                                                 \
}

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    switch( h->typeHisto ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram type not handled yet\n", proc );
      return( -1 );
      break;
    case FLOAT :
      _FILL1DHISTO_INDEXS32_TRSFEDIMAGE( s32, r32 );
      break;
    case DOUBLE :
      _FILL1DHISTO_INDEXS32_TRSFEDIMAGE( s32, r64 );
      break;
    }
    break;
  case FLOAT :
    switch( h->typeHisto ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram type not handled yet\n", proc );
      return( -1 );
      break;
    case FLOAT :
      _FILL1DHISTO_INDEXR32_TRSFEDIMAGE( r32, r32 );
      break;
    case DOUBLE :
      _FILL1DHISTO_INDEXR32_TRSFEDIMAGE( r32, r64 );
      break;
    }
    break;
  }

  return( 1 );
}





int fill1DHistogramFromMaskedResampledImage( typeHistogram *h, 
                                             void *theIm,
                                             bufferType theType,
                                             void *theMaskIm,
                                             bufferType theMaskType,
                                             double *mat,
                                             int *theDim )
{
  char *proc = "fill1DHistogramFromMaskedResampledImage";
  int i, j, k, n, index;
  int voxel;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int dimxy = dimx*dimy;
  typeWeightedContributions c;
  
  if ( _debug_ )
    fprintf( stderr, " ... entering %s\n", proc );

  zeroHistogram( h );

#define _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEMASK *theMask = (TYPEMASK*)theMaskIm;                              \
  TYPEINDEX min = h->xaxis.min.val_s32;                                  \
  TYPEINDEX max = h->xaxis.max.val_s32;                                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;                             \
  TYPEBUF v, *theBuf = (TYPEBUF*)theIm;                                  \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( voxel=0, j=0; j<dimy; j++ )                                    \
    for ( i=0; i<dimx; i++, voxel++ ) {                                  \
      if ( theMask[ voxel ] <= 0 ) continue;                             \
      _compute2DContributions( &c, i, j, 0, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        index = c.contribution[n].j * dimx + c.contribution[n].i;        \
        /* if ( theMask[ index ] <= 0 ) continue; */                     \
        v = theBuf[ index ];                                             \
        if ( (TYPEINDEX)v < min ) theHisto[0] += c.contribution[n].weight; \
        else if ( (TYPEINDEX)v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(v)-min ] += c.contribution[n].weight;       \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( voxel=0, k=0; k<dimz; k++ )                                    \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++, voxel++ ) {                                  \
      if ( theMask[ voxel ] <= 0 ) continue;                             \
      _compute3DContributions( &c, i, j, k, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        index = c.contribution[n].k * dimxy + c.contribution[n].j * dimx + c.contribution[n].i; \
        /* if ( theMask[ index ] <= 0 ) continue; */                     \
        v = theBuf[ index ];                                             \
        if ( (TYPEINDEX)v < min ) theHisto[0] += c.contribution[n].weight; \
        else if ( (TYPEINDEX)v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(v)-min ] += c.contribution[n].weight;       \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

#define _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, TYPEBUF ) { \
  TYPEMASK *theMask = (TYPEMASK*)theMaskIm;                              \
  TYPEINDEX min = h->xaxis.min.val_r32;                                  \
  TYPEINDEX max = h->xaxis.max.val_r32;                                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;                             \
  TYPEBUF v, *theBuf = (TYPEBUF*)theIm;                                  \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute2DContributions( &c, i, j, 0, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        index = c.contribution[n].j * dimx + c.contribution[n].i;        \
        if ( theMask[ index ] <= 0 ) continue;                           \
        v = theBuf[ index ];                                             \
        if ( v < min ) theHisto[0] += c.contribution[n].weight;          \
        else if ( v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(((float)(v)-min)/h->xaxis.binlength + 0.5) ] += c.contribution[n].weight; \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( k=0; k<dimz; k++ )                                             \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute3DContributions( &c, i, j, k, mat, theDim );               \
      if ( c.n == 0 ) continue;                                          \
      for ( n=0; n<c.n; n++ ) {                                          \
        index = c.contribution[n].k * dimxy + c.contribution[n].j * dimx + c.contribution[n].i; \
        if ( theMask[ index ] <= 0 ) continue;                           \
        v = theBuf[ index ];                                             \
        if ( v < min ) theHisto[0] += c.contribution[n].weight;          \
        else if ( v > max ) theHisto[h->xaxis.dim-1] += c.contribution[n].weight; \
        else theHisto[ (int)(((float)(v)-min)/h->xaxis.binlength + 0.5) ] += c.contribution[n].weight; \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}

#define _FILL1DHISTO_INDEXS32_MASKEDTRSFEDIMAGE( TYPEMASK, TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u8 ); \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s8 ); \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u16 ); \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s16 ); \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u32 ); \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXS32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s32 ); \
    break;                                                          \
  }                                                                 \
}

#define _FILL1DHISTO_INDEXR32_MASKEDTRSFEDIMAGE( TYPEMASK, TYPEINDEX, TYPEHISTO ) { \
  switch( theType ) {                                               \
  default :                                                         \
    if ( _verbose_ )                                                \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
    return( -1 );                                                   \
  case UCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u8 ); \
    break;                                                          \
  case SCHAR :                                                      \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s8 ); \
    break;                                                          \
  case USHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u16 ); \
    break;                                                          \
  case SSHORT :                                                     \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s16 ); \
    break;                                                          \
  case UINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, u32 ); \
    break;                                                          \
  case SINT :                                                       \
    _FILL1DHISTO_INDEXR32WITHMASKTRSF( TYPEMASK, TYPEINDEX, TYPEHISTO, s32 ); \
    break;                                                          \
  }                                                                 \
}


  switch( theMaskType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: mask type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch( h->xaxis.typeIndex ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: index type not handled yet\n", proc );
      return( -1 );
    case SINT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case FLOAT :
        _FILL1DHISTO_INDEXS32_MASKEDTRSFEDIMAGE( u8, s32, r32 );
        break;
      case DOUBLE :
        _FILL1DHISTO_INDEXS32_MASKEDTRSFEDIMAGE( u8, s32, r64 );
        break;
      }
      break;
    case FLOAT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case FLOAT :
        _FILL1DHISTO_INDEXR32_MASKEDTRSFEDIMAGE( u8, r32, r32 );
        break;
      case DOUBLE :
        _FILL1DHISTO_INDEXR32_MASKEDTRSFEDIMAGE( u8, r32, r64 );
        break;
      }
      break;
    }
    /* end of case theMaskType == UCHAR */
    break;
  }

  return( 1 );
}





int fill1DDifferenceHistogram( typeHistogram *hdiff,
                               typeHistogram *hjoint )
{
  char *proc = "fill1DDifferenceHistogram";
  int x, y, i;

  zeroHistogram( hdiff );

#define _FILLDIFFERENCES_S32S32S32( TYPEINDEXD, TYPEHISTOD, TYPEINDEXX, TYPEINDEXY, TYPEHISTOJ ) { \
  TYPEINDEXD dmin = hdiff->xaxis.min.val_s32;                           \
  TYPEINDEXD dmax = hdiff->xaxis.max.val_s32;                           \
  TYPEINDEXX *theX = hjoint->xaxis.index;                               \
  TYPEINDEXY *theY = hjoint->yaxis.index;                               \
  TYPEHISTOD *theDiff = (TYPEHISTOD *)hdiff->data;                      \
  TYPEHISTOJ *theJoint = (TYPEHISTOJ *)hjoint->data;                    \
  for ( y=0; y<hjoint->yaxis.dim; y++ )                                 \
  for ( x=0; x<hjoint->xaxis.dim; x++ ) {                               \
    i = theY[y] - theX[x];                                              \
    if ( i < dmin ) theDiff[0] += theJoint[ y * hjoint->xaxis.dim + x]; \
    else if ( i > dmax ) theDiff[hdiff->xaxis.dim-1] += theJoint[ y * hjoint->xaxis.dim + x]; \
    else theDiff[i-dmin] += theJoint[ y * hjoint->xaxis.dim + x];       \
  }                                                                     \
}

#define _FILLDIFFERENCES_S32S32S32_DIFFERENCE( TYPEINDEXD, TYPEHISTOD ) { \
  switch( hjoint->xaxis.typeIndex ) {                                     \
  default :                                                               \
    if ( _verbose_ )                                                      \
      fprintf( stderr, "%s: x index type not handled yet\n", proc );      \
    return( -1 );                                                         \
  case SINT :                                                             \
    switch( hjoint->yaxis.typeIndex ) {                                   \
    default :                                                             \
      if ( _verbose_ )                                                    \
        fprintf( stderr, "%s: y index type not handled yet\n", proc );    \
      return( -1 );                                                       \
    case SINT :                                                           \
      switch( hjoint->typeHisto ) {                                       \
      default :                                                           \
        if ( _verbose_ )                                                  \
          fprintf( stderr, "%s: joint, histogram type not handled yet\n", proc ); \
        return( -1 );                                                     \
        break;                                                            \
      case SINT :                                                         \
        _FILLDIFFERENCES_S32S32S32( TYPEINDEXD, TYPEHISTOD, s32, s32, s32 ); \
        break;                                                            \
      case UINT :                                                         \
        _FILLDIFFERENCES_S32S32S32( TYPEINDEXD, TYPEHISTOD, s32, s32, u32 ); \
        break;                                                            \
      case FLOAT :                                                        \
        _FILLDIFFERENCES_S32S32S32( TYPEINDEXD, TYPEHISTOD, s32, s32, r32 ); \
        break;                                                            \
      case DOUBLE :                                                       \
        _FILLDIFFERENCES_S32S32S32( TYPEINDEXD, TYPEHISTOD, s32, s32, r64 ); \
        break;                                                            \
      }                                                                   \
      break;                                                              \
    }                                                                     \
    break;                                                                \
  }                                                                       \
}

  switch( hdiff->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    switch( hdiff->typeHisto ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: histogram type not handled yet\n", proc );
      return( -1 );
      break;
    case SINT :
      _FILLDIFFERENCES_S32S32S32_DIFFERENCE( s32, s32 );
      break;
    case UINT :
      _FILLDIFFERENCES_S32S32S32_DIFFERENCE( s32, u32 );
      break;
    case FLOAT :
      _FILLDIFFERENCES_S32S32S32_DIFFERENCE( s32, r32 );
      break;
    case DOUBLE :
      _FILLDIFFERENCES_S32S32S32_DIFFERENCE( s32, r64 );
      break;
    }
    break;
  }

  return( 1 );
}









/**********************************************************************
 *
 * 2D histogram filling tools
 *
 **********************************************************************/



int fill2DHistogramFromImages( typeHistogram *h, 
                               void *theIm1,
                               bufferType theType1,
                               void *theMask1,
                               bufferType theMaskType1,
                               double *mat1,
                               void *theIm2,
                               bufferType theType2,
                               void *theMask2,
                               bufferType theMaskType2,
                               double *mat2,
                               int *theDim )

{
  size_t v = (size_t)theDim[0]*(size_t)theDim[1]*(size_t)theDim[2];

  if ( mat1 == (double*) NULL && mat2 == (double*) NULL ) {
    if ( theMask1 == (void*)NULL && theMask2 == (void*)NULL ) {
      return( fill2DHistogramFromBuffers( h, 
                                          theIm1, theType1,
                                          theIm2, theType2, v ) );
    }
    else if ( theMask1 != (void*)NULL && theMask2 != (void*)NULL ) {
      return( fill2DHistogramFromMaskedBuffers( h, 
                                                theIm1, theType1, theMask1, theMaskType1,
                                                theIm2, theType2, theMask2, theMaskType2, v ) );
    }
    else {
      return( -1 );
    }
  }

  if ( theMask1 == (void*)NULL && theMask2 == (void*)NULL ) { 
    return( fill2DHistogramFromResampledImages( h, 
                                                theIm1, theType1, mat1,
                                                theIm2, theType2, mat2, theDim ) );
  }
  else if ( theMask1 != (void*)NULL && theMask2 != (void*)NULL ) {
    return( fill2DHistogramFromMaskedResampledImages( h, 
                                                      theIm1, theType1, theMask1, theMaskType1, mat1,
                                                      theIm2, theType2, theMask2, theMaskType2, mat2, theDim ) );
  }

  return( -1 );
}







int fill2DHistogramFromBuffers( typeHistogram *h, 
                                void *theIm1,
                                bufferType theType1,
                                void *theIm2,
                                bufferType theType2,
                                size_t v )
{
  char *proc = "fill2DHistogramFromBuffers";
  size_t n;

  zeroHistogram( h );

#define _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, TYPEBUF1, TYPEBUF2 ) { \
  TYPEINDEXX xmin = h->xaxis.min.val_s32;                  \
  TYPEINDEXX xmax = h->xaxis.max.val_s32;                  \
  TYPEINDEXY ymin = h->yaxis.min.val_s32;                  \
  TYPEINDEXY ymax = h->yaxis.max.val_s32;                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF1 *theBuf1 = (TYPEBUF1*)theIm1;                   \
  TYPEBUF2 *theBuf2 = (TYPEBUF2*)theIm2;                   \
  int xh, yh;                                              \
  for ( n=0; n<v; n++, theBuf1++, theBuf2++ ) {            \
    if ( (TYPEINDEXX)*theBuf1 < xmin ) xh = 0;             \
    else if ( (TYPEINDEXX)*theBuf1 > xmax ) xh = h->xaxis.dim-1; \
    else xh = (int)(*theBuf1) - xmin;                      \
    if ( (TYPEINDEXY)*theBuf2 < ymin ) yh = 0;             \
    else if ( (TYPEINDEXY)*theBuf2 > ymax ) yh = h->yaxis.dim-1; \
    else yh = (int)(*theBuf2) - ymin;                      \
    theHisto[ yh * h->xaxis.dim + xh ] ++;                 \
  }                                                        \
}

#define _FILL2DHISTO_INDEXS32S32_BUFFERS( TYPEINDEXX, TYPEINDEXY, TYPEHISTO ) { \
  switch( theType1 ) {                                                \
  default :                                                           \
    if ( _verbose_ )                                                  \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );   \
    return( -1 );                                                     \
  case UCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == UCHAR */                                              \
    break;                                                              \
  case SCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SCHAR */                                              \
    break;                                                              \
  case USHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == USHORT */                                              \
    break;                                                              \
  case SSHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SSHORT */                                              \
    break;                                                              \
  }                                                                   \
}

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: x index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    switch( h->yaxis.typeIndex ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: y index type not handled yet\n", proc );
      return( -1 );
    case SINT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case SINT :
        _FILL2DHISTO_INDEXS32S32_BUFFERS( s32, s32, s32 );
        break;
      case UINT :
        _FILL2DHISTO_INDEXS32S32_BUFFERS( s32, s32, u32 );
        break;
      case FLOAT :
        _FILL2DHISTO_INDEXS32S32_BUFFERS( s32, s32, r32 );
        break;
      case DOUBLE :
        _FILL2DHISTO_INDEXS32S32_BUFFERS( s32, s32, r64 );
        break;
      }
      /* h->yaxis.typeIndex = SINT */
      break;
    }
    /* h->xaxis.typeIndex = SINT */
    break;
  }

  return( 1 );
}







int fill2DHistogramFromMaskedBuffers( typeHistogram *h, 
                                      void *theIm1,
                                      bufferType theType1,
                                      void *theMaskIm1,
                                      bufferType theMaskType1,
                                      void *theIm2,
                                      bufferType theType2,
                                      void *theMaskIm2,
                                      bufferType theMaskType2,
                                      size_t v )
{
  char *proc = "fill2DHistogramFromMaskedBuffers";
  size_t n;

 zeroHistogram( h );

#define _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, TYPEBUF1, TYPEBUF2 ) { \
  TYPEMASK1 *theMask1 = (TYPEMASK1*)theMaskIm1;            \
  TYPEMASK2 *theMask2 = (TYPEMASK2*)theMaskIm2;            \
  TYPEINDEXX xmin = h->xaxis.min.val_s32;                  \
  TYPEINDEXX xmax = h->xaxis.max.val_s32;                  \
  TYPEINDEXY ymin = h->yaxis.min.val_s32;                  \
  TYPEINDEXY ymax = h->yaxis.max.val_s32;                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF1 *theBuf1 = (TYPEBUF1*)theIm1;                   \
  TYPEBUF2 *theBuf2 = (TYPEBUF2*)theIm2;                   \
  int xh, yh;                                              \
  for ( n=0; n<v; n++, theBuf1++, theMask1++, theBuf2++, theMask2++ ) { \
    if ( *theMask1 <= 0 ) continue;                        \
    if ( *theMask2 <= 0 ) continue;                        \
    if ( (TYPEINDEXX)*theBuf1 < xmin ) xh = 0;             \
    else if ( (TYPEINDEXX)*theBuf1 > xmax ) xh = h->xaxis.dim-1; \
    else xh = (int)(*theBuf1) - xmin;                      \
    if ( (TYPEINDEXY)*theBuf2 < ymin ) yh = 0;             \
    else if ( (TYPEINDEXY)*theBuf2 > ymax ) yh = h->yaxis.dim-1; \
    else yh = (int)(*theBuf2) - ymin;                      \
    theHisto[ yh * h->xaxis.dim + xh ] ++;                 \
  }                                                        \
}

#define _FILL2DHISTO_INDEXS32S32_MASKEDBUFFERS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO ) { \
  switch( theType1 ) {                                                \
  default :                                                           \
    if ( _verbose_ )                                                  \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );   \
    return( -1 );                                                     \
  case UCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == UCHAR */                                           \
    break;                                                            \
  case SCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SCHAR */                                           \
    break;                                                            \
  case USHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == USHORT */                                          \
    break;                                                            \
  case SSHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKS( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SSHORT */                                              \
    break;                                                              \
  }                                                                   \
}


  switch( theMaskType1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: mask type #1 not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch( theMaskType2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: mask type #2 not handled yet\n", proc );
      return( -1 );
    case UCHAR :
      switch( h->xaxis.typeIndex ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: x index type not handled yet\n", proc );
        return( -1 );
      case SINT :
        switch( h->yaxis.typeIndex ) {
        default :
          if ( _verbose_ )
            fprintf( stderr, "%s: y index type not handled yet\n", proc );
          return( -1 );
        case SINT :
          switch( h->typeHisto ) {
          default :
            if ( _verbose_ )
              fprintf( stderr, "%s: histogram type not handled yet\n", proc );
            return( -1 );
            break;
          case SINT :
            _FILL2DHISTO_INDEXS32S32_MASKEDBUFFERS( u8, u8, s32, s32, s32 );
            break;
          case UINT :
            _FILL2DHISTO_INDEXS32S32_MASKEDBUFFERS( u8, u8, s32, s32, u32 );
            break;
          case FLOAT :
            _FILL2DHISTO_INDEXS32S32_MASKEDBUFFERS( u8, u8, s32, s32, r32 );
            break;
          case DOUBLE :
            _FILL2DHISTO_INDEXS32S32_MASKEDBUFFERS( u8, u8, s32, s32, r64 );
            break;
          }
          /* h->yaxis.typeIndex = SINT */
          break;
        }
        /* h->xaxis.typeIndex = SINT */
        break;
      }
      /* theMaskType2 == UCHAR */
      break;
    }
    /* theMaskType1 == UCHAR */
    break;
  }
  
  return( 1 );
}









int fill2DHistogramFromResampledImages( typeHistogram *h, 
                                        void *theIm1,
                                        bufferType theType1,
                                        double *mat1,
                                        void *theIm2,
                                        bufferType theType2,
                                        double *mat2,
                                        int *theDim )
{
  char *proc = "fill2DHistogramFromResampledImages";
  int i, j, k, n1, n2;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int dimxy = dimx*dimy;
  typeWeightedContributions c1, c2;

  zeroHistogram( h );

#define _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, TYPEBUF1, TYPEBUF2 ) { \
  TYPEINDEXX xmin = h->xaxis.min.val_s32;                  \
  TYPEINDEXX xmax = h->xaxis.max.val_s32;                  \
  TYPEINDEXY ymin = h->yaxis.min.val_s32;                  \
  TYPEINDEXY ymax = h->yaxis.max.val_s32;                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF1 v1, *theBuf1 = (TYPEBUF1*)theIm1;               \
  TYPEBUF2 v2, *theBuf2 = (TYPEBUF2*)theIm2;               \
  int xh, yh;                                              \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute2DContributions( &c1, i, j, 0, mat1, theDim );             \
      if ( c1.n == 0 ) continue;                                         \
      _compute2DContributions( &c2, i, j, 0, mat2, theDim );             \
      if ( c2.n == 0 ) continue;                                         \
      for ( n1=0; n1<c1.n; n1++ ) {                                      \
        v1 = theBuf1[ c1.contribution[n1].j * dimx                       \
                    + c1.contribution[n1].i ];                           \
        if ( (TYPEINDEXX)v1 < xmin ) xh = 0;                             \
        else if ( (TYPEINDEXX)v1 > xmax ) xh = h->xaxis.dim-1;           \
        else xh = (int)(v1) - xmin;                                      \
        for ( n2=0; n2<c2.n; n2++ ) {                                    \
          v2 = theBuf2[ c2.contribution[n2].j * dimx                     \
                      + c2.contribution[n2].i ];                         \
          if ( (TYPEINDEXY)v2 < ymin ) yh = 0;                           \
          else if ( (TYPEINDEXY)v2 > ymax ) yh = h->yaxis.dim-1;         \
          else yh = (int)(v2) - ymin;                                    \
          theHisto[ yh * h->xaxis.dim + xh ] += c1.contribution[n1].weight * c2.contribution[n2].weight; \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( k=0; k<dimz; k++ )                                             \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++ ) {                                           \
      _compute3DContributions( &c1, i, j, k, mat1, theDim );             \
      if ( c1.n == 0 ) continue;                                         \
      _compute3DContributions( &c2, i, j, k, mat2, theDim );             \
      if ( c2.n == 0 ) continue;                                         \
      for ( n1=0; n1<c1.n; n1++ ) {                                      \
        v1 = theBuf1[ c1.contribution[n1].k * dimxy                      \
                    + c1.contribution[n1].j * dimx                       \
                    + c1.contribution[n1].i ];                           \
        if ( (TYPEINDEXX)v1 < xmin ) xh = 0;                             \
        else if ( (TYPEINDEXX)v1 > xmax ) xh = h->xaxis.dim-1;           \
        else xh = (int)(v1) - xmin;                                      \
        for ( n2=0; n2<c2.n; n2++ ) {                                    \
          v2 = theBuf2[ c2.contribution[n2].k * dimxy                    \
                        + c2.contribution[n2].j * dimx                   \
                      + c2.contribution[n2].i ];                         \
          if ( (TYPEINDEXY)v2 < ymin ) yh = 0;                           \
          else if ( (TYPEINDEXY)v2 > ymax ) yh = h->yaxis.dim-1;         \
          else yh = (int)(v2) - ymin;                                    \
          theHisto[ yh * h->xaxis.dim + xh ] += c1.contribution[n1].weight * c2.contribution[n2].weight; \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}


#define _FILL2DHISTO_INDEXS32S32_TRSFEDIMAGES( TYPEINDEXX, TYPEINDEXY, TYPEHISTO ) { \
  switch( theType1 ) {                                                \
  default :                                                           \
    if ( _verbose_ )                                                  \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );   \
    return( -1 );                                                     \
  case UCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == UCHAR */                                              \
    break;                                                              \
  case SCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SCHAR */                                              \
    break;                                                              \
  case USHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == USHORT */                                              \
    break;                                                              \
  case SSHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHTRSF( TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SSHORT */                                              \
    break;                                                              \
  }                                                                   \
}

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: x index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    switch( h->yaxis.typeIndex ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: y index type not handled yet\n", proc );
      return( -1 );
    case SINT :
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
        break;
      case SINT :
        _FILL2DHISTO_INDEXS32S32_TRSFEDIMAGES( s32, s32, s32 );
        break;
      case UINT :
        _FILL2DHISTO_INDEXS32S32_TRSFEDIMAGES( s32, s32, u32 );
        break;
      case FLOAT :
        _FILL2DHISTO_INDEXS32S32_TRSFEDIMAGES( s32, s32, r32 );
        break;
      case DOUBLE :
        _FILL2DHISTO_INDEXS32S32_TRSFEDIMAGES( s32, s32, r64 );
        break;
      }
      /* h->yaxis.typeIndex = SINT */
      break;
    }
    /* h->xaxis.typeIndex = SINT */
    break;
  }

  return( 1 );
}









int fill2DHistogramFromMaskedResampledImages( typeHistogram *h, 
                                              void *theIm1,
                                              bufferType theType1,
                                              void *theMaskIm1,
                                              bufferType theMaskType1,
                                              double *mat1,
                                              void *theIm2,
                                              bufferType theType2,
                                              void *theMaskIm2,
                                              bufferType theMaskType2,
                                              double *mat2,
                                              int *theDim )
{
  char *proc = "fill2DHistogramFromMaskedResampledImages";
  int i, j, k, n1, n2, index1, index2;
  int voxel;
  int dimx = theDim[0];
  int dimy = theDim[1];
  int dimz = theDim[2];
  int dimxy = dimx*dimy;
  typeWeightedContributions c1, c2;

  zeroHistogram( h );

#define _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, TYPEBUF1, TYPEBUF2 ) { \
  TYPEMASK1 *theMask1 = (TYPEMASK1*)theMaskIm1;            \
  TYPEMASK2 *theMask2 = (TYPEMASK2*)theMaskIm2;            \
  TYPEINDEXX xmin = h->xaxis.min.val_s32;                  \
  TYPEINDEXX xmax = h->xaxis.max.val_s32;                  \
  TYPEINDEXY ymin = h->yaxis.min.val_s32;                  \
  TYPEINDEXY ymax = h->yaxis.max.val_s32;                  \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;               \
  TYPEBUF1 v1, *theBuf1 = (TYPEBUF1*)theIm1;               \
  TYPEBUF2 v2, *theBuf2 = (TYPEBUF2*)theIm2;               \
  int xh, yh;                                              \
  /* on reechantillonne dans une grille de meme dimension                \
     que l'image */                                                      \
  if ( theDim[2] == 1 ) {                                                \
    for ( voxel=0, j=0; j<dimy; j++ )                                    \
    for ( i=0; i<dimx; i++, voxel++ ) {                                  \
      if ( theMask1[ voxel ] <= 0 ) continue;                            \
      if ( theMask2[ voxel ] <= 0 ) continue;                            \
      _compute2DContributions( &c1, i, j, 0, mat1, theDim );             \
      if ( c1.n == 0 ) continue;                                         \
      _compute2DContributions( &c2, i, j, 0, mat2, theDim );             \
      if ( c2.n == 0 ) continue;                                         \
      for ( n1=0; n1<c1.n; n1++ ) {                                      \
        index1 = c1.contribution[n1].j * dimx + c1.contribution[n1].i;   \
        /* if ( theMask1[ index1 ] <= 0 ) continue; */                   \
        v1 = theBuf1[ index1 ];                                          \
        if ( (TYPEINDEXX)v1 < xmin ) xh = 0;                             \
        else if ( (TYPEINDEXX)v1 > xmax ) xh = h->xaxis.dim-1;           \
        else xh = (int)(v1) - xmin;                                      \
        for ( n2=0; n2<c2.n; n2++ ) {                                    \
          index2 = c2.contribution[n2].j * dimx + c2.contribution[n2].i; \
          /* if ( theMask2[ index2 ] <= 0 ) continue; */                 \
          v2 = theBuf2[ index2 ];                                        \
          if ( (TYPEINDEXY)v2 < ymin ) yh = 0;                           \
          else if ( (TYPEINDEXY)v2 > ymax ) yh = h->yaxis.dim-1;         \
          else yh = (int)(v2) - ymin;                                    \
          theHisto[ yh * h->xaxis.dim + xh ] += c1.contribution[n1].weight * c2.contribution[n2].weight; \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
  else {                                                                 \
    for ( voxel=0, k=0; k<dimz; k++ )                                    \
    for ( j=0; j<dimy; j++ )                                             \
    for ( i=0; i<dimx; i++, voxel++ ) {                                  \
      if ( theMask1[ voxel ] <= 0 ) continue;                            \
      if ( theMask2[ voxel ] <= 0 ) continue;                            \
      _compute3DContributions( &c1, i, j, k, mat1, theDim );             \
      if ( c1.n == 0 ) continue;                                         \
      _compute3DContributions( &c2, i, j, k, mat2, theDim );             \
      if ( c2.n == 0 ) continue;                                         \
      for ( n1=0; n1<c1.n; n1++ ) {                                      \
        index1 = c1.contribution[n1].k * dimxy + c1.contribution[n1].j * dimx + c1.contribution[n1].i; \
        /* if ( theMask1[ index1 ] <= 0 ) continue; */                   \
        v1 = theBuf1[ index1 ];                                          \
        if ( (TYPEINDEXX)v1 < xmin ) xh = 0;                             \
        else if ( (TYPEINDEXX)v1 > xmax ) xh = h->xaxis.dim-1;           \
        else xh = (int)(v1) - xmin;                                      \
        for ( n2=0; n2<c2.n; n2++ ) {                                    \
          index2 = c2.contribution[n2].k * dimxy + c2.contribution[n2].j * dimx + c2.contribution[n2].i; \
          /* if ( theMask2[ index2 ] <= 0 ) continue; */                 \
          v2 = theBuf2[ index2 ];                                        \
          if ( (TYPEINDEXY)v2 < ymin ) yh = 0;                           \
          else if ( (TYPEINDEXY)v2 > ymax ) yh = h->yaxis.dim-1;         \
          else yh = (int)(v2) - ymin;                                    \
          theHisto[ yh * h->xaxis.dim + xh ] += c1.contribution[n1].weight * c2.contribution[n2].weight; \
        }                                                                \
      }                                                                  \
    }                                                                    \
  }                                                                      \
}


#define _FILL2DHISTO_INDEXS32S32_MASKEDTRSFEDIMAGES( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO ) { \
  switch( theType1 ) {                                                \
  default :                                                           \
    if ( _verbose_ )                                                  \
      fprintf( stderr, "%s: buffer type not handled yet\n", proc );   \
    return( -1 );                                                     \
  case UCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == UCHAR */                                              \
    break;                                                              \
  case SCHAR :                                                        \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s8, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SCHAR */                                              \
    break;                                                              \
  case USHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, u16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == USHORT */                                              \
    break;                                                              \
  case SSHORT :                                                       \
    switch( theType2 ) {                                              \
    default :                                                         \
      if ( _verbose_ )                                                \
        fprintf( stderr, "%s: buffer type not handled yet\n", proc ); \
      return( -1 );                                                   \
    case UCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u8 ); \
      break;                                                          \
    case SCHAR :                                                      \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s8 ); \
      break;                                                          \
    case USHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u16 ); \
      break;                                                          \
    case SSHORT :                                                     \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s16 ); \
      break;                                                          \
    case UINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, u32 ); \
      break;                                                          \
    case SINT :                                                       \
      _FILL2DHISTO_INDEXS32S32WITHMASKSTRSF( TYPEMASK1, TYPEMASK2, TYPEINDEXX, TYPEINDEXY, TYPEHISTO, s16, s32 ); \
      break;                                                          \
    }                                                                 \
    /* theType1 == SSHORT */                                              \
    break;                                                              \
  }                                                                   \
}
  
  switch( theMaskType1 ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: mask type #1 not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    switch( theMaskType2 ) {
    default :
      if ( _verbose_ )
        fprintf( stderr, "%s: mask type #2 not handled yet\n", proc );
      return( -1 );
    case UCHAR :  switch( h->xaxis.typeIndex ) {
      default :
        if ( _verbose_ )
      fprintf( stderr, "%s: x index type not handled yet\n", proc );
        return( -1 );
      case SINT :
        switch( h->yaxis.typeIndex ) {
        default :
          if ( _verbose_ )
            fprintf( stderr, "%s: y index type not handled yet\n", proc );
          return( -1 );
        case SINT :
          switch( h->typeHisto ) {
          default :
            if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
            return( -1 );
            break;
          case SINT :
            _FILL2DHISTO_INDEXS32S32_MASKEDTRSFEDIMAGES( u8, u8, s32, s32, s32 );
            break;
          case UINT :
            _FILL2DHISTO_INDEXS32S32_MASKEDTRSFEDIMAGES( u8, u8, s32, s32, u32 );
            break;
          case FLOAT :
            _FILL2DHISTO_INDEXS32S32_MASKEDTRSFEDIMAGES( u8, u8, s32, s32, r32 );
        break;
          case DOUBLE :
            _FILL2DHISTO_INDEXS32S32_MASKEDTRSFEDIMAGES( u8, u8, s32, s32, r64 );
            break;
          }
          /* h->yaxis.typeIndex = SINT */
          break;
        }
        /* h->xaxis.typeIndex = SINT */
        break;
      }
       /* theMaskType2 == UCHAR */
      break;
    }
    /* theMaskType1 == UCHAR */
    break;
  }

  return( 1 );
}











/**********************************************************************
 *
 * histogram tools
 *
 **********************************************************************/



float maxhistogram( typeHistogram *h )
{
  char *proc = "maxhistogram";
  float max;
  int i, n;

  if( h->xaxis.dim <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no index values?\n", proc );
    return( -1 );
  }
  n = h->xaxis.dim;
  if ( h->yaxis.dim > 1 ) 
    n *= h->yaxis.dim;


  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1.0 );
  case SINT :
    {
      s32 *theHisto = (s32*)h->data;
      max = theHisto[0];
      for ( i=1; i<n; i++ )
        if ( max < theHisto[i] ) max = theHisto[i];
    }
    break;
  case UINT :
    {
      u32 *theHisto = (u32*)h->data;
      max = theHisto[0];
      for ( i=1; i<n; i++ )
        if ( max < theHisto[i] ) max = theHisto[i];
    }
    break;
  case FLOAT :
    {
      r32 *theHisto = (r32*)h->data;
      max = theHisto[0];
      for ( i=1; i<n; i++ )
        if ( max < theHisto[i] ) max = theHisto[i];
    }
    break;
  case DOUBLE :
    {
      r64 *theHisto = (r64*)h->data;
      max = theHisto[0];
      for ( i=1; i<n; i++ )
        if ( max < theHisto[i] ) max = theHisto[i];
    }
    break;
  }

  return( max );
}





static double sumhistogram( typeHistogram *h )
{
  char *proc = "sumhistogram";
  double sum=0.0;
  int i, n;

  if( h->xaxis.dim <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no index values?\n", proc );
    return( -1 );
  }
  n = h->xaxis.dim;
  if ( h->yaxis.dim > 1 ) 
    n *= h->yaxis.dim;


  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1.0 );
  case SINT :
    {
      s32 *theHisto = (s32*)h->data;
      for ( i=0; i<n; i++ )
        sum += theHisto[i];
    }
    break;
  case UINT :
    {
      u32 *theHisto = (u32*)h->data;
      for ( i=0; i<n; i++ )
        sum += theHisto[i];
    }
    break;
  case FLOAT :
    {
      r32 *theHisto = (r32*)h->data;
      for ( i=0; i<n; i++ )
        sum += theHisto[i];
    }
    break;
  }

  return( sum );
}





static double quantilehistogram ( typeHistogram *h, double quantile )
{
  char *proc = "quantilehistogram";
  int i, j, n;
  int iLeft, iRight, iLast;
  int iQuant;

  if( h->xaxis.dim <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no index values?\n", proc );
    return( -1 );
  }
  n = h->xaxis.dim;
  if ( h->yaxis.dim > 1 ) 
    n *= h->yaxis.dim;

  iQuant = (int)(quantile * n + 0.5 );
  

#define _COMPUTE_QUANTILE_( TYPE ) {                               \
  TYPE RES, TMP, *TAB = (TYPE*)NULL;                                   \
  TAB = (TYPE*)malloc( n * sizeof( TYPE ) );                       \
  if ( TAB == (TYPE*)NULL ) {                                      \
    if ( _verbose_ )                                               \
      fprintf( stderr, "%s: allocation failed\n", proc );          \
    return( -1.0 );                                                \
  }                                                                \
  (void)memcpy( TAB, h->data,  n * sizeof( TYPE ) );               \
  iLeft = 0;   iRight = n - 1;                                                \
  do {                                                                   \
    /* swap left et (left+right)/2 */                                   \
    j = (iLeft+iRight)/2;                                           \
    TMP = TAB[iLeft];   TAB[iLeft] = TAB[j];   TAB[j] = TMP;           \
    /* cut TAB into two */                                           \
    iLast = iLeft;                                                   \
    for ( i = iLeft+1; i <= iRight; i++ ) {                           \
      if ( TAB[i] < TAB[iLeft] ) {                                   \
        iLast ++;                                                   \
        TMP = TAB[i];   TAB[i] = TAB[iLast];   TAB[iLast] = TMP;   \
      }                                                                   \
    }                                                                   \
    TMP = TAB[iLeft];   TAB[iLeft] = TAB[iLast]; TAB[iLast] = TMP; \
                                                                   \
    if ( iLast >  iQuant ) iRight = iLast - 1;                           \
    if ( iLast <  iQuant ) iLeft  = iLast + 1;                           \
  } while ( iLast != iQuant );                                           \
  RES = TAB[iQuant];                                                       \
  free( TAB );                                                     \
  return( RES );                                                   \
}

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1.0 );
  case UINT :
    _COMPUTE_QUANTILE_( u32 );
    break;
  }

  return( -1.0 );
}




static void fprintfHistogramAxisInfo( FILE *f, typeHistogramHeader *h, char *prefix )
{
  char *proc = "fprintfHistogramAxisInfo";

  fprintf( f, "  %s-axis-dim: %d\n", prefix, h->dim );
  fprintf( f, "  %s-axis-bin-length: %f\n", prefix, h->binlength );
  switch( h->typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such axis type not handled yet\n", proc );
    return;
  case SINT :
    fprintf( f, "  %s-axis-type: SINT\n", prefix );
    fprintf( f, "  %s-axis-minimal-value: %d\n", prefix, h->min.val_s32 );
    fprintf( f, "  %s-axis-maximal-value: %d\n", prefix, h->max.val_s32 );
    fprintf( f, "  %s-axis-first-value:   %d\n", prefix, ((s32*)h->index)[0] );
    fprintf( f, "  %s-axis-last-value:    %d\n", prefix, ((s32*)h->index)[ h->dim-1 ] );
    break;
  case FLOAT :
    fprintf( f, "  %s-axis-type: FLOAT\n", prefix );
    fprintf( f, "  %s-axis-minimal-value: %f\n", prefix, h->min.val_r32 );
    fprintf( f, "  %s-axis-maximal-value: %f\n", prefix, h->max.val_r32 );
    fprintf( f, "  %s-axis-first-value:   %f\n", prefix, ((r32*)h->index)[0] );
    fprintf( f, "  %s-axis-last-value:    %f\n", prefix, ((r32*)h->index)[ h->dim-1 ] );
    break;
  }
  
}





void fprintfHistogramHeaderInfo( FILE *f, typeHistogram *h, char *desc )
{
  char *proc = "fprintfHistogramHeaderInfo";
  if ( desc != (char*)NULL ) 
    fprintf( f, "Information on histogram '%s'\n", desc );

  fprintf( f, "- axis information\n" );
  if ( h->xaxis.dim > 1 ) {
    fprintfHistogramAxisInfo( f, &(h->xaxis), "x" );
  }
  if ( h->yaxis.dim > 1 ) {
    fprintfHistogramAxisInfo( f, &(h->yaxis), "y" );
  }
  
  fprintf( f, "- histogram information\n" );

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such histogram type not handled yet\n", proc );
    break;
  case TYPE_UNKNOWN :
    fprintf( f, "  histogram-type: unknown (yet)\n" );
    break;
  case SINT :
    fprintf( f, "  histogram-type: SINT\n" );
    break;
  case UINT :
    fprintf( f, "  histogram-type: UINT\n" );
    break;
  case FLOAT :
    fprintf( f, "  histogram-type: FLOAT\n" );
    break;
  case DOUBLE :
    fprintf( f, "  histogram-type: DOUBLE\n" );
    break;
  }

}










/**********************************************************************
 *
 * 1D histogram tools
 *
 **********************************************************************/



int minMeanMaxStddev1DHistogramWithRange( typeHistogram *h,
                                          double fbound,
                                          double lbound,
                                          double *min,
                                          double *moy,
                                          double *max,
                                          double *ect )
{
  char *proc = "minMeanMaxStddev1DHistogramWithRange";
  int i;
  double m;


#define _MMMS1DHistogram( TYPEINDEX, TYPEHISTO, TYPEMSUM, TYPEN, TYPEESUM ) { \
  TYPEINDEX *theIndex = (TYPEINDEX*)h->xaxis.index;                     \
  TYPEHISTO *theHisto = (TYPEHISTO*)h->data;                            \
  TYPEMSUM msum;                                                        \
  TYPEN n;                                                              \
  TYPEESUM esum;                                                        \
  for ( msum=0, n=0, i=0; i<h->xaxis.dim; i++ ) {                       \
    if ( theIndex[i] < theFbound || theLbound < theIndex[i] ) continue; \
    if ( theHisto[i] > 0 ) {                                            \
      msum += (TYPEMSUM)theHisto[i] * (TYPEMSUM)theIndex[i];                      \
      n    += (TYPEMSUM)theHisto[i];                                    \
    }                                                                   \
  }                                                                     \
  *moy = m = (double)msum / (double)n;                                  \
  for ( esum=0, i=0; i<h->xaxis.dim; i++ ) {                            \
    if ( theIndex[i] < theFbound || theLbound < theIndex[i] ) continue; \
    if ( theHisto[i] > 0 ) {                                            \
      esum += theHisto[i] * (double)(theIndex[i] - m )*(double)(theIndex[i] - m ); \
    }                                                                   \
  }                                                                     \
  *ect = sqrt( (double)esum / (double)n );                              \
}


  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such index type not handled yet\n", proc );
    return( -1 );

  case SINT :
    {
      s32 theFbound = ( fbound < 0 ) ? (s32)(fbound - 0.5 ) : (s32)(fbound + 0.5 );
      s32 theLbound = ( lbound < 0 ) ? (s32)(lbound - 0.5 ) : (s32)(lbound + 0.5 );
      *min = ((s32*)h->xaxis.index)[ 0 ];
      *max = ((s32*)h->xaxis.index)[ h->xaxis.dim-1 ];
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: histogram type not handled yet\n", proc );
        return( -1 );
      case SINT :
        _MMMS1DHistogram( s32, s32, s64, s64, r64 );
        break;
      case UINT :
        _MMMS1DHistogram( s32, u32, s64, s64, r64 );
        break;
      case FLOAT :
        _MMMS1DHistogram( s32, r32, r64, r64, r64 );
        break;
      }
    }
    break;
    /* end of case SINT of switch( h->xaxis.typeIndex )
     */
  }

  return( 1 );
}










/**********************************************************************
 *
 * histogram transformation tools
 *
 **********************************************************************/



int pdfHistogram( typeHistogram *pdf, typeHistogram *h )
{
  char *proc = "pdfHistogram";
  int i, n;
  float sum = sumhistogram( h );

  if( h->xaxis.dim <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: no index values?\n", proc );
    return( -1 );
  }
  n = h->xaxis.dim;
  if ( h->yaxis.dim > 1 ) 
    n *= h->yaxis.dim;

  
  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such histogram type not handled yet\n", proc );
    return( -1 );

  case SINT :
    {
      s32 *theHisto = (s32*)h->data;
      switch( pdf->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such pdf histogram type not handled yet\n", proc );
        return( -1 );
      case FLOAT :
        {
          r32 *thePdf = (r32*)pdf->data;
          for ( i=0; i<n; i++ )
            thePdf[i] =  (r32)theHisto[i] / sum;
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == SINT */

  case UINT :
    {
      u32 *theHisto = (u32*)h->data;
      switch( pdf->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such pdf histogram type not handled yet\n", proc );
        return( -1 );
      case FLOAT :
        {
          r32 *thePdf = (r32*)pdf->data;
          for ( i=0; i<n; i++ )
            thePdf[i] =  (r32)theHisto[i] / sum;
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == UINT */

  case FLOAT :
    {
      r32 *theHisto = (r32*)h->data;
      switch( pdf->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such pdf histogram type not handled yet\n", proc );
        return( -1 );
      case FLOAT :
        {
          r32 *thePdf = (r32*)pdf->data;
          for ( i=0; i<n; i++ )
            thePdf[i] =  (r32)theHisto[i] / sum;
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == FLOAT */

  }

  return( 1 );
}










/**********************************************************************
 *
 * 1D histogram transformation tools
 *
 **********************************************************************/



int cumulative1DHistogram( typeHistogram *cumul, typeHistogram *h )
{
  char *proc = "cumulative1DHistogram";
  int i;
  
  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such histogram type not handled yet\n", proc );
    return( -1 );

  case SINT :
    {
      s32 *theHisto = (s32*)h->data;
      switch( cumul->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such cumulative histogram type not handled yet (SINT)\n", proc );
        fprintf( stderr, "%s: type is %d, SINT = %d\n", proc, cumul->typeHisto, SINT );
        return( -1 );
      case SINT :
        {
          s32 *theCumul = (s32*)cumul->data;
          theCumul[0] = theHisto[0];
          for ( i=1; i<h->xaxis.dim; i++ )
            theCumul[i] = theCumul[i-1] + theHisto[i];
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == SINT */

  case UINT :
    {
      u32 *theHisto = (u32*)h->data;
      switch( cumul->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such cumulative histogram type not handled yet (UINT)\n", proc );
        return( -1 );
      case UINT :
        {
          u32 *theCumul = (u32*)cumul->data;
          theCumul[0] = theHisto[0];
          for ( i=1; i<h->xaxis.dim; i++ )
            theCumul[i] = theCumul[i-1] + theHisto[i];
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == UINT */

  case FLOAT :
    {
      r32 *theHisto = (r32*)h->data;
      switch( cumul->typeHisto ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such cumulative histogram type not handled yet (FLOAT)\n", proc );
        return( -1 );
      case FLOAT :
        {
          r32 *theCumul = (r32*)cumul->data;
          theCumul[0] = theHisto[0];
          for ( i=1; i<h->xaxis.dim; i++ )
            theCumul[i] = theCumul[i-1] + theHisto[i];
        }
        break;
      }
    }
    break;
    /* end of h->typeHisto == FLOAT */

  }

  return( 1 );
}










/**********************************************************************
 *
 * 1D histogram I/O procedures
 *
 **********************************************************************/


static char *_BaseName( char *p )
{
  int l;
  if ( p == (char*)NULL ) return( (char*)NULL );
  l = strlen( p ) - 1;
  while ( l >= 0 && p[l] != '/' ) l--;
  if ( l < 0 ) l = 0;
  if ( p[l] == '/' ) l++;
  return( &(p[l]) );
}





static int _fprintf1DRawDataInFile( int fd, typeHistogram *h )
{
#ifndef WIN32
  char *proc = "_fprintf1DRawDataInFile";

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( write( fd, h->xaxis.index, h->xaxis.dim * sizeof(s32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case FLOAT :
    if ( write( fd, h->xaxis.index, h->xaxis.dim * sizeof(r32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  }

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( write( fd, h->data, h->xaxis.dim * sizeof(s32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case UINT :
    if ( write( fd, h->data, h->xaxis.dim * sizeof(u32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case FLOAT :
    if ( write( fd, h->data, h->xaxis.dim * sizeof(r32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case DOUBLE :
    if ( write( fd, h->data, h->xaxis.dim * sizeof(r64) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  }
  return( 1 );
#endif
  return 0;
}





static int _fprintf2DRawDataInFile( int fd, typeHistogram *h )
{
#ifndef WIN32
  char *proc = "_fprintf2DRawDataInFile";

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( write( fd, h->xaxis.index, h->xaxis.dim * sizeof(s32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing x axis\n", proc );
    }
    break;
  case FLOAT :
    if ( write( fd, h->xaxis.index, h->xaxis.dim * sizeof(r32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing x axis\n", proc );
    }
    break;
  }

  switch( h->yaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( write( fd, h->yaxis.index, h->yaxis.dim * sizeof(s32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing y axis\n", proc );
    }
    break;
  case FLOAT :
    if ( write( fd, h->yaxis.index, h->yaxis.dim * sizeof(r32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing y axis\n", proc );
    }
    break;
  }

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( write( fd, h->data, h->xaxis.dim * h->yaxis.dim * sizeof(s32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case UINT :
    if ( write( fd, h->data, h->xaxis.dim * h->yaxis.dim * sizeof(u32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case FLOAT :
    if ( write( fd, h->data, h->xaxis.dim * h->yaxis.dim * sizeof(r32) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  case DOUBLE :
    if ( write( fd, h->data, h->xaxis.dim * h->yaxis.dim * sizeof(r64) ) == -1 ) {
      fprintf( stderr, "%s: error when writing\n", proc );
    }
    break;
  }
  return( 1 );
#endif
  return 0;
}





#ifdef _UNUSED_
static void _write1DRawData( char *filename, typeHistogram *h )
{
#ifndef WIN32
  char *proc = "_write1DRawData";
  int fd;

  fd = open( filename, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH );
  if ( fd == -1 ) {
    free( filename );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  if ( _fprintf1DRawDataInFile( fd, h ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write data\n", proc );
    return;
  }

  close( fd );
#endif
}
#endif





int fprintf1DHistogramMatlab( FILE *f, int fd, 
                            typeHistogram *h, char *s )
{
  char *proc="fprintf1DHistogramMatlab";


  /* .raw file 
   */
  if ( _fprintf1DRawDataInFile( fd, h ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write data\n", proc );
    return( -1 );
  }

  /* .mat file
   */
  
  fprintf( f, "\n" );
  fprintf( f, "%%\n" );
  fprintf( f, "%% read data\n" );
  fprintf( f, "%%\n" );
  fprintf( f, "\n" );

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "INDEX=fread( myfile, [%d], 'int%lu');\n", h->xaxis.dim, 8*sizeof(s32) );
    else 
      fprintf( f, "INDEX_%s=fread( myfile, [%d], 'int%lu');\n", s, h->xaxis.dim, 8*sizeof(s32) );
    break;
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "INDEX=fread( myfile, [%d], 'float%lu');\n", h->xaxis.dim, 8*sizeof(r32) );
    else 
      fprintf( f, "INDEX_%s=fread( myfile, [%d], 'float%lu');\n", s, h->xaxis.dim, 8*sizeof(r32) );
    break;
  }

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=fread( myfile, [%d], 'int%lu');\n", h->xaxis.dim, 8*sizeof(s32) );
    else 
      fprintf( f, "HISTO_%s=fread( myfile, [%d], 'int%lu');\n", s, h->xaxis.dim, 8*sizeof(s32) );
    break;
  case UINT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=fread( myfile, [%d], 'uint%lu');\n", h->xaxis.dim, 8*sizeof(s32) );
    else 
      fprintf( f, "HISTO_%s=fread( myfile, [%d], 'uint%lu');\n", s, h->xaxis.dim, 8*sizeof(s32) );
    break;  
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=fread( myfile, [%d], 'float%lu');\n", h->xaxis.dim, 8*sizeof(r32) );
    else 
      fprintf( f, "HISTO_%s=fread( myfile, [%d], 'float%lu');\n", s, h->xaxis.dim, 8*sizeof(r32) );
    break;
  }

  fprintf( f, "\n" );
  fprintf( f, "%%\n" );
  fprintf( f, "%% figure\n" );
  fprintf( f, "%%\n" );
  fprintf( f, "\n" );

      
  fprintf( f, "\n" );
  fprintf( f, "figure;\n" );
  fprintf( f, "hold on;\n" );
  fprintf( f, "%% [1:%d];\n", h->xaxis.dim );
  if ( s == (char*)NULL )
    fprintf( f, "plot( INDEX, HISTO, 'k-' );\n" );
  else 
    fprintf( f, "plot( INDEX_%s, HISTO_%s, 'k-' );\n", s, s );
  fprintf( f, "\n" );

  return( 1 );
}





int fprintf1DHistogramScilab( FILE *f, int fd, 
                            typeHistogram *h, char *s )
{
  char *proc="fprintf1DHistogramScilab";


  /* .raw file 
   */
  if ( _fprintf1DRawDataInFile( fd, h ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write data\n", proc );
    return( -1 );
  }

  /* .sce file
   */
  
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "\n" );
  fprintf( f, "////////////////////////////////////////////////////////\n" );
  fprintf( f, "// read data\n" );
  fprintf( f, "//\n" );
  fprintf( f, "\n" );

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "INDEX=mget( %d, 'i', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "INDEX_%s=mget( %d, 'i', myfile );\n", s, h->xaxis.dim );
    break;
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "INDEX=mget( %d, 'f', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "INDEX_%s=mget( %d, 'f', myfile );\n", s, h->xaxis.dim );
    break;
  }

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1 );
  case UINT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=mget( %d, 'ui', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "HISTO_%s=mget( %d, 'ui', myfile );\n", s, h->xaxis.dim );
    break;
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=mget( %d, 'i', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "HISTO_%s=mget( %d, 'i', myfile );\n", s, h->xaxis.dim );
    break;
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=mget( %d, 'f', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "HISTO_%s=mget( %d, 'f', myfile );\n", s, h->xaxis.dim );
    break;
  case DOUBLE :
    if ( s == (char*)NULL )
      fprintf( f, "HISTO=mget( %d, 'd', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "HISTO_%s=mget( %d, 'd', myfile );\n", s, h->xaxis.dim );
    break;
  }

  fprintf( f, "\n" );
  fprintf( f, "//\n" );
  fprintf( f, "// figure\n" );
  fprintf( f, "//\n" );
  fprintf( f, "\n" );

  fprintf( f, "figure;\n" );
  fprintf( f, "myfig=gcf();\n" );
  fprintf( f, "myfig.background = color(\"white\");\n" );
  fprintf( f, "// [1:%d];\n", h->xaxis.dim );
  

  fprintf( f, "// a=get(\"current_axes\");\n" );
  fprintf( f, "a=gca();\n" );
  fprintf( f, "set(a,\"auto_clear\",\"off\");\n" );
  fprintf( f, "// removing the trailing ';' allows to see all properties\n" );

  fprintf( f, "\n" );
  
  if ( s == (char*)NULL ) {
    fprintf( f, "plot( INDEX, HISTO, \"k-\", \"thickness\", 2 );\n" );
    fprintf( f, "// plot2d2( INDEX, HISTO );\n" );
  }
  else {
    fprintf( f, "plot( INDEX_%s, HISTO_%s, \"k-\", \"thickness\", 2 );\n", s, s );
    fprintf( f, "// plot2d( INDEX_%s, HISTO_%s, leg=\"%s\" );\n", s, s, s );
    fprintf( f, "// plot2d2( INDEX_%s, HISTO_%s );\n", s, s );
  }

  fprintf( f, "\n" );
  fprintf( f, "// a.data_bounds = [0,0;%d,%f];\n",
           (int)(h->xaxis.dim*h->xaxis.binlength+0.5), maxhistogram(h)  );
  fprintf( f, "a.font_size = 3;\n" );
  fprintf( f, "a.font_style = 8;\n" );
  fprintf( f, "\n" );
  fprintf( f, "// a.title.text = \"Title\";\n" );
  fprintf( f, "// a.x_label.text = \"X Label\";\n" );
  fprintf( f, "// a.y_label.text = \"Y Label\";\n" );
  fprintf( f, "// or \n" );
  fprintf( f, "// xtitle( \"Title\", \"X Label\", \"Y Label\" );\n" );
  fprintf( f, "// a.title.font_size = 3;\n" );
  fprintf( f, "// a.x_label.font_size = 3;\n" );
  fprintf( f, "// a.y_label.font_size = 3;\n" );
  fprintf( f, "\n" );
  fprintf( f, "// or \n" );
  fprintf( f, "// xlabel( 'X Label', 'fontsize', 4, 'fontname', 8 );\n" );
  fprintf( f, "// ylabel( 'Y Label', 'fontsize', 4, 'fontname', 8 );\n" );
  if ( s != (char*)NULL ) {
    fprintf( f, "// a.title.text = \"%s\";\n", s );
    fprintf( f, "// ylabel( \"%s\", 'fontsize', 4, 'fontname', 8 );\n", s );
  }
  fprintf( f, "\n" );
  
  fprintf( f, "e = gce();\n" );
  fprintf( f, "// e.children(1).thickness = 3;\n" );
  fprintf( f, "e.children(1).foreground = 2;\n" );
  fprintf( f, "\n" );
  if ( s == (char*)NULL ) {
    fprintf( f, "// xs2png(gcf(),'FIG.png');\n"  );
  }
  else {
    fprintf( f, "// xs2png(gcf(),'FIG%s.png');\n", s  );
  }
  fprintf( f, "\n" );

  return( 1 );
}





int fprintf1DHistogramListScilab( FILE *f, int fd, 
                                  typeHistogramList *hl, char *s,
                                  double fbound,
                                  double lbound )
{
  char *proc="fprintf1DHistogramListScilab";
  int i;
  typeHistogram hmean;
  unionValues imin, imax;
  double *mean, min, max, ect;


  initHistogram( &hmean );
  imin.val_s32 = 1;
  imax.val_s32 = hl->n;

  if ( allocHistogramHeader( &(hmean.xaxis), &imin, &imax, 1.0, SINT ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram x index\n", proc );
    return( -1 );
  }
  
  if ( allocHistogramData( &hmean, DOUBLE ) != 1 ) {
    freeHistogram( &hmean );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }
  mean = (double*)hmean.data;
  
  for ( i=0; i<hl->n; i++ ) {
    if ( minMeanMaxStddev1DHistogramWithRange( &(hl->data[i]), fbound, lbound,
                                               &min, &(mean[i]), &max, &ect ) != 1 ) {
      free( mean );
      if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to compute statistics for histogram #%d\n", proc, i );
      return( -1 );
    }
  }

  if ( fprintf1DHistogramScilab( f, fd, &hmean, s ) != 1 ) {
     if ( _verbose_ ) 
        fprintf( stderr, "%s: unable to write scilab files\n", proc );
      return( -1 );
  }
  
  freeHistogram( &hmean );

  return( 1 );
}





int fprintf2DHistogramScilab( FILE *f, int fd, 
                              typeHistogram *h, char *s )
{
  char *proc="fprintf2DHistogramScilab";


  /* .raw file 
   */
  if ( _fprintf2DRawDataInFile( fd, h ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write data\n", proc );
    return( -1 );
  }

  /* .sce file
   */
  
  fprintf( f, "\n" );
  fprintf( f, "//\n" );
  fprintf( f, "// read data\n" );
  fprintf( f, "//\n" );
  fprintf( f, "\n" );

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "XINDEX=mget( %d, 'i', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "XINDEX_%s=mget( %d, 'i', myfile );\n", s, h->xaxis.dim );
    break;
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "XINDEX=mget( %d, 'f', myfile );\n", h->xaxis.dim );
    else 
      fprintf( f, "XINDEX_%s=mget( %d, 'f', myfile );\n", s, h->xaxis.dim );
    break;
  }

  switch( h->yaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: index type not handled yet\n", proc );
    return( -1 );
  case SINT :
    if ( s == (char*)NULL )
      fprintf( f, "YINDEX=mget( %d, 'i', myfile );\n", h->yaxis.dim );
    else 
      fprintf( f, "YINDEX_%s=mget( %d, 'i', myfile ;\n", s, h->yaxis.dim );
    break;
  case FLOAT :
    if ( s == (char*)NULL )
      fprintf( f, "YINDEX=mget( %d, 'f', myfile );\n", h->yaxis.dim );
    else 
      fprintf( f, "YINDEX_%s=mget( %d, 'f', myfile );\n", s, h->yaxis.dim );
    break;
  }

  switch( h->typeHisto ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram type not handled yet\n", proc );
    return( -1 );
  case UINT :
    fprintf( f, "HISTO_READ=mget( %d, 'ui', myfile );\n", h->xaxis.dim * h->yaxis.dim );
    break;
  case SINT :
    fprintf( f, "HISTO_READ=mget( %d, 'i', myfile );\n", h->xaxis.dim * h->yaxis.dim );
    break;
  case FLOAT :
    fprintf( f, "HISTO_READ=mget( %d, 'f', myfile );\n", h->xaxis.dim * h->yaxis.dim );
    break;
  case DOUBLE :
    fprintf( f, "HISTO_READ=mget( %d, 'd', myfile );\n", h->xaxis.dim * h->yaxis.dim );
    break;
  }
  if ( s == (char*)NULL )
    fprintf( f, "HISTO=matrix(HISTO_READ,%d,%d);\n", h->xaxis.dim, h->yaxis.dim );
  else
    fprintf( f, "HISTO_%s=matrix(HISTO_READ,%d,%d);\n", s, h->xaxis.dim, h->yaxis.dim );
  

  fprintf( f, "\n" );
  fprintf( f, "//\n" );
  fprintf( f, "// figure\n" );
  fprintf( f, "//\n" );
  fprintf( f, "\n" );

  fprintf( f, "figure;\n" );
  fprintf( f, "myfig=gcf();\n" );
  fprintf( f, "myfig.background = color(\"white\");\n" );
  fprintf( f, "\n" );  

  if ( s == (char*)NULL ) {
    fprintf( f, "plot3d( XINDEX, YINDEX, HISTO );\n" );
  }
  else {
    fprintf( f, "plot3d( XINDEX_%s, YINDEX_%s, HISTO_%s );\n", s, s, s );
  }
  fprintf( f, "e = gce();\n" );
  fprintf( f, "e.thickness= 0;\n" );
  fprintf( f, "\n" );

  fprintf( f, "// a=get(\"current_axes\");\n" );
  fprintf( f, "a=gca();\n" );
  fprintf( f, "set(a,\"auto_clear\",\"off\");\n" );
  fprintf( f, "a.cube_scaling = \"on\";\n" );
  fprintf( f, "// removing the trailing ';' allows to see all properties\n" );
  fprintf( f, "// a.data_bounds = [0,0,0;%d,%d,%f];\n", h->xaxis.dim, h->yaxis.dim, maxhistogram(h)  );
  if ( 0 ) {
    /* le calcul du quantile peut etre tres long
     */
    if ( h->xaxis.typeIndex == h->yaxis.typeIndex ) {
      switch( h->xaxis.typeIndex ) {
      default :
        break;
      case SINT :
        fprintf( f, "a.data_bounds = [%d,%d,0;%d,%d,%f];\n", ((s32*)h->xaxis.index)[0],((s32*)h->yaxis.index)[ 0 ],
                 ((s32*)h->xaxis.index)[ h->xaxis.dim-1 ], ((s32*)h->yaxis.index)[ h->yaxis.dim-1 ], quantilehistogram(h, 0.85)  );
        break;
      }
    }
  }
  fprintf( f, "a.font_size = 4;\n" );
  fprintf( f, "a.font_style = 8;\n" );
  fprintf( f, "\n" );
  fprintf( f, "// a.title.text = \"Title\";\n" );
  fprintf( f, "// a.x_label.text = \"X Label\";\n" );
  fprintf( f, "// a.y_label.text = \"Y Label\";\n" );
  fprintf( f, "// or \n" );
  fprintf( f, "// xtitle( \"Title\", \"X Label\", \"Y Label\" );\n" );
  fprintf( f, "// a.title.font_size = 3;\n" );
  fprintf( f, "// a.x_label.font_size = 3;\n" );
  fprintf( f, "// a.y_label.font_size = 3;\n" );
  fprintf( f, "a.rotation_angles=[45 210];\n" );
  fprintf( f, "\n" );
  fprintf( f, "// or \n" );
  fprintf( f, "// xlabel( 'X Label', 'fontsize', 4, 'fontname', 8 );\n" );
  fprintf( f, "// ylabel( 'Y Label', 'fontsize', 4, 'fontname', 8 );\n" );
  fprintf( f, "\n" );

  return( 1 );
}








void writeHistogramXxxlab( char *name, typeHistogram *h, char *s, 
                           enumHistogramFile xxxlab )
{
#ifndef WIN32
  char *proc = "writeHistogramXxxlab";
  char *defaultname = "histo";
  char *template;
  char *filename = (char*)NULL;
  FILE *f;
  int fd;

  template = ( name != (char*)NULL ) ? name : defaultname;
  filename = (char*)malloc( strlen( template ) + 5 );
  if ( filename == (char*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate file name\n", proc );
    return;
  }

  /* open files
   */
  sprintf( filename, "%s.raw", template );
  fd = open( filename, O_CREAT | O_TRUNC | O_WRONLY, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH );
  if ( fd == -1 ) {
    free( filename );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  switch( xxxlab ) {
  default :
    free( filename );
    close( fd );
    if ( _verbose_ )
      fprintf( stderr, "%s: such output type not known\n", proc );
    return;
  case _MATLAB_ :
    sprintf( filename, "%s.m", template );
    break;
  case _SCILAB_ :
    sprintf( filename, "%s.sce", template );
    break;
  }

  f = fopen( filename, "w" );

  if ( f == (FILE*)NULL ) {
    free( filename );
    close( fd );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  
  /* write data and script
   */
  switch( xxxlab ) {
  default :
    free( filename );
    close( fd );
    fclose( f );
    if ( _verbose_ )
      fprintf( stderr, "%s: such output type not known\n", proc );
    return;

  case _MATLAB_ :

    fprintf( f, "\n" );
    fprintf( f, "myfile=fopen('%s.raw','r');\n", _BaseName( template ) );
    fprintf( f, "\n" );

    if ( h->yaxis.dim <= 1 ) {
      if ( fprintf1DHistogramMatlab( f, fd, h, s ) != 1 ) {
        free( filename );
        close( fd );
        fclose( f );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write data and matlab script in '%s.raw' and '%s.m'\n",
                   proc, _BaseName( template ), _BaseName( template ) );
        return;
      }
    }
    else {
      free( filename );
      close( fd );
      fclose( f );
      if ( _verbose_ )
        fprintf( stderr, "%s: 2D histogram case not handled yet\n", proc );
      return;
    }
    
    
    fprintf( f, "\n" );
    fprintf( f, "fclose(myfile);\n" );
    fprintf( f, "\n" );

    break;
    
  case _SCILAB_ :

    fprintf( f, "\n" );
    fprintf( f, "myfile=mopen('%s.raw','r');\n", _BaseName( template ) );
    fprintf( f, "\n" );
    
    if ( h->yaxis.dim <= 1 ) {
      if ( fprintf1DHistogramScilab( f, fd, h, s ) != 1 ) {
        free( filename );
        close( fd );
        fclose( f );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write data and scilab script in '%s.raw' and '%s.sce'\n",
                   proc, _BaseName( template ), _BaseName( template ) );
        return;
      }
    }
    else {
      if ( fprintf2DHistogramScilab( f, fd, h, s ) != 1 ) {
        free( filename );
        close( fd );
        fclose( f );
        if ( _verbose_ )
          fprintf( stderr, "%s: unable to write data and scilab script in '%s.raw' and '%s.sce'\n",
                   proc, _BaseName( template ), _BaseName( template ) );
        return;
      }
    }

    fprintf( f, "\n" );
    fprintf( f, "mclose(myfile);\n" );
    fprintf( f, "\n" );

    break;
  }


  /* close files
   */
  close( fd );
  fclose( f );
  free( filename );
#endif
}





void writeHistogramMatlab( char *name, typeHistogram *h, char *s )
{
  writeHistogramXxxlab( name, h, s, _MATLAB_ );
}





void writeHistogramScilab( char *name, typeHistogram *h, char *s )
{
  writeHistogramXxxlab( name, h, s, _SCILAB_ );
}





void write1DHistogramTxt( char *name, typeHistogram *h )
{
  char *proc = "write1DHistogramTxt";
  char *defaultname = "histo.txt";
  char *filename;
  FILE *f;

  filename = ( name != (char*)NULL ) ? name : defaultname;
  f = fopen( filename, "w" );

  if ( f == (FILE*)NULL ) {
    fprintf( stderr, "%s: unable to open '%s' for writing\n", proc, filename );
    return;
  }

  fprintf1DHistogram( f, h );

  fclose( f );

}





void fprintf1DHistogram( FILE *theFile, typeHistogram *h )
{
  char *proc = "fprintf1DHistogram";
  FILE *f;
  int i;

  f = ( theFile == (FILE*)NULL ) ? stderr : theFile;
  
  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ ) {
      fprintf( stderr, "%s: index type not handled yet: ", proc );
      printType( stderr, h->xaxis.typeIndex );
      fprintf( stderr, "\n" );
    }
    return;
  case SINT :
    {
      s32 *theIndex = (s32*)h->xaxis.index;
      switch( h->typeHisto ) {
      default :
        if ( _verbose_ ) {
          fprintf( stderr, "%s: histogram type not handled yet:", proc );
          printType( stderr, h->typeHisto );
         }
        return;
      case UINT :
        {
          u32 *theHisto = (u32*)h->data;
          for ( i=0; i<h->xaxis.dim; i ++ )
            if ( theHisto[i] > 0.0 )
              fprintf( f, "%5d %u\n", theIndex[i], theHisto[i] );
        }
        break;
      case FLOAT :
        {
          r32 *theHisto = (r32*)h->data;
          for ( i=0; i<h->xaxis.dim; i ++ )
            if ( theHisto[i] > 0.0 )
              fprintf( f, "%5d %f\n", theIndex[i], theHisto[i] );
        }
        break;
      }
    }
    break;
  case FLOAT :
    {
      r32 *theIndex = (r32*)h->xaxis.index;
      switch( h->typeHisto ) {
      default :
          if ( _verbose_ ) {
            fprintf( stderr, "%s: histogram type not handled yet:", proc );
            printType( stderr, h->typeHisto);
           }
           return;
      case UINT :
        {
          u32 *theHisto = (u32*)h->data;
          for ( i=0; i<h->xaxis.dim; i ++ )
            if ( theHisto[i] > 0.0 )
              fprintf( f, "%f %u\n", theIndex[i], theHisto[i] );
        }
        break;
      case FLOAT :
        {
          r32 *theHisto = (r32*)h->data;
          for ( i=0; i<h->xaxis.dim; i ++ )
            if ( theHisto[i] > 0.0 )
              fprintf( f, "%f %f\n", theIndex[i], theHisto[i] );
        }
        break;
      }
    }
    break;
  }

}





/**********************************************************************
 *
 * buffer/image statistics tools
 *
 **********************************************************************/


static int _minMeanMaxStddevValues( void *theIm, bufferType theType, 
                                    int *theDim,
                                    unionValues *theMin, double *theMean,
                                    unionValues *theMax, double *theStandardDeviation )
{
  char *proc = "_minMeanMaxStddevValues";
  int z, s;
  size_t size;
  double ec, me, v;
  double moy, ect;

  z = theDim[2];
  size = theDim[0] * theDim[1];

  if ( (z <= 0) || (size <= 0) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird image dimensions", proc );
    return( -1 );
  }
  
  moy = ect = (double)0.0;
  
  /* on calcule une moyenne par plan, que l'on somme */
  /* dans moy, pour eviter de manipuler de trop   */
  /* grands nombres                                  */

  
  switch ( theType ) {
  default : 
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled yet\n", proc );
    return( -1 );
  case SCHAR :
    {
      s8 *b = (s8*)theIm;
      theMin->val_s8 = theMax->val_s8 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_s8 > *b ) theMin->val_s8 = *b;
          else if ( theMax->val_s8 < *b ) theMax->val_s8 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case UCHAR :
    {
      u8 *b = (u8*)theIm;
      theMin->val_u8 = theMax->val_u8 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_u8 > *b ) theMin->val_u8 = *b;
          else if ( theMax->val_u8 < *b ) theMax->val_u8 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case SSHORT :
    {
      s16 *b = (s16*)theIm;
      theMin->val_s16 = theMax->val_s16 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_s16 > *b ) theMin->val_s16 = *b;
          else if ( theMax->val_s16 < *b ) theMax->val_s16 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case USHORT :
    {
      u16 *b = (u16*)theIm;
      theMin->val_u16 = theMax->val_u16 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_u16 > *b ) theMin->val_u16 = *b;
          else if ( theMax->val_u16 < *b ) theMax->val_u16 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case SINT :
    {
      s32 *b = (s32*)theIm;
      theMin->val_s32 = theMax->val_s32 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_s32 > *b ) theMin->val_s32 = *b;
          else if ( theMax->val_s32 < *b ) theMax->val_s32 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case UINT :
    {
      u32 *b = (u32*)theIm;
      theMin->val_u32 = theMax->val_u32 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_u32 > *b ) theMin->val_u32 = *b;
          else if ( theMax->val_u32 < *b ) theMax->val_u32 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case FLOAT :
    {
      r32 *b = (r32*)theIm;
      theMin->val_r32 = theMax->val_r32 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_r32 > *b ) theMin->val_r32 = *b;
          else if ( theMax->val_r32 < *b ) theMax->val_r32 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  case DOUBLE :
    {
      r64 *b = (r64*)theIm;
      theMin->val_r64 = theMax->val_r64 = *b;
      for ( ; z > 0; z-- ) {
        for ( me = ec = 0.0, s = size; s > 0; s--, b++ ) {
          if ( theMin->val_r64 > *b ) theMin->val_r64 = *b;
          else if ( theMax->val_r64 < *b ) theMax->val_r64 = *b;
          v = (double)(*b);
          me += v;   ec += v*v;
        }
        moy += me / (double)(size);
        ect += ec / (double)(size);
      }}
    break;
  }
  
  if ( theDim[2] > 1 ) {
    moy /= (double)(theDim[2]);
    ect /= (double)(theDim[2]);
  }

  ect -= moy * moy;
  ect = sqrt( ect );
  
  *theMean = moy;
  *theStandardDeviation = ect;
  
  return( 1 );
}





static int _minMeanMaxStddevValuesWithMask( void *theIm, bufferType theType, 
                                    void *theMask, bufferType theMaskType,
                                    int *theDim,
                                    unionValues *theMin, double *theMean,
                                    unionValues *theMax, double *theStandardDeviation )
{
  char *proc = "_minMeanMaxStddevValuesWithMask";
  int z, s;
  int n, nz;
  size_t i;
  int empty = 1;
  size_t size;
  double ec, me, v;
  double moy, ect;

  if ( theMask == (void*)NULL ) {
    return( _minMeanMaxStddevValues( theIm, theType, 
                                     theDim,
                                     theMin, theMean, theMax, theStandardDeviation ) );
  }

  z = theDim[2];
  size = theDim[0] * theDim[1];

  if ( (z <= 0) || (size <= 0) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: weird image dimensions", proc );
    return( -1 );
  }
  
  moy = ect = (double)0.0;
  
  /* on calcule une moyenne par plan, que l'on somme */
  /* dans moy, pour eviter de manipuler de trop   */
  /* grands nombres                                  */



  switch( theMaskType ) {
  default : 
    if ( _verbose_ )
      fprintf( stderr, "%s: such mask image type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    {
      u8 *m = (u8*)theMask;
      switch( theType ) {
      default :
        if ( _verbose_ )
          fprintf( stderr, "%s: such image type not handled yet\n", proc );
        return( -1 );
      case SCHAR :
        {
          s8 *b = (s8*)theIm;
          for ( i=0, empty=1; i<z*size && empty==1; i++ ) {
            if ( m[i] > 0 ) {
              theMin->val_s8 = theMax->val_s8 = b[i];
              empty = 0;
            }
          }
          if ( empty == 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: empty mask image\n", proc );
            return( -1 );
          }
          for ( nz = 0; z > 0; z-- ) {
            for ( me = ec = 0.0, s = size, n = 0; s > 0; s--, m++, b++ ) {
              if ( *m > 0 ) {
                if ( theMin->val_s8 > *b ) theMin->val_s8 = *b;
                else if ( theMax->val_s8 < *b ) theMax->val_s8 = *b;
                v = (double)(*b);
                me += v;   ec += v*v;
                n++;
              }
            }
            if ( n > 0 ) {
              moy += me / (double)(n);
              ect += ec / (double)(n);
              nz ++;
            }
          }}
        break;
      case UCHAR :
        {
          u8 *b = (u8*)theIm;
          for ( i=0, empty=1; i<z*size && empty==1; i++ ) {
            if ( m[i] > 0 ) {
              theMin->val_u8 = theMax->val_u8 = b[i];
              empty = 0;
            }
          }
          if ( empty == 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: empty mask image\n", proc );
            return( -1 );
          }
          for ( nz = 0; z > 0; z-- ) {
            for ( me = ec = 0.0, s = size, n = 0; s > 0; s--, m++, b++ ) {
              if ( *m > 0 ) {
                if ( theMin->val_u8 > *b ) theMin->val_u8 = *b;
                else if ( theMax->val_u8 < *b ) theMax->val_u8 = *b;
                v = (double)(*b);
                me += v;   ec += v*v;
                n++;
              }
            }
            if ( n > 0 ) {
              moy += me / (double)(n);
              ect += ec / (double)(n);
              nz ++;
            }
          }}
        break;
      case SSHORT :
        {
          s16 *b = (s16*)theIm;
          for ( i=0, empty=1; i<z*size && empty==1; i++ ) {
            if ( m[i] > 0 ) {
              theMin->val_s16 = theMax->val_s16 = b[i];
              empty = 0;
            }
          }
          if ( empty == 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: empty mask image\n", proc );
            return( -1 );
          }
          for ( nz = 0; z > 0; z-- ) {
            for ( me = ec = 0.0, s = size, n = 0; s > 0; s--, m++, b++ ) {
              if ( *m > 0 ) {
                if ( theMin->val_s16 > *b ) theMin->val_s16 = *b;
                else if ( theMax->val_s16 < *b ) theMax->val_s16 = *b;
                v = (double)(*b);
                me += v;   ec += v*v;
                n++;
              }
            }
            if ( n > 0 ) {
              moy += me / (double)(n);
              ect += ec / (double)(n);
              nz ++;
            }
          }}
        break;
      case USHORT :
        {
          u16 *b = (u16*)theIm;
          for ( i=0, empty=1; i<z*size && empty==1; i++ ) {
            if ( m[i] > 0 ) {
              theMin->val_u16 = theMax->val_u16 = b[i];
              empty = 0;
            }
          }
          if ( empty == 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: empty mask image\n", proc );
            return( -1 );
          }
          for ( nz = 0; z > 0; z-- ) {
            for ( me = ec = 0.0, s = size, n = 0; s > 0; s--, m++, b++ ) {
              if ( *m > 0 ) {
                if ( theMin->val_u16 > *b ) theMin->val_u16 = *b;
                else if ( theMax->val_u16 < *b ) theMax->val_u16 = *b;
                v = (double)(*b);
                me += v;   ec += v*v;
                n++;
              }
            }
            if ( n > 0 ) {
              moy += me / (double)(n);
              ect += ec / (double)(n);
              nz ++;
            }
          }}
        break;
      case FLOAT :
        {
          r32 *b = (r32*)theIm;
          for ( i=0, empty=1; i<z*size && empty==1; i++ ) {
            if ( m[i] > 0 ) {
              theMin->val_r32 = theMax->val_r32 = b[i];
              empty = 0;
            }
          }
          if ( empty == 1 ) {
            if ( _verbose_ )
              fprintf( stderr, "%s: empty mask image\n", proc );
            return( -1 );
          }
          for ( nz = 0; z > 0; z-- ) {
            for ( me = ec = 0.0, s = size, n = 0; s > 0; s--, m++, b++ ) {
              if ( *m > 0 ) {
                if ( theMin->val_r32 > *b ) theMin->val_r32 = *b;
                else if ( theMax->val_r32 < *b ) theMax->val_r32 = *b;
                v = (double)(*b);
                me += v;   ec += v*v;
                n++;
              }
            }
            if ( n > 0 ) {
              moy += me / (double)(n);
              ect += ec / (double)(n);
              nz ++;
            }
          }}
        break;
      } /* end switch ( theType ) */
    }
    /* end case UCHAR (mask) */
    break;
  }
  
  if ( nz > 1 ) {
    moy /= (double)(nz);
    ect /= (double)(nz);
  }
  

  ect -= moy * moy;
  ect = sqrt( ect );
  
  *theMean = moy;
  *theStandardDeviation = ect;
  
  return( 1 );
}
  
  
  
  
  
int minMeanMaxStddevValues( void *theIm, bufferType theType, 
                            void *theMask, bufferType theMaskType,
                            int *theDim,
                            double *theMin, double *theMean,
                            double *theMax, double *theStandardDeviation )
{
  char *proc = "minMeanMaxStddevValues";
  unionValues min;
  unionValues max;
  
  if ( theMask == (void*)NULL ) {
    if ( _minMeanMaxStddevValues( theIm, theType, 
                                  theDim,
                                  &min, theMean, &max, theStandardDeviation ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when calculating min, mean and max\n", proc );
      return( -1 );
    }
  }
  else {
    if ( _minMeanMaxStddevValuesWithMask( theIm, theType, 
                                          theMask, theMaskType,
                                          theDim,
                                          &min, theMean, &max, theStandardDeviation ) != 1 ) {
      if ( _verbose_ )
        fprintf( stderr, "%s: error when calculating min, mean and max (with mask)\n", proc );
      return( -1 );
    }
  }

  *theMin = *theMax = 0.0;

  switch( theType ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such type not handled yet\n", proc );
    return( -1 );
  case UCHAR :
    *theMin = min.val_u8;
    *theMax = max.val_u8;
    break;
  case SCHAR :
    *theMin = min.val_s8;
    *theMax = max.val_s8;
    break;
  case USHORT :
    *theMin = min.val_u16;
    *theMax = max.val_u16;
    break;
  case SSHORT :
    *theMin = min.val_s16;
    *theMax = max.val_s16;
    break;
  case UINT :
    *theMin = min.val_u32;
    *theMax = max.val_u32;
    break;
  case SINT :
    *theMin = min.val_s32;
    *theMax = max.val_s32;
    break;
  case ULINT :
    *theMin = min.val_u64;
    *theMax = max.val_u64;
    break;
  case SLINT :
    *theMin = min.val_s64;
    *theMax = max.val_s64;
    break;
  case FLOAT :
    *theMin = min.val_r32;
    *theMax = max.val_r32;
    break;
  case DOUBLE :
    *theMin = min.val_r64;
    *theMax = max.val_r64;
    break;
  }
  return( 1 );
}





/**********************************************************************
 *
 *
 *
 **********************************************************************/
