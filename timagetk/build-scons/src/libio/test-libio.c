#include <stdio.h>
#include <stdlib.h>
#include <ImageIO.h>

int main(int argc, char *argv[])
{
  _image *theIm;
  
  if ( argc == 1 ) {
    fprintf( stderr, "Usage: %s input output\n", argv[0] );
    exit( -1 );
  }
  
  theIm = _readImage( argv[1] );
  if ( theIm == NULL ) {
    fprintf( stderr, "%s: unable to read '%s'\n", argv[0], argv[1] );
    exit( -1 );
  }

  fprintf( stderr, "Information on '%s'\n", argv[1] );
  fprintf( stderr, "- image size is %lu x %lu x %lu\n",
	   theIm->xdim, theIm->ydim,  theIm->zdim );
  fprintf( stderr, "- voxel size is %f x %f x %f\n",
	   theIm->vx, theIm->vy,  theIm->vz );
  fprintf( stderr, "- image encoding is " );

  switch( theIm->wordKind ) {
  default :
    fprintf( stderr, "not handled\n" );
    break;
  case WK_FLOAT :
    switch( theIm->wdim ) {
    default :
      fprintf( stderr, "not handled\n" );
      break;
    case 4 :
      fprintf( stderr, "float\n" );
      break;
    case 8 :
      fprintf( stderr, "double\n" );
      break;
    }
    break;
  case WK_FIXED :
    switch( theIm->sign ) {
    default :
      fprintf( stderr, "not handled\n" );
      break;
    case SGN_SIGNED :
      switch( theIm->wdim ) {
      default :
	fprintf( stderr, "not handled\n" );
	break;
      case 1 :
	fprintf( stderr, "signed char\n" );
	break;
      case 2 :
	fprintf( stderr, "signed short int\n" );
	break;
      case 4 :
	fprintf( stderr, "signed int\n" );
	break;
      }
      break;
    case SGN_UNSIGNED :
      switch( theIm->wdim ) {
      default :
	fprintf( stderr, "not handled\n" );
	break;
      case 1 :
	fprintf( stderr, "unsigned char\n" );
	break;
      case 2 :
	fprintf( stderr, "unsigned short int\n" );
	break;
      case 4 :
	fprintf( stderr, "unsigned int\n" );
	break;
      }
      break;
    } /* switch( theIm->sign ) */
    break;
  }

  /* image data is in the buffer 'theIm->data' that is void*, meaning you can not use it directly.
     You have to use a pointer with the right type and cast the data pointer, e.g. for 'signed short int'
     encoding,
     short int *buf = (short int*)theIm->data;
     buf[z*theIm->xdim*theIm->ydim + y*theIm->xdim + x] is then the image value at (x,y,z)
  */

   if ( argc > 2 ) {
     if ( _writeImage( theIm, argv[2] ) != 0 ) {
       fprintf( stderr, "%s: unable to write '%s'\n", argv[0], argv[2] );
       exit( -1 );
     }
   }

   _freeImage( theIm ); 
   exit( 0 );
}
