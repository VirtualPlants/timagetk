/*************************************************************************
 * test-edges-pnm.c - example program of edges detection
 *
 * $Id: test-edges-pnm.c,v 1.12 2006/04/14 08:38:55 greg Exp $
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

#include <linearFiltering.h>
#include <linearFiltering-contours.h>
#include <zcross.h>



typedef enum {
  _GRADIENT_MODULUS_,
  _HESSIAN_,
  _LAPLACIAN_,
  _GRADIENT_HESSIAN_,
  _GRADIENT_LAPLACIAN_,
  _EXTREMA_GRADIENT_,
} enumOutput;



static char program[256];
static char *usage = "image-in image-out [-a %f] [-s %f] [-neg|-pos]\n\
 [-gradient|-laplacian|-gradient-laplacian|-hessian|-gradient-hessian]\n\
 [-v|-nv] [-help]";
static char *details ="\n\
Edge detection\n\
--------------\n\
Edges are the maxima of the gradient modulus in the direction\n\
of the gradient.\n\
\n\
\t -a | -alpha : specifies the alpha of Deriche's recursive filters\n\
\t -s | -sigma : specifies the sigma of a recursive approximation\n\
\t               of the gaussian\n\
\t -v | -verbose\n\
\t -nv | -no-verbose\n\
\t -h | -help : print this message\n";

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
  int borderLengths[3] = {10,10,10};
  typeFilteringCoefficients filter[3];  
  float c;

  enumOutput typeOutput = _EXTREMA_GRADIENT_;



  initFilteringCoefficients( &(filter[0]) );
  initFilteringCoefficients( &(filter[1]) );
  initFilteringCoefficients( &(filter[2]) );
  filter[0].type = ALPHA_DERICHE;
  filter[1].type = ALPHA_DERICHE;
  filter[2].type = ALPHA_DERICHE;


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
	incrementVerboseInLinearFilteringContours();
	ZeroCrossings_verbose ( );
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
		(strcmp ( argv[i], "-nv" ) == 0) ) {
	setVerboseInLinearFilteringContours( 0 );
      }

      else if ( strcmp ( argv[i], "-neg" ) == 0 ) {
	ZeroCrossings_Are_Negative();
      }
      else if ( strcmp ( argv[i], "-pos" ) == 0 ) {
	ZeroCrossings_Are_Positive();
      }


      else if ( strcmp ( argv[i], "-gradient" ) == 0 ) {
	typeOutput = _GRADIENT_MODULUS_;
      }
      else if ( strcmp ( argv[i], "-laplacian" ) == 0 ) {
	typeOutput = _LAPLACIAN_;
      }
      else if ( strcmp ( argv[i], "-gradient-laplacian" ) == 0 ) {
	typeOutput = _GRADIENT_LAPLACIAN_;
      }
      else if ( strcmp ( argv[i], "-hessian" ) == 0 ) {
	typeOutput = _HESSIAN_;
      }
      else if ( strcmp ( argv[i], "-gradient-hessian" ) == 0 ) {
	typeOutput = _GRADIENT_HESSIAN_;
      }

      else if ( (strcmp ( argv[i], "-alpha" ) == 0) || 
		(strcmp ( argv[i], "-a" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -alpha...\n", 0 );
	status = sscanf( argv[i], "%f", &c );
	if ( status <= 0 ) ErrorMessage( "parsing -alpha...\n", 0 );
	filter[0].coefficient = filter[1].coefficient = filter[2].coefficient = c;
	filter[0].type = filter[1].type = filter[2].type = ALPHA_DERICHE;
      }

      else if ( (strcmp ( argv[i], "-sigma" ) == 0) || 
		(strcmp ( argv[i], "-s" ) == 0) ) {
	i += 1;
	if ( i >= argc)    ErrorMessage( "parsing -sigma...\n", 0 );
	status = sscanf( argv[i], "%f", &c );
	if ( status <= 0 ) ErrorMessage( "parsing -sigma...\n", 0 );
	filter[0].coefficient = filter[1].coefficient = filter[2].coefficient = c;
	filter[0].type = filter[1].type = filter[2].type = GAUSSIAN_DERICHE;
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


  fprintf( stderr, "%s: processing with coefficient = %f\n", argv[0], 
	   filter[0].coefficient );



  switch( typeOutput ) {

  case _GRADIENT_MODULUS_ :
    if ( gradientModulus( bufferIn, TYPE, bufferOut, TYPE,
			  bufferDims, borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;

  case _HESSIAN_ :
    if ( gradientHessianGradient( bufferIn, TYPE,
				  bufferOut, TYPE,
				  bufferDims,
				  borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;

  case _LAPLACIAN_ :
    if ( laplacian( bufferIn, TYPE,
		    bufferOut, TYPE,
		    bufferDims,
		    borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;

  case _GRADIENT_HESSIAN_ :
    if ( gradientOnGradientHessianGradientZC( bufferIn, TYPE,
					      bufferOut, TYPE,
					      bufferDims,
					      borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;
    
  case _GRADIENT_LAPLACIAN_ :
    if ( gradientOnLaplacianZC( bufferIn, TYPE,
				bufferOut, TYPE,
				bufferDims,
				borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;
    
  default :
  case _EXTREMA_GRADIENT_ :
    if ( gradientMaxima( bufferIn, TYPE,
			 bufferOut, TYPE,
			 bufferDims,
			 borderLengths, filter ) != 1 ) {
      fprintf( stderr, " processing failed.\n" );
      exit( 1 );
    }
    break;
  
  }

  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], nbytes, bufferOut );
  
  exit( 0 );
}
