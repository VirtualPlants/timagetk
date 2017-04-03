/*************************************************************************
 * test-schedule-openmp.c -
 *
 * $Id$
 *
 * Copyright (c) INRIA 2012
 *
 * AUTHOR:
 * Gregoire Malandain (gregoire.malandain@inria.fr)
 * 
 * CREATION DATE: 
 * Fri Nov 16 22:45:44 CET 2012
 *
 *
 * ADDITIONS, CHANGES
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
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
  long int i, nelts = 100000000;

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



  if ( 0 ) {
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for reduction(+:sum)
      for ( i=0; i<nelts; i++ ) {
	sum += elts[i];
      }
      _END
    }
    sprintf( desc, "parallel" );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc,  sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );
  }



  if ( 0 ) {
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(static) reduction(+:sum)
      for ( i=0; i<nelts; i++ ) {
	sum += elts[i];
      }
      _END
    }
    sprintf( desc, "schedule(static)" );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc,  sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );
  }



  if ( 0 ) {
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(dynamic) reduction(+:sum)
      for ( i=0; i<nelts; i++ ) {
	sum += elts[i];
      }
      _END
    }
    sprintf( desc, "schedule(dynamic)" );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );
  }



  if ( 0 ) {
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(guided) reduction(+:sum)
      for ( i=0; i<nelts; i++ ) {
	sum += elts[i];
      }
      _END
    }
    sprintf( desc, "schedule(guided)" );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );
  }

  


  if ( 0 ) {
    for ( chunksize=nelts/10; chunksize>=1000; chunksize /= 10 ) {
      
      sumtimeuser = sumtimeproc = sumratio = 0.0;
      for ( t=0; t<ntest; t ++ ) {
	_BEG
#pragma omp parallel for  schedule(static,chunksize) reduction(+:sum)
	for ( i=0; i<nelts; i++ ) {
	  sum += elts[i];
	}
	_END
      }
      
      sprintf( desc, "schedule(static,%d)", chunksize );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
    }
  }


  
  if ( 1 ) {

    for ( nchunks=omp_get_max_threads(); nchunks<=1000; nchunks += 20  ) {

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
#pragma omp parallel for 
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	  partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
	}
	
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];

	_END
      }
      
      sprintf( desc, "parallel-equal-nchunks=%d", chunks.n_allocated_chunks );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
  
      free( partialsum );
      freeChunks( &chunks );

    }

    for ( nchunks=omp_get_max_threads(); nchunks<=1000; nchunks+=20  ) {

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
#pragma omp parallel for schedule(static) 
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	  partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
	}
	
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];

	_END
      }
      
      sprintf( desc, "schedule(static)-equal-nchunks=%d", chunks.n_allocated_chunks );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
  
      free( partialsum );
      freeChunks( &chunks );

  }

    for ( nchunks=omp_get_max_threads(); nchunks<=1000; nchunks+=20  ) {

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
#pragma omp parallel for schedule(dynamic) 
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

    for ( nchunks=omp_get_max_threads(); nchunks<=1000; nchunks+=20  ) {

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
#pragma omp parallel for schedule(dynamic,1) 
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	  partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
	}
	
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];

	_END
      }
      
      sprintf( desc, "schedule(dynamic,1)-equal-nchunks=%d", chunks.n_allocated_chunks );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
  
      free( partialsum );
      freeChunks( &chunks );

    }

    for ( nchunks=omp_get_max_threads(); nchunks<=1000; nchunks+=20  ) {

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
#pragma omp parallel for schedule(guided) 
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	  partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
	}
	
	for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];

	_END
      }
      
      sprintf( desc, "schedule(guided)-equal-nchunks=%d", chunks.n_allocated_chunks );
      fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
      fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
      fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
      fprintf( stdout, "\n" );
  
      free( partialsum );
      freeChunks( &chunks );

    }

  }

  if ( 1 ) {

    initChunks( &chunks );
    if ( allocBuildInequalChunks( &chunks, 0, nelts-1,
				  2.0/3.0,
				  omp_get_max_threads(),
				  nelts/100,
				  1000 ) != 1 ) {
      fprintf( stderr, "error when allocating %d chunks\n", nchunks );
      exit( 0 );
    }

    printChunks( stdout, &chunks, (char*)NULL );
    partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
    if ( partialsum == (double*)NULL ) {
      fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
      exit( 0 );
    }
    
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for 
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
      }
      
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];
      
      _END
    }
    
    sprintf( desc, "parallel-inequal-nchunks=%d", chunks.n_allocated_chunks );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );
    
    free( partialsum );
    freeChunks( &chunks );
    


    initChunks( &chunks );
    if ( allocBuildInequalChunks( &chunks, 0, nelts-1,
				  2.0/3.0,
				  omp_get_max_threads(),
				  nelts/100,
				  1000 ) != 1 ) {
      fprintf( stderr, "error when allocating %d chunks\n", nchunks );
      exit( 0 );
    }

    printChunks( stdout, &chunks, (char*)NULL );
    partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
    if ( partialsum == (double*)NULL ) {
      fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
      exit( 0 );
    }
    
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(static)
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
      }
      
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];
      
      _END
    }
    
    sprintf( desc, "schedule(static)-inequal-nchunks=%d", chunks.n_allocated_chunks );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );

    free( partialsum );
    freeChunks( &chunks );
    
    

    initChunks( &chunks );
    if ( allocBuildInequalChunks( &chunks, 0, nelts-1,
				  2.0/3.0,
				  omp_get_max_threads(),
				  nelts/100,
				  1000 ) != 1 ) {
      fprintf( stderr, "error when allocating %d chunks\n", nchunks );
      exit( 0 );
    }

    printChunks( stdout, &chunks, (char*)NULL );
    partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
    if ( partialsum == (double*)NULL ) {
      fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
      exit( 0 );
    }
    
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(dynamic)
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
      }
      
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];
      
      _END
    }
    
    sprintf( desc, "schedule(dynamic)-inequal-nchunks=%d", chunks.n_allocated_chunks );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );

    free( partialsum );
    freeChunks( &chunks );
    
    

    initChunks( &chunks );
    if ( allocBuildInequalChunks( &chunks, 0, nelts-1,
				  2.0/3.0,
				  omp_get_max_threads(),
				  nelts/100,
				  1000 ) != 1 ) {
      fprintf( stderr, "error when allocating %d chunks\n", nchunks );
      exit( 0 );
    }

    printChunks( stdout, &chunks, (char*)NULL );
    partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
    if ( partialsum == (double*)NULL ) {
      fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
      exit( 0 );
    }
    
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(dynamic,1)
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
      }
      
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];
      
      _END
    }
    
    sprintf( desc, "schedule(dynamic,1)-inequal-nchunks=%d", chunks.n_allocated_chunks );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );

    free( partialsum );
    freeChunks( &chunks );
    


    initChunks( &chunks );
    if ( allocBuildInequalChunks( &chunks, 0, nelts-1,
				  2.0/3.0,
				  omp_get_max_threads(),
				  nelts/100,
				  1000 ) != 1 ) {
      fprintf( stderr, "error when allocating %d chunks\n", nchunks );
      exit( 0 );
    }

    printChunks( stdout, &chunks, (char*)NULL );
    partialsum = (double*)malloc( chunks.n_allocated_chunks * sizeof( double ) );
    if ( partialsum == (double*)NULL ) {
      fprintf( stderr, "error when allocating %d doubles\n", chunks.n_allocated_chunks );
      exit( 0 );
    }
    
    sumtimeuser = sumtimeproc = sumratio = 0.0;
    for ( t=0; t<ntest; t ++ ) {
      _BEG
#pragma omp parallel for schedule(guided)
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) {
	partialsum[i] = _partialSum( elts, chunks.data[i].first, chunks.data[i].last );
      }
      
      for ( i=0; i<chunks.n_allocated_chunks; i++ ) sum += partialsum[i];
      
      _END
    }
    
    sprintf( desc, "schedule(guided)-inequal-nchunks=%d", chunks.n_allocated_chunks );
    fprintf( stdout, "%s average-time-user = %7.3f\n", desc, sumtimeuser / (double)ntest );
    fprintf( stdout, "%s average-time-proc = %7.3f\n", desc, sumtimeproc / (double)ntest );
    fprintf( stdout, "%s average-ratio     = %7.5f\n", desc, sumratio / (double)ntest );
    fprintf( stdout, "\n" );

    free( partialsum );
    freeChunks( &chunks );
    
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




