/*************************************************************************
 * test-connexe-pnm.c - example program of connected components extraction
 *
 * $Id: test-connexe-pnm.c,v 1.2 2006/04/14 08:38:55 greg Exp $
 *
 * LICENSE:
 * GPL v3.0 (see gpl-3.0.txt for details)
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
 * Mon Mar  6 13:30:08 MET 2000
 *
 * ADDITIONS, CHANGES
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iopnm.h>
#include <connexe.h>



static char program[256];
static char *usage = "image-in image-out [-ms %d] [-mn %d] [-v] [-help]";
static char *details ="\n\
Connected components extraction\n\
-------------------------------\n\
The input image is suuposed to be binary:\n\
- object = {points with value > 0}\n\
- background = {points with value = 0}\n\
\n\
\t -ms | -minimal-size : specifies the minimal size of connected components\n\
\t -mn | -maximal-number : specifies the maximal number of ccs to be retained\n\
\t     if the specified number is positive, then the labels of the ccs are\n\
\t     sorted with respect to the cc's size (decreasing order). Thus the component\n\
\t     with label 1 is the largest one.\n\
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
  int size = 1;
  int number = -1;


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
        incrementVerboseInConnexe(  );
      }

      else if ( (strcmp ( argv[i], "-no-verbose" ) == 0) || 
                (strcmp ( argv[i], "-nv" ) == 0) ) {
        setVerboseInConnexe( 0 );
      }

      else if ( (strcmp ( argv[i], "-minimal-size" ) == 0) || 
                (strcmp ( argv[i], "-ms" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -minimal-size...\n", 0 );
        status = sscanf( argv[i],"%d", &size );
        if ( status <= 0 ) ErrorMessage( "parsing -minimal-size...\n", 0 );
      }

      else if ( (strcmp ( argv[i], "-maximal-number" ) == 0) || 
                (strcmp ( argv[i], "-mn" ) == 0) ) {
        i += 1;
        if ( i >= argc)    ErrorMessage( "parsing -maximal-number...\n", 0 );
        status = sscanf( argv[i],"%d", &number );
        if ( status <= 0 ) ErrorMessage( "parsing -maximal-number...\n", 0 );
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
  
  bufferOut = (void*)malloc( bufferDims[0] * bufferDims[1] * bufferDims[2] * sizeof(unsigned char) );


  /*
   * to change the connectivity -> Connexe_SetConnectivity
   * to change the minimal size of the connected components to be kept.
   *                            -> Connexe_SetMinimumSizeOfComponents
   * to change the maximum number of the connected components to be kept.
   *                            -> Connexe_SetMaximumNumberOfComponents
   */
  Connexe_SetMinimumSizeOfComponents( size );
  Connexe_SetMaximumNumberOfComponents( number );

  if ( CountConnectedComponents( bufferIn, TYPE,
                                 bufferOut, UCHAR,
                                 bufferDims ) < 0 ) {
    fprintf( stderr, " processing failed.\n" );
    exit( 1 );
  }

  /* there exists a more complete function (with more parameters)
     => CountConnectedComponentsWithAllParams()
     see connexe.h
  */


  _writePnmImage( nameImageOut, bufferDims[0], bufferDims[1], bufferDims[2], 1, bufferOut );
    
  exit( 0 );
}
