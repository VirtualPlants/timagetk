/*************************************************************************
 * test-thinning-pnm.c - 
 *
 * $Id: test-thinning-pnm.c,v 1.4 2006/04/14 08:38:55 greg Exp $
 *
 * Copyright (c) INRIA 2000
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
 * Tue Aug  8 10:03:00 MET DST 2000
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iopnm.h>
#include <chamferdistance.h>
#include <topological-operations-common.h>
#include <topological-thinning.h>



static char program[256];
static char *usage = "image-in image-out [-t %d] [-at %d] [-cbe %d] [-dbe %d]\n\
\t [-curves|-pure-curves|-surfaces|-pure-surfaces] [-chamfer %d] [-v] [-help]";
static char *details ="\n\
Thinning by ordering distance\n\
----------------\n\
The image is thresholded with the 'threshold' to yield\n\
a binary image.\n\
\n\
\t -t   | -threshold : specifies the threshold\n\
\t -at  | -anchor-threshold : points above this threshold can not be deleted\n\
\t -cbe | -cycles-before-end : minimum number of cycles to be done\n\
\t        before examining the end condition (whatever the distance is)\n\
\t -dbe | -distance-before-end : minimum distance value before examining the\n\
\t        end condition (without waiting for the minimal number of cycles)\n\
\t -curves   : end condition, yield curves\n\
\t -surfaces : end condition, yield surfaces\n\
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

  int threshold = 1;
  int chamfer = 3;
  int anchorValue;
  typeThinningParameters p;

  initTypeThinningParameters( &p );

  anchorValue = 256;
  p.cyclesBeforeEnding = 4;
  p.valueBeforeEnding = 1;
  p.typeThickness = _08THICKNESS_;
  p.typeEndPoint = _CURVE_;
  

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
        incrementVerboseInTopologicalThinning();
        setTimeInTopologicalThinning( 1 );
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
                (strcmp ( argv[i], "-nv" ) == 0) ) {
        setVerboseInChamferDistance( 0 );
        setVerboseInTopologicalThinning( 0 );
        setTimeInTopologicalThinning( 0 );
      }


      else if ( (strcmp ( argv[i], "-curve" ) == 0) || 
                (strcmp ( argv[i], "-curves" ) == 0) ) {
        p.typeEndPoint = _CURVE_;
      }

      else if ( (strcmp ( argv[i], "-pure-curve" ) == 0) || 
                (strcmp ( argv[i], "-pure-curves" ) == 0) ) {
        p.typeEndPoint = _PURE_CURVE_;
      }

      else if ( (strcmp ( argv[i], "-surface" ) == 0) || 
                (strcmp ( argv[i], "-surfaces" ) == 0) ) {
        p.typeEndPoint = _SURFACE_;
      }

      else if ( (strcmp ( argv[i], "-pure-surface" ) == 0) || 
                (strcmp ( argv[i], "-pure-surfaces" ) == 0) ) {
        p.typeEndPoint = _PURE_SURFACE_;
      }


      else if ( (strcmp ( argv[i], "-threshold" ) == 0) || 
                (strcmp ( argv[i], "-t" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -threshold...\n", 0 );
        status = sscanf( argv[i], "%d", &threshold );
        if ( status <= 0 ) ErrorMessage( "parsing -threshold...\n", 0 );
      }

      else if ( (strcmp ( argv[i], "-anchor-threshold" ) == 0) || 
                (strcmp ( argv[i], "-at" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -anchor-threshold...\n", 0 );
        status = sscanf( argv[i], "%d", &(anchorValue) );
        if ( status <= 0 ) ErrorMessage( "parsing -anchor-threshold...\n", 0 );
      }


      else if ( (strcmp ( argv[i], "-chamfer" ) == 0) || 
                (strcmp ( argv[i], "-c" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -chamfer...\n", 0 );
        status = sscanf( argv[i], "%d", &chamfer );
        if ( status <= 0 ) ErrorMessage( "parsing -chamfer...\n", 0 );
      }


      else if ( (strcmp ( argv[i], "-cycles-before-end" ) == 0) || 
                (strcmp ( argv[i], "-cbe" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -cycles-before-end...\n", 0 );
        status = sscanf( argv[i], "%d", &(p.cyclesBeforeEnding) );
        if ( status <= 0 ) ErrorMessage( "parsing -cycles-before-end...\n", 0 );
      }


      else if ( (strcmp ( argv[i], "-distance-before-end" ) == 0) || 
                (strcmp ( argv[i], "-dbe" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -distance-before-end...\n", 0 );
        status = sscanf( argv[i], "%d", &(p.valueBeforeEnding) );
        if ( status <= 0 ) ErrorMessage( "parsing -distance-before-end...\n", 0 );
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
  if ( nbytes == 2 ) {
    sprintf( nameImageIn, "can not handled two bytes image (%s)\n", argv[i] );
    ErrorMessage( nameImageIn, 0);
  }

  bufferOut = (void*)malloc( bufferDims[0] * bufferDims[1] * bufferDims[2] * sizeof(unsigned char) );


  {
    int i;
    int v = bufferDims[0] * bufferDims[1] * bufferDims[2];
    unsigned char *theBuf = bufferIn;
    unsigned char *resBuf = bufferOut;
    for ( i=0; i<v; i++ ) {
      if ( theBuf[i] < threshold ) {
        resBuf[i] = _BACKGROUND_VALUE_;
      } else if ( theBuf[i] < anchorValue ) {
        resBuf[i] = _CANBECHANGED_VALUE_;
      } else {
        resBuf[i] = _ANCHOR_VALUE_;
      }
    }
  }



  fprintf( stderr, "%s: processing with threshold = %d \n", argv[0], threshold );

  if ( chamferBasedThinning(  bufferOut, bufferDims,
                                chamfer, &p ) != 1 ) {
    fprintf( stderr, " processing failed.\n" );
    exit( 1 );
  }

  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], nbytes, bufferOut );
    
  exit( 0 );
}
