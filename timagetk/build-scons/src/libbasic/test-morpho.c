/*************************************************************************
 * test-morpho.c -
 *
 * $Id: test-morpho.c,v 1.1 2001/03/19 17:11:47 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * DESCRIPTION: 
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Wed Mar 14 13:35:25 MET 2001
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <iopnm.h>
#include <morphotools.h>

static int _verbose_ = 1;

extern int readStructuringElement( char *fileName,
			    typeStructuringElement *SE );

int main( int argc, char* argv[] )
{
  char nameImageIn[256];
  char nameImageOut[256];
  char nameStrctElt[256];

  void *bufferIn = (void*)NULL;
  void *bufferOut = (void*)NULL;
  int bufferDims[3] = {0,0,0};

  int nbNames = 0;
  int status, i;
  typeStructuringElement SE;

  
  for ( i=1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {
      
      if ( (strcmp ( argv[i], "-help" ) == 0) || 
	   (strcmp ( argv[i], "-h" ) == 0) ) {
	/*
	ErrorMessage( "help message\n", 1 );
	*/
      }
      
      else if ( (strcmp ( argv[i], "-verbose" ) == 0) || 
		(strcmp ( argv[i], "-v" ) == 0) ) {
	_verbose_ = 1;
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
		(strcmp ( argv[i], "-nv" ) == 0) ) {
	_verbose_ = 0;
      }
      
      else if ( (strcmp ( argv[i], "-elt" ) == 0) || 
		(strcmp ( argv[i], "-structuring-element" ) == 0) ) {
	i += 1;
	/*
	if ( i >= argc)    ErrorMessage( "parsing -sigma...\n", 0 );
	*/
	(void)strncpy( nameStrctElt, argv[i], 255 );
      }
    }
    
    else if ( argv[i][0] != 0 ) {
      if ( nbNames == 0 ) {
	strcpy( nameImageIn, argv[i] );
      } 
      else if ( nbNames == 1 ) {
	strcpy( nameImageOut, argv[i] );
      } 
      else {
	sprintf( nameImageIn, "too many image name (%s)\n", argv[i] );
	/*
	ErrorMessage( nameImageIn, 0);
	*/
      }
      nbNames ++;
    }
  }


  initStructuringElement( &SE );
  readStructuringElement( nameStrctElt, &SE );



  bufferIn = _readPnmImage( nameImageIn, &bufferDims[0], &bufferDims[1], &bufferDims[2] );
  bufferOut = (void*)malloc( bufferDims[0] * bufferDims[1] * bufferDims[2] * sizeof(unsigned char) );


  morphologicalDilation( bufferIn, bufferOut, UCHAR, bufferDims, &SE );

  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], bufferOut );
}













int readStructuringElement( char *fileName,
			    typeStructuringElement *SE )
{
  char *proc = "readStructuringElement";
  FILE *fp;
  char *tmp, *str = NULL;
  int str_size = 512;
  int dimx = 0;
  int dimy = 0;
  int dimz = 0;
  char *theBuf = NULL;
  int x, y, z, n=0;
  int v, nbpts=0;
  int centre[3];
  
  str = (char*)malloc( str_size * sizeof(char) );
  if ( str == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to allocate auxiliary string.\n", proc );
    }
    return( 0 );
  }
  

  fp = fopen ( fileName, "r" );
  if ( fp == NULL ) {
    if ( _verbose_ ) {
      fprintf( stderr, "%s: unable to open file '%s'.\n", proc, fileName );
    }
    return( 0 );
  }



  while( fgets( str, str_size, fp ) ) {

    if ( str[0] == '#' ) continue;

    if ( strncmp( str, "XDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimx ) != 1 ) {
	if ( _verbose_ ) 
	  fprintf( stderr, "%s: unable to read X dimension in '%s'\n", 
		   proc, fileName );
	if ( theBuf != NULL ) free( theBuf );
	free( str );
	fclose( fp );
	return( 0 );
      }
      continue;
    }
    if ( strncmp( str, "YDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimy ) != 1 ) {
	if ( _verbose_ ) 
	  fprintf( stderr, "%s: unable to read Y dimension in '%s'\n", 
		   proc, fileName );
	if ( theBuf != NULL ) free( theBuf );
	free( str );
	fclose( fp );
	return( 0 );
      }
      continue;
    }
    if ( strncmp( str, "ZDIM=", 5 ) == 0 ) {
      if ( sscanf(str+5, "%d", &dimz ) != 1 ) {
	if ( _verbose_ ) 
	  fprintf( stderr, "%s: unable to read Z dimension in '%s'\n", 
		   proc, fileName );
	if ( theBuf != NULL ) free( theBuf );
	free( str );
	fclose( fp );
	return( 0 );
      }
      continue;
    }
    
    if ( theBuf == NULL && dimx > 0 && dimy > 0 && dimz > 0 ) {
      theBuf = (char*)malloc( dimx*dimy*dimz*sizeof(char) );
      if ( theBuf == NULL ) {
	if ( _verbose_ ) 
	  fprintf( stderr, "%s: unable to allocate buffer.\n", proc );
	free( str );
	fclose( fp );
	return( 0 );
      }
    }
    
    tmp = str;
    while ( *tmp != '\n' && *tmp != '\0' && *tmp != EOF ) {
      /* skip trailing whitespace
       */
      while ( *tmp == ' ' || *tmp == '\t' )
	tmp++;
      if ( *tmp == '\n' || *tmp == EOF )
	continue;

      /* read '.'
       */
      if ( tmp[0] == '.' && ( tmp[1] == ' ' ||
			      tmp[1] == '\0' ||
			      tmp[1] == '\t' ||
			      tmp[1] == '\n' ||
			      tmp[1] == EOF ) ) {
	theBuf[n] = 0;
	n++;
	tmp ++;
	continue;
      }

      /* read '+'
       */
      if ( tmp[0] == '+' && ( tmp[1] == ' ' ||
			      tmp[1] == '\0' ||
			      tmp[1] == '\t' ||
			      tmp[1] == '\n' ||
			      tmp[1] == EOF ) ) {
	theBuf[n] = 100;
	n++;
	tmp ++;
	continue;
      }

      if ( tmp[0] == '-' && ( tmp[1] == ' ' ||
			      tmp[1] == '\0' ||
			      tmp[1] == '\t' ||
			      tmp[1] == '\n' ||
			      tmp[1] == EOF ) ) {
	theBuf[n] = -100;
	n++;
	tmp ++;
	continue;
      }

      /* read a number
       */
      if ( sscanf( tmp, "%d", &v ) != 1 ) {
	if ( _verbose_ ) {
	  fprintf( stderr, "%s: error in reading '%s'\n",
		   proc, fileName );
	  fprintf( stderr, "\t in line '%s' at [%d]:'%s'.\n", str, tmp-str, tmp );
	}
	if ( theBuf != NULL ) free( theBuf );
	free( str );
	fclose( fp );
	return( 0 );
      }
      if ( v > 0 ) theBuf[n] = 1;
      else         theBuf[n] = 0;
      n++;

      /* skip a number 
       */
      while ( (*tmp >= '0' && *tmp <= '9') || *tmp == '.' || *tmp == '-' )
	tmp++;
    }
  }

  free( str );
  fclose( fp );
  
  

  nbpts = 0;
  centre[0] = (dimx+1)/2 - 1;
  centre[1] = (dimy+1)/2 - 1;
  centre[2] = (dimz+1)/2 - 1;

  for ( n=0, z=0; z<dimz; z++ )
  for ( y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++,n++ ) {
    if ( theBuf[n] > 0 ) nbpts ++;
    if ( theBuf[n] == 100 || theBuf[n] == -100 ) {
      centre[0] = x;
      centre[1] = y;
      centre[2] = z;
    }
  }


  printf( "\n" );
  for ( n=0, z=0; z<dimz; z++ ) {
    for ( y=0; y<dimy; y++ ) {
      for ( x=0; x<dimx; x++,n++ ) {
	printf( " %4d", theBuf[n] );
      }
      printf( "\n" );
    }
    printf( "\n" );
  }
  printf( " nb d'elements = %d\n", nbpts );
  printf( " centre = [%d %d %d]\n", centre[0], centre[1], centre[2] );
  printf( "\n" );



  SE->userDefinedSE.list = (typeMorphoToolsPoint *)malloc(nbpts*sizeof(typeMorphoToolsPoint));
  SE->userDefinedSE.nb = nbpts;
  
  for ( v=0, n=0, z=0; z<dimz; z++ )
  for ( y=0; y<dimy; y++ )
  for ( x=0; x<dimx; x++,n++ ) {
    
    if ( theBuf[n] > 0 ) {
      SE->userDefinedSE.list[v].x = x - centre[0];
      SE->userDefinedSE.list[v].y = y - centre[1];
      SE->userDefinedSE.list[v].z = z - centre[2];
      v++;
    }
  }


  if ( theBuf != NULL ) free( theBuf );

  return( 1 );
}
