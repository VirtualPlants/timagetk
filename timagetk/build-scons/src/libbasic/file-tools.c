/*************************************************************************
 * file-tools.c -
 *
 * $Id: file-tools.c,v 1.3 2000/08/16 08:44:02 greg Exp $
 *
 * Copyright (c) INRIA 2000
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Apr 26 13:24:41 MET DST 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */


#include <file-tools.h>






enumTypeFile getImageFileType( char *name )
{
  char *proc = "getImageFileType";
  enumTypeFile typeFile = _TYPEFILE_UNKNOWN_;
  FILE *fd, *fopen();
  char magicString[13];
  int readSize;

  if ( name == (char*)NULL ) return( typeFile );
  fd = fopen( name, "r" );
  if ( fd == (FILE*)NULL ) {
    fprintf( stderr, "%s: unable to open file '%s' for reading\n", proc, name );
    return( typeFile );
  }
  
  /* synopsis of fread()
     size_t fread( void *pointer, size_t size, size_t num_items, FILE *stream);
  */
  readSize = fread( magicString, 1, 12 ,fd );
  if ( readSize < 12 ) {
    fprintf( stderr, "%s: unable to read magic string in '%s'\n", proc, name );\
    return( typeFile );
  }

  if ( fclose( fd ) != 0 ) {
    fprintf( stderr, "%s: unable to close file '%s'\n", proc, name );
  }

  
  
  if ( (memcmp(magicString , FUTIL_MAGIC_PACK     , 2) == 0) ||
       (memcmp(magicString , FUTIL_MAGIC_GZIP     , 2) == 0) ||
       (memcmp(magicString , FUTIL_MAGIC_OLD_GZIP , 4) == 0) ||
       (memcmp(magicString , FUTIL_MAGIC_LZH      , 2) == 0) ||
       (memcmp(magicString , FUTIL_MAGIC_PKZIP    , 2) == 0) ||
       (memcmp(magicString , FUTIL_MAGIC_LZW      , 2) == 0) ) {
    typeFile = _TYPEFILE_COMPRESSED_;
  } 
  else if ( memcmp(magicString , FUTIL_MAGIC_INR , 12) == 0 ) {
    typeFile = _TYPEFILE_INR_;
  }
  else if ( memcmp(magicString , FUTIL_MAGIC_GIF , 4 ) == 0 ) {
    typeFile = _TYPEFILE_GIF_;
  }
  else if ( memcmp(magicString , FUTIL_MAGIC_PGM , 2 ) == 0 ) {
    typeFile = _TYPEFILE_PGM_;
  }
  else if ( memcmp(magicString , FUTIL_MAGIC_PPM , 2 ) == 0 ) {
    typeFile = _TYPEFILE_PPM_;
  }
  else if ( (memcmp(magicString , FUTIL_MAGIC_ICO, 2 ) == 0) ||
	    (memcmp(magicString , FUTIL_MAGIC_BMP, 2 ) == 0) ||
	    (memcmp(magicString , FUTIL_MAGIC_PTR, 2 ) == 0) ||
	    (memcmp(magicString , FUTIL_MAGIC_ICO_COLOR, 2 ) == 0) ||
	    (memcmp(magicString , FUTIL_MAGIC_PTR_COLOR, 2 ) == 0) ||
	    (memcmp(magicString , FUTIL_MAGIC_ARRAY, 2 ) == 0) ) {
    typeFile = _TYPEFILE_BMP_;
  }
	    

  return( typeFile );
}











int getFileSize( char *name )
{
  char *proc = "getFileSize";
  struct stat stbuf;
  
  if ( name == (char*)NULL ) return( -1 );
  if ( stat( name, &stbuf ) != 0 ) {
    fprintf( stderr, "%s: unable to fill '%s' file info \n", proc, name );
    return( -1 );
  }
  return( stbuf.st_size );
}
