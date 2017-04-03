/*************************************************************************
 * test-file-tools.c -
 *
 * $Id: test-file-tools.c,v 1.1 2000/04/26 16:05:38 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Apr 26 14:26:00 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <file-tools.h>

int main (int argc, char *argv[] )
{
  enumTypeFile typeFile;
  int sizeFile;

  if ( argc <= 1 ) exit( 0 );

  typeFile = getImageFileType( argv[1] );
  sizeFile = getFileSize( argv[1] );
    
  printf( "type of file '%s' is ", argv[1] );
  switch ( typeFile ) {
  default :
  case _TYPEFILE_UNKNOWN_ :
    printf( "_TYPEFILE_UNKNOWN_\n");
    break;
  case _TYPEFILE_COMPRESSED_ :
    printf( "_TYPEFILE_COMPRESSED_\n");
    break;
  case _TYPEFILE_INR_ :
    printf( "_TYPEFILE_INR_\n");
    break;
  case _TYPEFILE_GIF_ :
    printf( "_TYPEFILE_GIF_\n");
    break;
  case _TYPEFILE_PGM_ :
    printf( "_TYPEFILE_PGM_\n");
    break;
  case _TYPEFILE_PNM_ :
    printf( "_TYPEFILE_PNM_\n");
    break;
  case _TYPEFILE_PPM_ :
    printf( "_TYPEFILE_PPM_\n");
    break;
  case _TYPEFILE_BMP_ :
    printf( "_TYPEFILE_BMP_\n");
    break;
  }

  printf( "size of file '%s' is %d\n", argv[1], sizeFile );

  exit( 0 );
}
