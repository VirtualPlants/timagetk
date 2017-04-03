/****************************************************
 * test-openmp.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Mon Nov 19 14:26:30 CET 2012
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <time.h>
#include <sys/time.h>

#include <chunks.h>



static double _GetTime();
static double _GetClock();



double _partialSum( double *elts, size_t f, size_t l )
{
  double s;
  size_t i;

  for ( s=0.0, i=f; i<=l; i++ ) 
    s += elts[i];
  return( s );
}



int main (int argc, char const *argv[])
{
  int t, ntest = 10;
  size_t i, nelts = 100000000;

  double *elts = NULL;
  double sum;

  double sumtimeuser;
  double sumtimeproc;
  double sumratio;

  double timeproc;
  double timeuser;
  double ratio;

  double *partialsum = NULL;

  int chunksize = 0;
  int nchunks;
  typeChunks chunks;

  char desc[1024];

  elts = (double*)malloc( nelts*sizeof(double) );
  if ( elts == NULL ) {
    fprintf( stderr, "pb allocation\n" );
    exit( 0 );
  }
  srandom( time(0) );

  for ( i=0; i<nelts; i++ )
    elts[i] = (double)random()/(double)(RAND_MAX);

#define _BEG { \
    timeuser = (- _GetTime()); \
    timeproc = (- _GetClock()); \
    sum = 0.0; \
}

#define _END { \
    timeproc += _GetClock(); \
    timeuser += _GetTime(); \
    ratio = timeproc / timeuser; \
    \
    fprintf( stdout, "test #%2d: user = %7.3f, proc = %7.3f, ratio = %7.5f --- sum = %g, average = %g\n", \
	     t, timeuser, timeproc, ratio, sum, sum/(double)nelts );	\
    \
    sumtimeuser += timeuser; \
    sumtimeproc += timeproc; \
    sumratio += ratio; \
}





  sumtimeuser = sumtimeproc = sumratio = 0.0;
  for ( t=0; t<ntest; t ++ ) {
    _BEG
    for ( i=0; i<nelts; i++ ) {
      sum += elts[i];
    }
    _END
  }
  sprintf( desc, "sequential" );
  fprintf( stdout, "%s average-time-user = %7.3f\n", desc,  sumtimeuser / (double)ntest );
  fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
  fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
  fprintf( stdout, "\n" );





  


  if ( 1 ) {

    for ( nchunks=1; nchunks<=102; nchunks+=20  ) {

      initChunks( &chunks );
      if ( allocBuildEqualChunks( &chunks, 0, nelts-1, nchunks ) != 1 ) {
	fprintf( stderr, "error when allocating %d chunks\n", nchunks );
	exit( 0 );
      }
      partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
      if ( partialsum == (double*)NULL ) {
	fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
	exit( 0 );
      }

      sumtimeuser = sumtimeproc = sumratio = 0.0;
      for ( t=0; t<ntest; t ++ ) {
	_BEG
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic) 
#endif
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	  partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
	}
	
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];

	_END
      }
      
      sprintf( desc, "schedule(dynamic)-equal-nchunks=%d", chunks.n_allocated_chunks );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
  
      free( partialsum );
      freeChunks( &chunks );

    }

  
  
  }

 


  return( 0 );
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




