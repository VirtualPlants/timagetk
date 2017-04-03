

#include <chamferdistance.h>
#include <connexe.h>

#include <vt_cellfilter.h>



static int _verbose_ = 1;
/*
static int _debug_ = 0;
*/



int removeExternalExtension( vt_image *image,
			     vt_image *opening,
			     int low_threshold,
			     int high_threshold,
			     int connectivity,
			     int cc_size )
{
  char *proc = "removeExternalExtension";
  vt_image distance;
  int theDim[3];
  unsigned short int *distBuf;
  int i, v;
  int ncc;



  if ( VT_Test2Image( image, opening, proc ) == -1 ) {
    return( -1 );
  }
  if ( image->type != opening->type ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: input images have different types\n", proc );
    return( -1 );
  }

  VT_InitFromImage( &distance, image, "distance.inr.gz", USHORT );
  if ( VT_AllocImage( &distance ) != 1 ) {
    if ( _verbose_ )
      fprintf( stderr, "%s: allocation error\n", proc );
    return( -1 );
  }

  theDim[0] = image->dim.x;
  theDim[1] = image->dim.y;
  theDim[2] = image->dim.z;



  /* calcul de la distance
   */

  if ( Compute3DNormalizedChamfer5x5x5( opening->buf, opening->type,
					distance.buf, distance.type,
					theDim ) < 0 ) {
    VT_FreeImage( &distance );
    if ( _verbose_ )
      fprintf( stderr, "%s: distance computation error\n", proc );
    return( -1 );
  }



  /* masquage par l'image
   */

  distBuf = (unsigned short int*)distance.buf;
  v =  image->dim.x * image->dim.y * image->dim.z;

  switch( image->type ) {
  default :
    VT_FreeImage( &distance );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled\n", proc );
    return( -1 );
    break;
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char*)image->buf;
      for ( i=0; i<v; i++ )
	if ( theBuf[i] == 0 ) distBuf[i] = 0;
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int*)image->buf;
      for ( i=0; i<v; i++ )
	if ( theBuf[i] == 0 ) distBuf[i] = 0;
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int*)image->buf;
      for ( i=0; i<v; i++ )
	if ( theBuf[i] == 0 ) distBuf[i] = 0;
    }
    break;
  }



  /* seuillage par hysteresis
   */
  ncc = HysteresisThresholdingWithAllParams( distance.buf, USHORT, distance.buf, USHORT, theDim,
					     (double)low_threshold, (double)high_threshold, 
					     connectivity, cc_size, 1,
					     -1, 0 );
  if ( ncc < 0 ) {
    VT_FreeImage( &distance );
    if ( _verbose_ )
      fprintf( stderr, "%s: hysteresis computation error\n", proc );
    return( -1 );
  }


  switch( image->type ) {
  default :
    VT_FreeImage( &distance );
    if ( _verbose_ )
      fprintf( stderr, "%s: such image type not handled (2)\n", proc );
    return( -1 );
    break;
  case UCHAR :
    {
      unsigned char *theBuf = (unsigned char*)image->buf;
      unsigned char *resBuf = (unsigned char*)opening->buf;
      for ( i=0; i<v; i++ )
	resBuf[i] = ( distBuf[i] > 0 ) ? 0 : theBuf[i];
    }
    break;
  case USHORT :
    {
      unsigned short int *theBuf = (unsigned short int*)image->buf;
      unsigned short int *resBuf = (unsigned short int*)opening->buf;
      for ( i=0; i<v; i++ )
	resBuf[i] = ( distBuf[i] > 0 ) ? 0 : theBuf[i];
    }
    break;
  case SSHORT :
    {
      short int *theBuf = (short int*)image->buf;
      short int *resBuf = (short int*)opening->buf;
      for ( i=0; i<v; i++ )
	resBuf[i] = ( distBuf[i] > 0 ) ? 0 : theBuf[i];
    }
    break;
  }


  VT_FreeImage( &distance );

  return( ncc );
}
