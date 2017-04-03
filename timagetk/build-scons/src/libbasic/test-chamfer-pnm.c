/*************************************************************************
 * test-chamfer-pnm.c - example program of chamfer distance computation
 *
 * $Id: test-chamfer-pnm.c,v 1.6 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 1999
 *
 * DESCRIPTION: 
 *
 * Input must be of PBM/PGM/PPM raw format, output will be the same.
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * Thu Oct  7 22:56:37 MET DST 1999
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 * * Tue Dec  7 12:06:32 MET 1999 (G. Malandain)
 *   - parameters parsing
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iopnm.h>
#include <chamferdistance.h>



static char program[256];
static char *usage = "image-in image-out [-c %d] [-t %f] [-v] [-help]";
static char *details ="\n\
Chamfer distance\n\
----------------\n\
The image is thresholded with the 'threshold' to yield\n\
a binary image.\n\
\n\
\t -c | -chamfer : size of the chamfer maks\n\
\t -t | -threshold : specifies the threshold\n\
\t -v | -verbose\n\
\t -nv | -no-verbose\n\
\t -h | -help : print this message";

static void ErrorMessage( char *str, int flag )
{
  (void)fprintf(stderr,"Usage: %s %s\n",program, usage);
  if ( flag == 1 ) (void)fprintf(stderr,"%s",details);
  (void)fprintf(stderr,"Error: %s",str);
  exit(0);
}







int main( int argc, char* argv[] )
{
  char nameImageIn[256];
  char nameImageOut[256];
  int nbNames = 0;
  int status, i;

  void *bufferIn = (void*)NULL;
  void *bufferOut = (void*)NULL;
  int bufferDims[3] = {0,0,0};
  int nbytes;
  bufferType TYPE = UCHAR;

  float threshold = 1.0;
  int chamfer = 3;

  strcpy( program, argv[0] );

  if ( argc == 1 ) ErrorMessage( "\n", 1 );

  for ( i=1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {

      if ( (strcmp ( argv[i], "-help" ) == 0) || 
	   (strcmp ( argv[i], "-h" ) == 0) ) {
	ErrorMessage( "help message\n", 1 );
      }
      
      else if ( (strcmp ( argv[i], "-verbose" ) == 0) || 
		(strcmp ( argv[i], "-v" ) == 0) ) {
	incrementVerboseInChamferDistance();
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
		(strcmp ( argv[i], "-nv" ) == 0) ) {
	setVerboseInChamferDistance( 0 );
      }

      else if ( (strcmp ( argv[i], "-threshold" ) == 0) || 
		(strcmp ( argv[i], "-t" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -threshold...\n", 0 );
	status = sscanf( argv[i], "%f", &threshold );
	if ( status <= 0 ) ErrorMessage( "parsing -threshold...\n", 0 );
      }

      else if ( (strcmp ( argv[i], "-chamfer" ) == 0) || 
		(strcmp ( argv[i], "-c" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -chamfer...\n", 0 );
	status = sscanf( argv[i], "%d", &chamfer );
	if ( status <= 0 ) ErrorMessage( "parsing -chamfer...\n", 0 );
      }

      else {
	sprintf( nameImageIn, "unknown option %s\n", argv[i] );
	ErrorMessage( nameImageIn, 0);
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
	ErrorMessage( nameImageIn, 0);
      }
      nbNames ++;
    }
  }

  
  bufferIn = _readPnmImage( nameImageIn, &bufferDims[0], &bufferDims[1], &bufferDims[2], &nbytes );
  if ( nbytes == 2 ) TYPE = USHORT;

 
  bufferOut = (void*)malloc( bufferDims[0] * bufferDims[1] * bufferDims[2] * nbytes * sizeof(unsigned char) );


  fprintf( stderr, "%s: processing with threshold = %f \n", argv[0], threshold );


  switch ( chamfer ) {
  default :
  case 3 :
    fprintf( stderr, " processing 3x3 chamfer.\n" );
    if ( Compute2DNormalizedChamfer3x3WithThreshold( bufferIn, TYPE,
					   bufferOut, TYPE,
					   bufferDims,
					   threshold ) < 0 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;
  case 5 :
    fprintf( stderr, " processing 5x5 chamfer.\n" );
    if ( Compute2DNormalizedChamfer5x5WithThreshold( bufferIn, TYPE,
					   bufferOut, TYPE,
					   bufferDims,
					   threshold ) < 0 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;
  }  

  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], nbytes, bufferOut );
    
  exit( 0 );
}
