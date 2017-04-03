/*************************************************************************
 * test-edges.c - example program of edges detection
 *
 * $Id: test-edges.c,v 1.4 2002/04/19 15:54:08 greg Exp $
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
 * June, 9 1998
 *
 * Copyright Gregoire Malandain, INRIA
 *
 * ADDITIONS, CHANGES
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h> /* open, close */
#include <sys/stat.h> /* open, close */
#include <sys/types.h> /* open, close */
#include <unistd.h> /* write */

#include <time.h>
#include <sys/time.h>



static double _GetTime();
static double _GetClock();

#include <linearFiltering-contours.h>



int main( int argc, char* argv[] )
{
  void *bufferIn = (void*)NULL;
  void *bufferOut = (void*)NULL;
  int bufferDims[3] = {0,0,0};
  int borderLengths[3] = {0,0,0};
  typeFilteringCoefficients filter[3];

  int fd;
  int bufferLength;
  int example;

  double time_init;
  double time_exit;
  double clock_init;
  double clock_exit;


  initFilteringCoefficients( &(filter[0]) );
  initFilteringCoefficients( &(filter[1]) );
  initFilteringCoefficients( &(filter[2]) );


  /*
   * 2D image, example of use
   */ 
  time_init = _GetTime();
  clock_init = _GetClock();

  /*
   * allocation of a 64 x 64 image
   */
  example = 1;
  bufferLength = 4096;
  bufferIn = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferIn == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of first buffer failed.\n", example );
    exit( 1 );
  }
  bufferOut = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferOut == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of second buffer failed.\n", example );
    exit( 1 );
  }
  
  /*
   * reading the 2D image (raw data)
   */
  fd = open( "images/square64x64.data", 0 );
  {
    char *b = (char*)bufferIn;
    int toberead = bufferLength;
    int nbread = 0;
    while ( (toberead > 0) && ((nbread = read( fd, b, toberead )) > 0) ) {
      toberead -= nbread;
      b += nbread;
    }
    if ( toberead > 0 ) {
      fprintf( stderr, " example #%d: read %d bytes instead of %d.\n",
	       example, (bufferLength- toberead), bufferLength );
      exit( 1 );
    }
  }
  close( fd );

  /*
   * processing the 2D image
   */
  bufferDims[0] = 64;
  bufferDims[1] = 64;
  bufferDims[2] = 1;
  if ( gradientMaxima2D( bufferIn, UCHAR,
			 bufferOut, UCHAR,
			 bufferDims,
			 borderLengths,
			 filter ) != 1 ) {
    fprintf( stderr, " example #%d: processing failed.\n", example );
    exit( 1 );
  }

  /*
   * writing the 2D image (raw data)
   */
  fd = creat( "images/square64x64.tmp", 0644 );
  if ( write( fd, bufferOut, bufferLength ) != bufferLength ) {
    fprintf( stderr, " example #%d: error in writing.\n", example );
    exit( 1 );
  }
  close( fd );

  /*
   *
   */
  fprintf( stderr, " example #%d: processing is complete.\n", example );
  fprintf( stderr, "             image/square64x64.tmp was written.\n");
  free( bufferIn );
  free( bufferOut );

  time_exit = _GetTime();
  clock_exit = _GetClock();
  fprintf( stderr, "             elapsed (real) time = %f\n", time_exit - time_init );
  fprintf( stderr, "             elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
  fprintf( stderr, "             ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );
  


  /*
   * 3D image, example of use
   */ 
  time_init = _GetTime();
  clock_init = _GetClock();

  /*
   * allocation of a 64 x 64 x 64 image
   */
  example = 2;
  bufferLength = 262144;
  bufferIn = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferIn == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of first buffer failed.\n", example );
    exit( 1 );
  }
  bufferOut = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferOut == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of second buffer failed.\n", example );
    exit( 1 );
  }
  
  /*
   * reading the 3D image (raw data)
   */
  fd = open( "images/disk64x64x64.data", 0 );
  {
    char *b = (char*)bufferIn;
    int toberead = bufferLength;
    int nbread = 0;
    while ( (toberead > 0) && ((nbread = read( fd, b, toberead )) > 0) ) {
      toberead -= nbread;
      b += nbread;
    }
    if ( toberead > 0 ) {
      fprintf( stderr, " example #%d: read %d bytes instead of %d.\n",
	       example, (bufferLength- toberead), bufferLength );
      exit( 1 );
    }
  }
  close( fd );

  /*
   * processing the 3D image
   */
  bufferDims[0] = 64;
  bufferDims[1] = 64;
  bufferDims[2] = 64;
  if ( gradientMaxima3D( bufferIn, UCHAR,
			 bufferOut, UCHAR,
			 bufferDims,
			 borderLengths,
			 filter ) != 1 ) {
    fprintf( stderr, " example #%d: processing failed.\n", example );
    exit( 1 );
  }

  /*
   * writing the 3D image (raw data)
   */
  fd = creat( "images/disk64x64x64.tmp", 0644 );
  if ( write( fd, bufferOut, bufferLength ) != bufferLength ) {
    fprintf( stderr, " example #%d: error in writing.\n", example );
    exit( 1 );
  }
  close( fd );

  /*
   *
   */
  fprintf( stderr, " example #%d: processing is complete.\n", example );
  fprintf( stderr, "             image/disk64x64x64.tmp was written.\n");
  free( bufferIn );
  free( bufferOut );
  
  time_exit = _GetTime();
  clock_exit = _GetClock();
  fprintf( stderr, "             elapsed (real) time = %f\n", time_exit - time_init );
  fprintf( stderr, "             elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
  fprintf( stderr, "             ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );



  /*
   * 3D image, second example of use
   */ 
  time_init = _GetTime();
  clock_init = _GetClock();

  /*
   * allocation of a 64 x 64 x 64 image
   */
  example = 3;
  bufferLength = 262144;
  bufferIn = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferIn == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of first buffer failed.\n", example );
    exit( 1 );
  }
  bufferOut = (void*)malloc( bufferLength * sizeof(unsigned char) );
  if ( bufferOut == (void*)NULL ) {
    fprintf( stderr, " example #%d: allocation of second buffer failed.\n", example );
    exit( 1 );
  }
  
  /*
   * reading the image
   */
  fd = open( "images/mri64x64x64.data", 0 );
  {
    char *b = (char*)bufferIn;
    int toberead = bufferLength;
    int nbread = 0;
    while ( (toberead > 0) && ((nbread = read( fd, b, toberead )) > 0) ) {
      toberead -= nbread;
      b += nbread;
    }
    if ( toberead > 0 ) {
      fprintf( stderr, " example #%d: read %d bytes instead of %d.\n",
	       example, (bufferLength- toberead), bufferLength );
      exit( 1 );
    }
  }
  close( fd );

  /*
   * processing
   */
  bufferDims[0] = 64;
  bufferDims[1] = 64;
  bufferDims[2] = 64;
  /*
   * here it is better (even necessary) to add
   * points at both ends of lines when filetring.
   */
  borderLengths[0] = 10;
  borderLengths[1] = 10;
  borderLengths[2] = 10;
  if (gradientMaxima3D( bufferIn, UCHAR,
			bufferOut, UCHAR,
			bufferDims,
			borderLengths,
			filter ) != 1 ) {
    fprintf( stderr, " example #%d: processing failed.\n", example );
    exit( 1 );
  }

  /*
   * writing 
   */
  fd = creat( "images/mri64x64x64.tmp", 0644 );
  if ( write( fd, bufferOut, bufferLength ) != bufferLength ) {
    fprintf( stderr, " example #%d: error in writing.\n", example );
    exit( 1 );
  }
  close( fd );

  /*
   *
   */
  fprintf( stderr, " example #%d: processing is complete.\n", example );
  fprintf( stderr, "             image/mri64x64x64.tmp was written.\n");
  free( bufferIn );
  free( bufferOut );
  
  time_exit = _GetTime();
  clock_exit = _GetClock();
  fprintf( stderr, "             elapsed (real) time = %f\n", time_exit - time_init );
  fprintf( stderr, "             elapsed (user) time = %f (processors)\n", clock_exit - clock_init );
  fprintf( stderr, "             ratio (user)/(real) = %f\n", (clock_exit - clock_init)/(time_exit - time_init) );



  /*
   *
   */
  exit( 0 );
}



static double _GetTime() 
{
  struct timeval tv;
  gettimeofday(&tv, (void *)0);
  return ( (double) tv.tv_sec + tv.tv_usec*1e-6 );
}

static double _GetClock() 
{
  return ( (double) clock() / (double)CLOCKS_PER_SEC );
}
