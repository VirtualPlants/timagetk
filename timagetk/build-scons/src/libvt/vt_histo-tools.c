/*************************************************************************
 * vt_histo-tools.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2014, all rights reserved
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mer  9 jul 2014 23:06:12 CEST
 *
 * WARNING
 * This file contains procedures that are frontend fo the histogram structure
 * from libbasic/histogram.h, *not* from vt_histo.h
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */



#include <histogram.h>

#include <vt_common.h>
#include <vt_histo-tools.h>

static int _verbose_ = 1;



/**************************************************
 *
 * I/O procedures
 *
 **************************************************/



int VT_ReadImageAsHistogram( char *filename, typeHistogram *h )
{
  char *proc = "VT_ReadImageAsHistogram";
  vt_image *im;
  
  im = _VT_Inrimage( filename );
  if ( im == (vt_image*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to read image '%s'\n", proc, filename );
    return( -1 );
  }

  if ( VT_ImageToHistogram( im, h ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to convert image '%s' into an histogram\n", proc, filename );
    return( -1 );
  }

  VT_FreeImage( im );
  VT_Free( (void**)&im );

  return( 1 );
}





int VT_WriteHistogramAsImage( char *filename, typeHistogram *h )
{
  char *proc = "VT_WriteHistogramAsImage";
  vt_image im;  

  VT_Image( &im );

  if ( VT_HistogramToImage( h, &im ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to convert histogram into image\n", proc );
    return( -1 );
  }

  (void)VT_CopyName( im.name, filename);
  if ( VT_WriteInrimage( &im ) == -1 ) {
    VT_FreeImage( &im );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to write image '%s'\n", proc, filename );
    return( -1 );
  }

  return( 1 );
}





int VT_WriteHistogram( char *filename, typeHistogram *h, char *description )
{
  char *proc = "VT_WriteHistogram";
  char *name;
  int l;

  if ( filename == (char*)NULL ) {
    fprintf1DHistogram( stdout, h );
    return( 1 );
  }

  l = strlen( filename );
  if ( l <= 0 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: file name seems of null length\n", proc );
    return( -1 );
  }

  name = (char*)malloc( (l+1) * sizeof(char) );
  if ( name == (char*)NULL ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate filename\n", proc );
    return( -1 );
  }
  (void)strncpy( name, filename, l );
  name[l] = '\0';


  if ( l > 4 && strncmp( &filename[l-4], ".sce", 4 ) == 0 ) {
    name[l-1] = name[l-2] = name[l-3] = name[l-4] = '\0';
    writeHistogramXxxlab( name, h, description, _SCILAB_ );
  }
  else if ( l > 2 && strncmp( &filename[l-2], ".m", 2 ) == 0 ) {
    name[l-1] = name[l-2] = '\0';
    writeHistogramXxxlab( name, h, description, _MATLAB_ );
  }
  else if ( l > 4 && strncmp( &filename[l-4], ".txt", 4 ) == 0 ) {
    /* c'est idiot d'enlever l'extension ...
     * name[l-1] = name[l-2] = name[l-3] = name[l-4] = '\0';
     */
    if ( h->yaxis.dim <= 1 ) {
      write1DHistogramTxt( name, h );
    }
    else {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to write txt file\n", proc );
      return( -1 );
    }
  }
  else {
    if ( VT_WriteHistogramAsImage( filename, h ) != 1 ) {
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to write histogram as an image\n", proc );
      free( name );
      return( -1 );
    }
  }

  free( name );

  return( 1 );
}









/**************************************************
 *
 * conversion procedures
 *
 **************************************************/



int VT_ImageToHistogram( vt_image *im, typeHistogram *h  )
{
  char *proc = "VT_ImageToHistogram";
  unionValues xmin, xmax;
  unionValues ymin, ymax;

  if ( im->dim.z != 1 || im->dim.v != 1 ) {
    VT_FreeImage( im );
    VT_Free( (void**)&im );
    if ( _verbose_ )
      fprintf( stderr, "%s: image dimensions incompatible with histogram\n", proc );
    return( -1 );
  }

  if ( im->off.x >= 0 )
    xmin.val_s32 = (int)(im->off.x + 0.5);
  else
    xmin.val_s32 = (int)(im->off.x - 0.5);
  xmax.val_s32 = im->dim.x + xmin.val_s32 - 1;

  if ( im->dim.y > 1 ) {
    if ( im->off.y >= 0 )
      ymin.val_s32 = (int)(im->off.y + 0.5);
    else
      ymin.val_s32 = (int)(im->off.y - 0.5);
    ymax.val_s32 = im->dim.y + ymin.val_s32 - 1;
  }



  freeHistogram( h );

  /* index type is SINT (signed int) for integer valued images if binlength is 1.0 or negative
   */
  if ( allocHistogramHeader( &(h->xaxis), &xmin, &xmax, 1.0, SINT ) != 1 ) {
    VT_FreeImage( im );
    VT_Free( (void**)&im );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram xaxis\n", proc );
    return( -1 );
  }
  if ( im->dim.y > 1 ) {
    if ( allocHistogramHeader( &(h->yaxis), &ymin, &ymax, 1.0, SINT ) != 1 ) {
      VT_FreeImage( im );
      VT_Free( (void**)&im );
      if ( _verbose_ )
	fprintf( stderr, "%s: unable to allocate histogram yaxis\n", proc );
      return( -1 );
    }
  }


  if ( allocHistogramData( h, im->type ) != 1 ) {
    freeHistogram( h );
    VT_FreeImage( im );
    VT_Free( (void**)&im );
    if ( _verbose_ )
      fprintf( stderr, "%s: unable to allocate histogram data\n", proc );
    return( -1 );
  }

  (void)memcpy( h->data, im->buf, VT_SizeImage( im ) );


  return( 1 );
}





int VT_HistogramToImage( typeHistogram *h, vt_image *im )
{
  char *proc = "VT_HistogramToImage";
  int xmin, xmax, xdim;
  int ymin, ymax, ydim;

  /* histogram length -> image dimension
     index is in [min,max]
  */
  if ( h->xaxis.binlength < 0.999999 || 1.000001 < h->xaxis.binlength ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram bin length along x is different from 1\n", proc );
    return( -1 );
  }

  if ( h->yaxis.dim > 1 && (h->yaxis.binlength < 0.999999 || 1.000001 < h->yaxis.binlength) ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: histogram bin length along y is different from 1\n", proc );
    return( -1 );
  }

  switch( h->xaxis.typeIndex ) {
  default :
    if ( _verbose_ )
      fprintf( stderr, "%s: such index type not handled yet\n", proc );
    return( -1 );
  case SCHAR :
    xmin = h->xaxis.min.val_s8;
    xmax = h->xaxis.max.val_s8;
    break;
  case UCHAR :
    xmin = h->xaxis.min.val_u8;
    xmax = h->xaxis.max.val_u8;
    break;
  case SSHORT :
    xmin = h->xaxis.min.val_s16;
    xmax = h->xaxis.max.val_s16;
    break;
  case USHORT :
    xmin = h->xaxis.min.val_u16;
    xmax = h->xaxis.max.val_u16;
    break;
  case SINT :
    xmin = h->xaxis.min.val_s32;
    xmax = h->xaxis.max.val_s32;
    break;
  case UINT :
    xmin = h->xaxis.min.val_u32;
    xmax = h->xaxis.max.val_u32;
    break;
  }
  
  xdim =  h->xaxis.dim;
  
  if ( h->yaxis.dim > 1 ) {
    switch( h->yaxis.typeIndex ) {
    default :
      if ( _verbose_ )
	fprintf( stderr, "%s: such index type not handled yet\n", proc );
      return( -1 );
    case SCHAR :
      ymin = h->yaxis.min.val_s8;
      ymax = h->yaxis.max.val_s8;
      break;
    case UCHAR :
      ymin = h->yaxis.min.val_u8;
      ymax = h->yaxis.max.val_u8;
      break;
    case SSHORT :
      ymin = h->yaxis.min.val_s16;
      ymax = h->yaxis.max.val_s16;
      break;
    case USHORT :
      ymin = h->yaxis.min.val_u16;
      ymax = h->yaxis.max.val_u16;
      break;
    case SINT :
      ymin = h->yaxis.min.val_s32;
      ymax = h->yaxis.max.val_s32;
      break;
    case UINT :
      ymin = h->yaxis.min.val_u32;
      ymax = h->yaxis.max.val_u32;
      break;
    }    
    ydim =  h->yaxis.dim;
  }



  /* image allocation
   */
  im->dim.x = xdim;
  im->dim.y = 1;
  im->dim.z = 1;
  im->dim.v = 1;
  im->type = h->typeHisto;
  im->off.x = xmin;

  if ( h->yaxis.dim > 1 ) {
    im->dim.y = ydim;
    im->off.y = ymin;
  }


  if ( VT_AllocImage( im ) != 1 ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate image (xdim=%d)", proc, xdim );
      if ( h->yaxis.dim > 1 )
	fprintf( stderr, " (ydim=%d)", ydim );
      fprintf( stderr, "\n" );
    }
    return( -1 );
  }

  (void)memcpy( im->buf, h->data,  VT_SizeImage( im ) );

  return( 1 );
}
