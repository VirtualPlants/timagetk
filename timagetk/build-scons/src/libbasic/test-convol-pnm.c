/*************************************************************************
 * test-convol-pnm.c - 
 *
 * $Id: test-convol-pnm.c,v 1.2 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 1999
 * DESCRIPTION: 
 *
 * Input must be of PBM/PGM/PPM raw format, output will be the same.
 *
 *
 * AUTHOR:
 * Gregoire Malandain (greg@sophia.inria.fr)
 * 
 * CREATION DATE: 
 * June, 9 1998
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iopnm.h>
#include <convolution.h>



typedef enum {
  _GRADIENT_EXTREMA_,
  _GRADIENT_MODULUS_,
  _LAPLACIAN_,
  _GRADIENT_LAPLACIAN_,
  _HESSIAN_,
  _GRADIENT_HESSIAN_
} enumOutput;



static char program[256];
static char *usage = "image-in image-out [-f %d] [-h]";
static char *details ="\n\
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
  int f, fwhm[3] = { 1, 1, 1 };

  strcpy( program, argv[0] );

  if ( argc == 1 ) ErrorMessage( "\n", 1 );

  for ( i=1; i<argc; i++ ) {
    if ( argv[i][0] == '-' ) {

      if ( (strcmp ( argv[i], "-help" ) == 0) || 
	   (strcmp ( argv[i], "-h" ) == 0) ) {
	ErrorMessage( "help message\n", 1 );
      }
      
      else if ( (strcmp ( argv[i], "-fwhm" ) == 0) || 
		(strcmp ( argv[i], "-f" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -fwhm...\n", 0 );
	status = sscanf( argv[i], "%d", &f );
	if ( status <= 0 ) ErrorMessage( "parsing -fwhm...\n", 0 );
	fwhm[0] = fwhm[1] = fwhm[2] = f;
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


  ConvolutionWithFwhm( bufferIn, TYPE, bufferOut, TYPE, bufferDims, fwhm );


  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], nbytes, bufferOut );

  exit( 0 );
}
